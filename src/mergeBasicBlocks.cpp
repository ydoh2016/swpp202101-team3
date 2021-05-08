#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <vector>
using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

class MergeBasicBlocks : public PassInfoMixin<MergeBasicBlocks> {
private:
  pair<BasicBlock *, BasicBlock*> classifyMergeType(BasicBlock *BB);
  void mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc);
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

void MergeBasicBlocks::mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc) {
  // If the predecessor dominates the successor, simpy merge the successor into
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
pair<BasicBlock *, BasicBlock*> MergeBasicBlocks::classifyMergeType(BasicBlock *BB) {

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

  if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isOne()) {
    // br i1 true, label %BB_LEFT, label %BB_RIGHT
    //outs() << "Case1 " << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isZero()) {
    //br i1 false, label %BB_LEFT, label %BB_RIGHT
    //outs() << "Case2 " << BB_RIGHT->getName() << "\n";
    return {BB, BB_RIGHT};
  } else if (match(I, m_Br(m_Value(COND), m_BasicBlock(BB_LEFT), m_Deferred(BB_LEFT)))) {
    //br i1 COND, label %BB_LEFT, label %BB_LEFT
    //outs() << "Case3 " << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else if (match(I, m_UnconditionalBr(BB_LEFT))) {
    //br label %BB_LEFT
    //outs() << "Case4 " << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else {
    //outs() << "Case5\n";
    return {nullptr, nullptr};
  }
}

PreservedAnalyses MergeBasicBlocks::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  vector<pair<BasicBlock*, BasicBlock*>> BBPairToMerge;

  // First, search for mergeable pairs.
  for (auto &BB : F) {
    pair<BasicBlock*, BasicBlock*> mergePair = classifyMergeType(&BB);
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

  return PreservedAnalyses::all();
}

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "mergeBasicBlocks", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
          ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "mergeBasicBlocks") {
            FPM.addPass(MergeBasicBlocks());
            return true;
          }
          return false;
        }
      );
    }
  };
}
