#include <iostream>
#include <fstream>
#include <sstream>
#include "Token.h"
#include <vector>
#include <array>
#include "Parsing.h"
#include <map>
#include <chrono>
#include <memory>
#include <stdexcept>
#include "CodeGen.h"

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
std::vector<SyntaxNode*> parse_arguments(bool use_expression = true);

IntLiteral* integer_node(Token& integer_token) {
	IntLiteral* integer_node = new IntLiteral();
	std::string token_value = tokenizer.get_identifier_name(integer_token);
	integer_node->value = std::stoi(token_value);
	return integer_node;
}

SyntaxNode* float_node(Token& integer_token, Token& fractional_token) {
	std::string float_string = tokenizer.get_identifier_name(integer_token) + "." + tokenizer.get_identifier_name(fractional_token);
	FloatLiteral* float_node = new FloatLiteral();
	float_node->value = std::stof(float_string);
	return float_node;
}

SyntaxNode* string_node(Token& string_token) {
	StringLiteral* string_node = new StringLiteral();
	std::string token_value = tokenizer.get_identifier_name(string_token);
	string_node->value = token_value.substr(1, token_value.length() - 2); // remove the quotation marks
	return string_node;
}

SyntaxNode* parse_subexpression() {
	Token* token = tokenizer.next_token();

	if (token->type == TokenType::IDENTIFIER) {
		Token* next_token = tokenizer.peek_next_token();

		// we have a function call
		if (next_token->type == TokenType::OPEN_PARENTHESIS) {
			ProcedureCall* proc_call = new ProcedureCall();
			proc_call->name = tokenizer.get_identifier_name(*token);
			tokenizer.next_token();
			proc_call->inputs = parse_arguments();
			return proc_call;
		}
		else {
			VariableCall* var_call = new VariableCall();
			var_call->name = tokenizer.get_identifier_name(*token);
			return var_call;
		}
	}

	if (token->type == TokenType::TRUE) {
		BooleanLiteral* bool_literal = new BooleanLiteral();
		bool_literal->value = true;
		return bool_literal;
	}
	if (token->type == TokenType::FALSE) {
		BooleanLiteral* bool_literal = new BooleanLiteral();
		bool_literal->value = false;
		return bool_literal;
	}

	if (token->type == TokenType::INTEGER_LITERAL) {
		// check for floats
		if (tokenizer.peek_next_token()->type == TokenType::DOT) {
			tokenizer.next_token();
			Token* fractional = tokenizer.next_token();
			return float_node(*token, *fractional);
		}
		else {
			return integer_node(*token);
		}
	}

	if (token->type == TokenType::MINUS) {
		std::cout << "sub expression minus" << std::endl;
		SyntaxNode* sub_expression = parse_subexpression();
		if (sub_expression->type == SyntaxNode::Type::INTEGER_LITERAL) {
			IntLiteral* int_literal = (IntLiteral*)sub_expression;
			int_literal->value *= -1;
			return int_literal;
		}
	}

	if (token->type == TokenType::STRING_LITERAL) {
		return string_node(*token);
	}
	return new ParseError("couldn't parse subexpression");
}

SyntaxNode* parse_expression(int priority = -9999) {
	Token* next_token = tokenizer.peek_next_token();

	// check for termination tokens
	if(next_token->type == TokenType::SEMI_COLON || next_token->type == TokenType::COMMA || next_token->type == TokenType::CLOSE_PARENTHESIS) {
		return new ParseError("expression with no value");
	}

	SyntaxNode* sub_expression = parse_subexpression();
	next_token = tokenizer.peek_next_token();

	// check for termination tokens
	if (next_token->type == TokenType::SEMI_COLON || next_token->type == TokenType::COMMA || next_token->type == TokenType::CLOSE_PARENTHESIS) {
		return sub_expression;
	}

	tokenizer.next_token();

	BinaryOperator* binary_operator = new BinaryOperator();
	binary_operator->left = sub_expression;

	if (next_token->type == TokenType::PLUS) {
		binary_operator->operation = BinaryOperator::Type::ADD;
	}
	if (next_token->type == TokenType::STAR) {
		binary_operator->operation = BinaryOperator::Type::MULTIPLY;
	}
	if (next_token->type == TokenType::MINUS) {
		binary_operator->operation = BinaryOperator::Type::SUBTRACT;
	}
	if (next_token->type == TokenType::FORWARD_SLASH) {
		binary_operator->operation = BinaryOperator::Type::DIVIDE;
	}
	if (next_token->type == TokenType::LESS_THAN) {
		binary_operator->operation = BinaryOperator::Type::LESS_THAN;
	}
	if (next_token->type == TokenType::GREATER_THAN) {
		binary_operator->operation = BinaryOperator::Type::GREATER_THAN;
	}
	if (next_token->type == TokenType::DOUBLE_EQUALS) {
		binary_operator->operation = BinaryOperator::Type::EQUAL;
	}

	binary_operator->right = parse_expression();
	return binary_operator;
}

Block* parse_block() {
	Block* block = new Block();

	while (true) {
		Token* next_token = tokenizer.peek_next_token();
		if (next_token->type == TokenType::CLOSE_BRACE) {
			break;
		}
		else if (next_token->type == TokenType::OPEN_BRACE) {
			tokenizer.next_token(); // eat the open brace
			block->statements.push_back(parse_block());
			tokenizer.next_token(); // eat the ending brace
		}
		else {
			block->statements.push_back(parse_statement());
			next_token = tokenizer.next_token();
			if (next_token->type != TokenType::SEMI_COLON && next_token->type != TokenType::CLOSE_BRACE) {
				block->statements.push_back(new ParseError("missing semi colon"));
			}
		}
	}
	return block;
}

std::vector<SyntaxNode*> parse_arguments(bool use_expression) {
	std::vector<SyntaxNode*> arguments;

	Token* next_token = tokenizer.peek_next_token(); // check quickly for immediate close
	if (next_token->type == TokenType::CLOSE_PARENTHESIS) {
		tokenizer.next_token();
		return arguments;	
	}

	while (true) {
		if (use_expression) {
			arguments.push_back(parse_expression());
		}
		else {
			arguments.push_back(parse_statement());
		}

		next_token = tokenizer.next_token();

		if (next_token->type == TokenType::CLOSE_PARENTHESIS) {
			break;
		}
		else if (next_token->type != TokenType::COMMA) {
			arguments.push_back(new ParseError("no comma")); // if its not a closed parenthesis
		}
	}
	return arguments;
}

Procedure* parse_procedure() {
	Token* open_parenthesis = tokenizer.next_token();
	if (open_parenthesis->type != TokenType::OPEN_PARENTHESIS) {
		std::cout << "no open parenthesis ?? wtf" << std::endl;
		return nullptr;
	}

	Procedure* procedure = new Procedure();

	// parse input
	procedure->inputs = parse_arguments(false);
	tokenizer.next_token(); // could change 
	// eventually parse return type in here
	procedure->body = parse_block();
	return procedure;
}

SyntaxNode* parse_statement() {
	Token* start_token = tokenizer.next_token();

	if (start_token->type == TokenType::BACK_ARROW) {
		// parse return statement
		ReturnStatement* return_statement = new ReturnStatement();
		return_statement->expression = parse_expression();
		return return_statement;
	}

	if (start_token->type == TokenType::WHILE) {
		tokenizer.next_token(); // (
		SyntaxNode* condition = parse_arguments()[0]; // using argument parsing for while argument <- WEIRD HACK
		tokenizer.next_token(); // {
		Block* block = parse_block();

		WhileStatement* while_statement = new WhileStatement();
		while_statement->condition = condition;
		while_statement->body = block;
		return while_statement;
	}

	if (start_token->type == TokenType::IF) {
		tokenizer.next_token();
		SyntaxNode* condition = parse_arguments()[0];
		tokenizer.next_token();
		Block* block = parse_block();

		IfStatement* if_statement = new IfStatement();
		if_statement->condition = condition;
		if_statement->body = block;
		return if_statement;
	}

	if (start_token->type != TokenType::IDENTIFIER) {
		return new ParseError("statement doesnt start with an identifier");
	}

	std::string identifier = tokenizer.get_identifier_name(*start_token);

	Token* token = tokenizer.next_token();
	if (token->type == TokenType::COLON) {
		token = tokenizer.next_token();

		if (token->type == TokenType::IDENTIFIER) {
			VariableDecleration* variable_decleration = new VariableDecleration();
			variable_decleration->name = identifier;
			variable_decleration->type_name = tokenizer.get_identifier_name(*token);
			return variable_decleration;
		}
		else if (token->type == TokenType::COLON) {
			ProcedureDecleration* procedure_decleration = new ProcedureDecleration();
			// constant decleration for now we just assume its a function
			procedure_decleration->name = identifier;
			procedure_decleration->procedure = parse_procedure();
			return procedure_decleration;
		}
	}
	else if (token->type == TokenType::EQUALS) {
		VariableAssignment* variable_assignment = new VariableAssignment();
		variable_assignment->name = identifier;
		variable_assignment->value = parse_expression();
		return variable_assignment;
	}
	else if (token->type == TokenType::OPEN_PARENTHESIS) {
		ProcedureCall* procedure_call = new ProcedureCall();
		procedure_call->name = identifier;
		procedure_call->inputs = parse_arguments();
		return procedure_call;
	}
	return nullptr;
}


SyntaxNode* evaluate_block(Block* block);

std::map<std::string, Procedure*> procedures;
std::map<std::string, SyntaxNode*> variables;

SyntaxNode* evaluate_node(SyntaxNode* node) {
	switch (node->type)
	{
	case SyntaxNode::Type::INTEGER_LITERAL:
	case SyntaxNode::Type::FLOAT_LITERAL:
	case SyntaxNode::Type::STRING_LITERAL:
	case SyntaxNode::Type::BOOLEAN_LITERAL:
		return node;

	case SyntaxNode::Type::BLOCK:
	{
		return evaluate_block((Block*)node);
	}
	break;

	case SyntaxNode::Type::WHILE_STATEMENT:
	{
		WhileStatement* while_statement = (WhileStatement*)node;
		while (true) {
			SyntaxNode* condition_eval = evaluate_node(while_statement->condition);
			if (condition_eval->type != SyntaxNode::Type::BOOLEAN_LITERAL) {
				std::cout << "while statement condition is not a boolean expression" << std::endl;
				return nullptr;
			}
			BooleanLiteral* boolean_condition = (BooleanLiteral*)condition_eval;
			if (!boolean_condition->value) {
				break;
			}

			evaluate_block(while_statement->body);
		}
	}
	break;

	case SyntaxNode::Type::IF_STATEMENT:
	{
		IfStatement* if_statement = (IfStatement*)node;
		SyntaxNode* condition_eval = evaluate_node(if_statement->condition);
		if (condition_eval->type != SyntaxNode::Type::BOOLEAN_LITERAL) {
			std::cout << "if statement condition is not a boolean expression" << std::endl;
			return nullptr;
		}
		BooleanLiteral* boolean_condition = (BooleanLiteral*)condition_eval;
		if (boolean_condition->value) {
			evaluate_block(if_statement->body);
		}
	}
	break;

	case SyntaxNode::Type::VARIABLE_CALL:
	{
		VariableCall* var_call = (VariableCall*)node;
		return variables[var_call->name];
	}
	break;

	case SyntaxNode::Type::VARIABLE_DECLERATION:
	{
		VariableDecleration* var_decl = (VariableDecleration*)node;
		variables[var_decl->name] = nullptr;
	}
	break;

	case SyntaxNode::Type::PROCEDURE_DECLERATION:
	{
		ProcedureDecleration* proc_decl = (ProcedureDecleration*)node;
		procedures[proc_decl->name] = proc_decl->procedure;
	}
	break;

	case SyntaxNode::Type::VARIABLE_ASSIGNMENT:
	{
		VariableAssignment* var_assign = (VariableAssignment*)node;
		variables[var_assign->name] = evaluate_node(var_assign->value);
	}
	break;

	case SyntaxNode::Type::PROCEDURE_CALL:
	{
		ProcedureCall* procedure_call = (ProcedureCall*)node;
		if (procedure_call->name == "print") {
			for (auto input : procedure_call->inputs) {
				SyntaxNode* evaluated = evaluate_node(input);
				evaluated->print();
			}
			std::cout << std::endl;
			return nullptr;
		}
		if (procedure_call->name == "time_nano_seconds") {
			int time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			IntLiteral* int_literal = new IntLiteral();
			int_literal->value = time;
			return int_literal;
		}
		else {
			Procedure* procedure = procedures[procedure_call->name];

			if (!procedure) {
				std::cout << "we dont have this procedure!";
				return nullptr;
			}
			
			// adding variables 
			for (int i = 0; i < procedure->inputs.size(); i++) {
				VariableDecleration* input_decl = (VariableDecleration*)procedure->inputs[i];
				SyntaxNode* input_value = procedure_call->inputs[i];
				variables[input_decl->name] = evaluate_node(input_value);
			}

			SyntaxNode* result = evaluate_node(procedure->body);
			return result;
		}
	}
	break;

	case SyntaxNode::Type::BINARY_OPERATOR:
	{
		BinaryOperator* binary_operator = (BinaryOperator*)node;
		
		SyntaxNode* left = evaluate_node(binary_operator->left);
		SyntaxNode* right = evaluate_node(binary_operator->right);

		if (left->type == SyntaxNode::Type::INTEGER_LITERAL && right->type == SyntaxNode::Type::INTEGER_LITERAL) {
			
			IntLiteral* left_int = (IntLiteral*)left;
			IntLiteral* right_int = (IntLiteral*)right;


			BooleanLiteral* bool_literal = new BooleanLiteral();
			IntLiteral* int_literal = new IntLiteral();

			switch (binary_operator->operation)
			{
			case BinaryOperator::Type::ADD:
				int_literal->value = left_int->value + right_int->value;
				break;
			case BinaryOperator::Type::MULTIPLY:
				int_literal->value = left_int->value * right_int->value;
				break;
			case BinaryOperator::Type::SUBTRACT:
				int_literal->value = left_int->value - right_int->value;
				break;
			case BinaryOperator::Type::DIVIDE:
				int_literal->value = left_int->value / right_int->value;
				break;

			// BOOLEAN
			case BinaryOperator::Type::LESS_THAN:
				bool_literal->value = left_int->value < right_int->value;
				return bool_literal;
			case BinaryOperator::Type::GREATER_THAN:
				bool_literal->value = left_int->value > right_int->value;
				return bool_literal;
			case BinaryOperator::Type::EQUAL:
				bool_literal->value = left_int->value == right_int->value;
				return bool_literal;
			default:
				break;
			}
			return int_literal;
		}
		
	}
	break;

	default:
		break;
	}
	return nullptr;
}

SyntaxNode* evaluate_block(Block* block) {
	for (SyntaxNode* statement : block->statements) {
		if (statement->type == SyntaxNode::Type::RETURN_STATEMENT) {
			ReturnStatement* return_statement = (ReturnStatement*)statement;
			return evaluate_node(return_statement->expression);
		}
		else {
			evaluate_node(statement);
		}
	}
	return nullptr;
}

int generated_name_counter = 0;
std::string get_generated_name() {
	std::string name = string_format("generated_ident_%d", generated_name_counter);
	generated_name_counter++;
	return name;
}

SyntaxNode* generate_link(SyntaxNode* expression, std::vector<SyntaxNode*>& generated_statements){
	// make into var assign and usage
	VariableDecleration* decl = new VariableDecleration();
	decl->name = get_generated_name();
	decl->type_name = "int";
	generated_statements.push_back(decl);

	VariableAssignment* assignment = new VariableAssignment();
	assignment->name = decl->name;
	assignment->value = expression;
	generated_statements.push_back(assignment);

	VariableCall* var_call = new VariableCall();
	var_call->name = decl->name;
	return var_call;
}

// returns the new identifier (variable call)
// appends to list of new statements if necessary
SyntaxNode* flatten_expression(SyntaxNode* expression, std::vector<SyntaxNode*>& generated_statements, bool top_level = false) {
	if (expression->type == SyntaxNode::Type::PROCEDURE_CALL) {
		ProcedureCall* procedure_call = (ProcedureCall*)expression;
		for (int i = 0; i < procedure_call->inputs.size(); i++) {
			SyntaxNode* input_expression = procedure_call->inputs[i];
			input_expression = flatten_expression(input_expression, generated_statements);
			if (!is_literal(input_expression)) {
				input_expression = generate_link(input_expression, generated_statements);
			}
			procedure_call->inputs[i] = input_expression;
		}
		if (top_level) {
			return procedure_call;
		}
		return generate_link(procedure_call, generated_statements);
	}

	if (expression->type == SyntaxNode::Type::BINARY_OPERATOR) {
		BinaryOperator* binary_operator = (BinaryOperator*)expression;
		binary_operator->left = flatten_expression(binary_operator->left, generated_statements);
		binary_operator->right = flatten_expression(binary_operator->right, generated_statements);
		if (!top_level) {
			return generate_link(binary_operator, generated_statements);
		}
	}
	return expression;
}

void flatten(Block* block) {
	std::vector<SyntaxNode*> modified_statements;
	for (int i = 0; i < block->statements.size(); i++) {

		SyntaxNode* statement = block->statements[i];
		if (statement->type == SyntaxNode::Type::VARIABLE_ASSIGNMENT) {
			VariableAssignment* assignment = (VariableAssignment*)statement;

			assignment->value = flatten_expression(assignment->value, modified_statements, true);
		}
		if (statement->type == SyntaxNode::Type::RETURN_STATEMENT) {
			ReturnStatement* return_statement = (ReturnStatement*)statement;
			return_statement->expression = flatten_expression(return_statement->expression, modified_statements, true);
		}
		if (statement->type == SyntaxNode::Type::PROCEDURE_DECLERATION) {
			ProcedureDecleration* proc_decl = (ProcedureDecleration*)statement;
			flatten(proc_decl->procedure->body);
		}
		if (statement->type == SyntaxNode::Type::WHILE_STATEMENT) {
			WhileStatement* while_statement = (WhileStatement*)statement;
			while_statement->condition = flatten_expression(while_statement->condition, modified_statements, true);
			flatten(while_statement->body);
		}
		if (statement->type == SyntaxNode::Type::IF_STATEMENT) {
			IfStatement* if_statement = (IfStatement*)statement;
			if_statement->condition = flatten_expression(if_statement->condition, modified_statements, true);
			flatten(if_statement->body);
		}
		if (statement->type == SyntaxNode::Type::PROCEDURE_CALL) {
			flatten_expression(statement, modified_statements, true);
		}
		modified_statements.push_back(statement);
	}
	block->statements = modified_statements;
}

int main(int argc, char** argv) {
	char* program_file = argv[1];
	
	bool run = false;
	if (argc > 2) {
		std::string run_arg = argv[2];
		if (run_arg == "-run") {
			run = true;
		}
	}

	std::string program_text = get_file_contents_as_text(program_file);
	std::vector<Token> tokens;

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

	
		// parse comments
		if (text_char == '/' && (text_index + 1) < program_text.size() && program_text[text_index + 1] == '/') {
			Token comment;
			comment.start_index = text_index;
			for (int i = text_index + 2; i < program_text.size(); i++) {
				char next_char = program_text[i];
				if (next_char == '\n') {
					comment.end_index = i + 1;
					comment.type = TokenType::COMMENT;

					tokens.push_back(comment);
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

	
	// print_tokens(std::cout, tokens, program_text);
	// std::cout << "\n";

	tokenizer.program_text = program_text;
	tokenizer.tokens = &tokens;
	tokenizer.index = 0;
	Block* block = parse_block();
	//evaluate_block(block);
	flatten(block);
	// run program
	//evaluate_block(procedures["main"]->body);
	std::string filename = program_file;
	const std::string extension = ".graph";
	filename = filename.substr(0, filename.size() - extension.size());
	compile(block, filename, run);
}