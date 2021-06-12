#include "../core/Team3Passes.h"

#include "llvm/IR/CFG.h"
#include "llvm/Pass.h"

#include <vector>

using namespace std;
using namespace llvm;
using namespace llvm::PatternMatch;

namespace backend {
PreservedAnalyses LoopInterchange::run(Function &F, FunctionAnalysisManager &FAM) {
  //F.materialize();
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
      case1 = true;
    }
    else {
      case1 = false;
    }
            outs() << case1;

    interchange.at(i) = case1;
    // 2. is there no instruction other than inner loop
    //    idea by @ObjectOrientedLife Thank you :)
    //    loop header(for.body of outer loop) should contain only initializing i and br inst
    //    -> it can be / will be optimized further if we success on pushing initialization to start of outer-est loop
    bool case2;
    BasicBlock* BBHeader = outer->getHeader();
    Instruction* terminator = dyn_cast<Instruction>(BBHeader->getTerminator());
    int successorCount = terminator->getNumSuccessors();
    for(int j = 0; j < successorCount; ++j) {
      BasicBlock* successor = terminator->getSuccessor(j);
      outs() << "UFEWF" << successor->size();
      if(successor->size() == 3) {
        case2 = true;
      } else {
        case2 = false;
      }
    }
        outs() << case2;

    interchange.at(i) = interchange.at(i) & case2;
  }
  /*
   * get loop information
   */
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
  for(int i = 0; i < outers.size(); ++i) {
    Loop* outer = outers.at(i);
    outs() << interchange.at(i);
    if(!interchange.at(i)) continue;
    auto *HdrTerm = dyn_cast<BranchInst>(outer->getHeader()->getTerminator());
    if(!HdrTerm) continue;
    // outs() << "Let's interchange~!\n";
    /*
     * Interchange
     */
    Value* X;
    BasicBlock* entry;
    BasicBlock* outer_cond = outer->getHeader();
    BasicBlock* outer_body;
    BasicBlock* outer_inc;
    BasicBlock* outer_end;
    BasicBlock* inner_cond = inners.at(i).at(0)->getHeader();
    BasicBlock* inner_body;
    BasicBlock* inner_inc;
    BasicBlock* inner_end;
    entry = outer_cond->getSinglePredecessor();
    BranchInst* i1 = dyn_cast<BranchInst>(outer_cond->getTerminator());
    match(i1, m_Br(m_Value(X), m_BasicBlock(outer_body), m_BasicBlock(outer_end)));
    BranchInst* i2 = dyn_cast<BranchInst>(inner_cond->getTerminator());
    match(i2, m_Br(m_Value(X), m_BasicBlock(inner_body), m_BasicBlock(inner_end)));
    outer_inc = inner_end->getSingleSuccessor();
    /*
     * modify edges
     * issue : if there are several blocks in loop_body?
     */
    BranchInst* I1 = dyn_cast<BranchInst>(entry->getTerminator());
    BranchInst* I2 = dyn_cast<BranchInst>(outer_cond->getTerminator());
    BranchInst* I3 = dyn_cast<BranchInst>(outer_body->getTerminator());
    BranchInst* I4 = dyn_cast<BranchInst>(outer_inc->getTerminator());
    BranchInst* I5 = dyn_cast<BranchInst>(outer_end->getTerminator());
    BranchInst* I6 = dyn_cast<BranchInst>(inner_cond->getTerminator());
    BranchInst* I7 = dyn_cast<BranchInst>(inner_body->getTerminator());
    BranchInst* I8 = dyn_cast<BranchInst>(inner_inc->getTerminator());
    BranchInst* I9 = dyn_cast<BranchInst>(inner_end->getTerminator());
    I1->setSuccessor(0, inner_cond);
    I2->setSuccessor(0, inner_body);
    I3->setSuccessor(0, outer_cond);
    I4->setSuccessor(0, inner_cond);
    I6->setSuccessor(0, outer_body);
    I7->setSuccessor(0, outer_inc);
  }
  outs() << "Done!\n";
  return PreservedAnalyses::all();
}
}