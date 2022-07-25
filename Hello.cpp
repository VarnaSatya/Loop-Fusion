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
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
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

      std::vector<Loop*> lv;     

      for(Loop *L: LI)
      {
        lv.push_back(L);
      }

      int size=lv.size()-1;
      BasicBlock *h1=lv[size]->getHeader();
      BasicBlock *l1=lv[size]->getLoopLatch();
      BasicBlock *b1=lv[size]->getBlocksVector()[1]; //check for function
      BasicBlock *e1=lv[size]->getExitBlock();

      //errs()<<size<<"\n";

      for(int j=size-1;j>=0;j--)
      {
        //errs()<<"j "<<j<<"\n";
        BasicBlock *e2=lv[j]->getExitBlock();
        BasicBlock *h2=lv[j]->getHeader();
        BasicBlock *l2=lv[j]->getLoopLatch();
        BasicBlock *b2=lv[j]->getBlocksVector()[1]; //check for function

        //errs()<<*l2<<"\n";

        std::vector<const SCEV*> sti;      
        bool flag=true;

        //bool 	containsAddRecurrence (const SCEV *S)

        for(Instruction &I: *b1)
        {
          Value *instruction= &I;
          if (auto *AI = dyn_cast<StoreInst>(instruction)) 
          { 
            auto *s=SE.getSCEV(AI->getPointerOperand());
            auto *ar = dyn_cast<SCEVAddRecExpr>(s);
            sti.push_back(s);
          }
        }

        for(Instruction &I: *b2)
        {
          Value *instruction= &I;        
          if (auto *AI = dyn_cast<LoadInst>(instruction)) 
          { 
            auto *s=SE.getSCEV(AI->getPointerOperand());
            auto *ex = dyn_cast<SCEVAddRecExpr>(s);
            //errs()<<*s<<"\n";
            
            for(auto i: sti)
            {
              auto *a=dyn_cast<SCEVAddRecExpr>(i);
              if(true)
              {
                SmallVector<const SCEV *, 2> Operands;
                Operands.push_back(ex->getOperand(0));
                Operands.push_back(ex->getOperand(1));
                auto *changed=SE.getAddRecExpr(Operands, lv[size], ex->getNoWrapFlags());
                if(SE.getCouldNotCompute()!=SE.getMinusSCEV(changed,a))
                {
                  if(SE.isKnownNegative(SE.getMinusSCEV(changed,a)))
                  {
                    flag=false; //cannot be fused
                    errs()<<"Cannot be fused\n";
                    return false;
                  }
                  else
                  {
                    errs()<<"Can be done\n";
                  }
                }
              }
            }
            
          }
        }
        int x= dyn_cast<ConstantInt>(h1->begin()->getOperand(1))->getZExtValue();
        int y= dyn_cast<ConstantInt>(h2->begin()->getOperand(1))->getZExtValue();

        auto *p=SE.getMinusSCEV(SE.getSymbolicMaxBackedgeTakenCount(lv[j]),SE.getSymbolicMaxBackedgeTakenCount(lv[size]));
      
        int a=dyn_cast<ConstantInt>(l1->begin()->getOperand(1))->getZExtValue();
        int b=dyn_cast<ConstantInt>(l2->begin()->getOperand(1))->getZExtValue();

        if(e1->getSingleSuccessor()!=NULL)
        {
          //e1->getSingleSuccessor()==h2
          if(x==y && p->isZero() && (e1->getSingleSuccessor()==h2) && a==b && l1->begin()->getOpcode()==l2->begin()->getOpcode() && flag)
          {
            lv[j]->getCanonicalInductionVariable()->replaceAllUsesWith(lv[size]->getCanonicalInductionVariable());
            l1->moveAfter(lv[j]->getBlocksVector()[1]);
            b1->getTerminator()->setSuccessor(0,lv[j]->getBlocksVector()[1]);
            lv[j]->getBlocksVector()[1]->getTerminator()->setSuccessor(0,l1);       
            h1->getTerminator()->setSuccessor(1,e2);
            e1=e2;
            b1=b2;
            EliminateUnreachableBlocks(F);
          }
          else
          {
            errs()<<"NO\n";
            errs()<<x<<"\t"<<y<<"\n";
            errs()<<(e1->getSingleSuccessor()==h2)<<"\n";
            errs()<<a<<"\t"<<b<<"\n";
            errs()<<(l1->begin()->getOpcode()==l2->begin()->getOpcode())<<"\n";
            errs()<<flag<<"\n";
            errs()<<"Cannot be fused\n";
            break;
          }
        }
        else
        {
          errs()<<"Cannot be fused\n";
          break;
        }
      }      
      F.dump();
      return false;    
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
