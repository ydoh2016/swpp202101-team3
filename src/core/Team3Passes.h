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

#include <set>
#include <utility>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
class ConstantFolding : public PassInfoMixin<ConstantFolding> {
public:
  bool foldICmp(ICmpInst::Predicate Pred, ConstantInt* C1, ConstantInt* C2);
  bool checkConstant(Value* X, Value* Y);
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
};

class Heap2Stack : public PassInfoMixin<Heap2Stack> {
  set<string>& mallocLikeFunc;
public:
  bool checkConv(Function* F, set<string>& mallocLikeFunc);
  Heap2Stack(set<string>& mallocLikes):mallocLikeFunc(mallocLikes){};
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
  set<string> getMallocLikes() const { return mallocLikeFunc; };
};

class AbbrMemPass : public PassInfoMixin<AbbrMemPass> {
private:
  static int MAXSEQ;
  int getMask(const vector<Instruction*> &sequence);
  void getInst(BasicBlock *BB, unsigned opCode, vector<Instruction*> *instList);
  void getSequences(const vector<Instruction*> &instList, vector<vector<Instruction*>> *sequences);
  bool isIdentical(Value *V1, Value *V2);
  void processModule(Module *M);
  bool inSameSequence(Instruction *I1, Instruction *I2, int *difference);
  void replaceInstructions(Module *M);
public:
  PreservedAnalyses run(Module& M, ModuleAnalysisManager& MAM);
};

#endif