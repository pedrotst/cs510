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

void PrintPath(std::list<BasicBlock *> path){
    int lineno = 0;
    int prev_line = 0;
    for (auto bb: path) {
        for (Instruction& i : *bb) {
            if ((&i)->getDebugLoc()) {
                int lineno = (&i)->getDebugLoc().getLine();
                if (prev_line != lineno){
                    outs() << lineno;
                    // Check if this is the last instruction
                    // if(!(&i)->isTerminator())
                    outs() << " -> ";
                    prev_line = lineno;
                }
            }
        }
    }
    outs() << "\n";
}

void DiscoverPaths(std::list<BasicBlock *> path, BasicBlock *bb)
{
    path.push_back(bb);
    // outs() << "entrou Func\n";
    for (Instruction &i : (*bb))
    {
        BranchInst *BI = dyn_cast<BranchInst>(&i);
        if (BI)
        {
            for (int j = 0 ; j < BI->getNumSuccessors(); j++)
            {
                // outs() << "entrou For" << j << "\n";
                BasicBlock * target = BI->getSuccessor(j);
                DiscoverPaths(path, target);
            }
            return;
        }
    }
    // outs() << "saiu Func\n";
    paths.push_back(path);
    return;
}

void print_aliases(std::map<std::string, std::string> aliases){
    for (auto alias : aliases){
        outs() << alias.first << " = " << alias.second << ", ";

    }
    outs() << "\n";
}

bool MemoryLeakDetection(std::list<BasicBlock *> path)
{
    std::map<std::string, Instruction*> malloc_vars;
    malloc_vars.clear();
    std::map<std::string, std::string> aliases;
    for (auto bb: path) {
        for (Instruction& i : *bb) {

            LoadInst *LI = dyn_cast<LoadInst>(&i);
            if(LI){
                // write your code
                Instruction *inst = (&i)->getNextNonDebugInstruction();
                if(inst){
                    StoreInst *nextStore = dyn_cast<StoreInst>(inst);

                    if(nextStore){
                        std::string alias = nextStore->getOperand(1)->getName().str();
                        std::string var = LI->getOperand(0)->getName().str();
                        aliases[alias] = var;
                    }
                }
            }
            

            StoreInst *SI = dyn_cast<StoreInst>(&i);
            if(SI){ 
                
                // Write your code 

                // Example of accessing to getOperand()
                if(DEBUG){
                    //outs() << "[DEBUG][STORE] "  << i << "\n";
                    //outs() << "[DEBUG][STORE] "  << SI->getOperand(1) << "\n";
                }
            }
             
            CallInst * CI = dyn_cast<CallInst>(&i);
            if (CI){ 
                if (CI->getCalledFunction()->getName().find("malloc") != std::string::npos) {
                    StoreInst *nextStore = dyn_cast<StoreInst>((&i)->getNextNonDebugInstruction());

                    if(nextStore){
                        std::string var = nextStore->getOperand(1)->getName().str();
                        malloc_vars.insert({var, &i});

                        // Write your code
                        if(DEBUG){
                            // outs() << i << "\n";
                            // outs() << *nextStore << "\n";
                            // outs() << var << "\n";
                        }
                    }

                }
                if (CI->getCalledFunction()->getName().find("free") != std::string::npos) {
                    LoadInst *previousLI = dyn_cast<LoadInst>((&i)->getPrevNonDebugInstruction());

                    if(previousLI){
                        std::string var = previousLI->getOperand(0)->getName().str();

                        // erase var, or it's alias
                        // print_aliases(aliases);
                        do{
                            // outs() << var << "\n";
                            int n = malloc_vars.erase(var);
                            // didn't erase var
                            if(n == 0) {
                                if (aliases.count(var) < 1)
                                    break;
                                var = aliases.at(var);
                            } else break;
                        } while(1);

                        if(DEBUG){
                            // outs() << *previousLI << "\n";
                            // outs() << var << "\n";
                            // outs() << i << "\n";
                        }
                    }
                    // Write your code
                    
                }           
            }
         
        }
    }
    /*
     * return True of False 
     */
    if(malloc_vars.empty())
        return false;

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

        for (auto p: paths) { 
            // PrintPath(p);
            bool b = MemoryLeakDetection(p);
            if (b)
                PrintPath(p);
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
