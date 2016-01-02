/*
	词法分析器
	参考 <<编译原理>> 附录1
*/

#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "token.h"

using std::vector;
using std::map;
using std::string;
using std::ifstream;


class Lexer {
public:
	Lexer(const char *filePath);
	~Lexer();
	Token scan();                 // 由语法分析器调用,每次从aql文本中提取一个token
	void printCurrentPosition();  // 输出当前正在解析的位置,用于输出错误
private:
	int row;
	int col;
	string filePath;
	char peek;
	ifstream ifs;

	map<string, Token> words;    // 已确认的Token键值对
	void reserve(Token token);   // 将Token作为保留字
	void readch();               // 读取下一个字符
};
#endif
