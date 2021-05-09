#ifndef CONSTANT_FOLDING_H
#define CONSTANT_FOLDING_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace llvm;

namespace backend {
class ConstantFolding : public PassInfoMixin<ConstantFolding> {
public:
  bool checkConstant(Value* X, Value* Y);
  PreservedAnalyses run(Function &, FunctionAnalysisManager &);
};
}

#endif