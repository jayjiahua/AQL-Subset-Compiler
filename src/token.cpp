#include "token.h"

Token::Token() {}

Token::Token(AQLToken::Tag type, string value) {
	this->type = type;
	this->value = value;
}

string Token::getValue() const {
	return this->value;
}

AQLToken::Tag Token::getType() const {
	return this->type;
}
