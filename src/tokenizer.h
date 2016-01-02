/*
	Tokenizer.h
	文本解析器
	用于将英文文本分词(不是用于aql!)
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cctype>
#include "view.h"
using namespace std;


class Tokenizer {
public:
	Tokenizer(const char* path);
	string getText() const;            // 获取文件的整个文本
	vector<Span> getTokens() const;   // 获得分词列表
	int getWordPosition(int pos) const;  // 根据当前字符的位置来判断它处于第几个单词,用于计算单词距离

private:
	vector<Span> tokens;             // 分词列表
	vector<int> wordPositionList;    // 字符位置-单词位置对应表 如wordPositionList[0] = 0, 代表文本的第0个字符属于第0个单词
	string text;
};

#endif
