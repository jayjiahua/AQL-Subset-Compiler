/*
  Token.h
	用于保存aql语句的token
*/

#ifndef TOKEN_H
#define TOKEN_H
#include <string>
using std::string;

// aql的Token的类型
namespace AQLToken {
	enum Tag {
		CREATE,
		VIEW,
		AS,
		OUTPUT,
		SELECT,
		FROM,
		EXTRACT,
		REGEX,
		ON,
		RETURN,
		GROUP,
		AND,
		TOKEN,
		PATTERN,
		ID,
		DOT,
		REG,
		NUM,
		LEFT_PARENTHESIS, // 圆括号
		RIGHT_PARENTHESIS,
		LEFT_ANGLE_BACKET, // 尖括号
		RIGHT_ANGLE_BACKET,
		LEFT_BRACE, // 花括号
		RIGHT_BRACE,
		COMMA,
		SEMICOLON,
		END
	};
}

/*
  Token有两种属性, 类型和值
*/
class Token {
public:
	Token();
	Token(AQLToken::Tag type, string value);
	string getValue() const;
	AQLToken::Tag getType() const;
private:
	AQLToken::Tag type;
	string value;
};

#endif
