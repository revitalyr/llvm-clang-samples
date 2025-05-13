//------------------------------------------------------------------------------
// This is currently ad-hoc
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <ranges>
#include <format>

// using namespace llvm;

namespace rv = std::ranges::views;

class Printer 
{
  public:
      using difference_type = int;
      using iterator_category = std::output_iterator_tag;
  
  private:
      llvm::raw_ostream * m_os;
  
  public:
      Printer(llvm::raw_ostream * os) : m_os{ os } {}
  
      Printer& operator*() { return *this; }
      Printer& operator++() { return *this; }
      Printer& operator++(difference_type) { return *this; }
  
      template <class T>
      Printer& operator=(const T& x) {
          *m_os << x;
          return *this;
      }
  };
  
int main(int argc, char **argv) {
  Printer printer{ &llvm::outs() };

  if (argc == 2) {
    // Parse the input LLVM IR file into a module.
    llvm::SMDiagnostic Err;
    llvm::LLVMContext Context;

    if (auto Mod{llvm::parseIRFile(argv[1], Err, Context)}) {
      // Go over all named mdnodes in the module
      for (const auto &I : llvm::make_range(Mod->named_metadata_begin(),
                                            Mod->named_metadata_end())) {
        llvm::outs() << "Found MDNode:\n";
        // These dumps only work with LLVM built with a special cmake flag
        // enabling dumps. I->dump();

        for (auto i : rv::iota(0u, I.getNumOperands())) {
          const auto *Op = I.getOperand(i);

          if (const auto *N = dyn_cast<llvm::MDNode>(Op)) {
            std::format_to(printer, "  Has MDNode operand:\n    {} operands\n", N->getNumOperands());
          }
        }
      }

      return 0;
    } else {
      Err.print(argv[0], llvm::errs());
      return 1;
    }
  }

  std::format_to(printer, "Usage: {} <IR file>\n", argv[0]);
  return 1;
}
