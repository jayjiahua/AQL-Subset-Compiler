#include "lexer.h"
#include "token.h"
#include "exception.h"
#include <ctype.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>

using std::cout;
using std::endl;
using std::getline;
using std::setw;
using std::setfill;

Lexer::Lexer(const char *filePath) : filePath(filePath) {
	ifs.open(filePath);
	this->filePath = string(filePath);
	if (!ifs) {
		cout << "Fail to open \"" << filePath << "\"" << endl;
		throw FileOpenException();
	}
	// 将下面的关键字保留
	reserve(Token(AQLToken::CREATE, "create"));
	reserve(Token(AQLToken::VIEW, "view"));
	reserve(Token(AQLToken::AS, "as"));
	reserve(Token(AQLToken::OUTPUT, "output"));
	reserve(Token(AQLToken::SELECT, "select"));
	reserve(Token(AQLToken::FROM, "from"));
	reserve(Token(AQLToken::EXTRACT, "extract"));
	reserve(Token(AQLToken::REGEX, "regex"));
	reserve(Token(AQLToken::ON, "on"));
	reserve(Token(AQLToken::RETURN, "return"));
	reserve(Token(AQLToken::GROUP, "group"));
	reserve(Token(AQLToken::AND, "and"));
	reserve(Token(AQLToken::TOKEN, "Token"));
	reserve(Token(AQLToken::PATTERN, "pattern"));
	peek = ' ';
	row = 1;
	col = 0;
}

Lexer::~Lexer() {
	ifs.close();
}

void Lexer::reserve(Token token) {
	words[token.getValue()] = token;
}

void Lexer::readch() {
	peek = ifs.get();
	col++;
}

Token Lexer::scan() {
	while (!ifs.eof()) {
		if (peek == '\n') {
			row++;
			col = 0;
		}
		else if (!(peek == ' ' || peek == '\t' || peek == '\r')) {
			break;
		}
		readch();
	}
	switch (peek) {
		case '.':
			readch();
			return Token(AQLToken::DOT, ".");
		case '(':
			readch();
			return Token(AQLToken::LEFT_PARENTHESIS, "(");
		case ')':
			readch();
			return Token(AQLToken::RIGHT_PARENTHESIS, ")");
		case '<':
			readch();
			return Token(AQLToken::LEFT_ANGLE_BACKET, "<");
		case '>':
			readch();
			return Token(AQLToken::RIGHT_ANGLE_BACKET, ">");
		case '{':
			readch();
			return Token(AQLToken::LEFT_BRACE, "{");
		case '}':
			readch();
			return Token(AQLToken::RIGHT_BRACE, "}");
		case ',':
			readch();
			return Token(AQLToken::COMMA, ",");
		case ';':
			readch();
			return Token(AQLToken::SEMICOLON, ";");
	}

	// 匹配正则表达式(形如 /abc/)
	if (peek == '/') {
		string reg;
		while (!ifs.eof()) {
			readch();
			if (peek == '/') { // 正则表达式的右端
				readch();
				break;
			}
			else {
				reg += peek;
				if (peek == '\\') {
					readch();
					reg += peek;
				}
			}
		}
		return Token(AQLToken::REG, reg);
	}

	// 匹配数字
	if (isdigit(peek)) {
		string num;
		while (!ifs.eof() && isdigit(peek)) {
			num += peek;
			readch();
		}
		return Token(AQLToken::NUM, num);
	}

	// 匹配ID
	if (isalpha(peek) || peek == '_') {
		string identity;
		while (!ifs.eof() && (isalnum(peek) || peek == '_')) {
			identity += peek;
			readch();
		}
		if (words.find(identity) == words.end()) { // 找不到则新建
			words[identity] = Token(AQLToken::ID, identity);
		}
		return words[identity];
	}

  // 若无匹配则抛出异常
	if (!ifs.eof()) {
		cout << "Lexical error: In row " << row << " and column " << col << "." << endl;
		throw LexicalErrorException();
	}
	return Token(AQLToken::END, "");
}

void Lexer::printCurrentPosition() {
	ifstream fs(filePath.c_str());
	int currentRow = 1;
	string line;
	while (getline(fs, line) && currentRow != this->row) {
		currentRow++;
	}
	cout << "In file \"" << filePath << "\", line " << row << ", col " << col << ":" << endl;
	cout << setw(4) << row;
	cout << "|  " << line << endl;
	cout << setfill(' ') << setw(5 + col) << "";
	cout << "^" << endl;
	cout << endl;
}
