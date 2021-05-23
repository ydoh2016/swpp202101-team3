#ifndef TEAM3PASSES_H
#define TEAM3PASSES_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <utility>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
class MergeBasicBlocksPass : public PassInfoMixin<MergeBasicBlocksPass> {
private:
  pair<BasicBlock *, BasicBlock *> classifyMergeType(BasicBlock *BB);
  void mergeSafely(Function *F, const DominatorTree &DT, BasicBlock *BBPred, BasicBlock *BBSucc);
  void removeDanglingPhi(Function *F);
public:
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
};

class AbbrMemPass : public PassInfoMixin<AbbrMemPass> {
private:
  static int MAXSEQ;
  int getMask(const vector<Instruction*> &sequence);
  void getInst(BasicBlock *BB, unsigned opCode, vector<Instruction*> *instList);
  void getSequences(const vector<Instruction*> &instList, vector<vector<Instruction*>> *sequences);
  bool isIdentical(Value *V1, Value *V2);
  void processModule(Module *M);
  bool inSameSequence(Value *V1, Value *V2, int * difference);
  void replaceInstructions(Module *M);
public:
  PreservedAnalyses run(Module& M, ModuleAnalysisManager& MAM);
};
}

#endif