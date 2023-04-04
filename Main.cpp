#include <iostream>
#include <fstream>
#include <sstream>
#include "Token.h"
#include <vector>

std::string get_file_contents_as_text(const std::string filename) {
	std::ifstream file_stream(filename);
	std::stringstream string_stream;
	string_stream << file_stream.rdbuf();
	std::string contents = string_stream.str();
	return contents;
}

std::string text_from_tokens(std::vector<Token>& tokens, std::string& program_text) {
	std::string text = "";
	for (Token& token : tokens) {
		if (token.type == TokenType::IDENTIFIER) {
			for (int i = 0; i < (token.end_index - token.start_index); i++) {
				text += "?";
			}
		}
		else {
			text += program_text.substr(token.start_index, token.end_index - token.start_index);
		}
	}
	return text;
}

bool is_number(std::string text) {
	const int zero = int('0');
	const int nine = int('9');

	for (char c : text) {
		if (c < zero || c > nine) {
			return false;
		}
	}
	return true;
}

enum class NodeType {

	// Literals
	INTEGER_LITERAL,

	// OPERATORS
	MULTIPLY,
	ADD,

	FUNCTION_CALL,
	PARSE_ERROR,
	BLOCK,
	SEQUENCE,

	// statements
	VARIABLE_DECLERATION,
	VARIABLE_ASSIGNMENT,
	FUNCTION_DECLARATION,

	IDENTIFIER
};

// PARSING
struct SyntaxNode {
	NodeType type;
	SyntaxNode* left;
	SyntaxNode* right;

	// literals
	std::string identifier_value;
	int int_value;
};

struct Tokenizer {
	std::vector<Token>* tokens;
	std::string program_text;
	int index = 0;

	/// skips punctuation
	Token* next_token() {
		if (index >= tokens->size())return nullptr;
		while (true) {
			Token* token = &(*tokens)[index];
			index += 1;
			if (is_whitespace(*token))continue;
			return token;
		}
	}
	Token* peek_next_token() {
		if (index >= tokens->size()) {
			return nullptr;
		}
		int temp_index = index;
		while (true) {
			Token* token = &(*tokens)[temp_index];
			temp_index += 1;
			if (is_whitespace(*token))continue;
			return token;
		}
	}
	std::string get_identifier_name(Token& identifier) {
		return program_text.substr(identifier.start_index, identifier.end_index - identifier.start_index);
	}
};

Tokenizer tokenizer;
SyntaxNode* parse_statement();

SyntaxNode* identifier_node(Token& identifier_token) {
	SyntaxNode* identifier_node = new SyntaxNode();
	identifier_node->type = NodeType::IDENTIFIER;
	identifier_node->identifier_value = tokenizer.get_identifier_name(identifier_token);
	return identifier_node;
}

SyntaxNode* parse_expression() {
	SyntaxNode* expression = new SyntaxNode();
	Token* integer = tokenizer.next_token(); // just eat one token to fix the parsing for now
	if (integer->type != TokenType::IDENTIFIER) {
		std::cout << "theres an issue" << std::endl;
	}
	expression->type = NodeType::INTEGER_LITERAL;
	expression->identifier_value = tokenizer.get_identifier_name(*integer);
	return expression;
}

SyntaxNode* parse_block() {
	SyntaxNode* node = new SyntaxNode();
	node->type = NodeType::BLOCK;

	SyntaxNode* current_node = node;
	while (true) {
		Token* next_token = tokenizer.peek_next_token();
		if (next_token->type == TokenType::CLOSE_BRACE) {
			tokenizer.next_token(); // eat the close brace
			break;
		}

		current_node->left = parse_statement();
		SyntaxNode* sequence = new SyntaxNode();
		sequence->type = NodeType::SEQUENCE;
		current_node->right = sequence;
		current_node = sequence;
	}
	return node;
}

SyntaxNode* parse_arguments() {
	SyntaxNode* node = new SyntaxNode();
	node->type = NodeType::SEQUENCE;
	while (true) {
		Token* next_token = tokenizer.peek_next_token();
		if (next_token->type == TokenType::CLOSE_PARENTHESIS) {
			tokenizer.next_token();
			break;
		}
		node->left = parse_expression();
		next_token = tokenizer.next_token();
		if (next_token->type != TokenType::COMMA)break;
	}
	return node;
}

SyntaxNode* parse_statement() {
	Token& start_token = *(tokenizer.next_token());
	if (start_token.type == TokenType::OPEN_BRACE) {
		return parse_block();
	}
	if (start_token.type != TokenType::IDENTIFIER) {
		SyntaxNode* parse_error = new SyntaxNode();
		parse_error->type = NodeType::PARSE_ERROR;
		return parse_error;
	}

	SyntaxNode* statement = new SyntaxNode();
	statement->left = identifier_node(start_token);

	Token* token = tokenizer.next_token();
	if (token->type == TokenType::COLON) {
		statement->type = NodeType::VARIABLE_DECLERATION;

		token = tokenizer.next_token();
		if (token->type == TokenType::IDENTIFIER) {
			statement->right = identifier_node(*token);
		}
	}
	else if (token->type == TokenType::EQUALS) {
		statement->type = NodeType::VARIABLE_ASSIGNMENT;
		statement->right = parse_expression();
	}
	else if(token->type == TokenType::OPEN_PARENTHESIS) {
		statement->type = NodeType::FUNCTION_CALL;
		statement->right = parse_arguments();
	}

	Token* termination = tokenizer.next_token();
	if (termination->type != TokenType::SEMI_COLON) {
		statement->type = NodeType::PARSE_ERROR;
	}

	return statement;
}

int main(int argc, char* argv) {
	std::string program_text = get_file_contents_as_text("main.gra");

	std::cout << "parsing started" << std::endl;
	std::vector<Token>* tokens = new std::vector<Token>();

	Token start_brace;
	start_brace.type = TokenType::OPEN_BRACE;
	tokens->push_back(start_brace);


	int text_index = 0;
	int identifier_chars = 0;

	while (text_index < program_text.size()) {

		bool found_token = false;

		for (auto const& entry : token_map) {
			int matched_chars = 0;
			int start_index = text_index;

			const std::string token_string = entry.first;

			for (int i = 0; i < token_string.size(); i++) {
				char token_char = token_string[matched_chars];
				char program_char = program_text[start_index + matched_chars];
				if (token_char == program_char) {
					matched_chars++;
				}
				else {
					break;
				}
			}

			if (matched_chars != token_string.size()) {
				continue; // try next token
			}

			if (identifier_chars > 0) {
				Token identifier;
				identifier.end_index = text_index;
				identifier.start_index = text_index - identifier_chars;
				identifier_chars = 0;
				identifier.type = TokenType::IDENTIFIER;
				tokens->push_back(identifier);
			}


			Token token;
			token.start_index = text_index;
			token.end_index = text_index + matched_chars;
			token.type = entry.second;
			tokens->push_back(token);
			found_token = true;
			text_index += matched_chars; // shunt the text index along
			break;
		}

		if (!found_token) {
			identifier_chars++;
			text_index++;
		}
	}

	Token end_brace;
	end_brace.type = TokenType::CLOSE_BRACE;
	tokens->push_back(end_brace);

	tokenizer.program_text = program_text;
	tokenizer.tokens = tokens;
	tokenizer.index = 0;
	SyntaxNode* parse_tree = parse_statement();

	std::cout << "parsing end" << std::endl;
}

