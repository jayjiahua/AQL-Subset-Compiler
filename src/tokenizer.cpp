#include "tokenizer.h"
#include "exception.h"

Tokenizer::Tokenizer(const char* path) {
	ifstream ifs(path);
	if (!ifs) {
		cout << "Fail to open \"" << path << "\"" << endl;
		throw FileOpenException();
	}
	ifs.unsetf(ios_base::skipws);
	char c;
	int pos = 0;
	int wordPosition = 0;
	string current;

	// 空格隔开的为一个Token,符号也为一个Token
	while (ifs.get(c)) {
		text += c;
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			this->wordPositionList.push_back(wordPosition);
			if (current != "") {
				tokens.push_back(Span(current, pos - current.length(), pos, wordPosition, wordPosition));
				wordPosition++;
				current = "";
			}
		}
		else if (isalnum(c)) {
			this->wordPositionList.push_back(wordPosition);
			current += c;
		}
		else {
			if (current != "") {
				tokens.push_back(Span(current, pos - current.length(), pos, wordPosition, wordPosition));
				//pos++;
				//this->wordPositionList.push_back(wordPosition);
				wordPosition++;
				current = "";
			}
			string sign;
			sign += c;
			tokens.push_back(Span(sign, pos, pos + 1, wordPosition, wordPosition));
			this->wordPositionList.push_back(wordPosition);
			wordPosition++;
		}
		pos++;
	}
	if (current != "") {
		tokens.push_back(Span(current, pos - current.length(), pos, wordPosition, wordPosition));
		wordPosition++;
	}
	ifs.close();
}

string Tokenizer::getText() const {
	return this->text;
}

vector<Span> Tokenizer::getTokens() const {
	return this->tokens;
}

int Tokenizer::getWordPosition(int pos) const {
	return this->wordPositionList[pos];
}
