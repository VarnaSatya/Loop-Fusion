//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Type.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include <list>
#include<vector>
#include <map>
using namespace llvm;

#define DEBUG_TYPE "hello"

static LLVMContext Context;


STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass");

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct Hello2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {

      Module *M=F.getParent();
      ++HelloCounter;
      
      auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo(); 

      //errs()<<"\nLoop:  "<< LI. <<"\n";
      /*
      getExitBlock()
      getBlocksVector()
      BranchInst::Create(Truebb,Falsebb);
      BB.getTerminator()
      changeLoopFor(BB,L)
      */

     std::vector<Loop*> lv;     

     for(Loop *L: LI)
     {
        lv.push_back(L);
     }

     std::vector<BasicBlock*> b1=lv[0]->getBlocksVector();
     std::vector<BasicBlock*> b0=lv[1]->getBlocksVector();

     b0[0]->moveAfter(b1[0]);
     b1[0]->getTerminator()->removeFromParent();
     b1[0]->replacePhiUsesWith(lv[1]->getExitBlock(),b0[0]->getSinglePredecessor());

     LI.changeLoopFor(b1[0],lv[1]);

     b1[1]->moveAfter(b0[1]);
     b0[1]->getTerminator()->removeFromParent();
     b1[1]->getTerminator()->removeFromParent();
     b1[1]->replacePhiUsesWith(b1[0],b0[1]);

     LI.changeLoopFor(b1[1],lv[1]);

     Instruction *i=b1[2]->getTerminator();
     i->removeFromParent();
     b1[2]->moveBefore(b0[2]);
     b0[2]->replacePhiUsesWith(b0[1],b1[2]);

     LI.changeLoopFor(b1[2],lv[1]);

     BasicBlock *b=lv[1]->getExitBlock();
     b->getTerminator()->removeFromParent();
     BasicBlock *a=lv[0]->getExitBlock();
     a->moveAfter(b);

     a->replacePhiUsesWith(b1[0],b);

     LI.changeLoopFor(a,lv[1]);



     


     //F.dump();

    
      return false;

    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override 
    {
      AU.setPreservesAll();      
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
    }

    void changeSuccessor(BasicBlock *a,BasicBlock *b)
    {

    }

    /*BranchInst* getBranchInstruction(BasicBlock *BB)
    {
      BasicBlock::iterator i;
      for(i=BB->begin();i!=BB->end();i++)
      {
          errs()<<"\n"<<*i; 
          if(BranchInst *b=dyn_cast<BranchInst>(i))
          {          
            return b;
          }
      }
    }*/

    

  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");
