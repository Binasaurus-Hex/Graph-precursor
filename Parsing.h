#pragma once
#include <string>
#include <vector>

struct SyntaxNode {

	enum class Type {
		// Literals
		INTEGER_LITERAL,
		STRING_LITERAL,
		FLOAT_LITERAL,
		BOOLEAN_LITERAL,

		//
		PROCEDURE_CALL,
		VARIABLE_CALL,
		PARSE_ERROR,
		BLOCK,
		BINARY_OPERATOR,
		RETURN_STATEMENT,

		// statements
		VARIABLE_DECLERATION,
		PROCEDURE_DECLERATION,
		VARIABLE_ASSIGNMENT,
		PROCEDURE,

		WHILE_STATEMENT,
		IF_STATEMENT
	};

	SyntaxNode::Type type;
	virtual void print() {
	}
};

struct Block : SyntaxNode {
	Block() { type = Type::BLOCK; }
	std::vector<SyntaxNode*> statements;

	void print() {
		for (SyntaxNode* statement : statements) {
			if (!statement) {
				std::cout << "null value" << std::endl;
			}
			else {
				statement->print();
			}
		}
	}
};

struct IntLiteral : SyntaxNode {

	IntLiteral() { type = Type::INTEGER_LITERAL; }
	int value;

	void print() {
		std::cout << value;
	}
};

struct BooleanLiteral : SyntaxNode {
	BooleanLiteral() { type = Type::BOOLEAN_LITERAL; }
	bool value;

	void print() {
		std::cout << value;
	}
};

struct ParseError : SyntaxNode {
	ParseError(const char* e) {
		error = e;
		type = Type::PARSE_ERROR;
	}
	const char* error;
	void print() {
		std::cout << error;
	}
};

struct FloatLiteral : SyntaxNode {
	FloatLiteral() { type = Type::FLOAT_LITERAL; }
	float value;

	void print() {
		std::cout << value;
	}
};

struct StringLiteral : SyntaxNode {
	StringLiteral() { type = Type::STRING_LITERAL; }
	std::string value;

	void print() {
		std::cout << value;
	}
};

struct VariableDecleration : SyntaxNode {
	VariableDecleration() { type = Type::VARIABLE_DECLERATION; }
	std::string name;
	std::string type_name;
	void print() {
		std::cout << "var decl";
	}
};

struct Procedure : SyntaxNode {
	Procedure() { type = Type::PROCEDURE; }
	std::vector<SyntaxNode*> inputs;
	std::vector<SyntaxNode*> outputs;
	Block* body;

	void print() {
		std::cout << "PROC(";
		for (SyntaxNode* input : inputs) {
			input->print();
			std::cout << ",";
		}
		std::cout << ")";
		body->print();
	}
};

struct WhileStatement : SyntaxNode {
	WhileStatement() { type = Type::WHILE_STATEMENT; }
	SyntaxNode* condition;
	Block* body;
	void print() {}
};

struct IfStatement : SyntaxNode {
	IfStatement() { type = Type::IF_STATEMENT; }
	SyntaxNode* condition;
	Block* body;
};

struct ProcedureCall : SyntaxNode {
	ProcedureCall() { type = Type::PROCEDURE_CALL; }
	std::string name;
	std::vector<SyntaxNode*> inputs;

	void print() {
		for (SyntaxNode* input : inputs) {
			input->print();
		}
	}
};

struct VariableCall : SyntaxNode {
	VariableCall() {type = Type::VARIABLE_CALL;}
	std::string name;
	void print() {
		std::cout << "var call";
	}
};

struct ReturnStatement : SyntaxNode {
	ReturnStatement() { type = Type::RETURN_STATEMENT; }
	SyntaxNode* expression;
};

struct ProcedureDecleration : SyntaxNode {
	ProcedureDecleration() { type = Type::PROCEDURE_DECLERATION; }
	std::string name;
	Procedure* procedure;

	void print() {
		std::cout << "PROC_DECL(" << name << ",";
		procedure->print();
	}
};

struct VariableAssignment : SyntaxNode {
	VariableAssignment() { type = Type::VARIABLE_ASSIGNMENT; }
	std::string name;
	SyntaxNode* value;

	void print() {
		std::cout << "var assign";
		std::cout << name << " : ";
		value->print();
	}
};

struct BinaryOperator : SyntaxNode {
	BinaryOperator() { type = SyntaxNode::Type::BINARY_OPERATOR; }
	enum class Type {
		
		// math
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,

		// comparison
		LESS_THAN,
		GREATER_THAN,
		LESS_THAN_EQUAL,
		GREATER_THAN_EQUAL,
		EQUAL
	};

	BinaryOperator::Type operation;
	SyntaxNode* left;
	SyntaxNode* right;

	void print() {
		left->print();
		switch (operation)
		{
		case BinaryOperator::Type::ADD:
			std::cout << "+";
			break;
		case BinaryOperator::Type::SUBTRACT:
			std::cout << "-";
			break;
		case BinaryOperator::Type::MULTIPLY:
			std::cout << "*";
			break;
		case BinaryOperator::Type::DIVIDE:
			std::cout << "/";
			break;
		case BinaryOperator::Type::LESS_THAN:
			std::cout << "<";
			break;
		case BinaryOperator::Type::GREATER_THAN:
			std::cout << ">";
			break;
		case BinaryOperator::Type::LESS_THAN_EQUAL:
			std::cout << "<=";
			break;
		case BinaryOperator::Type::GREATER_THAN_EQUAL:
			std::cout << ">=";
			break;
		default:
			break;
		}
		right->print();
	}
};

bool is_expression(SyntaxNode* node) {
	if (
		node->type == SyntaxNode::Type::BINARY_OPERATOR ||
		node->type == SyntaxNode::Type::PROCEDURE_CALL  ||
		node->type == SyntaxNode::Type::VARIABLE_CALL
		) {
		return true;
	}
	return false;
}


bool is_literal(SyntaxNode* node) {
	if (
		node->type == SyntaxNode::Type::INTEGER_LITERAL ||
		node->type == SyntaxNode::Type::BOOLEAN_LITERAL ||
		node->type == SyntaxNode::Type::STRING_LITERAL  ||
		node->type == SyntaxNode::Type::FLOAT_LITERAL
		) {
		return true;
	}
	return false;
}