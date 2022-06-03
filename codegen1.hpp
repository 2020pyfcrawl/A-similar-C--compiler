/* PYF writes all */
/* finally finished on May 20th 18:11 */
/* reference: the official tutorial Kaleidoscope
   https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html */

#ifndef CODEGEN_H
#define CODEGEN_H

/* include file */ 
#include "ParExp1.hpp"
#include "util.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
//
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Instructions.h"
#include <cassert>
#include <system_error>
#include <utility>
//
#include "llvm/Transforms/Utils.h"
//

/* namespace */ 
using namespace llvm;
using namespace std;

/* extern */
extern std::unique_ptr<LLVMContext> TheContext;
extern std::unique_ptr<Module> TheModule;
extern std::unique_ptr<IRBuilder<>> Builder;
extern std::map<std::string, AllocaInst *> NameIntValues;
extern std::vector<std::pair<std::string,int>> NameIntAryValues; 

/* initialize the basic component */
void InitializeModule();

/* create some space for the local variable in the function */
static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
                                          StringRef VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getInt16Ty(*TheContext), nullptr, VarName);
}

/* global variable definition */
void Define_Initialize(string type,string name,int size = -1);
/* get the global value of the variable, return the address of global variable */
GlobalVariable * GetValue(string name,int index = -1 );


/* ---------------------------------------------------*/
/* all class definition */
/* 0 */
/* Expression AST - Base class for all expression nodes. */
class ExprAST {
public:
  virtual ~ExprAST() = default;
  /* virtual function for code generation */
  virtual Value *codegen() = 0;
};

/* print error */
/* LogError* - These are little helper functions for error handling. */
std::unique_ptr<ExprAST> LogError1(const char *Str) ;
Value *LogErrorV1(const char *Str) ;

/* 1 */
/* NumberExprAST - Expression class for numeric literals like "1". */
class NumberExprAST : public ExprAST {
  int Val;

public:
  NumberExprAST(int Val) : Val(Val) {}
  string getType(){ return "int";} /* may not be used */
  Value *codegen() override;
};

/* 2 */
/* VariableExprAST - Expression class for referencing a variable, like "a". */
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
  Value *codegen() override;
  const std::string &getName() const { return Name; }
};

/* 3 */
/* ArrayExprAST 
   this can be initialized with all 0, by making a variableExprAST vector 
   with value 0 NUM times, element of the array is stored as NUM variables,
   so it is not the true array and the pointer, just an array of variables 
   with almost the same outlook "ArrayName[index]",  like "a[0]". */
class ArrayExprAST : public ExprAST {
  int length; 
  std::string Name;
  vector <std::unique_ptr<VariableExprAST>> arrays;
  /* values can be changed in the specific areas, no need to do something there
     only provid interfaces to get array values */

public:
  ArrayExprAST(int NUM, string Name);
  string getType(){ return "int_array";}
  Value *codegen() override;
  const std::string &getName() const { return Name; }
};

/* 4 */
/* BinaryExprAST - Expression class for a binary operator.
  String2Token["="]=ASSIGN;
	String2Token["<"]=LT;
	String2Token[">"]=GT;
	String2Token["=="]=EQ;
	String2Token["!="]=NEQ;
	String2Token[">="]=GEQ;
	String2Token["<="]=LEQ;

	String2Token["+"]=PLUS;
	String2Token["-"]=MINUS;
	String2Token["*"]=TIMES;
	String2Token["/"]=OVER;*/
class BinaryExprAST : public ExprAST {
  int Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(int op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
    : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
  Value *codegen() override;
};

/* 5 */
/* StatementsAST - corresponding to statement-lists */
class StatementsAST : public ExprAST {
  vector <std::unique_ptr<ExprAST>> statements;

public:
  StatementsAST(vector <std::unique_ptr<ExprAST>> statements1):statements(std::move(statements1)){}
  Value *codegen() override;
};

/* 6 */
/* IfExprAST - Expression class for if/then/else. */
class IfExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StatementsAST>  Then, Else;

public:
  IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StatementsAST> Then,
            std::unique_ptr<StatementsAST> Else)
    : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  Value *codegen() override;
};

/* 7 */
/* WhileExprAST--while is somehow similar to if else if condition 
   then compound-statement else exit may be reused */
class WhileExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StatementsAST> Then;

public:
  WhileExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StatementsAST> Then)
    : Cond(std::move(Cond)), Then(std::move(Then)) {}

  Value *codegen() override;
};

/* 8 */
/* ReturnExprAST */
class ReturnExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Ret;

public:
  ReturnExprAST(std::unique_ptr<ExprAST> Ret_expression) 
  : Ret(std::move(Ret_expression)) {}
  Value *codegen() override;
};

/* 9 */
/* CallExprAST - Expression class for function calls. */
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
    : Callee(Callee), Args(std::move(Args)) {}
  Value *codegen() override;
};

/* 10 */
/* PrototypeAST - This class represents the "prototype" for a function,
   which captures its name, and its argument types and names (thus implicitly 
   the number of arguments the function takes).*/
class PrototypeAST {
  std::string RetType;
  std::string Name;
  std::vector<std::string> Args;
  std::vector<std::string> Types;

public:
  PrototypeAST(const std::string rettype, const std::string &name, std::vector<std::string> Args,
  std::vector<std::string> Types)
    : RetType(rettype),Name(name), Args(std::move(Args)), Types(std::move(Types)) {}//to de modified to add arguments

  const std::string &getName() const { return Name; }
  Function *codegen();
};

/* 11 */
/*  functionbody
    local declaration and statements */
class VarExprAST : public ExprAST {
  std::vector<std::string> IntNames;
  std::vector<std::pair<std::string,int>> IntArrays;
  std::unique_ptr<StatementsAST> Body;

public:
  VarExprAST(
      std::vector<std::string> Names, std::vector<std::pair<std::string,int>> Arrays,
      std::unique_ptr<StatementsAST> Body)
      : IntNames(std::move(Names)), IntArrays(std::move(Arrays)), Body(std::move(Body)) {}

  Value *codegen() override;
};

/* 12 */
/* FunctionAST - This class represents a function definition itself. */
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<VarExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<VarExprAST> Body)
    : Proto(std::move(Proto)), Body(std::move(Body)) {}
  Function *codegen();
};
/* all class definition done */
/* ---------------------------------------------------*/


#endif