/* interface by YHR */
/* semantic analysis by PYG*/
/* finally finished on May 21st */

#ifndef PAREXP_H
#define PAREXP_H
#include"codegen1.hpp"
#include"util.hpp"
#include <memory>
#include <map> 
using namespace std;

extern TreeNode *root1;
class ExprAST;
class NumberExprAST;
class VariableExprAST;
class ArrayExprAST;
class BinaryExprAST ;
class IfExprAST;
class ReturnExprAST;
class CallExprAST;
class WhileExprAST;
class PrototypeAST ;
class VarExprAST;
class FunctionAST;
class StatementsAST;


extern std::vector<std::string> GlobalVar;
extern  std::vector<std::string> LocalVar;

extern int flag ;//0:global 1:local
extern std::map<std::string,int> GlobalVariables;
class FuncInfo{
public:
  string ret_type;
  std::map<std::string,int> arguments;
  std::map<std::string,int> localvar;
  FuncInfo(string type):ret_type(type){}
};

extern std::map<std::string, FuncInfo> GlobalFunctions;
extern std::string FuncName ;
extern int errors ;
extern bool ispoorCall;
bool CheckNameExist(string checkallname);
bool CheckGlobalName(string checkname,int index = -1);
bool InsertGlobalName(string newname, int size = -1);

std::unique_ptr<ExprAST> ParseExpression(TreeNode *t);
std::unique_ptr<ExprAST> ParseNumberExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseIdentifierExpr(TreeNode *t);
std::unique_ptr<StatementsAST> ParseStatementsExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseIfExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseArrayExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseWhileExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseCallExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseArrayElementExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseBinaryExpr(TreeNode *t);
std::unique_ptr<FunctionAST> ParseFuncExpr(TreeNode *t);
std::unique_ptr<ExprAST> ParseVarDeclExpr(TreeNode *t);
std::unique_ptr<FunctionAST> ParseTopLevelExpr(TreeNode *t) ;
void HandleDefinition(TreeNode *t);
void HandleTopLevelExpression(TreeNode *t);
bool ParseTree(TreeNode *root);
#endif