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

// Merging Equivalent Basic Blocks Pass

// To reduce unnecessary branching cost caused by unconditional branches,
// merge all the basic blocks that are equivalent to each other. That is,
// if two basic blocks are linked through an uncondition branch, merge them
// together according two the following two conditions.
// 1) BB_1 flows into BB_2 with an unconditional branch
// 2) BB_1 dominates BB_2
// If two BBs don't satisfy the second condition, merge the second BB into the
// first one and keep the second one intact.

// Returns 
pair<BasicBlock *, BasicBlock*> classifyMergeType(BasicBlock *BB) {

  Instruction *I = BB->getTerminator();
  // If this is not a branch instruction, return
  if (isa<BranchInst>(I)) { 
    return;
  }

  // Check if this branch instruction is mergeable
  Value *COND;
  Value *LEFT;
  Value *RIGHT;

  BasicBlock *BB_LEFT = dyn_cast<BasicBlock>(LEFT);
  BasicBlock *BB_RIGHT = dyn_cast<BasicBlock>(RIGHT);
  assert(BB_LEFT != nullptr);
  assert(BB_RIGHT != nullptr);

  ConstantInt *C;
  if (match(I, m_Br(m_ConstantInt(C), m_Value(LEFT), m_Value(RIGHT))) &&
      C->isOne()) {
    outs() << BB_LEFT->getName() << ", " << BB_RIGHT->getName() << "\n";
    // br i1 true, label %BB_LEFT, label %BB_RIGHT
    return {BB, BB_LEFT};
  } else if (match(I, m_Br(m_ConstantInt(C), m_Value(LEFT), m_Value(RIGHT))) &&
      C->isZero()) {
    outs() << BB_LEFT->getName() << ", " << BB_RIGHT->getName() << "\n";
    //br i1 false, label %BB_LEFT, label %BB_RIGHT
    return {BB, BB_RIGHT};
  } else if (match(I, m_Br(m_Value(COND), m_Value(LEFT), m_Deferred(LEFT)))) {
    outs() << BB_LEFT->getName() << ", " << BB_RIGHT->getName() << "\n";
    //br i1 COND, label %BB_LEFT, label %BB_LEFT
    return {BB, BB_LEFT};
  } else {
    outs() << BB_LEFT->getName() << ", " << BB_RIGHT->getName() << "\n";
    return {nullptr, nullptr};
  }
}

PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  vector<pair<BasicBlock*, BasicBlock*>> BBPairToMerge;
  vector<BasicBlock> BBsToRemove; // Which basic blocks will be away?

  for (auto &BB : F) {
    pair<BasicBlock*, BasicBlock*> mergePair;
    if (mergePair.first != nullptr && mergePair.second != nullptr) {
      BBPairToMerge.push_back(mergePair);
    }
  }

  return PreservedAnalyses::all();
}