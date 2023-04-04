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

struct StringView {
	const char* start;
	int length;

	bool operator == (StringView& other) {
		if (length != other.length)return false;

		for (int i = 0; i < length; i++) {
			char a = *(start + i);
			char b = *(other.start + i);

			if (a != b) {
				return false;
			}
		}
		return true;
	}

	void print(std::ostream& stream) const {
		for (int i = 0; i < length; i++) {
			char c = *(start + i);
			stream << c;
		}
	}
};

StringView get_string_view(const Token& token, const std::string& program_text) {
	return StringView{ &(program_text[token.start_index]), token.end_index - token.start_index };
}

std::ostream& operator <<(std::ostream& stream, const StringView view) {
	view.print(stream);
	return stream;
}

void print_tokens(std::ostream& stream, std::vector<Token>& tokens, std::string& program_text) {
	for (Token& token : tokens) {
		if (token.type == TokenType::IDENTIFIER) {
			for (int i = 0; i < (token.end_index - token.start_index); i++) {
				stream << "?";
			}
		}
		else {
			if (token.start_index > 0 && token.end_index < program_text.size())
				stream << get_string_view(token, program_text);
		}
	}
}

bool is_number(const char c) {
	const int zero = int('0');
	const int nine = int('9');
	return c >= zero && c <= nine;
}

bool is_number(StringView& string_view) {
	for (int i = 0; i < string_view.length; i++) {
		char c = *(string_view.start + i);
		if (!is_number(c))return false;
	}
	return true;
}

void comp() {
	const std::string source_a = "hello world";
	const std::string source_b = "welcome to hell";

	StringView view_a{&source_a[0], 4};
	StringView view_b{&source_b[11], 4};

	std::cout << "hello world " << view_a << " " << view_b << std::endl;

	if (view_a == view_b) {
		std::cout << "views are equal" << std::endl;
	}
	else {
		std::cout << "views are not equal" << std::endl;
	}
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
		std::cout << "theres an issue";
		if (integer->type == TokenType::STRING_LITERAL) {
			std::cout << " ...actually dw its a string";
		}
		std::cout << std::endl;
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

#if 1
int main(int argc, char* argv) {
	std::string program_text = get_file_contents_as_text("main.gra");

	std::cout << "parsing started" << std::endl;
	std::vector<Token>* tokens = new std::vector<Token>();

	Token start_brace;
	start_brace.type = TokenType::OPEN_BRACE;
	tokens->push_back(start_brace);


	int text_index = 0;
	int last_token_index = 0;

	while (text_index < program_text.size()) {

		bool found_token = false;

		// parse strings
		char text_char = program_text[text_index];
		if (text_char == '"') {
			Token string_literal;
			string_literal.start_index = text_index;
			for (int i = text_index + 1; i < program_text.size(); i++) {
				char next_char = program_text[i];
				if (next_char == '"') {
					string_literal.end_index = i + 1;
					string_literal.type = TokenType::STRING_LITERAL;

					tokens->push_back(string_literal);
					text_index = i + 1;
					last_token_index = text_index;
					found_token = true;
				}
			}
		}
		if (found_token)continue;

		// parse fixed tokens
		for (auto const& entry : token_map) {

			const std::string token_string = entry.first;
			StringView token_view{&token_string[0], token_string.size()};
			StringView program_view{&program_text[text_index], token_string.size()};

			// check if it matches the current token
			if (token_view == program_view) {

				// if we've had a gap in between tokens, it must be an identifier
				if (text_index > last_token_index) {
					Token identifier;
					identifier.end_index = text_index;
					identifier.start_index = last_token_index;
					identifier.type = TokenType::IDENTIFIER;
					tokens->push_back(identifier);
				}

				Token token;
				token.start_index = text_index;
				token.end_index = text_index + token_string.size();
				token.type = entry.second;
				tokens->push_back(token);

				text_index += token_string.size();
				last_token_index = text_index;
				found_token = true;
				break;
			}
		}
		if (found_token)continue;

		text_index++; // if we havent found a token, just start from the next character
	}

	for (Token& token : *tokens) {
		if (token.type == TokenType::IDENTIFIER) {
			StringView token_string = get_string_view(token, program_text);
			if (is_number(token_string)) {
				token.type = TokenType::INT_LITERAL;
			}
		}
	}

	for (Token& token : *tokens) {

	}

	Token end_brace;
	end_brace.type = TokenType::CLOSE_BRACE;
	tokens->push_back(end_brace);

	print_tokens(std::cout, *tokens, program_text);

	tokenizer.program_text = program_text;
	tokenizer.tokens = tokens;
	tokenizer.index = 0;
	SyntaxNode* parse_tree = parse_statement();

	std::cout << "parsing end" << std::endl;

}
#endif // 0

#ifdef DEBUG
int main(int argc, char* argv) {
	comp();
}
#endif // DEBUG