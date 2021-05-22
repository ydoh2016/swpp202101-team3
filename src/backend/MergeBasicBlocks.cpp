#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"

#include <vector>
#include <algorithm>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
PreservedAnalyses MergeBasicBlocksPass::run(Function& F, FunctionAnalysisManager& FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  vector<BasicBlock *> bfs;
  BasicBlock *BBEntry = &F.getEntryBlock();
  for (auto it = bf_begin(BBEntry); it != bf_end(BBEntry); ++it) {
    bfs.push_back(*it);
  }

  reverse(bfs.begin(), bfs.end());
  
  vector<pair<BasicBlock*, BasicBlock*>> BBPairToMerge;
  // First, search for mergeable pairs.
  for (auto &BB : bfs) {
    pair<BasicBlock *, BasicBlock *> mergePair = classifyMergeType(BB);
    if (mergePair.first != nullptr && mergePair.second != nullptr) {
      BBPairToMerge.push_back(mergePair);
    }
  }

  // Then, merge the pairs safely. 
  for (const auto &BBPair : BBPairToMerge) {
    BasicBlock *BBPred = BBPair.first;
    BasicBlock *BBSucc = BBPair.second;
    mergeSafely(&F, DT, BBPred, BBSucc);
  }

  // Get rid of dangling BBs.
  EliminateUnreachableBlocks(F);
  // Finally, remove dangling phi nodes
  removeDanglingPhi(&F);
  
  return PreservedAnalyses::all();
}

// Remove phis that are referring to deleted basic blocks
void MergeBasicBlocksPass::removeDanglingPhi(Function *F) {
  vector<PHINode*> phisToRemove;
  for (auto &BB : *F) {
    for (auto &phi : BB.phis()) {
      int incomingNum = phi.getNumIncomingValues();
      for (int i = incomingNum - 1; i >= 0; --i) {
        if (!phi.getIncomingBlock(i)->hasName()) {
          phi.removeIncomingValue(i);
        }
      }

      // Delete a phi node if it has only one incoming value
      if(phi.getNumIncomingValues() == 1) {
        phisToRemove.push_back(&phi);
      }
    }
  }

  // Replace the phi node uses with its unique incoming value
  for (PHINode *phi : phisToRemove) {
    for (auto it = phi->use_begin(), end = phi->use_end(); it != end;) {
      Use &U = *it++;
      User *Usr = U.getUser();
      U.set(phi->getIncomingValue(0));
    }
    phi->eraseFromParent();
  }
}

void MergeBasicBlocksPass::mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc) {

  // If there is a phi node in the successor, return without merging
  for (auto &phi : BBSucc->phis()) {
    return;
  }

  // If there is a incoming loop predecessor in the successor, return without merging
  for (auto it = df_begin(BBSucc); it != df_end(BBSucc); ++it) {
    auto BBOffspring = *it;
    int succCount = BBOffspring->getTerminator()->getNumSuccessors();
    for (int i = 0; i < succCount; ++i) {
      if (BBOffspring->getTerminator()->getSuccessor(i) == BBSucc) {
        return;
      }
    }
  }
  
  // Otherwise copy the successor and merge the copied one into the predecessor.
  ValueToValueMapTy VM;
  BasicBlock *BBDummy = CloneBasicBlock(BBSucc, VM, "", F);

  // Since the CloneBasicBlock function does not do a remapping for us, manually
  // remap operands of the BBDummy with the VM.
  for (auto &I : *BBSucc) {
    Value *to = VM[&I];
    for (auto it = I.use_begin(), end = I.use_end(); it != end;) {
      Use &U = *it++;
      User *Usr = U.getUser();
      Instruction *UsrI = dyn_cast<Instruction>(Usr);
      assert(UsrI); // This must be an instruction
      if (UsrI->getParent() == BBDummy) {
        U.set(to);
      }
    }
  }

  // Modify the grandson's phi node
  auto terminator = BBSucc->getTerminator();
  int succNum = terminator->getNumSuccessors();
  for (int i = 0; i < succNum; ++i) {
    auto BBGrandson = terminator->getSuccessor(i);
    for (auto &phi : BBGrandson->phis()) {
      int incomingIdx = phi.getBasicBlockIndex(BBSucc);
      if (incomingIdx != -1) {
        Value *incomingVal = phi.getIncomingValue(incomingIdx);
        Value *replacingVal = VM[incomingVal];
        if (replacingVal) {
          phi.addIncoming(replacingVal, BBPred);
        } else {
          phi.addIncoming(incomingVal, BBPred);
        }
      }
    }
  }
  
  BranchInst *predBranchInst = dyn_cast<BranchInst>(BBPred->getTerminator());
  assert(predBranchInst != nullptr);

  // Change all the successors into the BBDummy so that the BBDummy has only
  // one predecessor and the predecessor has only one successor.
  for (unsigned i = 0; i < predBranchInst->getNumSuccessors(); ++i) {
    predBranchInst->setSuccessor(i, BBDummy);
  }
  
  // Finally, merge the dummy BB into the predecessor. Note that phi nodes
  // in the successor are removed.
  bool result = MergeBlockIntoPredecessor(BBDummy);
}

// Return a pair of mergeable BBs
pair<BasicBlock *, BasicBlock*> MergeBasicBlocksPass::classifyMergeType(BasicBlock *BB) {
  Instruction *I = BB->getTerminator();
  // If this is not a branch instruction, return.
  if (!isa<BranchInst>(I)) { 
    return {nullptr, nullptr};
  }
  // Check if this branch instruction is mergeable.
  Value *COND;
  BasicBlock *BB_LEFT;
  BasicBlock *BB_RIGHT;
  ConstantInt *C;
  // Match each case
  if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isOne()) {
    // br i1 true, label %BB_LEFT, label %BB_RIGHT
    return {BB, BB_LEFT};
  } else if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isZero()) {
    //br i1 false, label %BB_LEFT, label %BB_RIGHT
    return {BB, BB_RIGHT};
  } else if (match(I, m_Br(m_Value(COND), m_BasicBlock(BB_LEFT), m_Deferred(BB_LEFT)))) {
    //br i1 COND, label %BB_LEFT, label %BB_LEFT
    return {BB, BB_LEFT};
  } else if (match(I, m_UnconditionalBr(BB_LEFT))) {
    //br label %BB_LEFT
    return {BB, BB_LEFT};
  } else {
    return {nullptr, nullptr};
  }
}
}
