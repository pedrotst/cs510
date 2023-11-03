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
    if(std::count(path.begin(), path.end(), bb) > 2)
        return;
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

void print_malloc_structs(std::map<std::string, std::map<int, int>> malloc_structs){
    outs() << "----------------- Malloc structs ------------------------ \n";
    for (auto vars : malloc_structs){
        outs() << vars.first << " {\n";
        for (auto n : vars.second){
            outs() << "\t" << n.first << " - " << n.second << ";\n";
        }
        outs() << "}\n";

    }
    outs() << "--------------------------------------------------------- \n\n";
}

std::string gep_uid(std::string s, int i)
{
    return s + "." + std::to_string(i);
}

bool MemoryLeakDetection(std::list<BasicBlock *> path)
{
    int malloc_id = 0;
    // List of all mallocs
    std::vector<Instruction*> mallocs;
    // Variable -> index where that variable was mallocd
    std::map<std::string, int> malloc_vars;
    // Map structvaname to the index of the struct to the index of the malloc
    std::map<std::string, std::map<int, int>> malloc_structs;
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

                        auto search = malloc_vars.find(var);
                        if(search != malloc_vars.end()){
                            malloc_vars[alias] = search->second;
                        }
                    }
                }
            }
            

            StoreInst *SI = dyn_cast<StoreInst>(&i);
            if(SI){ 
                /*
                ConstantExpr *CE = dyn_cast<ConstantExpr>(SI->getOperand(1));
                if(CE){
                    GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(CE->getAsInstruction(&i));
                    if(GEPI){
                        if(DEBUG){
                            // outs() << *GEPI << "\n";
                        }
                    }
                    // Example of accessing to getOperand()
                }
                */
            }
             
            CallInst * CI = dyn_cast<CallInst>(&i);
            if (CI){ 
                if (CI->getCalledFunction()->getName().find("llvm.memcpy.p0i8.p0i8.i64") != std::string::npos) {
                    ConstantExpr *lhs = dyn_cast<ConstantExpr>(CI->getOperand(0));
                    ConstantExpr *rhs = dyn_cast<ConstantExpr>(CI->getOperand(1));
                    if(lhs && rhs && lhs->getOpcode() == Instruction::BitCast && rhs->getOpcode() == Instruction::BitCast){
                        std::string lhsOp = lhs->getOperand(0)->getName().str();
                        std::string rhsOp = rhs->getOperand(0)->getName().str();
                        malloc_structs[lhsOp] = malloc_structs[rhsOp];
                        if(DEBUG){
                            outs() << "memcpy: "<< *CI << "\n";
                            outs() << "lhs: "<< lhsOp << "\n";
                            outs() << "rhs: "<< rhsOp << "\n";
                        }
                    }
                    
                }
                if (CI->getCalledFunction()->getName().find("malloc") != std::string::npos) {
                    StoreInst *nextStore = dyn_cast<StoreInst>((&i)->getNextNonDebugInstruction());

                    if(nextStore){
                        ConstantExpr *CE = dyn_cast<ConstantExpr>(nextStore->getOperand(1));
                        if(CE){
                            GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(CE->getAsInstruction(&i));
                            if(GEPI){
                                std::string varname = GEPI->getOperand(0)->getName().str();
                                auto index = GEPI->getOperand(2);
                                if (ConstantInt *I = dyn_cast<ConstantInt>(index))
                                {
                                    int indexValue = I->getZExtValue();
                                    // auto key = gep_uid(varname, indexValue);
                                    auto m = malloc_structs[varname];
                                    m.insert({indexValue, malloc_id});
                                    malloc_structs[varname] = m;
                                    mallocs.push_back(&i);
                                    malloc_id++;
                                    // ...
                                    if (DEBUG)
                                    {
                                        outs() << *GEPI << "\n";
                                        // outs() << "GEP operand we want: " << indexValue << "\n";
                                        // outs() << "GEP variable we want: " << varname << "\n";
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::string var = nextStore->getOperand(1)->getName().str();
                            mallocs.push_back(&i);
                            malloc_vars.insert({var, malloc_id});
                            malloc_id++;

                            // Write your code
                            if (DEBUG)
                            {
                                // outs() << i << "\n";
                                outs() << *nextStore << "\n";
                                // outs() << var << "\n";
                            }
                        }
                    }

                }
                if (CI->getCalledFunction()->getName().find("free") != std::string::npos) {
                    LoadInst *previousLI = dyn_cast<LoadInst>((&i)->getPrevNonDebugInstruction());

                    if(previousLI){
                        std::string var = previousLI->getOperand(0)->getName().str();
                        auto search = malloc_vars.find(var);
                        if(search != malloc_vars.end()){
                            int id = search->second;
                            mallocs[id]= NULL;
                        }

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

    if(DEBUG){
        print_malloc_structs(malloc_structs);
    }
    /*
     * return True of False 
     */
    for(auto i : mallocs){
        if(i != NULL)
            return true;
    }

    return false;
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
