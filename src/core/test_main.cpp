#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"
#include "../backend/Backend.h"
#include "Team3Passes.h"

#include <iostream>
#include <fstream>

using namespace llvm;
using namespace std;

string readFileIntoString(const string& path) {
    ifstream input_file(path);
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << path << "'" << endl;
        exit(EXIT_FAILURE);
    }
    return string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

TEST(TestDemo, CheckMain) {
  // Show that the assembler correctly emits 'start main 0' as well as 'end main'
  LLVMContext Context;
  unique_ptr<Module> M(new Module("MyTestModule", Context));
  auto *I64Ty = Type::getInt64Ty(Context);
  auto *TestFTy = FunctionType::get(I64Ty, {}, false);
  Function *TestF = Function::Create(TestFTy, Function::ExternalLinkage,
                                     "main", *M);

  BasicBlock *Entry = BasicBlock::Create(Context, "entry", TestF);
  IRBuilder<> EntryBuilder(Entry);
  EntryBuilder.CreateRet(ConstantInt::get(I64Ty, 0));

  // execute IR passes
  ModuleAnalysisManager MAM;

  string str = "tmp.txt";
  Backend B(str, false);
  B.run(*M, MAM);

  str = readFileIntoString(str);
  // These strings should exist in the assembly!
  EXPECT_NE(str.find("start main 0:"), string::npos);
  EXPECT_NE(str.find("end main"), string::npos);
}


// TEST(InstructionRedundancyTest, MergeBlock1) {
//   LLVMContext Context;
//   SMDiagnostic Error;
//   unique_ptr<Module> M = parseAssemblyFile("./filechecks/MergeBasicBlocks/check1.ll", Error, Context);
//   ASSERT_TRUE(M);

  
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
