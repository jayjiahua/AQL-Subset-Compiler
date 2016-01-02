/*
	语法分析器
	基本思想:
	1. 递归下降的语法分析
	2. 每次向前看一个Token(不把它吃掉)以决定使用哪条产生式
	3. 根据token, 将Token列表结构化为c++对象, 并自底向上传递, 体现了分治的思想.
		避免将所有Token都交给最上层处理而导致上层的逻辑结构十分庞大, 调试困难.
	4. 对于无法掌握足够的Token来进行结构化的产生式,直接将Token的列表返回,
		直到存在能够将这些Token结构化的上级
*/

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "view.h"
#include "tokenizer.h"
#include <map>
#include <string>
#include <cstdlib>
#include <set>

using std::vector;
using std::map;
using std::string;
using std::set;

/* from_item 产生式的结构 */
struct FromItem {
	string viewName;
	string id;
	FromItem(string viewName, string id) : viewName(viewName), id(id) {}
};

/* from_item 产生式的结构 */
struct SelectItem {
	string viewName;
	string field;
	string alias;
	SelectItem(string viewName, string field, string alias) {
		this->viewName = viewName;
		this->field = field;
		this->alias = alias;
	}
};

/* single_group 产生式的结构 */
struct Group {
	int groupNum;
	string id;
	Group(string num, string id) {
		this->id = id;
		this->groupNum = atoi(num.c_str());
	}
};

/*
  atom 产生式的结构, 并且对结构做了一些扩展
	1. 最少匹配次数
	2. 最大匹配次数
	3. 未被处理的token列表,由于结构相当简单(ID.ID 或 REG 或 Token)就不另外建立新的数据结构了
*/
struct Atom {
	int minTime;
	int maxTime;
	vector<Token> tokens;
	set<int> groups;

	Atom(vector<Token> tokens = vector<Token>(), int minTime = 1, int maxTime = 1) {
		this->tokens = tokens;
		this->minTime = minTime;
		this->maxTime = maxTime;
	}
};

/*
  extract_spec 产生式的结构
	包含:
	1. 未被处理的Token
	2. 结构化的atom列表
	3. group列表
*/
struct ExtractSpec {
	vector<Token> tokenList;
	vector<Atom> atomList;
	vector<Group> groupList;

	ExtractSpec(vector<Token> tokenList, vector<Atom> atomList, vector<Group> groupList) {
		this->tokenList = tokenList;
		this->atomList = atomList;
		this->groupList = groupList;
	}
};


class Parser {
public:
	Parser(Lexer &lexer, Tokenizer &tokenizer);  // 构造初始的view, 即Document
	void program();			// 开始解析

private:
	map<string, View> views;  // view列表, 对应: viewName - View
	Token look;               // 正在扫描的token
	Lexer &lexer;
	Tokenizer &tokenizer;
	string document;          // 当前分析的整个文本

	void move();												// 使词法分析器向前扫描一个token
	string peek();											// 获得当前正在扫描的token的值(token.value)
	void syntaxError(string errMsg);    // 处理语法错误
	void semanticError(string errMsg);  // 处理语义错误
	void term(AQLToken::Tag tag);       // 判断当前的token类型的正确性并且吃掉, 若不正确, 则会抛出语法错误
	bool match(AQLToken::Tag tag);      // 判断当前token类型的正确性, 但不吃掉(只向前看)

	// 根据atom来生成对应的列
	vector<Span> dealWithAtom(Atom atom, map<string, string> fromList);

	// 将两个列根据单词距离合并, 最后产生一个新的列
	vector<Span> mergeColumn(vector<Span>& firstColumn, vector<Span>& secondColumn, int minCount = -1, int maxCount = 0);


	/* 以下为产生式 */
	void aqlStmt();
	void createStmt();
	void outputStmt();
	View viewStmt();
	string alias();

	View selectStmt();
	vector<SelectItem> selectList();
	SelectItem selectItem();
	map<string, string> fromList();
	FromItem fromItem();

	View extractStmt();
	ExtractSpec extractSpec();
	ExtractSpec regexSpec();
	vector<Token> column();
	vector<Group> nameSpec();
	vector<Group> groupSpec();
	Group singleGroup();

	ExtractSpec patternSpec();
	void patternExpr();
	void patternPkg();
	void atom();
	void patternGroup();

};


#endif
