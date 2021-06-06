#include "../core/Team3Passes.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace std;
using namespace llvm;

namespace backend {
PreservedAnalyses LoopInterchange::run(Function &F, FunctionAnalysisManager &FAM) {
  return PreservedAnalyses::all();
}
}