#include <iostream>
#include "util.hpp"
using namespace std;

/*int lineNumber,colNumber;
unordered_map<TokenType,string> Token2String;
unordered_map<TreeNodeType,string> TreeNode2String;
Token GlobalToken;
vector<Node> dfa;
unordered_map<string,TokenType> String2Token;*/

bool IsLetter(char ch)
{
	return 'A' <= ch && ch <= 'Z' || 'a' <= ch && ch <= 'z';
}

bool IsDigit(char ch)
{
	return '0' <= ch && ch <= '9';
}

bool IsOtherOp(char ch)
{
	switch (ch)
	{
	case '+': case '-': case '*':
	case ':':
	case ',':
	case '(': case ')':
	case '[': case ']':
	case '{':case '}':
	case ';':
		return true;
	default:
		return false;
	}
}

bool IsBlank(char ch)
{
	return ch==' '||ch=='\t';
}

bool IsOther(char ch)
{
	return true;
}

void BuildDFA()       //构建dfa
{
	DFA.clear();

	Node node0;
	node0.type = StateType::SUNACCEPTED;
	node0.edges.emplace_back(1, IsLetter);
	node0.edges.emplace_back(2, IsDigit);
	node0.edges.emplace_back(3, [](char ch) {return ch == '<' || ch == '>' || ch == '=' || ch == '!'; });
	node0.edges.emplace_back(5, IsOtherOp);
	node0.edges.emplace_back(6, [](char ch) {return ch == '/'; });
	DFA.push_back(node0);

	Node node1;
	node1.type = StateType::SID;
	node1.edges.emplace_back(1, IsLetter);
	DFA.push_back(node1);

	Node node2;
	node2.type = StateType::SNUMBER;
	node2.edges.emplace_back(2, IsDigit);
	DFA.push_back(node2);

	Node node3;
	node3.type = StateType::SOPERATOR;
	node3.edges.emplace_back(4, [](char ch) {return ch == '='; });
	DFA.push_back(node3);

	Node node4;
	node4.type = StateType::SOPERATOR;
	DFA.push_back(node4);

	Node node5;
	node5.type = StateType::SOPERATOR;
	DFA.push_back(node5);

	Node node6;
	node6.type = StateType::SOPERATOR;
	node6.edges.emplace_back(7, [](char ch) {return ch == '*'; });
	DFA.push_back(node6);

	Node node7;
	node7.type = StateType::SUNACCEPTED;
	node7.edges.emplace_back(8, [](char ch) {return ch == '*'; });
	node7.edges.emplace_back(7, IsOther);

	DFA.push_back(node7);

	Node node8;
	node8.type = StateType::SUNACCEPTED;
	node8.edges.emplace_back(9, [](char ch) {return ch == '/'; });
	node8.edges.emplace_back(7, IsOther);
	DFA.push_back(node8);

	Node node9;
	node9.type = StateType::SCOMMENT;
	DFA.push_back(node9);

}

Token StateTpye2TokenType(int line,string token,StateType state){       //状态类型转token
	switch (state)
	{
	case StateType::SID:
		if(String2Token[token]>0){
			return Token(lineNumber,token,String2Token[token]);
		}else{
			return Token(lineNumber,token,TokenType::ID);
		}
	case StateType::SNUMBER:
		return Token(lineNumber,token,TokenType::NUMBER);
	case StateType::SENDFILE:
		return Token(lineNumber,token,TokenType::ENDFILE);
	default:
		return Token(lineNumber,token,String2Token[token]);
	}
}

Token GetToken(){           //获取token
	int state = 0;
	string token="";
	while (true){
		char ch = fgetc(stdin);
		colNumber++;
		bool flag = false;
		for (auto it : DFA[state].edges){
			if (it.action(ch)){
				flag = true;
				state = it.transition;
				token.push_back(ch);
				break;
			}
		}
		if (!flag){
			if (DFA[state].type != StateType::SUNACCEPTED){
				if (!IsBlank(ch)){
					ungetc(ch,stdin);
				}
				if (DFA[state].type != StateType::SCOMMENT){
					return StateTpye2TokenType(lineNumber,token,DFA[state].type);
				}else{
					token.clear();
					state=0;
				}
			}else if ((ch!='\n'&&ch!='\t'&&ch!=' '&&ch != EOF)){
				ShowLexError(token);
			}
		}
		if (ch == '\n'){
			lineNumber++;
			colNumber = 0;
		}else if (ch == EOF){
			return StateTpye2TokenType(lineNumber,token,StateType::SENDFILE);
		}
	}
}

void LexInit(){                         //每个token的对应值初始化
	lineNumber = 1, colNumber = 0;
	String2Token["if"]=IF;
	String2Token["else"]=ELSE;
	String2Token["int"]=INT;
	String2Token["void"]=VOID;
	String2Token["while"]=WHILE;
	String2Token["return"]=RETURN;

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
	String2Token["/"]=OVER;
	String2Token["("]=LPAREN;
	String2Token[")"]=RPAREN;
	String2Token["["]=LBRACKET;
	String2Token["]"]=RBRACKET;
	String2Token["{"]=LBRACE;
	String2Token["}"]=RBRACE;
	String2Token[";"]=SEMI;
	String2Token[","]=COMMA;


	Token2String[IF]="if";
	Token2String[ELSE]="else";
	Token2String[INT]="int";
	Token2String[VOID]="void";
	Token2String[WHILE]="while";
	Token2String[RETURN]="return";

	Token2String[ASSIGN]="=";
	Token2String[LT]="<";
	Token2String[GT]=">";
	Token2String[EQ]="==";
	Token2String[NEQ]="!=";
	Token2String[GEQ]=">=";
	Token2String[LEQ]="<=";

	Token2String[PLUS]="+";
	Token2String[MINUS]="-";
	Token2String[TIMES]="*";
	Token2String[OVER]="/";
	Token2String[LPAREN]="(";
	Token2String[RPAREN]=")";
	Token2String[LBRACKET]="[";
	Token2String[RBRACKET]="]";
	Token2String[LBRACE]="{";
	Token2String[RBRACE]="}";
	Token2String[SEMI]=";";
	Token2String[COMMA]=",";
}

void ShowLexError(string token){        //报错
	cout << "lex error:" << token
		<<" in line "<<lineNumber
		<<" , col "<<colNumber<<endl;
	exit(1);
}

void TokenOut(){       //输出token
	while(1){
		Token tok=GetToken();
		TokenType &type=tok.type;
		string token=tok.value;
		if(type==TokenType::ENDFILE){
			break;
		}
		cout<<tok.line<<":\t";
		switch (type){
			case TokenType::ID:
				cout << "ID, name = " << token << endl;
				break;
			case TokenType::IF: case TokenType::ELSE:case TokenType::INT: 
			case TokenType::RETURN: case TokenType::VOID: case TokenType::WHILE:
				cout << "reserved word: " << token << endl;
				break;
			case TokenType::NUMBER:
				cout << "number: " << token << endl;
				break;
			default:
				cout << token << endl;
				break;
		}
	}	
}