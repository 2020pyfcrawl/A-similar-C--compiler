/* interface by YHR */
/* semantic analysis by PYG*/
/* finally finished on May 22nd */
#include"ParExp1.hpp"
int flag = 1;
// extern TreeNode *root1;

std::vector<std::string> GlobalVar;
std::vector<std::string> LocalVar;
//int name size0
//int[] name size
// function type name  argements (type name) variable 
std::map<std::string,int> GlobalVariables;


std::map<std::string, FuncInfo> GlobalFunctions;
std::string FuncName = "";
int errors = 0;
bool ispoorCall = true;

bool CheckNameExist(string checkallname){
  if(GlobalVariables.size() == 0) {
    if(GlobalFunctions.size() == 0){
      return false;
    }
    else{
      return GlobalFunctions.find(checkallname) != GlobalFunctions.end();
    }
  }
  else if(GlobalFunctions.size() == 0){
    if(GlobalVariables.size() == 0){
      return false;
    }
    else{
      return GlobalVariables.find(checkallname) != GlobalVariables.end();
    }
  }
  // map<std::string,int>::iterator iter_variable = GlobalVariables.find(checkallname);
  // map<std::string, FuncInfo>::iterator iter_function = GlobalFunctions.find(checkallname);
  
  return GlobalVariables.find(checkallname) != GlobalVariables.end() | GlobalFunctions.find(checkallname) != GlobalFunctions.end();
}


bool CheckGlobalName(string checkname,int index /*= -1*/){
  cout << "size=" + to_string(GlobalVariables.size());cout << "\n";
  if(GlobalVariables.size() == 0) return true;
  map<std::string,int>::iterator iter=GlobalVariables.find(checkname);
  //cout << "size=" + GlobalVariables.size();
  
  if(iter != GlobalVariables.end()){
    cout << iter->first + "\n";
    if(index < -1){
      cout << "program error\n";
      return false;
    }
    /* found */
    else if( index == -1 ){
      if(iter->second == -1)
        return true;
      else {
        cout << checkname + string(" is an array, not int\n");
        errors ++;
        return false;
      }
    }
    else {
      if( index >= iter->second ){
        /* out of bound */
        cout << checkname + to_string(index) + string("out of bound\n");
        errors ++;
        return false;
      }
      else{
        return true;
      }      
    }
  }
  return true;
}

bool InsertGlobalName(string newname, int size /*= -1*/){
  bool ifexist;
  if(size == -1)
    ifexist = CheckGlobalName(newname,size);
  else
    ifexist = CheckGlobalName(newname,0);
  if(ifexist){
    cout << "insert size" + to_string(size) + "\n";
    GlobalVariables.insert(pair<string,int>(newname,size));
  }
  return ifexist;
}



//make a NumberExprAST
std::unique_ptr<ExprAST> ParseNumberExpr(TreeNode *t) {
  // fprintf(stderr, "ParsenUMBER: %d\n",atoi(t->token.val));
  auto Result = std::make_unique<NumberExprAST>(atoi(t->token.val));
  return std::move(Result);
}


/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
//make a VariableExprAST
std::unique_ptr<ExprAST> ParseIdentifierExpr(TreeNode *t) {
  //a normal variable
  std::string IdName = t->token.val;
  /* find if name exists */
  map<std::string, FuncInfo>::iterator iter_function = GlobalFunctions.find(FuncName); 
  if(iter_function->second.arguments.size() != 0 &&
    (iter_function->second.arguments.find(IdName) != iter_function->second.arguments.end())){
    if(iter_function->second.arguments.find(IdName)->second > -1){
      cout << IdName + " is an array, not a variable!\n";
      errors++;
    }
  }
  else if(iter_function->second.localvar.size() != 0 &&
         (iter_function->second.localvar.find(IdName) != iter_function->second.localvar.end())){
    if(iter_function->second.localvar.find(IdName)->second > -1){
      cout << IdName + " is an array, not a variable!\n";
      errors++;
    }
  }
  else{
    if(CheckGlobalName(IdName,-1) == false){
      cout << "this idName: \"" + IdName + "\" does not exists!\n";
      //errors ++;
    }
    
  }
  return std::make_unique<VariableExprAST>(IdName);
  

}
 

//make a StatementsAST
std::unique_ptr<StatementsAST> ParseStatementsExpr(TreeNode *t) {
  TreeNode * tmp = t;
  // many statements
  std::vector<std::unique_ptr<ExprAST>> States;
  while(tmp!=nullptr){
    if (auto aState = ParseExpression(tmp)){
      States.push_back(std::move(aState));
      tmp = tmp->brother;
    }
    else
      return std::make_unique<StatementsAST>(std::move(States));
    }
  return std::make_unique<StatementsAST>(std::move(States));
}

//make a IfExprAST
std::unique_ptr<ExprAST> ParseIfExpr(TreeNode *t) {
  // condition.
  //condition
  ispoorCall = false;
  auto Cond = ParseExpression(t->child[0]);
  ispoorCall = true;
  if (!Cond)
    return nullptr;
    //if condition is true
  auto Then = ParseStatementsExpr(t->child[1]);
  if (!Then)
    return nullptr;
    //if condition is false
  auto Else = ParseStatementsExpr(t->child[2]);
  if (!Else)
    return nullptr;
  return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),std::move(Else));
}


//Array declaration
std::unique_ptr<ExprAST> ParseArrayExpr(TreeNode *t) {
  std::string Name = t->child[1]->token.val;
  int length = atoi(t->child[2]->token.val);
  /* insert global value */
  InsertGlobalName(Name,length);
  Define_Initialize("int_array",Name,length);
  return nullptr;
}

//make a WhileExprAST
std::unique_ptr<ExprAST> ParseWhileExpr(TreeNode *t) {
  //condition
  ispoorCall = false;
  auto Cond = ParseExpression(t->child[0]);
  ispoorCall = true;
  if (!Cond)
    return nullptr;
    //if condition is true then
  auto Then = ParseStatementsExpr(t->child[1]);
  if (!Then)
    return nullptr;
  return std::make_unique<WhileExprAST>(std::move(Cond), std::move(Then));
}

//make a ReturnExprAST
std::unique_ptr<ExprAST> ParseReturnExpr(TreeNode *t) {
  if(t == nullptr | t->child[0] == nullptr)
    return std::make_unique<BinaryExprAST>(16,std::move(std::make_unique<NumberExprAST>(1)),std::move(std::make_unique<NumberExprAST>(1)));;

  //fprintf(stderr, "ParseReturn: \n");
  ispoorCall = false;
  auto Ret = ParseExpression(t->child[0]);
  ispoorCall = true;
  return std::make_unique<ReturnExprAST>(std::move(Ret));
}

//make a CallExprAST
std::unique_ptr<ExprAST> ParseCallExpr(TreeNode *t) {
   //callee name
  std::string IdName = t->child[0]->token.val;
  if(GlobalFunctions.size() == 0 &&
    (GlobalFunctions.find(IdName) != GlobalFunctions.end())){
    cout << "Function \"" + IdName +"\" does not existed!\n";
    errors++;
  }
  if(ispoorCall == false){
    if(GlobalFunctions.find(IdName)->second.ret_type == "void"){
      cout << "Function \"" + IdName +"\" is void, cannot return value!\n";
      errors++;
    }
  }
  //args
  std::vector<std::unique_ptr<ExprAST>> Args;
  TreeNode *tmp = t->child[1]->child[0];
  while (tmp!=nullptr) {
    if (auto Arg = ParseExpression(tmp)){
      Args.push_back(std::move(Arg));
      tmp = tmp->brother;
    }
    else
      return nullptr;
  }
  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}


//make a VariableExprAST
std::unique_ptr<ExprAST> ParseArrayElementExpr(TreeNode *t) {
  std::string tmp= t->child[0]->token.val;
  //array element's name
  if(t->child[1]->TreenodeType != Const){
    cout << "the index of array should be the const number!\n";
    errors++;
    return std::make_unique<VariableExprAST>(std::move("non-exist"));
  }
  int index = atoi(t->child[1]->token.val);
  map<std::string, FuncInfo>::iterator iter_function = GlobalFunctions.find(FuncName); 
  if(iter_function->second.arguments.size() != 0 &&
    (iter_function->second.arguments.find(tmp) != iter_function->second.arguments.end())){
    if(iter_function->second.arguments.find(tmp)->second == -1){
      cout << tmp + " is an variable, not an array!\n";
      errors++;
    }
    else if(iter_function->second.arguments.find(tmp)->second <= index){
      cout << tmp + " with the index " + t->child[1]->token.val + " out of bound!\n";
      errors++;
    }
  }
  else if(iter_function->second.localvar.size() != 0 &&
         (iter_function->second.localvar.find(tmp) != iter_function->second.localvar.end()))
  {
    if(iter_function->second.localvar.find(tmp)->second == -1){
      cout << tmp + " is an variable, not an array!\n";
      errors++;
    }
    else if(iter_function->second.localvar.find(tmp)->second <= index){
      cout << tmp + " with the index " + t->child[1]->token.val + " out of bound!\n";
      errors++;
    }
  }
  else{
    if(CheckGlobalName(tmp,index) == false){
      //cout << "this idName: \"" + IdName + "\" does not exists!\n";
      //errors ++;
    }
  }
  tmp = tmp+"["+t->child[1]->token.val+"]";
  return std::make_unique<VariableExprAST>(std::move(tmp));
}

//make a BinaryExprAST
std::unique_ptr<ExprAST> ParseBinaryExpr(TreeNode *t) {
  int Op = 0;
  //assign
  ispoorCall = false;
  if(t->TreenodeType == Assign){
    Op = ASSIGN;
  }else{
    //other binary operation
    std::string s = t->token.val;
    Op = String2Token[s];
  }
 // fprintf(stderr, "Op =  %d\n",Op);
 //parse left of op
  auto LHS = ParseExpression(t->child[0]);
  //parse right of op
  auto RHS = ParseExpression(t->child[1]);
  ispoorCall = true;
  return std::make_unique<BinaryExprAST>(Op,std::move(LHS),std::move(RHS));
}

//make a FunctionAST
std::unique_ptr<FunctionAST> ParseFuncExpr(TreeNode *t) {
  //return type
  std::string RetType = "";
  if(t->child[0]->TreenodeType == Int){
    RetType = "int";
  }else if(t->child[0]->TreenodeType == Void){
    RetType = "void";
  }
  //cout << "return type: " + RetType;
  //function name
  std::string Name = t->child[1]->token.val;
  FuncName = Name;
  /* check if there is the same name */  
  if(CheckNameExist(Name)){
    // name repeated
    cout << "This function name: " + Name + " is used in other declaration. Error!";
    errors ++;
    return nullptr;
  }
  
  class FuncInfo func(RetType);
  GlobalFunctions.insert(make_pair(Name,func));
  map<std::string, FuncInfo>::iterator iter_function = GlobalFunctions.find(Name);

  //args' names and their types 
  std::vector<std::string> Args;
  std::vector<std::string> Types;
  std::map<std::string,int> newarguments;
  TreeNode *tmp = t->child[2];
  if(!tmp){
    // fprintf(stderr, "params is null\n");
  }
  // fprintf(stderr, "TreeNodeType = %d\n",tmp->TreenodeType);
  tmp = tmp->child[0];
  //means no args
  if(tmp->TreenodeType == Void){
    // iter_function->second.arguments has nothing
  }
  else{
    while (tmp) {
      // fprintf(stderr, "tmp!=null\n");
      string ttmp = std::string(tmp->child[0]->token.val);
      //push back args types  
      string stmp;
      //push back args names
      if(tmp->child[1]){
        stmp = tmp->child[1]->token.val;
        Args.push_back(stmp);
        LocalVar.push_back(stmp);
      }
      if(tmp->child[2]){
        /* array is not allowed now */
        if(iter_function->second.arguments.size() != 0 &&
          (iter_function->second.arguments.find(stmp) != iter_function->second.arguments.end())){
          /* already exist */
          cout << "the argument " + stmp + " already exists!\n";
          errors ++;
        }
        else{
          iter_function->second.arguments.insert(make_pair(stmp,10));
        }
          
        ttmp = "int_array";
        Types.push_back(ttmp);
        cout << "array type can not by arguments now!\n";
          errors++;
      }
      else{
        if(iter_function->second.arguments.size() != 0 &&
          (iter_function->second.arguments.find(stmp) != iter_function->second.arguments.end()) ){
          /* already exist */
          cout << "the argument " + stmp + " already exists!\n";
          errors ++;
        }
        else{
          iter_function->second.arguments.insert(make_pair(stmp,-1));
        }
        ttmp = "int";
        Types.push_back(ttmp);
      }    
	    tmp = tmp->brother;
    }
  }

  //fprintf(stderr, "finish get params\n");
  auto Proto = std::make_unique<PrototypeAST>(RetType,Name,std::move(Args),std::move(Types));
  // fprintf(stderr, "finish build proto\n");
  //fprintf(stderr, "Proto Name: %s\n",Name.c_str());
  //local int variable's name
  std::vector<std::string> IntNames;
  //local int array's name and size
  std::vector<std::pair<std::string,int>> IntArrays;
  std::pair<std::string,int> tmppair;
  tmp = t->child[3]->child[0];
  while(tmp!=nullptr){
    if(tmp->child[1]){ 
      if(!tmp->child[3]){
        std::string s = tmp->child[1]->token.val;
        if((iter_function->second.arguments.size() != 0 && 
            iter_function->second.arguments.find(s) != iter_function->second.arguments.end() ) | 
            (iter_function->second.localvar.size() != 0 && 
            iter_function->second.localvar.find(s) != iter_function->second.localvar.end() )){
          /* already exist */
          cout << "the variable " + s + " already exists in local variables or arguments!\n";
          errors ++;
        }else{
          iter_function->second.localvar.insert(make_pair(s,-1));
        }
        
        IntNames.push_back(s);
      }
      else if(tmp->child[3]->TreenodeType == Arry_Decl){
        std::string s = tmp->child[1]->token.val;
        if((iter_function->second.arguments.size() != 0 && 
            iter_function->second.arguments.find(s) != iter_function->second.arguments.end() )| 
            (iter_function->second.localvar.size() != 0 && 
            iter_function->second.localvar.find(s) != iter_function->second.localvar.end() )){
          /* already exist */
          cout << "the variable " + s + " already exists in local variables or arguments!\n";
          errors ++;
        }else{
          iter_function->second.localvar.insert(make_pair(s,atoi(tmp->child[2]->token.val)));
        }
		    tmppair = std::make_pair(s,atoi(tmp->child[2]->token.val));
		    IntArrays.push_back(tmppair);
      }
	    tmp = tmp->brother;
    }
  }
  //the expression statements in a function
  auto States = ParseStatementsExpr(t->child[3]->child[1]);
  //the body of the function
  auto Body = std::make_unique<VarExprAST>(IntNames,IntArrays,std::move(States));
  return std::make_unique<FunctionAST>(std::move(Proto),std::move(Body));
}

//Variable declaration
std::unique_ptr<ExprAST> ParseVarDeclExpr(TreeNode *t) {
  //if it is a array declaration , it will have child[3] and child[3]'s TreenodeType = Arry_Decl
  if(t->child[3]!=NULL && t->child[3]->TreenodeType == Arry_Decl){
    return ParseArrayExpr(t);
  }
  // it is a normal variable declartion
  else if(t->child[1]->TreenodeType == Id){
	  std::string IdName = t->child[1]->token.val;
    /* insert global value */
    InsertGlobalName(IdName);
  Define_Initialize("int",IdName);
  return nullptr;
  }
  else return nullptr;
}

std::unique_ptr<FunctionAST> ParseTopLevelExpr(TreeNode *t) {
  auto E = ParseExpression(t);
  return nullptr;
}
/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
///   ::= forexpr
///   ::= varexpr
std::unique_ptr<ExprAST> ParseExpression(TreeNode *t) {
  switch (t->TreenodeType) {
  default:
    //return LogError("unknown token when expecting an expression");
  case Id:
    return ParseIdentifierExpr(t);
  case Const:
    return ParseNumberExpr(t);
  // case '(':
  //   return ParseParenExpr();
  case Selection_Stmt:
    return ParseIfExpr(t);
  //   case Arry_Decl:
  //     return ParseArrayExpr(t);
  case Iteration_Stmt:
  // case tok_for:
    return ParseWhileExpr(t);
  case Return_Stmt:
    return ParseReturnExpr(t); 
  case Call:
    return ParseCallExpr(t);
  //   case Statements:
  //     return ParseStatementsExpr(t);
  case Arry_Elem:
    return ParseArrayElementExpr(t);
  case Assign:
    return ParseBinaryExpr(t);
  case Op:
    return ParseBinaryExpr(t);
  //   case Func:
  //     return ParseFuncExpr(t);
  case Var_Decl:
    return ParseVarDeclExpr(t);
  }
}

//parse a function
void HandleDefinition(TreeNode *t) { 
  // fprintf(stderr, "Begin ParseFuncExpr\n");
  if (auto FnAST = ParseFuncExpr(t)) {
    if(errors > 0){
      return ;
    }
  //  fprintf(stderr, "Finish ParseFuncExpr\n");
    if (auto *FnIR = FnAST->codegen()) {
      //   fprintf(stderr, "Finish codegen\n");
      //  fprintf(stderr, "Read function definition:");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  }
}

//parse a declare
void HandleTopLevelExpression(TreeNode *t) {
  // Evaluate a top-level expression into an anonymous function.
  auto FnAST = ParseTopLevelExpr(t);
  return;
}

bool ParseTree(TreeNode *root){				//parse a tree
  if(!root) {
    fprintf(stderr, "root is null\n");
    return false;
  }
	TreeNode * tmp = root;
  //   if(!tmp) {
  //   fprintf(stderr, "tmp is null\n");
  //   return;
  // }
  //parse all the brothers of the root
	while(tmp!=nullptr){
    //  fprintf(stderr, "TreeNodeType = %d\n",tmp->TreenodeType);
		if(tmp->TreenodeType == Func){
    // flag = 1;
    //  fprintf(stderr, "Begin HandleDefinitio\n");
		HandleDefinition(tmp);
    //  fprintf(stderr, "Finish HandleDefinitio\n");
    LocalVar.clear();
	  }
	  else if(tmp->TreenodeType == Var_Decl){
      //flag = 0;
		  HandleTopLevelExpression(tmp);
	  }
	tmp = tmp->brother;
	}
  if (errors > 0){
    return false;
  }
  return true;
}


