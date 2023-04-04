#pragma once

#include <string>
#include <map>

enum class TokenType {
	SEMI_COLON,
	IDENTIFIER,
	EQUALS,
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
	PLUS,
	MINUS,
	FORWARD_ARROW,
	BACK_ARROW,
	NUMBER
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
	case TokenType::NUMBER:
		return "NUMBER";
	case TokenType::SEMI_COLON:
		return "SEMI COLON";
	default:
		break;
	}
}

std::map<const std::string, TokenType> token_map = {
	{"->", TokenType::FORWARD_ARROW},
	{"<-", TokenType::BACK_ARROW},
	{"+", TokenType::PLUS},
	{"-", TokenType::MINUS},
	{"=", TokenType::EQUALS},
	{".", TokenType::DOT},
	{" ", TokenType::SPACE},
	{",", TokenType::COMMA},
	{"{", TokenType::OPEN_BRACE},
	{"}", TokenType::CLOSE_BRACE},
	{":", TokenType::COLON},
	{"\n", TokenType::NEWLINE},
	{"\t", TokenType::TAB},
	{"(", TokenType::OPEN_PARENTHESIS},
	{")", TokenType::CLOSE_PARENTHESIS},
	{";", TokenType::SEMI_COLON}
};

struct Token {
	TokenType type;
	int start_index;
	int end_index;
};

bool is_whitespace(Token& token) {
	return token.type == TokenType::NEWLINE || token.type == TokenType::TAB || token.type == TokenType::SPACE;
}