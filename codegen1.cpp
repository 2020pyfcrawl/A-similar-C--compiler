/* PYF writes all */
/* finally finished on May 20th 16:50 */
/* reference: the official tutorial Kaleidoscope
   https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html */

#include"codegen1.hpp"
#include "util.hpp"

using namespace std;
using namespace llvm;

/* sth necesssary in llvm */
std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
/* local variables and memory space records */
std::map<std::string, AllocaInst *> NameIntValues;  
/* array name and size records */
std::vector<std::pair<std::string,int>> NameIntAryValues; 
/* pass manager, used for optimization */
std::unique_ptr<legacy::FunctionPassManager> TheFPM;    


/* initialize the basic component */
void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("666 compiling principal PYF YHR", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
}


/* global variable definition */
void Define_Initialize(string type,string name,int size){
  //Type * glo_type;
  if(type == "int"){
    /* insert a i16 with the name */
    TheModule->getOrInsertGlobal(name, Type::getInt16Ty(*TheContext));
    /* get the address of this global variable */
    GlobalVariable *gVar = TheModule->getNamedGlobal(name);
    /* set property */
    gVar->setAlignment(Align(2));
    gVar->setInitializer(Builder->getInt16(0));
    // cout << "define :  " + name + "\n";
  }

  else if(type == "int_array"){
    //store array as variable for simpilication
    for(int i = 0; i < size ; i++){
      string ele_name = name + '[';
      ele_name += to_string(i);
      ele_name += ']';
      TheModule->getOrInsertGlobal(ele_name, Type::getInt16Ty(*TheContext));
      GlobalVariable *gVar = TheModule->getNamedGlobal(ele_name);
      gVar->setAlignment(Align(2));
      gVar->setInitializer(Builder->getInt16(0));
    //   cout << "define :  " + ele_name + "\n";
    }    
  }
}


/* get the global value of the variable, return the address of global variable */
GlobalVariable * GetValue(string name,int index){
  if(index < 0){
    return TheModule->getGlobalVariable(name);
  }
  else{
    string ele_name = name + "[";
    ele_name += index;
    ele_name += "]";
    return TheModule->getGlobalVariable(ele_name);
  }
}


/* print error */
std::unique_ptr<ExprAST> LogError1(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
Value *LogErrorV1(const char *Str) {
  LogError1(Str);
  return nullptr;
}


/* ---------------------------------------------------*/
/* code generation part */

/* 1 */
/* number i16 generation */
Value *NumberExprAST::codegen() {
  return ConstantInt::get(*TheContext, APInt(16,Val));
}


/* 2 */
/* variable generation */
Value *VariableExprAST::codegen(){
//   cout << "-----------------------\n";
//   cout << "get name:  " + Name + "\n";
  /* get local variabls' address */
  AllocaInst *A = NameIntValues[Name];
    if (!A){
      /* no local variable can be get, get global variable */
      GlobalVariable *B = GetValue(Name);
      if (!B)
      /* no variable */
      return LogErrorV1("Unknown variable name");
      else 
      /* load the value from the address and return */
      /* this creates a statement in IR */
      return Builder->CreateLoad(B->getValueType(),B, Name);
    }
  return Builder->CreateLoad(A->getAllocatedType(), A, Name.c_str());
}


/* 3 */
/* array construction */
/* array is stored as each element a new name "a[i]", so it is actually 
   some variables with similar struct, not the real array in C,
   this is a kind of simplication, making the compiler not so powerful as well */
ArrayExprAST::ArrayExprAST(int NUM, string Name) : length(NUM),Name(Name){
  NameIntAryValues.push_back(std::pair<std::string,int>(Name,length));
  for (int i = 0 ; i < length ; i++){
    string tmp_name =  (Name + "[" );
    tmp_name += to_string(i);
    tmp_name.append("]");
    std::unique_ptr<VariableExprAST> tmp(new VariableExprAST(std::move(tmp_name)));
    arrays.push_back(std::move(tmp));
  }
}

/* array generation */
Value *ArrayExprAST::codegen() {   
    for( int i = 0 ; i < length; i++){
      Value * eachVariable = arrays[i]->codegen();
      if(!eachVariable){
        string error = "this element of array:" + Name + "does not exist.";
        return LogErrorV1(error.c_str()); 
        break;
      }
    }
    /* return 0 */
    return Constant::getNullValue(Type::getInt16Ty(*TheContext));
}


/* 4 */
/* Binary expression codegen */
Value *BinaryExprAST::codegen() {
  if (Op == ASSIGN) {
    /* Assignment requires the LHS to be an identifier.
       This assume we're building without RTTI because LLVM builds that way by
       default.  If you build LLVM with RTTI this can be changed to a
       dynamic_cast for automatic error checking. */
    VariableExprAST *LHSE = static_cast<VariableExprAST *>(LHS.get());
    if (!LHSE)
      return LogErrorV1("destination of '=' must be a variable");
    /* Codegen the RHS. */
    Value *Val = RHS->codegen();
    if (!Val)
      return nullptr;

    /* Look up the name. */
    // cout << "-----------------------\n";
    // cout << "get name:  " + LHSE->getName() + "\n";
    Value *Variable = NameIntValues[LHSE->getName()];
    if (!Variable){
      Variable = GetValue(LHSE->getName());
      if (!Variable)
      return LogErrorV1("Unknown variable name");
    }
    /* type casting, the RHS's result may be i1, casting it to i16,
       mention that signed i1 only has 0 and -1, so we need set false as 
       unsigned i1 can be 0 or 1 */
    Val = Builder->CreateIntCast(Val, Type::getInt16Ty(*TheContext), false);

    /* this creates a statement in IR, storing the value to the address */
    Builder->CreateStore(Val, Variable);
    return Val;
  }
  
  /* if not ASSIGN, both sides are expression */
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;
  /* create statement according to Op, its value it enum in "util.hpp" */
  switch (Op) {
  case PLUS:
    return Builder->CreateAdd(L, R, "addtmp");
  case MINUS:
    return Builder->CreateSub(L, R, "subtmp");
  case TIMES:
    return Builder->CreateMul(L, R, "multmp");
  case OVER:
    return Builder->CreateSDiv(L, R, "multmp");
  case LT:
    L = Builder->CreateICmpSLT(L, R, "lttmp");break;
  case GT:
    L = Builder->CreateICmpSGT(L, R, "gttmp");break;
  case EQ:
    L = Builder->CreateICmpEQ(L, R, "eqtmp");break;
  case NEQ:
    L = Builder->CreateICmpNE(L, R, "neqtmp");break;
  case GEQ:
    L = Builder->CreateICmpSGE(L, R, "geqtmp");break;
  case LEQ:
    L = Builder->CreateICmpSLE(L, R, "leqtmp");break;
    
  default:
    return LogErrorV1("invalid binary operator");
  }
  return L;
}


/* 5 */
/* statements codegen */
/* statements are composed of some statements, according to the non-terminal "statements"
   in CFG, the statements may be selection, loop, return, call or simple statement */
Value *StatementsAST::codegen() {
    
    for( int i = 0 ; i < statements.size(); i++){
      Value * eachVariable = statements[i]->codegen();
      if(!eachVariable){
        return LogErrorV1("this statement is wrong."); 
        break;
      }
    }
    //return 0
    return Constant::getNullValue(Type::getInt16Ty(*TheContext));
}


/* 6 */
/* selection-statement codegen */
Value *IfExprAST::codegen() {
  Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  /* Convert condition to a bool by comparing non-equal to 0 */
  CondV = Builder->CreateICmpNE(
      CondV, ConstantInt::get(*TheContext, APInt(1,0)), "ifcond");//?

  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  /* Create blocks for the then and else cases.  Insert the 'then' block at the
     end of the function now to generate code in this block. */
  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  /* conditional jump */
  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  /* then generate code in Then block */
  Builder->SetInsertPoint(ThenBB);

  Value *ThenV = Then->codegen();
  if (!ThenV)
    return nullptr;
  
  /* unconditional jump to merge block */
  Builder->CreateBr(MergeBB);

  /* Codegen of 'Then' can change the current block, update ThenBB for the PHI. 
     needed if there is a PHI */
  ThenBB = Builder->GetInsertBlock();

  /* push Else block in the blocklist of the funciton */
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);

  Value *ElseV = Else->codegen();
  if (!ElseV)
    return nullptr;

  Builder->CreateBr(MergeBB);
  
  /* Codegen of 'Else' can change the current block, update ElseBB for the PHI. */
  ElseBB = Builder->GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);
  /* the block should not be empty so we will insert sth in another statement codegen */
  return Constant::getNullValue(Type::getInt16Ty(*TheContext));
}


/* 7 */
/* loop-statement codegen */
Value *WhileExprAST::codegen(){
  Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  /* Convert condition to a bool by comparing non-equal to 0 */
  CondV = Builder->CreateICmpNE(
      CondV, ConstantInt::get(*TheContext, APInt(1,0)), "ifcond");

  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  /* Create blocks for the then case.  Insert the 'then' block at the
     end of the function. */
  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, MergeBB);

  Builder->SetInsertPoint(ThenBB);

  Value *ThenV = Then->codegen();
  if (!ThenV)
    return nullptr;
  /* check the condition again in Then block */
  CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  CondV = Builder->CreateICmpNE(
      CondV, ConstantInt::get(*TheContext, APInt(1,0)), "ifcond");

  Builder->CreateCondBr(CondV, ThenBB, MergeBB);
  
  ThenBB = Builder->GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);

  return Constant::getNullValue(Type::getInt16Ty(*TheContext));
}


/* 8 */
/* return-statement codegen */
Value *ReturnExprAST::codegen(){
    Value * RetVal = Ret->codegen();
    /* no return, return 0 to prevent error */
    if(!RetVal){
      RetVal = Constant::getNullValue(Type::getInt16Ty(*TheContext));
    }
    return Builder->CreateRet(RetVal);
}


/* 9 */
/* call-statement codegen */
Value *CallExprAST::codegen() {
  /* Look up the name in the global module table. */
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV1("Unknown function referenced");

  /* If argument mismatch error. */
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV1("Incorrect # arguments passed");
  
  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    /* push the argument value in */
    /* do not allow arrays */
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}


/* 10 */
/* Prototype codegen */
Function *PrototypeAST::codegen() {
  
  /* print some information for this is not easy to check in lldb */
//   cout << "-------------------------------------------\n";
//   cout << "proto info\n";
//   cout << "rettype: " + RetType + "\n";
//   cout << "Name:    " + Name + "\n";
//   cout << "size:    " ;
//   cout << Args.size() ;
//   cout << "\n";
//   for(int i = 0;i<Args.size();i++){
//     cout << "Arg" ;
//     cout << i ;
//     cout << ":   ";
//     cout << Args[i] + "\n";
//   }
//   for(int i = 0;i<Types.size();i++){
//     cout << "Types" ;
//     cout << i ;
//     cout << ":   ";
//     cout << Types[i] + "\n";
//   }
//   cout << "------------------done-------------------------\n";
  
  /* the vector for Type * of the function */
  std::vector<Type *> Arguments;
  for(vector<std::string>::iterator it = Types.begin(); it != Types.end(); it++){
    if(*it == "int"){
      Arguments.push_back(Type::getInt16Ty(*TheContext));
    }
    else if(*it == "int_array"){
      Arguments.push_back(Type::getInt16PtrTy(*TheContext));
      /* not set the size, do not know if there will be errors */
      /* only use pointer of i16 iin this place, do not allow array as arguments of functions */
    }
  }
  
  /* generate functiontype with return value set i16 for simpilication, the senmatics checks
     is down before */
  /* return value, arguments type, the number of arguments can not be changed ---false */
  FunctionType *FT =
      FunctionType::get(Type::getInt16Ty(*TheContext), Arguments, false);
  
  /*generate function in the Module with the name and type and externallinkage */
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

  /* Set names for all arguments. */
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}


/* 11 */
/* Var and statements codegen */
/* this part is the body of function, beginning with local definitions and 
   then the statements */
Value *VarExprAST::codegen() {
  /* record the old values before this codegen, just like storing value in the stack */
  std::vector<AllocaInst *> OldBindings;

  /* get the function* */
  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  /* Register all variables and emit their initializer. */
  for (unsigned i = 0, e = IntNames.size(); i != e; ++i) {
    
    const std::string &VarName = IntNames[i];
    // std::cout << "var name " + VarName + " initialization:" ;
    /*initialize to be 0 */
    Value *InitVal;
    InitVal = ConstantInt::get(*TheContext, APInt(16,0));  
    
    /* allocate the space for the new local variable */ 
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    Builder->CreateStore(InitVal, Alloca);

    /* Remember the old variable binding so that we can restore the binding when
       we call anothe function. */
    // cout << "-----------------------\n";
    // cout << "store old name:  " + VarName + "\n";
    OldBindings.push_back(NameIntValues[VarName]);

    /* Remember the new binding. */
    // cout << "-----------------------\n";
    // cout << "store new name:  " + VarName + "\n";
    NameIntValues[VarName] = Alloca;
  }
  
  /* array there is the same to variables */
  for (unsigned i = 0, e = IntArrays.size(); i != e; ++i) {
    string ArrayName = IntArrays[i].first;
    int size = IntArrays[i].second;
    for(unsigned j = 0; j < size; ++j ){
      string Element_Name =  (ArrayName + "[" );
      Element_Name += to_string(i);
      Element_Name.append("]");
      /*initialize to be 0 */
      Value *InitVal;
      InitVal = ConstantInt::get(*TheContext, APInt(16,0));  

      AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Element_Name);
      Builder->CreateStore(InitVal, Alloca);

    //   cout << "-----------------------\n";
    //   cout << "store old name:  " + Element_Name + "\n";
      OldBindings.push_back(NameIntValues[Element_Name]);

    //   cout << "-----------------------\n";
    //   cout << "store new name:  " + Element_Name + "\n";
      NameIntValues[Element_Name] = Alloca;
    }
        
  }
  /* above is initialization */
//    cout << "var expr   ---------------------\n";

  /* Codegen the body, now that all vars are in scope after senmatics */
  Value *BodyVal = Body->codegen();
  if (!BodyVal)
    return nullptr;

  /* Pop all our variables from scope and restore the allocate space */
  for (unsigned i = 0, e = IntNames.size(); i != e; ++i)
    NameIntValues[IntNames[i]] = OldBindings[i];

  for (unsigned i = 0, e = IntArrays.size(); i != e; ++i) {
    string ArrayName = IntArrays[i].first;
    int size = IntArrays[i].second;
    for(unsigned j = 0; j < size; ++j ){
      string Element_Name =  (ArrayName + "[" );
      Element_Name += to_string(i);
      Element_Name.append("]");
      NameIntValues[Element_Name] = OldBindings[i];
    }
  }
  /* Return the body computation value, which is useless in fact */
  return BodyVal;
}


/* 12 */
/* function codegen */
Function *FunctionAST::codegen() {
  
  /* get the function* from the Module according to the name */
  Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  /* Create a new basic block to start the functions' insertion into. */
  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  std::vector<AllocaInst *> OldBindings;
  std::vector<StringRef> VarNames;
  int number = 0;
  /* allocate the space for the argument and put the value to them */
  for (auto &Arg : TheFunction->args()) {
    StringRef VarName = Arg.getName();
    // cout << "var:   " + std::string(VarName) + "\n";
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    Builder->CreateStore(&Arg, Alloca);

    // cout << "-----------------------\n";
    // cout << "store new name:  " + std::string(VarName) + "\n";
    OldBindings.push_back(NameIntValues[std::string(VarName)]);
    /* Remember this new binding. */
    NameIntValues[std::string(VarName)] = Alloca;
    VarNames.push_back(VarName);
    number ++;
  }

  if (Value *RetVal = Body->codegen()) {
    
    /* Finish off the function. */
    /* this create return is used for ensuring the blocks' safety */
    Builder->CreateRet(RetVal);

    /* Validate the generated code, checking for consistency. */
    verifyFunction(*TheFunction);

    return TheFunction;
  }
  
  /* restoring the value */
  for (unsigned i = 0; i < number; ++i)
    NameIntValues[std::string(VarNames[i])] = OldBindings[i];

  /* Error reading body, remove function from the Module. */
  TheFunction->eraseFromParent();
  return nullptr;
}


int main(){
    InitializeModule();
    char inputfile[200] = "input";    
	  /*	strcpy(inputfile,argv[1]); */

	  freopen(inputfile,"r",stdin);
	  if (stdin==NULL)
	  {
		  fprintf(stderr,"File %s Not Found\n",inputfile);
	  	exit(1);
	  }
    /* build the DFA of the token */
	  BuildDFA();              
	  /* initialization of the token parser */
    LexInit();               
	  /* initialization of the CFG parser */
    PraseInit();
    /* parse the originall ast */       
	  ParseSyntax();     
    // fprintf(stderr, "Begin ParseTree\n");
    
    /* convert the originall ast to ExprAST classes */
	  bool ret = ParseTree(root1);
    /* the input is "input" in this directory */
    if(ret == false){
      cout << "the program exits in advance due to " + to_string(errors) + " errors!\n";
      return 0;
    }
    /* print of all global information of this Moudle */
    TheModule->print(errs(), nullptr);
    return 0;
}


