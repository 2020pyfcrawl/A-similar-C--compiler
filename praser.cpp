#include <iostream>
#include "util.hpp"
using namespace std;
TreeNode *root1 = nullptr;

int lineNumber,colNumber;
unordered_map<TokenType,string> Token2String;
unordered_map<TreeNodeType,string> TreeNode2String;
Token GlobalToken;
vector<Node> DFA;
unordered_map<string,TokenType> String2Token;

void PraseInit(){
	lineNumber = 1, colNumber = 0;
	
	TreeNode2String[Func]="Func";      //函数
	TreeNode2String[Int]="Int";       //int值
	TreeNode2String[Id]="Id";         //标识符
	TreeNode2String[Params]="Params";  //函数参数列表
	TreeNode2String[Param]="Param";       //函数参数
	TreeNode2String[Comp]="Comp";       //复合语句
	TreeNode2String[Const]="Const";          //常量
	TreeNode2String[Call]="Call";             //回调

	TreeNode2String[Args]="Args";         //回调函数参数列表
	TreeNode2String[Void]="Void";         //空值
	TreeNode2String[Var_Decl]="Var_Decl";  //变量声明（数组变量，普通变量）
	TreeNode2String[Arry_Decl]="Arry_Decl";   //数组变量
	TreeNode2String[Arry_Elem]="Arry_Elem";   //数组变量元素
	TreeNode2String[Assign]="Assign";           //赋值
	TreeNode2String[Op]="Op";                 //运算符
	TreeNode2String[Selection_Stmt]="Selection_Stmt";       //选择语句if
	TreeNode2String[Iteration_Stmt]="Iteration_Stmt";        //迭代语句while
	TreeNode2String[Return_Stmt]="Return_Stmt";          //返回语句return
	TreeNode2String[Statements]="Statements";
}
  
void ShowGrammarError(){            //报错
	cout<<"unexpected token:"<<GlobalToken.value
		<<" in line "<<lineNumber
		<<" , col "<<colNumber<<endl;
	exit(1);
}

void match(TokenType expected)      //检测当前token类型是否符合，并获取下一个token
{
	if(GlobalToken.type==expected){
		GlobalToken=GetToken();
	}else{
		ShowGrammarError();
	}
}
void TokenCopy(TreeTokenNode &src,Token dst){
	src.type=dst.type;
	// cout<<"dst.value="<<dst.value<<endl;
	strcpy(src.val,dst.value.c_str());
}

TreeNode *newNode(TreeNodeType typ)
{ 
	TreeNode *ret = (TreeNode *) malloc(sizeof(TreeNode));
	for (int i=0;i<4;i++) {
		ret->child[i] = NULL;
	}
    ret->brother = NULL;
    ret->TreenodeType = typ;
	return ret;
}

//declaration_list->declaration_list{declaration}
TreeNode * declaration_list()
{
	TreeNode * root = declaration();
	TreeNode * now =root;
	while(GlobalToken.type==INT||GlobalToken.type==VOID){       //只有两种类型声明，int 和 void
		TreeNode *nxt = declaration();
		if (nxt!=NULL){
			now->brother=nxt;
			now=nxt;
		}
	}
	match(TokenType::ENDFILE);
	return root;
}

TreeNode * declaration()
{
	TreeNode * root = NULL;
	TreeNode * fir = NULL,* sec = NULL,* thi = NULL,* fou = NULL;
	if (GlobalToken.type==INT){             //先确定类型
		fir=newNode(Int);
		match(TokenType::INT);
	}else{//VOID
		fir=newNode(Void);
		match(TokenType::VOID);
	}
	if(GlobalToken.type==ID){          //下一个token是ID则正常
		sec = newNode(Id);
		TokenCopy(sec->token,GlobalToken);
		match(TokenType::ID);
		switch (GlobalToken.type)
		{
		case LPAREN:       //'('     Id后面的token要是左小括号表明是函数定义
			root = newNode(Func);             //函数声明
			root->child[0] = fir;   
			root->child[1] = sec;	

			match(TokenType::LPAREN);
			root->child[2] = params();       //函数参数s
			match(TokenType::RPAREN);      //')'
			
			root->child[3] = compound_stmt();     //函数定义中复合语句
			break;
		case LBRACKET:                       //'['数组声明，比如int a[10]
			root = newNode(Var_Decl);
			fou = newNode(Arry_Decl);
			root->child[0] = fir;   
			root->child[1] = sec;

			match(TokenType::LBRACKET);
			thi = newNode(Const);
			TokenCopy(thi->token,GlobalToken);
			match(TokenType::NUMBER);
			root->child[2]=thi;
			root->child[3]=fou;
			match(TokenType::RBRACKET);
			match(TokenType::SEMI);
			break;
		case SEMI:                        //','变量声明 比如int a
			root = newNode(Var_Decl);
			root->child[0] = fir;
			root->child[1] = sec;
			match(SEMI);
			break;
		default:
			ShowGrammarError();
			break;
		}
	}else{
		ShowGrammarError();
	}
	return root;
}


//params_list | void
TreeNode * params(){         //判断参数是否为void
	TreeNode * root = newNode(Params);
	TreeNode * now  = NULL;
	if (GlobalToken.type==VOID){
		now = newNode(Void);
		match(VOID);
		root->child[0] = now;
	}else{        //不是void则是一个参数列表。调用param_list
		root->child[0] = param_list(now);
	}
	return root;
}

//params_list->param{,param}
TreeNode * param_list(TreeNode * par)
{
	TreeNode * root = param(par);
	TreeNode * now = root; 
	par = NULL;
	while (GlobalToken.type==COMMA){    //','
		TreeNode * nxt = NULL;
		match(COMMA);
		nxt = param(par);
		if (nxt!=NULL){
			now->brother = nxt;                 //参数之间是以brother形式
			now = nxt;
		}
	}
	return root;
}

TreeNode * param(TreeNode * par)
{
	TreeNode * root = newNode(Param);
	TreeNode * now = NULL,* nxt = NULL;
	if(par==NULL){                     //类型判断
		if(GlobalToken.type==VOID){
			now = newNode(Void);
			match(VOID);
		}else if(GlobalToken.type==INT){
			now = newNode(Int);
			match(INT);
		}
	}else{
		now = par;
	}
	if(now!=NULL){                //变量  int a
		root->child[0] = now;
		if (GlobalToken.type==ID){
			nxt = newNode(Id);
			TokenCopy(nxt->token,GlobalToken);
			root->child[1] = nxt;
			match(ID);
		}
		if (GlobalToken.type==LBRACKET&&(root->child[1]!=NULL)){//num[]      int a[]
			match(LBRACKET);
			root->child[2] = newNode(Id);
			strcpy(root->child[2]->token.val,"[]");
			match(RBRACKET);
		}else{  
			return root; 
		}		
	}else {
		ShowGrammarError();
	}
	return root;
}
 
TreeNode * compound_stmt()        //复合语句{...}
{  
	TreeNode * t = newNode(Comp);
	match(LBRACE);          //'{'
	t->child[0] = local_declaration();      //本地声明。        注意这导致，必须代码模块只能是先声明后语句
	t->child[1] = statements();     //语句列表             如 {int a;int b;a=10;b=a+10;}不能{int a;a=10;int b; }
	match(RBRACE);
	return t;
}
 
TreeNode * local_declaration()       //本地声明
{  
	TreeNode * root = NULL,* now = NULL,* nxt = NULL;
	while(GlobalToken.type==INT || GlobalToken.type==VOID){  //类型判断
		nxt = newNode(Var_Decl);
		if(GlobalToken.type==INT){
			nxt->child[0] = newNode(Int);
			match(INT);
		}else{
			nxt->child[0] = newNode(Void);
			match(VOID);
		}
		if(GlobalToken.type==ID){                   //类型后面一定跟着id 
			nxt->child[1] = newNode(Id);
			TokenCopy(nxt->child[1]->token,GlobalToken);
			match(ID);
			if(GlobalToken.type==LBRACKET){    //'['  遇到左中括号说明是数组
				nxt->child[3] = newNode(Arry_Decl);
				match(LBRACKET);
				//
				TreeNode * thi = newNode(Const);
				TokenCopy(thi->token,GlobalToken);
				match(TokenType::NUMBER);
				nxt->child[2]=thi;

				match(RBRACKET);
				match(SEMI);
			}else{
				match(SEMI);
			}
		} 
		if(root==NULL){
			root = now = nxt;
		}else{
			now->brother = nxt;              //声明之间是brother
			now = nxt;
		}
	}
	return root;
}
TreeNode * statement_list()         //语句列表
{  
	if(GlobalToken.type == LBRACE){
		match(LBRACE);
		TreeNode* root = statements();
		match(RBRACE); 
		return root;
	}
	else{
		return statement();
	}
	
}

TreeNode * statements()         //语句列表
{  
	 
	TreeNode * root = statement();   
	TreeNode * now = root;
	set<int> s{IF,LBRACKET,ID,WHILE,RETURN,SEMI,NUMBER};
	while (s.find(GlobalToken.type)!=s.end()){  
		TreeNode * nxt;
		nxt = statement();       //语句
		if(nxt!=NULL){
			if(root==NULL){ 
				root = now = nxt;
			}else{  
				now->brother = nxt;
				now = nxt;
			}
		}
	}
	
	return root;
}

TreeNode * statement()        //判断是什么类型的语句
{  
	switch(GlobalToken.type){
		case IF:return selection_stmt();      //选择
		case WHILE:return iteration_stmt();   //循环
		case RETURN:return return_stmt();     //返回
		// case LBRACE:return compound_stmt();   //'{'组合语句
		case ID: case SEMI: case LPAREN: case NUMBER: return expression_stmt();   //表达式语句
		default:ShowGrammarError();return nullptr;
	}
}
 
TreeNode * selection_stmt()   //选择
{  
	TreeNode * root = newNode(Selection_Stmt);
	match(IF);
	match(LPAREN);
	root->child[0] = expression();         //表达式
	match(RPAREN);
	root->child[1] = statement_list();
	if(GlobalToken.type==ELSE){ 
		match(ELSE);
		root->child[2] = statement_list();
	}
	return root;
}
 
TreeNode * iteration_stmt()           //循环
{  
	TreeNode * root = newNode(Iteration_Stmt);
	match(WHILE);
	match(LPAREN);
	root->child[0] = expression();
	match(RPAREN);
	root->child[1] = statement_list();
	return root;
}
 
TreeNode * return_stmt()        //返回
{  
	TreeNode * root = newNode(Return_Stmt);
	match(RETURN);
	if (GlobalToken.type==SEMI){  
		match(SEMI);
		return root;
	}else{ 
		root->child[0] = expression();
	}
	match(SEMI);
	return root;
}
 
TreeNode * expression_stmt(){    //表达式语句
	TreeNode * root = NULL;
	if(GlobalToken.type==SEMI){ 
		match(SEMI);
		return root;
	}else{
		root = expression();
		match(SEMI);
	}
	return root;
}
 
TreeNode * expression()   //表达式
{  
	TreeNode * t = var();
	if(t==NULL){ 
		t = simple_expression(t); 
	}else{    
		TreeNode * now = NULL;
		if(GlobalToken.type==ASSIGN){ 
			now = newNode(Assign);
			match(ASSIGN);
			now->child[0] = t;
			now->child[1] = expression();
			return now;
		}else{ 
			t = simple_expression(t);
		}
	}	   
	return t;
}
 
TreeNode * var()
{  
	TreeNode * root = NULL,* fir = NULL,* sec = NULL;
	if(GlobalToken.type==ID){
		fir = newNode(Id);
		TokenCopy(fir->token,GlobalToken);
		match(ID);
		if(GlobalToken.type==LBRACKET) { 
			match(LBRACKET);
			sec = expression();
			match(RBRACKET);
			root = newNode(Arry_Elem);
			root->child[0] = fir;
			root->child[1] = sec;
		}else{
			root = fir;
		}
	}
	return root;
}
 
TreeNode * simple_expression(TreeNode * par)
{   
	TreeNode * root = additive_expression(par);
	par = NULL;
	set<int> s{EQ,GT,GEQ,LT,LEQ,NEQ};
	if(s.find(GlobalToken.type)!=s.end()){  
		TreeNode * now = newNode(Op);
		TokenCopy(now->token,GlobalToken);
		now->child[0] = root;
		root = now;
		match(GlobalToken.type);
		root->child[1] = additive_expression(par);		 
		return root;
	}
	return root;
}
 
TreeNode * additive_expression(TreeNode * par)         //算数表达式
{  
	TreeNode * root = term(par);
	par = NULL;
	while((GlobalToken.type==PLUS)||(GlobalToken.type==MINUS))
	{ 
		TreeNode * now = newNode(Op);
		TokenCopy(now->token,GlobalToken);
		now->child[0] = root; 
		match(GlobalToken.type);
		now->child[1] = term(par);
		root = now;		
	}
	return root;
}
 
TreeNode * term(TreeNode * par)       //按照文法来，解决优先级问题
{  
	TreeNode * root = factor(par);
	par = NULL;
	while((GlobalToken.type==TIMES)||(GlobalToken.type==OVER)){ 
		TreeNode * now = newNode(Op);
		TokenCopy(now->token,GlobalToken);
		now->child[0] = root; 
		root = now;
		match(GlobalToken.type);
		now->child[1] = factor(par);
	}
	return root;
}
 
TreeNode * factor(TreeNode * par)
{  
	TreeNode * root = NULL;
	if(par!=NULL){
		if(GlobalToken.type==LPAREN && par->TreenodeType!=Arry_Elem){ 
			root = call(par);
		}else{ 
			root = par; 
		}
	}else{ 
		switch(GlobalToken.type){
		case LPAREN:
			match(LPAREN);
			root = expression();
			match(RPAREN);
			break;
		case ID:
			par = var();
			if(LPAREN==GlobalToken.type && par->TreenodeType!=Arry_Elem) { 
				root = call(par);
			}else{
				root=par;
			}
			break;
		case NUMBER:
			root = newNode(Const);
			TokenCopy(root->token,GlobalToken);
			match(NUMBER);
			break;
		default:
			ShowGrammarError();
		} 
	}
	return root;
}
 
TreeNode * call(TreeNode * par)       //函数回调，允许递归
{  
	TreeNode * root = newNode(Call);
	if(par!=NULL)
		root->child[0] = par;
	match(LPAREN);
	if(GlobalToken.type==RPAREN)  {  
		match(RPAREN);
		return root;
	}else if(par!=NULL){ 
		root->child[1] = args();
		match(RPAREN);
	}
	return root;
}
 
TreeNode * args()        //函数回调中的参数列表
{  
	TreeNode * root = newNode(Args);
	TreeNode * tmp = NULL,* now = NULL;
	if(GlobalToken.type!=RPAREN){
		tmp = expression();
		now = tmp;
		while(GlobalToken.type==COMMA){   
			TreeNode * nxt;
			match(COMMA);
			nxt = expression();
			if(nxt!=NULL){
				if(tmp==NULL){ 
					tmp = now = nxt;
				}else{  
					now->brother = nxt;
					now = nxt;
				}
			}
		}
	}	
	if(tmp!=NULL){
		root->child[0] = tmp;
	}
	return root;
}
 
 int blankCnt=0;
void PreOrder(TreeNode* t)  //前序打印树
{
	string blank="  ";
	for(int i=0;i<blankCnt;i++){
		blank+=" ";
	}
	if(t!=NULL)
	{	
		if(t->TreenodeType==Op){
			cout<<blank<<"Op: "<<Token2String[t->token.type]<<endl;
		}else{
			cout<<blank<<TreeNode2String[t->TreenodeType];
			if(t->TreenodeType==Id||t->TreenodeType==Const){
				cout<<": "<<t->token.val;
			}
			cout<<endl;
		}
	}
	for(int i=0;i<4;i++){
	    if(t->child[i]!=NULL){
			blankCnt+=2;
			//cout<<"<ischild>"<<i<<endl;
	        PreOrder(t->child[i]);
			blankCnt-=2;
	    }
	}
	if(t->brother!=NULL){	
		//cout<<"<isbrother>"<<endl;
		PreOrder(t->brother);
	}
}
 
void ParseSyntax(/*TreeNode *rootS*/)     //语法树分析
{
    cout<<"Syntax Tree:"<<endl;
	GlobalToken = GetToken();
	root1 = declaration_list();
	PreOrder(root1);
	//return root;
}