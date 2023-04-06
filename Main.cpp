#include <iostream>
#include <fstream>
#include <sstream>
#include "Token.h"
#include <vector>
#include <array>

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
	STRING_LITERAL,

	// OPERATORS
	MULTIPLY,
	ADD,
	SUBTRACT,
	DIVIDE,

	PROCEDURE_CALL,
	PARSE_ERROR,
	BLOCK,
	SEQUENCE,

	// statements
	VARIABLE_DECLERATION,
	VARIABLE_ASSIGNMENT,
	PROCEDURE_DECLERATION,

	PROCEDURE_HEADER,

	IDENTIFIER
};

// PARSING
struct SyntaxNode {
	NodeType type;
	SyntaxNode* left;
	SyntaxNode* right;

	// literals
	std::string string_value; // reused for identifier value
	int int_value;
	float float_value;
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
SyntaxNode* parse_statement(TokenType termination_type = TokenType::SEMI_COLON);
SyntaxNode* parse_arguments();

SyntaxNode* identifier_node(Token& identifier_token) {
	SyntaxNode* identifier_node = new SyntaxNode();
	identifier_node->type = NodeType::IDENTIFIER;
	identifier_node->string_value = tokenizer.get_identifier_name(identifier_token);
	return identifier_node;
}

SyntaxNode* integer_node(Token& integer_token) {
	SyntaxNode* integer_node = new SyntaxNode();
	integer_node->type = NodeType::INTEGER_LITERAL;

	std::string token_value = tokenizer.get_identifier_name(integer_token);
	integer_node->int_value = std::stoi(token_value);
	return integer_node;
}

SyntaxNode* string_node(Token& string_token) {
	SyntaxNode* string_node = new SyntaxNode();
	string_node->type = NodeType::STRING_LITERAL;

	std::string token_value = tokenizer.get_identifier_name(string_token);
	string_node->string_value = token_value.substr(1, token_value.length() - 2); // remove the quotation marks

	return string_node;
}

SyntaxNode* parse_error() {
	SyntaxNode* error_node = new SyntaxNode();
	error_node->type = NodeType::PARSE_ERROR;
	return error_node;
}

SyntaxNode* parse_subexpression() {
	Token* token = tokenizer.next_token();
	SyntaxNode* node = parse_error();

	if (token->type == TokenType::IDENTIFIER) {
		Token* next_token = tokenizer.peek_next_token();

		// we have a function call
		if (next_token->type == TokenType::OPEN_PARENTHESIS) {
			node->type = NodeType::PROCEDURE_CALL;
			node->left = identifier_node(*token);
			tokenizer.next_token();
			node->right = parse_arguments();
		}
		else {
			node = identifier_node(*token);
		}
	}
	if (token->type == TokenType::INTEGER_LITERAL) {
		node = integer_node(*token);
	}
	if (token->type == TokenType::STRING_LITERAL) {
		node = string_node(*token);
	}
	return node;
}

SyntaxNode* parse_expression(int priority = -9999) {
	SyntaxNode* expression = new SyntaxNode();
	Token* next_token = tokenizer.peek_next_token();

	// check for termination tokens
	if(next_token->type == TokenType::SEMI_COLON || next_token->type == TokenType::COMMA || next_token->type == TokenType::CLOSE_PARENTHESIS) {
		return nullptr;
	}

	expression->left = parse_subexpression();
	next_token = tokenizer.peek_next_token();

	// check for termination tokens
	if (next_token->type == TokenType::SEMI_COLON || next_token->type == TokenType::COMMA || next_token->type == TokenType::CLOSE_PARENTHESIS) {
		return expression->left;
	}
	tokenizer.next_token();

	if (next_token->type == TokenType::PLUS) {
		expression->type = NodeType::ADD;
		expression->right = parse_expression();
	}
	if (next_token->type == TokenType::STAR) {
		expression->type = NodeType::MULTIPLY;
		expression->right = parse_expression();
	}
	if (next_token->type == TokenType::MINUS) {
		expression->type = NodeType::SUBTRACT;
		expression->right = parse_expression();
	}
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

	Token* next_token = tokenizer.peek_next_token(); // check quickly for immediate close
	if (next_token->type == TokenType::CLOSE_PARENTHESIS) {
		tokenizer.next_token();
		return node;
	}

	SyntaxNode* current_node = node;
	while (true) {
		current_node->left = parse_expression();
		next_token = tokenizer.next_token();
		current_node->right = new SyntaxNode();
		current_node->right->type = NodeType::SEQUENCE;

		if (next_token->type == TokenType::CLOSE_PARENTHESIS) {
			break;
		}
		else if (next_token->type != TokenType::COMMA) {
			return parse_error(); // if its not a closed parenthesis
		}
		current_node = current_node->right;
	}
	return node;
}

SyntaxNode* parse_procedure() {
	Token* open_parenthesis = tokenizer.next_token();
	if (open_parenthesis->type != TokenType::OPEN_PARENTHESIS) {
		return parse_error();
	}

	SyntaxNode* procedure = new SyntaxNode();
	procedure->type = NodeType::PROCEDURE_DECLERATION;

	// parse header
	SyntaxNode* header = new SyntaxNode();
	header->type = NodeType::PROCEDURE_HEADER;
	procedure->left = header;

	// parse input
	while(true){
		break;
	}

	// parse output

	// parse body
	return procedure;
}

SyntaxNode* parse_statement(TokenType termination_type) {
	Token& start_token = *(tokenizer.next_token());
	if (start_token.type == TokenType::OPEN_BRACE) {
		return parse_block();
	}
	if (start_token.type != TokenType::IDENTIFIER) {
		return parse_error();
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
		else if (token->type == TokenType::COLON) {
			// constant decleration for now we just assume its a function
			statement->right = parse_procedure();
			return statement;
		}
	}
	else if (token->type == TokenType::EQUALS) {
		statement->type = NodeType::VARIABLE_ASSIGNMENT;
		statement->right = parse_expression();
	}
	else if(token->type == TokenType::OPEN_PARENTHESIS) {
		statement->type = NodeType::PROCEDURE_CALL;
		statement->right = parse_arguments();
	}

	Token* termination = tokenizer.next_token();
	if (termination->type != termination_type) {
		statement->type = NodeType::PARSE_ERROR;
	}

	return statement;
}

std::string node_to_string(SyntaxNode* node) {
	switch (node->type)
	{
	case NodeType::MULTIPLY:
		return "*";
	case NodeType::ADD:
		return "+";
	case NodeType::SUBTRACT:
		return "-";
	case NodeType::PROCEDURE_CALL:
		return "proc call";
	case NodeType::IDENTIFIER:
		return node->string_value;
	case NodeType::STRING_LITERAL:
		return "\"" + node->string_value + "\"";
	case NodeType::INTEGER_LITERAL:
		return std::to_string(node->int_value);
	case NodeType::BLOCK:
		return "block";
	case NodeType::SEQUENCE:
		return " ";
	case NodeType::VARIABLE_DECLERATION:
		return "var decl";
	case NodeType::VARIABLE_ASSIGNMENT:
		return "var assign";
	case NodeType::PARSE_ERROR:
		return "ERROR";
	default:
		return "?";
	}
}

std::string printBT(const std::string& prefix, SyntaxNode* node, bool isLeft)
{
	if (node != nullptr)
	{
		std::string output = "";
		output += prefix;

		output += (isLeft ? "|-  ": "L-  ");

		// print the value of the node
		output += node_to_string(node) += "\n";

		// enter the next tree level - left and right branch
		output += printBT(prefix + (isLeft ? "|   " : "    "), node->left, true);
		output += printBT(prefix + (isLeft ? "|   " : "    "), node->right, false);
		return output;
	}
	return "";
}

#if 1
int main(int argc, char* argv) {
	std::string program_text = get_file_contents_as_text("main.gra");
	std::vector<Token> tokens;

	Token start_brace;
	start_brace.type = TokenType::OPEN_BRACE;
	tokens.push_back(start_brace);


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

					tokens.push_back(string_literal);
					text_index = i + 1;
					last_token_index = text_index;
					found_token = true;
					break;
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

				// if we've had a gap in between tokens, it must be an identifier, or an int literal
				if (text_index > last_token_index) {
					Token identifier;
					identifier.end_index = text_index;
					identifier.start_index = last_token_index;
					identifier.type = TokenType::IDENTIFIER;

					// check if its actually an int literal
					StringView token_view = get_string_view(identifier, program_text);
					if(is_number(token_view)){
						identifier.type = TokenType::INTEGER_LITERAL;
					}

					tokens.push_back(identifier);
				}

				Token token;
				token.start_index = text_index;
				token.end_index = text_index + token_string.size();
				token.type = entry.second;
				tokens.push_back(token);

				text_index += token_string.size();
				last_token_index = text_index;
				found_token = true;
				break;
			}
		}
		if (found_token)continue;

		text_index++; // if we havent found a token, just start from the next character
	}

	Token end_brace;
	end_brace.type = TokenType::CLOSE_BRACE;
	tokens.push_back(end_brace);

	print_tokens(std::cout, tokens, program_text);
	std::cout << "\n";

	tokenizer.program_text = program_text;
	tokenizer.tokens = &tokens;
	tokenizer.index = 0;
	SyntaxNode* parse_tree = parse_statement();

	std::string output = printBT("", parse_tree, false);
	std::cout << output << std::endl;
}
#endif // 0

#if 0
int main(int argc, char* argv) {
	TreeBlock block = print_parse_tree(0, true);
	block.print(std::cout);
}
#endif // DEBUG