#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/Operator.h"

#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <sstream>
#include <map>

/*
 * When you debug, please use this flag, 
 *  e.g., if (DEBUG) : outs << ... << '\n';
 * Turn this off when you submit
 */ 
bool DEBUG = true;
using namespace llvm;


std::list<std::list<BasicBlock *>> paths;
// or std::list<std::list<Instruction *>> paths;
void DiscoverPaths(std::list<BasicBlock *> path, BasicBlock *bb)
{
    path.push_back(bb);
    for (Instruction &i : (*bb))
    {
        std::list<BasicBlock *> path
            BranchInst *BI = dyn_cast<BranchInst>(&i);
        if (BI)
        {

            // write your code
            // You may need to do a recursive call
            // for (int j = 0 ; j < BI->getNumSuccessors(); j++)
            //{
            //   BasicBlock * target = BI->getSuccessor(j);
            //
            //    ...
            //}
        }
    }
    paths.push_back(path);
    return;
}

bool MemoryLeakDetection(std::list<BasicBlock *> path)
{

    for (auto bb: path) {
        for (Instruction& i : *bb) {

            LoadInst *LI = dyn_cast<LoadInst>(&i);
            if(LI){
                // write your code
            }
            

            StoreInst *SI = dyn_cast<StoreInst>(&i);
            if(SI){ 
                
                // Write your code 

                // Example of accessing to getOperand()
                if(DEBUG){
                    outs() << "[DEBUG][STORE] "  << i << "\n";
                    outs() << "[DEBUG][STORE] "  << SI->getOperand(1) << "\n";
                }
            }
             
            CallInst * CI = dyn_cast<CallInst>(&i);
            if (CI){ 
                if (CI->getCalledFunction()->getName().find("malloc") != std::string::npos) {
                    // Write your code
                    if(DEBUG){
                        outs() << "Found Malloc\n";
                    }

                }
                if (CI->getCalledFunction()->getName().find("free") != std::string::npos) {
                    // Write your code
                    
                }           
            }
         
        }
    }
    /*
     * return True of False 
     */
    return true;
}


namespace {
struct LeakDetector : public ModulePass {
  static char ID;
  LeakDetector() : ModulePass(ID) {}
  bool runOnModule(Module &M) override {
    for (Function& fun : M){

        // function in external library 
        if (fun.begin() == fun.end())
            continue;

        
        // Empty path and this will grow in DiscoverPaths()
        std::list<BasicBlock *> path;
        path.clear();

        // paths will be a list of list of BBs (or istructions?)    
        DiscoverPaths(path, &(fun.front()));
        // outs() << "Paths running\n";

        for (auto p: paths) 
        { 
            for (auto bb: p) {

                for (Instruction& i : *bb) {

                    if ((&i)->getDebugLoc()) {
                        int lineno = (&i)->getDebugLoc().getLine();
                        outs() << lineno << " -> ";
                    }
                }
            }
            outs() << "\n";
        }
    
    }
  return false; 
  } 
};
}


char LeakDetector::ID = 0;
static RegisterPass<LeakDetector> X("ldetector", "LeakDetector Pass", 
                             true,
                             true);


static void registerLeakDetectorPass(const PassManagerBuilder &, legacy::PassManagerBase &PM)
{
        PM.add(new LeakDetector());
}

static RegisterStandardPasses RegisterLeakDetectorPass(
    PassManagerBuilder::EP_ModuleOptimizerEarly, registerLeakDetectorPass);

static RegisterStandardPasses RegisterLeakDetectorPass0(
    PassManagerBuilder::EP_EnabledOnOptLevel0, registerLeakDetectorPass);
