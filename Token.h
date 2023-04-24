#pragma once

#include <string>
#include <utility>
#include <vector>

enum class TokenType {

	//Operators in reverse order of presidence
	PLUS,
	MINUS,
	FORWARD_SLASH,
	STAR,
	LESS_THAN,
	GREATER_THAN,
	LESS_THAN_EQUAL,
	GREATER_THAN_EQUAL,

	SEMI_COLON,
	IDENTIFIER,
	EQUALS,
	DOUBLE_EQUALS,
	SPACE,
	COMMA,
	DOT,
	OPEN_BRACE,
	CLOSE_BRACE,
	COLON,
	NEWLINE,
	TAB,
	OPEN_PARENTHESIS,
	CLOSE_PARENTHESIS,
	FORWARD_ARROW,
	BACK_ARROW,
	WHILE,
	IF,

	TRUE,
	FALSE,

	// literals
	STRING_LITERAL,
	INTEGER_LITERAL,
	FLOAT_LITERAL,

	COMMENT
};

std::string get_token_name(TokenType type) {
	switch (type)
	{
	case TokenType::IDENTIFIER:
		return "IDENTIFIER";
	case TokenType::EQUALS:
		return "EQUALS";
	case TokenType::SPACE:
		return "SPACE";
	case TokenType::COMMA:
		return "COMMA";
	case TokenType::DOT:
		return "DOT";
	case TokenType::OPEN_BRACE:
		return "OPEN BRACE";
	case TokenType::CLOSE_BRACE:
		return "CLOSE BRACE";
	case TokenType::COLON:
		return "COLON";
	case TokenType::NEWLINE:
		return "NEWLINE";
	case TokenType::TAB:
		return "TAB";
	case TokenType::OPEN_PARENTHESIS:
		return "OPEN PARENTHESIS";
	case TokenType::CLOSE_PARENTHESIS:
		return "CLOSE PARENTHESIS";
	case TokenType::FORWARD_ARROW:
		return "FORWARD ARROW";
	case TokenType::BACK_ARROW:
		return "BACK ARROW";
	case TokenType::PLUS:
		return "PLUS";
	case TokenType::MINUS:
		return "MINUS";
	case TokenType::SEMI_COLON:
		return "SEMI COLON";
	default:
		break;
	}
};

std::vector<std::pair<const std::string, TokenType>> token_map{
	std::make_pair("true", TokenType::TRUE),
	std::make_pair("false", TokenType::FALSE),
	std::make_pair("while", TokenType::WHILE),
	std::make_pair("if", TokenType::IF),
	std::make_pair("->", TokenType::FORWARD_ARROW),
	std::make_pair("<-", TokenType::BACK_ARROW),
	std::make_pair("*", TokenType::STAR),
	std::make_pair("+", TokenType::PLUS),
	std::make_pair("-", TokenType::MINUS),
	std::make_pair("/", TokenType::FORWARD_SLASH),
	std::make_pair("<", TokenType::LESS_THAN),
	std::make_pair(">", TokenType::GREATER_THAN),
	std::make_pair(">=", TokenType::GREATER_THAN_EQUAL),
	std::make_pair("<=", TokenType::LESS_THAN_EQUAL),
	std::make_pair("==", TokenType::DOUBLE_EQUALS),
	std::make_pair("=", TokenType::EQUALS),
	std::make_pair(".", TokenType::DOT),
	std::make_pair(" ", TokenType::SPACE),
	std::make_pair(",", TokenType::COMMA),
	std::make_pair("{", TokenType::OPEN_BRACE),
	std::make_pair("}", TokenType::CLOSE_BRACE),
	std::make_pair(":", TokenType::COLON),
	std::make_pair("\n", TokenType::NEWLINE),
	std::make_pair("\t", TokenType::TAB),
	std::make_pair("(", TokenType::OPEN_PARENTHESIS),
	std::make_pair(")", TokenType::CLOSE_PARENTHESIS),
	std::make_pair(";", TokenType::SEMI_COLON)
};

struct Token {
	TokenType type;
	int start_index;
	int end_index;
};

bool is_whitespace(Token& token) {
	return token.type == TokenType::NEWLINE || token.type == TokenType::TAB || token.type == TokenType::SPACE || token.type == TokenType::COMMENT;
};