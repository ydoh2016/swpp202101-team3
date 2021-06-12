#include "../core/Team3Passes.h"

#include "llvm/IR/CFG.h"
#include "llvm/Pass.h"

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
  vector<vector<Loop*>> inners;
  int count = 0;
  bool invariant = true;
  /*
   * find loops
   */
  for(auto& outer : LA.getTopLevelLoopsVector()) {
    outers.push_back(outer);
    for(int i = 0; i < outer->getSubLoopsVector().size(); ++i) {
      if(i == 0) {
        vector<Loop*> tmp;
        inners.push_back(tmp);
      }
      Loop* inner;
      inner = outer->getSubLoopsVector().at(i);
      inners.back().push_back(inner);
    }
    outs() << "Loop " << outers.size() << " inners : " << inners.at(outers.size() - 1).size() << "\n";
  }
  /*
   * find interchange-able loops
   * conditions should be founded / deleted
   */
  vector<bool> interchange;
  interchange.resize(outers.size());
  for(int i = 0; i < outers.size(); ++i) {
    Loop* outer = outers.at(i);
    // 1. is loop is doubley nested?
    bool case1;
    if(outer->getSubLoopsVector().size() == 1 && outer->getSubLoopsVector().at(0)->getSubLoopsVector().empty()) {
      outs() << "case 1\n";
      case1 = true;
    }
    else {
      case1 = false;
    }
    //interchange.at(i) = interchange.at(i) & case1;
    interchange.at(i) = case1;
    // 2. is there no instruction other than inner loop
    //    idea by @ObjectOrientedLife Thank you :)
    bool case2;
    BasicBlock* BBHeader = outer->getHeader();
    Instruction* terminator = dyn_cast<Instruction>(BBHeader->getTerminator());
    int successorCount = terminator->getNumSuccessors();
    int instCount = 0;
    for(int j = 0; j < successorCount; ++j) {
      BasicBlock* successor = terminator->getSuccessor(j);
      outs() << "successor : " << successor->getName().str() << "\n"; // todo: successor's name seems to be null infilecheck
      if(successor->getName().str().find("for.body") != string::npos) {
        for(auto& I : *successor) {
          instCount++;
          outs() << instCount;
        }
      }
    }
    if(instCount == 0) {
      case2 = true;
    } else {
      case2 = false;
    }
    interchange.at(i) = interchange.at(i) & case2;
  }
  for(int j = 0; j < interchange.size(); ++j) {
    outs() << interchange.at(j) << "\n";
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