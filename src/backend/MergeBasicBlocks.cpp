#include "MergeBasicBlocks.h"

namespace backend {
PreservedAnalyses MergeBasicBlocksPass::run(Function& F, FunctionAnalysisManager& FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  vector<pair<BasicBlock*, BasicBlock*>> BBPairToMerge;

  // First, search for mergeable pairs.
  for (auto &BB : F) {
    pair<BasicBlock *, BasicBlock *> mergePair = classifyMergeType(&BB);
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

  // Finally, get rid of dangling BBs.
  EliminateUnreachableBlocks(F);

  return PreservedAnalyses::all();
}

void MergeBasicBlocksPass::mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc) {
  // If the predecessor dominates the successor, simply merge the successor into
  // the predecessor.
  if (DT.dominates(BBPred, BBSucc)) {
    MergeBlockIntoPredecessor(BBSucc);
  } else { 
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
    // Since the CloneBasicBlock function merely gets rid of phi nodes, replace 
    // every phi node in the successor manually.
    for (auto &I : *BBDummy) {
      PHINode *PN = dyn_cast<PHINode>(&I);
      if (PN != nullptr) {
        Value *replacingVal = PN->getIncomingValueForBlock(BBPred);
        if (replacingVal != nullptr) {
          //outs() << replacingVal->getName() << "\n";
          for (auto it = I.use_begin(), end = I.use_end(); it != end;) {
            Use &U = *it++;
            U.set(replacingVal);
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
    MergeBlockIntoPredecessor(BBDummy);
  }
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
