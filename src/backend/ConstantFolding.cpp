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
      bool bresult;
      for(auto& BB : F) {
        for(auto iit = BB.begin(); iit != BB.end();) {
          /* Including constructing iteration, there were many helps in fixing errors in this code from my teammates,
             @JeffLee77-prog, @ObjectOrientedLife, @Chavo-Kim, so I briefly keep reference in comment. */
          auto& I = *iit++;
          LLVMContext context;
          Value* X;
          Value* Y;
          BasicBlock* BB1;
          BasicBlock* BB2;
          ICmpInst::Predicate Pred;
          if(match(&I, m_ICmp(Pred, m_Value(X), m_Value(Y)))) { // fold icmp inst
            /* Folding icmp instruction is not working well since casting in bool -> constant -> value* is not working.
               In codes below, bresult is boolean variable. 
               For getBool() to return Constant* so that it satisfies parameter conditions in replaceAllUsesWith(),
               Type* should be first parameter for getBool().
               But I found no methods to turn bool into Type*, excluding getInt1Ty() which eventually leads to an error as:
               void llvm::Value::doRAUW(llvm::Value*, llvm::Value::ReplaceMetadataUses): Assertion `New->getType() == getType() && "replaceAllUses of value with new value of different type!"' failed.
               replaceAllUses() needs Value* as parameter, but Constant* can be casted into Value*, so specific reason for this error is yet unknown to me.
               I will continue with this idea and put more time on this issue. if you have some idea on casting bool into type usable in replaceAllUsesWith(),
               please leave them as comment in PR before merging. */
            if(checkConstant(X, Y)) { // todo: fix bool -> type* issue
              if(Pred == ICmpInst::ICMP_EQ) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() == dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpEq = ConstantInt::getBool(Type::getInt1Ty(context), bresult);
                I.replaceAllUsesWith(CICmpEq);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_NE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() != dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpNe = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpNe);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_UGT) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() > dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpUgt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpUgt);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_SGT) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() > dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpSgt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpSgt);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_UGE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() >= dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpUge = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpUge);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_SGE) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() >= dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpSge = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpSge);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_ULT) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() < dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpUlt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpUlt);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_SLT) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() < dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpSlt = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpSlt);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_ULE) {
                if(dyn_cast<ConstantInt>(X) -> getZExtValue() <= dyn_cast<ConstantInt>(Y) -> getZExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpUle = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpUle);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else if(Pred == ICmpInst::ICMP_SLE) {
                if(dyn_cast<ConstantInt>(X) -> getSExtValue() <= dyn_cast<ConstantInt>(Y) -> getSExtValue()) bresult = true;
                else bresult = false;
                auto* CICmpSle = ConstantInt::getBool(dyn_cast<ConstantInt>(X) -> getType(), bresult);
                I.replaceAllUsesWith(CICmpSle);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              }
            }
          }
          else if(match(&I, m_Br(m_Value(X), m_BasicBlock(BB1), m_BasicBlock(BB2)))) { // fold br inst
            auto* C = dyn_cast<ConstantInt>(X);
            BasicBlock::iterator bbit(I);
            if(C != nullptr) {
              if(C -> getZExtValue()) {
                auto* CBr = BranchInst::Create(BB1);
                ReplaceInstWithInst(I.getParent() -> getInstList(), bbit, CBr);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              } else {
                auto* CBr = BranchInst::Create(BB2);
                ReplaceInstWithInst(I.getParent() -> getInstList(), bbit, CBr);
                instsToRemove.push_back(&I);
                I.eraseFromParent();
              }
            }
          }
          else if(match(&I, m_Add(m_Value(X), m_Value(Y)))) { // fold add inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() + dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_Sub(m_Value(X), m_Value(Y)))) { // fold sub inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() - dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_Mul(m_Value(X), m_Value(Y)))) { // fold mul inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() * dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_UDiv(m_Value(X), m_Value(Y)))) { // fold udiv inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() / dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_SDiv(m_Value(X), m_Value(Y)))) { // fold sdiv inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getSExtValue() / dyn_cast<ConstantInt>(Y) -> getSExtValue(), true));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_URem(m_Value(X), m_Value(Y)))) { // fold urem inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getZExtValue() % dyn_cast<ConstantInt>(Y) -> getZExtValue(), false));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
          else if(match(&I, m_SRem(m_Value(X), m_Value(Y)))) { // fold srem inst
            if(checkConstant(X, Y)) {
              I.replaceAllUsesWith(ConstantInt::get(dyn_cast<ConstantInt>(X) -> getType(), dyn_cast<ConstantInt>(X) -> getSExtValue() % dyn_cast<ConstantInt>(Y) -> getSExtValue(), true));
              instsToRemove.push_back(&I);
              I.eraseFromParent();
            }
          }
        }
      }
    return PreservedAnalyses::none();
  }
}