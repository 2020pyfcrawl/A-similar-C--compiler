#ifndef  UTIL_H
#define  UTIL_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <list>
#include <set>
using namespace std;
extern int lineNumber,colNumber;   //发生错误，给出错误行列号

enum TokenType       //token类型定义
{
	ENDFILE=0,
	IF,ELSE,INT,RETURN,VOID,WHILE,
	ID,NUMBER,
	ASSIGN,LT,GT,EQ,NEQ,GEQ,LEQ,
	PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,LBRACKET,RBRACKET,LBRACE,RBRACE,COMMA,SEMI,
};

enum StateType        //dfa中的状态类型
{
	SUNACCEPTED = 0,
	SID,
	SRESERVED,
	SNUMBER,
	SOPERATOR,
	SCOMMENT,
	SENDFILE
};
enum TreeNodeType     //语法树中的节点类型
{
	Func,Int,Id,Params,Param,Comp,Const,Call,Args,Void,Var_Decl,Arry_Decl,Arry_Elem,Assign,Op,
	Selection_Stmt,Iteration_Stmt,Return_Stmt,Statements
}; 

typedef bool(*ACTION)(char);

struct Edge            //构建dfa的edge
{
	int transition;
	ACTION action;

	Edge(int transition, ACTION action)
	{
		this->transition = transition;
		this->action = action;
	}
};

struct Node           //节点
{
	StateType type;
	std::list<Edge> edges;
};

struct Token       //token
{
	int line;
	std::string value;
	TokenType type;
	Token(){
		line=1;
		type=INT;
		value.reserve(100);
	}
	Token(const int line,const std::string& value, TokenType type)
	{
		this->line = line;
		this->value = value;
		this->type = type;
	}
};

//全局变量定义
extern vector<Node> DFA;
extern unordered_map<string,TokenType> String2Token;
extern unordered_map<TokenType,string> Token2String;
extern unordered_map<TreeNodeType,string> TreeNode2String;
//token识别函数
bool IsLetter(char ch);
bool IsDigit(char ch);
bool IsOtherOp(char ch);
bool IsBlank(char ch);
bool IsOther(char ch);
void BuildDFA();
Token StateTpye2TokenType(int line,string token,StateType state);
Token GetToken();

void LexInit();
void PraseInit();

extern Token GlobalToken;

struct TreeTokenNode{ 
	char val[30];
	TokenType type;
	TreeTokenNode(){
		memset(val,0,sizeof(val));
	}
};
struct TreeNode          //树节点
{
	struct TreeNode *child[4];
	struct TreeNode *brother;
    TreeNodeType TreenodeType;
	TreeTokenNode token;
};

void match(TokenType expected);
void TokenCopy(TreeTokenNode &src,Token dst);
TreeNode *newNode(TreeNodeType typ);
//文法实现
TreeNode * declaration_list();
TreeNode * declaration();
TreeNode * params();
TreeNode * param_list(TreeNode * par);
TreeNode * param(TreeNode * par);
TreeNode * compound_stmt();
TreeNode * local_declaration();
TreeNode * statement_list();
TreeNode * statement();
TreeNode * expression_stmt();
TreeNode * selection_stmt();
TreeNode * iteration_stmt();
TreeNode * return_stmt();
TreeNode * expression();
TreeNode * var();
TreeNode * simple_expression(TreeNode * par);
TreeNode * additive_expression(TreeNode * par);
TreeNode * term(TreeNode * par);
TreeNode * factor(TreeNode * par);
TreeNode * call(TreeNode * par);
TreeNode * args();
TreeNode * statements()  ;
 

void ParseSyntax(/*TreeNode * root*/);
void TokenOut();
void PreOrder(TreeNode * root);

void ShowLexError(string token);
void ShowGrammarError();
#endif