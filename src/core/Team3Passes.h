#ifndef TEAM3PASSES_H
#define TEAM3PASSES_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"

#include <utility>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
class MergeBasicBlocksPass : public PassInfoMixin<MergeBasicBlocksPass> {
private:
  pair<BasicBlock *, BasicBlock *> classifyMergeType(BasicBlock *BB);
  void mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc);
public:
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
};

class ConstantFolding : public PassInfoMixin<ConstantFolding> {
public:
  bool foldICmp(ICmpInst::Predicate Pred, ConstantInt* C1, ConstantInt* C2);
  bool checkConstant(Value* X, Value* Y);
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
};

#include <set>

class Heap2Stack : public PassInfoMixin<Heap2Stack> {
  set<string>& mallocLikeFunc;
public:
  Heap2Stack(set<string>& mallocLikes):mallocLikeFunc(mallocLikes){};
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
  set<string> getMallocLikes() const { return mallocLikeFunc; }
};
}

#endif