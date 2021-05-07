#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include <set>
#include <vector>
using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

class MergeBasicBlocks : public PassInfoMixin<MergeBasicBlocks> {
private:
  pair<BasicBlock *, BasicBlock*> classifyMergeType(BasicBlock *BB);
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

// Return a pair of mergeable BBs
pair<BasicBlock *, BasicBlock*> MergeBasicBlocks::classifyMergeType(BasicBlock *BB) {

  Instruction *I = BB->getTerminator();
  // If this is not a branch instruction, return
  if (!isa<BranchInst>(I)) { 
    return {nullptr, nullptr};
  }

  // Check if this branch instruction is mergeable
  Value *COND;
  BasicBlock *BB_LEFT;
  BasicBlock *BB_RIGHT;
  ConstantInt *C;

  if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isOne()) {
    // br i1 true, label %BB_LEFT, label %BB_RIGHT
    outs() << "Case1" << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else if (match(I, m_Br(m_ConstantInt(C), m_BasicBlock(BB_LEFT), m_BasicBlock(BB_RIGHT))) &&
      C->isZero()) {
    //br i1 false, label %BB_LEFT, label %BB_RIGHT
    outs() << "Case2" << BB_RIGHT->getName() << "\n";
    return {BB, BB_RIGHT};
  } else if (match(I, m_Br(m_Value(COND), m_BasicBlock(BB_LEFT), m_Deferred(BB_LEFT)))) {
    //br i1 COND, label %BB_LEFT, label %BB_LEFT
    outs() << "Case3" << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else if (match(I, m_UnconditionalBr(BB_LEFT))) {
    //br label %BB_LEFT
    outs() << "Case4" << BB_LEFT->getName() << "\n";
    return {BB, BB_LEFT};
  } else {
    outs() << "Case5\n";
    return {nullptr, nullptr};
  }
}

PreservedAnalyses MergeBasicBlocks::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  vector<pair<BasicBlock*, BasicBlock*>> BBPairToMerge;
  vector<BasicBlock> BBsToRemove; // Which basic blocks will be away?

  for (auto &BB : F) {
    pair<BasicBlock*, BasicBlock*> mergePair = classifyMergeType(&BB);
    if (mergePair.first != nullptr && mergePair.second != nullptr) {
      BBPairToMerge.push_back(mergePair);
    }
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
