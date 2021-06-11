#include "../core/Team3Passes.h"

#include "llvm/IR/CFG.h"

#include <vector>

using namespace std;
using namespace llvm;

namespace backend {
PreservedAnalyses LoopInterchange::run(Function &F, FunctionAnalysisManager &FAM) {
  auto& LA = FAM.getResult<LoopAnalysis>(F);
  vector<vector<BasicBlock*>> loopblock;
  vector<BasicBlock*> headers;
  vector<BasicBlock*> exiters;
  vector<Loop*> outers;
  vector<Loop*> inners;
  int count = 0;
  bool invariant = true;
  /*
   * find interchange-able loop
   */
  for(auto& outer : LA.getTopLevelLoopsVector()) {
    Loop* inner;
    if(outer->getSubLoopsVector().size() == 1) {
      inner = outer->getSubLoopsVector().at(0);
      outers.push_back(outer);
      inners.push_back(inner);
      for(BasicBlock* BB : outer->getBlocksVector()) {
        for(auto& I : *BB) {
          // is it ok to interchange loop?
          // if not, invariant <- false
        }
      }
    }
  }
  /*
   * get loop information
   */
  //for(auto& BB : F) {
  //  if(LA.getLoopDepth(&BB) == 0) continue;
  //  if(LA.isLoopHeader(&BB) && LA.getLoopDepth(&BB) == 2) {
  //    headers.push_back(&BB); // get header
  //  }
  //  if(LA.getLoopDepth(&BB) > loopblock.size()) {
  //    vector<BasicBlock*> v;
  //    loopblock.push_back(v);
  //  }
  //  loopblock.at(LA.getLoopDepth(&BB) - 1).push_back(&BB);
  //  //outs() << LA.getLoopDepth(&BB) << "\n";
  //}
  //outs() << "R : " << LA.getTopLevelLoopsVector().size();
  for(auto& L : outers) {
    headers.push_back(L->getHeader());
    // use exit block or exit blocks
    if(L->getExitBlock()) {
      exiters.push_back(L->getExitBlock());
    }
  }
  /*
   * modify loop
   * ... -> outer loop hdr -> (...) -> inner loop hdr -> ... -> inner loop exit -> (...) -> outer loop exit -> ...
   * ->
   * ... -> inner loop hdr -> outer loop hdr -> ... -> outer loop exit -> inner loop exit -> ...
   */
  if(invariant) {
    for(auto& H : headers) {
      for(auto& BR : *H) {
        // modify br inst
      }
    }
  }
  return PreservedAnalyses::all();
}
}