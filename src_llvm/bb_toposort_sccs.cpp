//------------------------------------------------------------------------------
// bb_toposort_sccs LLVM sample. Demonstrates:
//
// * How to implement DFS & topological sort over the control-flow graph (CFG)
//   of a function.
// * How to use po_iterator for post-order iteration over basic blocks.
// * How to use scc_iterator for post-order iteration over strongly-connected
//   components in the graph of basic blocks.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <vector>
#include <ranges>

//using namespace llvm;
using llvm::outs;
using llvm::BasicBlock;

namespace rv = std::ranges::views;

// Runs a topological sort on the basic blocks of the given function. Uses
// the simple recursive DFS from "Introduction to algorithms", with 3-coloring
// of vertices. The coloring enables detecting cycles in the graph with a simple
// test.
class TopoSorter {
public:
  void runToposort(const llvm::Function &F) {
    
    outs() << "Topological sort of " << F.getName() << ":\n";
    // Initialize the color map by marking all the vertices white.
    for (auto &I : llvm::make_range(F.begin(), F.end())) {
      ColorMap[&I] = TopoSorter::WHITE;
    }

    // The BB graph has a single entry vertex from which the other BBs should
    // be discoverable - the function entry block.
    if (recursiveDFSToposort(&F.getEntryBlock())) {
      // Now we have all the BBs inside SortedBBs in reverse topological order.
      for (auto RI : llvm::make_range(SortedBBs.rbegin(), SortedBBs.rend())) {
        outs() << "  " << RI->getName() << "\n";
      }
    } else {
      outs() << "  Sorting failed\n";
    }
  }

private:
  enum Color { WHITE, GREY, BLACK };
  // Color marks per vertex (BB).
  using BBColorMap = llvm::DenseMap<const BasicBlock *, Color>;
  // Collects vertices (BBs) in "finish" order. The first finished vertex is
  // first, and so on.
  using BBVector = llvm::SmallVector<const BasicBlock *, 32>;

  BBColorMap ColorMap;
  BBVector SortedBBs;

  // Helper function to recursively run topological sort from a given BB.
  // Returns true if the sort succeeded and false otherwise; topological sort
  // may fail if, for example, the graph is not a DAG (detected a cycle).
  bool recursiveDFSToposort(const BasicBlock *BB) {
    ColorMap[BB] = TopoSorter::GREY;
    // For demonstration, using the lowest-level APIs here. A BB's successors
    // are determined by looking at its terminator instruction.
    const auto *TInst = BB->getTerminator();
    for (unsigned I: rv::iota(0u, TInst->getNumSuccessors())) {
      const auto *Succ = TInst->getSuccessor(I);
      auto SuccColor = ColorMap[Succ];
      if (SuccColor == TopoSorter::WHITE) {
        if (!recursiveDFSToposort(Succ))
          return false;
      } else if (SuccColor == TopoSorter::GREY) {
        // This detects a cycle because grey vertices are all ancestors of the
        // currently explored vertex (in other words, they're "on the stack").
        auto const *bb_name{BB->getName().data()};
        auto const *succ_name{Succ->getName().data()};
        outs() << "  Detected cycle: edge from " << bb_name << " to "
               << succ_name << "\n";
        return false;
      }
    }
    // This BB is finished (fully explored), so we can add it to the vector.
    ColorMap[BB] = TopoSorter::BLACK;
    SortedBBs.push_back(BB);
    return true;
  }
};

class AnalyzeBBGraph : public llvm::FunctionPass {
public:
  AnalyzeBBGraph(const std::string &AnalysisKind)
      : FunctionPass(ID), AnalysisKind(AnalysisKind) {}

  virtual bool runOnFunction(llvm::Function &F) {
    if (AnalysisKind == "-topo") {
      TopoSorter TS;
      TS.runToposort(F);
    } else if (AnalysisKind == "-po") {
      // Use LLVM's post-order iterator to produce a reverse topological sort.
      // Note that this doesn't detect cycles so if the graph is not a DAG, the
      // result is not a true topological sort.
      outs() << "Basic blocks of " << F.getName() << " in post-order:\n";
      for (auto I: llvm::make_range(po_begin(&F.getEntryBlock()), po_end(&F.getEntryBlock()))) {
        outs() << "  " << I->getName() << "\n";
      }
    } else if (AnalysisKind == "-scc") {
      // Use LLVM's Strongly Connected Components (SCCs) iterator to produce
      // a reverse topological sort of SCCs.
      outs() << "SCCs for " << F.getName() << " in post-order:\n";
      for (const auto I: llvm::make_range(scc_begin(&F), scc_end(&F))) {
        // Obtain the vector of BBs in this SCC and print it out.
        const auto &SCCBBs = I;
        outs() << "  SCC: ";
        for (const auto BBI: llvm::make_range(SCCBBs.begin(), SCCBBs.end())) {
          outs() << BBI->getName() << "  ";
        }
        outs() << "\n";
      }
    } else {
      outs() << "Unknown analysis kind: " << AnalysisKind << "\n";
    }
    return false;
  }

  // The address of this member is used to uniquely identify the class. This is
  // used by LLVM's own RTTI mechanism.
  static char ID;

private:
  std::string AnalysisKind;
};

char AnalyzeBBGraph::ID = 0;

int main(int argc, char **argv) {
  if (argc < 3) {
    // Using very basic command-line argument parsing here...
    llvm::errs() << "Usage: " << argv[0] << " -[topo|po|scc] <IR file>\n";
    return 1;
  }

  // Parse the input LLVM IR file into a module.
  llvm::SMDiagnostic Err;
  llvm::LLVMContext Context;
  std::unique_ptr<llvm::Module> Mod(parseIRFile(argv[2], Err, Context));
  if (!Mod) {
    Err.print(argv[0], llvm::errs());
    return 1;
  }

  // Create a pass manager and fill it with the passes we want to run.
  llvm::legacy::PassManager PM;
  PM.add(new AnalyzeBBGraph(std::string(argv[1])));
  PM.run(*Mod);

  return 0;
}
