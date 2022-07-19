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
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/IR/InstrTypes.h"
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

      LLVMContext &context = M->getContext();
      IRBuilder<> IR(context);

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

      BasicBlock *h1=lv[1]->getHeader();
      BasicBlock *l1=lv[1]->getLoopLatch();
      BasicBlock *b1=lv[1]->getBlocksVector()[1]; //check for function

      BasicBlock *e1=lv[1]->getExitBlock();
      BasicBlock *e2=lv[0]->getExitBlock();

      BasicBlock *h2=lv[0]->getHeader();
      BasicBlock *l2=lv[0]->getLoopLatch();
      BasicBlock *b2=lv[0]->getBlocksVector()[1]; //check for function

      int x= dyn_cast<ConstantInt>(h1->begin()->getOperand(1))->getZExtValue();
      int y= dyn_cast<ConstantInt>(h2->begin()->getOperand(1))->getZExtValue();

      auto *p=SE.getMinusSCEV(SE.getSymbolicMaxBackedgeTakenCount(lv[0]),SE.getSymbolicMaxBackedgeTakenCount(lv[1]));
    
      int a=dyn_cast<ConstantInt>(l1->begin()->getOperand(1))->getZExtValue();
      int b=dyn_cast<ConstantInt>(l2->begin()->getOperand(1))->getZExtValue();

      if(e1->getSingleSuccessor()!=NULL)
      {
        if(x==y && p->isZero() && (e1->getSingleSuccessor()==h2) && a==b && l1->begin()->getOpcode()==l2->begin()->getOpcode())
        {
          //errs()<<*lv[1]->getCanonicalInductionVariable()<<"\n";

          lv[0]->getCanonicalInductionVariable()->replaceAllUsesWith(lv[1]->getCanonicalInductionVariable());

          l1->moveAfter(lv[0]->getBlocksVector()[1]);

          b1->getTerminator()->setSuccessor(0,lv[0]->getBlocksVector()[1]);
          lv[0]->getBlocksVector()[1]->getTerminator()->setSuccessor(0,l1);


          

          //errs()<<*l<<"\n";

          h1->getTerminator()->setSuccessor(1,e2);

          EliminateUnreachableBlocks(F);

          F.dump();
        }

      


     

      return false;

    }
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override 
    {
      AU.setPreservesAll();      
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
    }

  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");
