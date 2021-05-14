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
    for(auto& BB : F) {
      for(auto I_it = BB.begin(); I_it != BB.end();) {
        /* Including constructing iteration,
         * there were helps in fixing errors in this code from my teammates,
         * @JeffLee77-prog, @ObjectOrientedLife, @Chavo-Kim,
         * so I briefly keep reference in comment. */
        auto& I = *I_it++;
        Value* X;
        Value* Y;
        BasicBlock* BB1;
        BasicBlock* BB2;
        ICmpInst::Predicate Pred;
        if(match(&I, m_ICmp(Pred, m_Value(X), m_Value(Y)))) { // fold icmp inst
          /* use getbool(C1 ->)Error : static llvm::Constant* llvm::ConstantInt::getFalse(llvm::Type*): Assertion `Ty->isIntOrIntVectorTy(1) && "Type not i1 or vector of i1."' failed. 
             Error : void llvm::Value::doRAUW(llvm::Value*, llvm::Value::ReplaceMetadataUses): Assertion `New->getType() == getType() && "replaceAllUses of value with new value of different type!"' failed. */
          if(checkConstant(X, Y)) { // todo: fix bool -> type* issue
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            bool bresult; // this is the result of constant comparison
            if(Pred == ICmpInst::ICMP_EQ) {
              if(C1 -> getZExtValue() == dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpEq = ConstantInt::getBool(F.getContext(), bresult);
              I.replaceAllUsesWith(CICmpEq);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_NE) {
              if(dyn_cast<ConstantInt>(X) -> getZExtValue() != dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpNe = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpNe);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_UGT) {
              if(dyn_cast<ConstantInt>(X) -> getZExtValue() > dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpUgt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpUgt);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_SGT) {
              if(dyn_cast<ConstantInt>(X) -> getSExtValue() > dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpSgt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpSgt);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_UGE) {
              if(dyn_cast<ConstantInt>(X) -> getZExtValue() >= dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpUge = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpUge);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_SGE) {
              if(dyn_cast<ConstantInt>(X) -> getSExtValue() >= dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpSge = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpSge);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_ULT) {
              if(dyn_cast<ConstantInt>(X) -> getZExtValue() < dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpUlt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpUlt);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_SLT) {
              if(dyn_cast<ConstantInt>(X) -> getSExtValue() < dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpSlt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpSlt);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_ULE) {
              if(dyn_cast<ConstantInt>(X) -> getZExtValue() <= dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpUle = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpUle);
              I.eraseFromParent();
            } else if(Pred == ICmpInst::ICMP_SLE) {
              if(dyn_cast<ConstantInt>(X) -> getSExtValue() <= dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
              else bresult = false;
              auto* CICmpSle = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
              I.replaceAllUsesWith(CICmpSle);
              I.eraseFromParent();
            }
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
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() + C2 -> getZExtValue(), false));
            I.eraseFromParent();
          }
        } else if(match(&I, m_Sub(m_Value(X), m_Value(Y)))) { // fold sub inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() - C2 -> getZExtValue(), false));
            I.eraseFromParent();
          }
        } else if(match(&I, m_Mul(m_Value(X), m_Value(Y)))) { // fold mul inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() * C2 -> getZExtValue(), false));
            I.eraseFromParent();
          }
        } else if(match(&I, m_UDiv(m_Value(X), m_Value(Y)))) { // fold udiv inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() / C2 -> getZExtValue(), false));
            I.eraseFromParent();
          }
        } else if(match(&I, m_SDiv(m_Value(X), m_Value(Y)))) { // fold sdiv inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getSExtValue() / C2 -> getSExtValue(), true));
            I.eraseFromParent();
          }
        } else if(match(&I, m_URem(m_Value(X), m_Value(Y)))) { // fold urem inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getZExtValue() % C2 -> getZExtValue(), false));
            I.eraseFromParent();
          }
        } else if(match(&I, m_SRem(m_Value(X), m_Value(Y)))) { // fold srem inst
          if(checkConstant(X, Y)) {
            ConstantInt* C1 = dyn_cast<ConstantInt>(X);
            ConstantInt* C2 = dyn_cast<ConstantInt>(Y);
            I.replaceAllUsesWith(ConstantInt::get(C1 -> getType(), C1 -> getSExtValue() % C2 -> getSExtValue(), true));
            I.eraseFromParent();
          }
        }
      }
    }
  return PreservedAnalyses::none();
}
}