#include "../core/Team3Passes.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/MemoryBuiltins.h"

#include <vector>

#define MAX_SIZE 1024

using namespace std;
using namespace llvm;

namespace backend {
PreservedAnalyses Heap2Stack::run(Function &F, FunctionAnalysisManager &FAM) {
  
}
}