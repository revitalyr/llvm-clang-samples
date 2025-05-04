//------------------------------------------------------------------------------
// simple_bb_pass LLVM sample. Demonstrates:
//
// * How to write a BasicBlockPass
// * How to iterate over instructions in a basic block and detect instructions
//   of a certain type.
// * How to query for a type's size using the module's data layout.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <iostream>
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

//using namespace llvm;

class AllocaSizeDetect : public llvm::Pass {
public:
  AllocaSizeDetect() : Pass(llvm::PassKind::PT_Module, ID) {}

  bool runOnModule(llvm::Module &M) {
    std::cout << __FUNCTION__ << "\n";
    auto const & BB = M;
    const llvm::DataLayout &DL = M.getDataLayout();
    for (auto II = BB.begin(), II_e = BB.end(); II != II_e;
         ++II) {
      // Iterate over each instruction in the BasicBlock. If the instruction
      // is an alloca, dump its type and query the type's size.
      if (llvm::AllocaInst const *Alloca = dyn_cast<llvm::AllocaInst>(II)) {
        llvm::Type *AllocType = Alloca->getAllocatedType();
        AllocType->print(llvm::outs());
        llvm::outs() << " size " << DL.getTypeSizeInBits(AllocType) << " bits\n";
      }
    }

    // Return false to signal that the basic block was not modified by this
    // pass.
    return false;
  }

  Pass *createPrinterPass(llvm::raw_ostream &OS,
    const std::string &Banner) const override {
      assert(false  &&  __FUNCTION__);
    };


  // The address of this member is used to uniquely identify the class. This is
  // used by LLVM's own RTTI mechanism.
  static char ID;
};

char AllocaSizeDetect::ID = 0;

int main(int argc, char **argv) {
  if (argc < 2) {
    llvm::errs() << "Usage: " << argv[0] << " <IR file>\n";
    return 1;
  }

  try {
    // Parse the input LLVM IR file into a module.
    llvm::SMDiagnostic Err;
    llvm::LLVMContext Context;
    auto Mod{ parseIRFile(argv[1], Err, Context) };

    // Create a pass manager and fill it with the passes we want to run.
    llvm::legacy::PassManager PM;
    auto  asd{ std::make_unique<AllocaSizeDetect>() };
    auto  p_asd{ asd.get() };
    PM.add(p_asd);
    PM.run(*Mod);
    std::cout << "done\n";
  } catch (...) {
    std::cerr << "exception\n";
    return 1;
  }

  return 0;
}
