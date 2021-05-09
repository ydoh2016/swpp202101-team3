#include "ConstantFolding.h"

using namespace std;
using namespace llvm;
using namespace llvm::PatternMatch;

namespace backend {
  bool ConstantFolding::checkConstant(Value* X, Value* Y) {
    auto* C1 = dyn_cast<ConstantInt>(X);
    auto* C2 = dyn_cast<ConstantInt>(Y);
    if(C1 == nullptr || C2 == nullptr) return false;
    return true;
  }
  PreservedAnalyses ConstantFolding::run(Function &F, FunctionAnalysisManager &FAM) {
      std::vector<Instruction*> instsToRemove;
      uint64_t ua;
      uint64_t ub;
      uint64_t uresult;
      int64_t sresult;
      for(auto& BB : F) {
        for(auto& I : BB) {
          Value* X;
          Value* Y;
          BasicBlock* BB1;
          BasicBlock* BB2;
          ICmpInst::Predicate Pred;
          if(match(&I, m_ICmp(Pred, m_Value(X), m_Value(Y)))) { // fold icmp inst
            if(checkConstant(X, Y)) {
              if(Pred == ICmpInst::ICMP_EQ) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() == dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpEq = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpEq);                instsToRemove.push_back(&I);
              } else if(Pred == ICmpInst::ICMP_NE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() != dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpNe = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpNe);
              } else if(Pred == ICmpInst::ICMP_UGT) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() > dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpUgt = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpUgt);
              } else if(Pred == ICmpInst::ICMP_SGT) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() > dyn_cast<ConstantInt>(Y) -> getSExtValue()) sresult = 1;
                else sresult = 0;
                auto* CICmpSgt = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), sresult, false);
                I.replaceAllUsesWith(CICmpSgt);
              } else if(Pred == ICmpInst::ICMP_UGE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() >= dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpUge = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpUge);
              } else if(Pred == ICmpInst::ICMP_SGE) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() >= dyn_cast<ConstantInt>(Y) -> getSExtValue()) sresult = 1;
                else sresult = 0;
                auto* CICmpSge = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), sresult, false);
                I.replaceAllUsesWith(CICmpSge);
              } else if(Pred == ICmpInst::ICMP_ULT) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() < dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpUlt = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpUlt);
              } else if(Pred == ICmpInst::ICMP_SLT) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() < dyn_cast<ConstantInt>(Y) -> getSExtValue()) sresult = 1;
                else sresult = 0;
                auto* CICmpSlt = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), sresult, false);
                I.replaceAllUsesWith(CICmpSlt);
              } else if(Pred == ICmpInst::ICMP_ULE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() <= dyn_cast<ConstantInt>(Y) -> getZExtValue()) uresult = 1;
                else uresult = 0;
                auto* CICmpUle = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), uresult, false);
                I.replaceAllUsesWith(CICmpUle);
              } else if(Pred == ICmpInst::ICMP_SLE) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() <= dyn_cast<ConstantInt>(Y) -> getSExtValue()) sresult = 1;
                else sresult = 0;
                auto* CICmpSle = ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), sresult, false);
                I.replaceAllUsesWith(CICmpSle);
              }
            }
          }
          else if(match(&I, m_Br(m_Value(X), m_BasicBlock(BB1), m_BasicBlock(BB2)))) { // fold br inst
            auto* C = dyn_cast<ConstantInt>(X);
            if(C != nullptr) {
              if(C -> getZExtValue()) {
                auto* CBr = BranchInst::Create(BB1);
                I.replaceAllUsesWith(CBr);
              } else {
                auto* CBr = BranchInst::Create(BB2);
                I.replaceAllUsesWith(CBr);
              }
            }
          }
          else if(match(&I, m_Add(m_Value(X), m_Value(Y)))) { // fold add inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() + dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
            }
          }
          else if(match(&I, m_Sub(m_Value(X), m_Value(Y)))) { // fold sub inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() - dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
            }
          }
          else if(match(&I, m_Mul(m_Value(X), m_Value(Y)))) { // fold mul inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() * dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
            }
          }
          else if(match(&I, m_UDiv(m_Value(X), m_Value(Y)))) { // fold udiv inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() / dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
            }
          }
          else if(match(&I, m_SDiv(m_Value(X), m_Value(Y)))) { // fold sdiv inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getSExtValue() / dyn_cast<ConstantInt>(Y) -> getSExtValue(), false));
            }
          }
          else if(match(&I, m_URem(m_Value(X), m_Value(Y)))) { // fold urem inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() % dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
            }
          }
          else if(match(&I, m_SRem(m_Value(X), m_Value(Y)))) { // fold srem inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getSExtValue() % dyn_cast<ConstantInt>(Y) -> getSExtValue(), false));
            }
          }
          instsToRemove.push_back(&I);
        }
      }
      for(auto& I : instsToRemove) {
        I -> eraseFromParent();
      }
    return PreservedAnalyses::none();
  }
}