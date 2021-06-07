#include "../core/Team3Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <cassert>
#include <string>

using namespace llvm;
using namespace std;

namespace backend {

// Benchmark Loop Detector
bool LoopReverseTerminatorPass::isLoopCondBlock(string bbname){
    if(bbname.find("for.cond") != string::npos) 
        return true;
    
    if(bbname.find("while.cond") != string::npos) 
        return true;
    
    return false;
}

// Change Predicate that make Reverse Result
// ex. EQ -> NE, GE -> LT ...
CmpInst::Predicate LoopReverseTerminatorPass::getReversePredicate(CmpInst::Predicate pred) {
    if(pred == CmpInst::ICMP_EQ)
        return CmpInst::ICMP_NE;
    if(pred == CmpInst::ICMP_NE)
        return CmpInst::ICMP_EQ;
    if(pred == CmpInst::ICMP_UGT)
        return CmpInst::ICMP_ULE;
    if(pred == CmpInst::ICMP_ULE)
        return CmpInst::ICMP_UGT;
    if(pred == CmpInst::ICMP_UGE)
        return CmpInst::ICMP_ULT;
    if(pred == CmpInst::ICMP_ULT)
        return CmpInst::ICMP_UGE;
    if(pred == CmpInst::ICMP_SGT)
        return CmpInst::ICMP_SLE;
    if(pred == CmpInst::ICMP_SLE)
        return CmpInst::ICMP_SGT;
    if(pred == CmpInst::ICMP_SGE)
        return CmpInst::ICMP_SLT;
    if(pred == CmpInst::ICMP_SLT)
        return CmpInst::ICMP_SGE;

    return pred;
}

// Main Method
bool LoopReverseTerminatorPass::tryReverseTerminator(BasicBlock *ExitingBlock) {
    auto *TermIns = dyn_cast<BranchInst>(ExitingBlock->getTerminator());
    
    if(TermIns)
    {
        if(!(TermIns->isConditional()))
            return false;
        IRBuilder<> builder(TermIns);
        auto *CondVal = dyn_cast<CmpInst>(TermIns->getCondition());
        //Reverse Predicate
        CondVal->setPredicate(getReversePredicate(CondVal->getPredicate()));
        //Reverse True/False
        auto *lhs = TermIns->getSuccessor(0);
        auto *rhs = TermIns->getSuccessor(1);
        auto *NewInst = BranchInst::Create(rhs, lhs, CondVal);
        ReplaceInstWithInst(TermIns, NewInst);
        return true;
    }
    //Handle case for binary_tree
    else
    {
        auto *TermIns2 = dyn_cast<SwitchInst>(ExitingBlock->getTerminator());

        if(!TermIns2)
            return false;

        if(TermIns2->getNumSuccessors() != 2)
            return false;
        
        IRBuilder<> builder(TermIns2);
        auto *CondVal = TermIns2->getCondition();
        auto *NewCondVal = builder.CreateICmpEQ(CondVal, ConstantInt::get(Type::getInt32Ty(TermIns2->getContext()), 1, true));
        auto *lhs = TermIns2->getDefaultDest();
        auto *rhs = TermIns2->getSuccessor(1);
        auto *NewInst = BranchInst::Create(lhs, rhs, NewCondVal);
        ReplaceInstWithInst(TermIns2, NewInst);
        return true;
    }    

    return false;  
}
    
PreservedAnalyses LoopReverseTerminatorPass::run(Function &F, FunctionAnalysisManager &FAM) {
    auto &LI = FAM.getResult<LoopAnalysis>(F);

    bool changed = false;

    //In Benchmark Program
    for(auto &ExitingBlock : F) 
    {
        if(!isLoopCondBlock(ExitingBlock.getName().str()))
            continue;

        changed = tryReverseTerminator(&ExitingBlock);
    }

    //For Other LLVM Program
    for(auto &L : LI)
    {
        BasicBlock *ExitingBlock = L->getLoopLatch();
        if(!ExitingBlock || !L->isLoopExiting(ExitingBlock))
            L->getExitingBlock();

        if(ExitingBlock)
            changed = tryReverseTerminator(ExitingBlock);
    }

    if(!changed)
        return PreservedAnalyses::all();

    return getLoopPassPreservedAnalyses();
}
}