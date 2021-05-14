#include "ConstantFolding.h"

using namespace std;
using namespace llvm;
using namespace llvm::PatternMatch;

namespace backend {
bool ConstantFolding::foldICmp(ICmpInst::Predicate Pred, ConstantInt* C1, ConstantInt* C2) {
  if(Pred == ICmpInst::ICMP_EQ) {
    if(C1 -> getZExtValue() == C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_NE) {
    if(C1 -> getZExtValue() != C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_UGT) {
    if(C1 -> getZExtValue() > C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_SGT) {
    if(C1 -> getSExtValue() > C2 -> getSExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_UGE) {
    if(C1 -> getZExtValue() >= C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_SGE) {
    if(C1 -> getSExtValue() >= C2 -> getSExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_ULT) {
    if(C1 -> getZExtValue() < C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_SLT) {
    if(C1 -> getSExtValue() < C2 -> getSExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_ULE) {
    if(C1 -> getZExtValue() <= C2 -> getZExtValue()) return true;
    else return false;
  } else if(Pred == ICmpInst::ICMP_SLE) {
    if(C1 -> getSExtValue() <= C2 -> getSExtValue()) return true;
    else return false;
  }
  outs() << "Unclassified predicate!\n"; // process should not reach here.

  return true;
}
bool ConstantFolding::checkConstant(Value* X, Value* Y) {
  auto* C1 = dyn_cast<ConstantInt>(X);
  auto* C2 = dyn_cast<ConstantInt>(Y);
  if(C1 == nullptr || C2 == nullptr) return false;
  return true;
}
PreservedAnalyses ConstantFolding::run(Function &F, FunctionAnalysisManager &FAM) {
    for(auto& BB : F) {
      for(auto I_it = BB.begin(); I_it != BB.end();) {
        /* Including constructing iteration,
         * my teammates helped with fixing errors,
         * @JeffLee77-prog, @ObjectOrientedLife, @Chavo-Kim,
         * therefore I briefly keep reference through this comment. */
        auto& I = *I_it++;
        Value* X;
        Value* Y;
        BasicBlock* BB1;
        BasicBlock* BB2;
        ICmpInst::Predicate Pred;
        if(match(&I, m_ICmp(Pred, m_Value(X), m_Value(Y)))) { // fold icmp inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CICmp = ConstantInt::getBool(F.getContext(), foldICmp(Pred, C1, C2));
            I.replaceAllUsesWith(CICmp);
            I.eraseFromParent();
          }
        } else if(match(&I, m_Br(m_Value(X), m_BasicBlock(BB1), m_BasicBlock(BB2)))) { // fold br inst
          BasicBlock::iterator bbit(I);
          if(checkConstant(X, X)) {
            ConstantInt* C = dyn_cast<ConstantInt>(X);
            ReplaceInstWithInst(I.getParent() -> getInstList(), bbit, C -> getZExtValue() ? BranchInst::Create(BB1) : BranchInst::Create(BB2));
          }
        } else if(match(&I, m_Add(m_Value(X), m_Value(Y)))) { // fold add inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CAdd = ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() + C2 -> getZExtValue(), false);
            I.replaceAllUsesWith(CAdd);
            I.eraseFromParent();
          }
        } else if(match(&I, m_Sub(m_Value(X), m_Value(Y)))) { // fold sub inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CSub = ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() - C2 -> getZExtValue(), false);
            I.replaceAllUsesWith(CSub);
            I.eraseFromParent();
          }
        } else if(match(&I, m_Mul(m_Value(X), m_Value(Y)))) { // fold mul inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CMul = ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() * C2 -> getZExtValue(), false);
            I.replaceAllUsesWith(CMul);
            I.eraseFromParent();
          }
        } else if(match(&I, m_UDiv(m_Value(X), m_Value(Y)))) { // fold udiv inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CUDiv = ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() / C2 -> getZExtValue(), false);
            I.replaceAllUsesWith(CUDiv);
            I.eraseFromParent();
          }
        } else if(match(&I, m_SDiv(m_Value(X), m_Value(Y)))) { // fold sdiv inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CSDiv = ConstantInt::get(C1 -> getType(), C1 -> getSExtValue() / C2 -> getSExtValue(), true);
            I.replaceAllUsesWith(CSDiv);
            I.eraseFromParent();
          }
        } else if(match(&I, m_URem(m_Value(X), m_Value(Y)))) { // fold urem inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CURem = ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() % C2 -> getZExtValue(), false);
            I.replaceAllUsesWith(CURem);
            I.eraseFromParent();
          }
        } else if(match(&I, m_SRem(m_Value(X), m_Value(Y)))) { // fold srem inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            auto* CSRem = ConstantInt::get(C1 -> getType(), C1 -> getSExtValue() % C2 -> getSExtValue(), true);
            I.replaceAllUsesWith(CSRem);
            I.eraseFromParent();
          }
        }
      }
    }
  return PreservedAnalyses::none();
}
}