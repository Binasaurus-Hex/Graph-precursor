#pragma once
#include <fstream>
#include <functional>
#include "Parsing.h"
#include "Utils.h"
#include <chrono>

typedef std::function<void(std::ofstream&)> writer;

void program_header(std::ofstream& out) {
	out <<
		"bits 64\n"
		"default rel\n"
		"segment .text\n"
		"global main\n"
		"extern ExitProcess\n"
		"extern printf\n";
}

void data_segment(std::ofstream& out) {
	out <<
		"segment .data\n"
		"	msg db \"%d\", 0xd, 0xa, 0\n";
}

void reserve_stack(std::ofstream& out, int bytes) {
	out << 
		"push rbp\n"
		"mov rbp, rsp\n"
		"sub rsp, " << bytes << "\n";
}

const char* registers[] = { "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
const int num_registers = 12;

int calculate_stack_size(Procedure* procedure);
void declare_expression(std::ofstream& out, SyntaxNode* expression, std::map<std::string, std::string>& scope);

void declare_procedure_call(std::ofstream& out, ProcedureCall* procedure, std::map<std::string, std::string>& scope) {
	out << "; procedure "<< procedure->name << " start " << "\n";

	for (int i = 0; i < procedure->inputs.size(); i++) {
		SyntaxNode* input = procedure->inputs[i];
		std::string input_register = registers[i + 2]; // COULD SPILL OFF OFF REGISTERS IF TOO MANY PARAMS !!!
		
		if (input->type == SyntaxNode::Type::VARIABLE_CALL) {
			VariableCall* var_call = (VariableCall*)input;
			out << "mov " << input_register << ", " << scope[var_call->name] << "\n";
		}
		if (input->type == SyntaxNode::Type::INTEGER_LITERAL) {
			IntLiteral* int_literal = (IntLiteral*)input;
			out << "mov " << input_register << ", " << int_literal->value << "\n";
		}
		out << "\n";
	}
	out << "call " << procedure->name << "\n";
}

void declare_expression(std::ofstream& out, SyntaxNode* expression, std::map<std::string, std::string>& scope) {
	// all results go into rbx

	if (expression->type == SyntaxNode::Type::INTEGER_LITERAL) {
		IntLiteral* int_literal = (IntLiteral*)expression;
		out << "mov " << registers[1] << "," << int_literal->value << "\n";
	}
	if (expression->type == SyntaxNode::Type::VARIABLE_CALL) {
		VariableCall* var_call = (VariableCall*)expression;
		out << "mov " << registers[1] << "," << scope[var_call->name] << "\n";
	}

	if (expression->type == SyntaxNode::Type::PROCEDURE_CALL) {
		ProcedureCall* proc_call = (ProcedureCall*)expression;
		declare_procedure_call(out, proc_call, scope);
		out << "mov " << registers[1] << ", " << registers[0] << "\n";
	}

	if (expression->type == SyntaxNode::Type::BINARY_OPERATOR) {
		BinaryOperator* binary_operator = (BinaryOperator*)expression;
		declare_expression(out, binary_operator->right, scope);

		std::string operation;
		std::string comparison_operation;

		switch (binary_operator->operation)
		{
		case BinaryOperator::Type::ADD:
			operation = "add";
			break;
		case BinaryOperator::Type::MULTIPLY:
			operation = "imul";
			break;
		case BinaryOperator::Type::SUBTRACT:
			operation = "sub";
			break;
		case BinaryOperator::Type::LESS_THAN:
			operation = "cmp";
			comparison_operation = "setl";
			break;
		case BinaryOperator::Type::GREATER_THAN:
			operation = "cmp";
			comparison_operation = "setg";
			break;
		case BinaryOperator::Type::EQUAL:
			operation = "cmp";
			comparison_operation = "sete";
			break;

		default:
			break;
		}

		if (binary_operator->left->type == SyntaxNode::Type::INTEGER_LITERAL) {
			IntLiteral* int_literal = (IntLiteral*)binary_operator->left;
			out << operation << " " << registers[1] << ", " << int_literal->value << "\n";
		}
		if (binary_operator->left->type == SyntaxNode::Type::VARIABLE_CALL) {
			VariableCall* var_call = (VariableCall*)binary_operator->left;
			out << operation << " " << registers[1] << ", " << scope[var_call->name] << "\n";
		}
		if (binary_operator->left->type == SyntaxNode::Type::PROCEDURE_CALL) {
			// SHOULD NEVER HAPPEN NEED TO CHECK
			declare_procedure_call(out, (ProcedureCall*)binary_operator->left, scope);
			out << operation << " " << registers[1] << ", " << registers[0] << "\n";
		}

		if (comparison_operation.length() > 0) {
			out << comparison_operation << " " << "al" << "\n";
		}
	}
}

int ident_count = 0;

void declare_block(std::ofstream& out, Block* block, std::map<std::string, std::string>& scope, int& stack_offset) {
	out << "xor rbx, rbx\n";
	for (SyntaxNode* statement : block->statements) {
		if (statement->type == SyntaxNode::Type::VARIABLE_DECLERATION) {
			VariableDecleration* decl = (VariableDecleration*)statement;
			stack_offset += 8;
			scope[decl->name] = string_format("QWORD[rbp - %d]", stack_offset);
		}
		if (statement->type == SyntaxNode::Type::VARIABLE_ASSIGNMENT) {
			VariableAssignment* assignment = (VariableAssignment*)statement;
			declare_expression(out, assignment->value, scope);
			out << "mov " << scope[assignment->name] << "," << registers[1] << "\n"; // rbx is accumilator

		}
		if (statement->type == SyntaxNode::Type::WHILE_STATEMENT) {
			WhileStatement* while_statment = (WhileStatement*)statement;
			int while_id = ident_count++;
			out << ".while_head"<<while_id<<":\n";
			declare_expression(out, while_statment->condition, scope);
			out << "cmp al, 0\n";
			out << "jne " << ".while_end"<<while_id<<"\n";

			declare_block(out, while_statment->body, scope, stack_offset);
			out << "jmp " << ".while_head"<<while_id<<"\n";
			out << ".while_end"<<while_id<<":\n";
		}
		if (statement->type == SyntaxNode::Type::RETURN_STATEMENT) {
			ReturnStatement* return_statement = (ReturnStatement*)statement;
			declare_expression(out, return_statement->expression, scope);
			out << "mov rax, rbx\n";
		}
	}
}

void declare_procedure(std::ofstream& out, ProcedureDecleration* procedure_decl, std::map<std::string, std::string> scope) {
	out << procedure_decl->name << ":\n";

	int stack_size = calculate_stack_size(procedure_decl->procedure);
	std::cout << "stack size of " << procedure_decl->name << " is " << stack_size << std::endl;
	reserve_stack(out, stack_size);

	int stack_offset = 32;


	Procedure* proc = procedure_decl->procedure;
	std::map<std::string, std::string> sub_scope(scope);

	out << "; move the inputs to stack adresses" << "\n";
	int register_offset = 2;
	for (SyntaxNode* input : proc->inputs) {
		VariableDecleration* decl = (VariableDecleration*)input;
		stack_offset -= 8;
		std::string input_register = registers[register_offset];
		register_offset++;
		std::string mem_location = string_format("QWORD[rbp - %d]", stack_offset);
		sub_scope[decl->name] = mem_location;
		out << "mov " << mem_location << ", " << input_register << "\n";
	}

	declare_block(out, proc->body, sub_scope, stack_offset);

	if (procedure_decl->name == "main") {
		
		out << "lea rcx, [msg]\n"
			<< "mov rdx, rax\n"
			<< "call printf\n";
	}

	out <<
		"leave\n"
		"ret\n";
}

int calculate_stack_size(Block* block) {
	int stack_size = 0;
	const int int_size = 8;

	for (SyntaxNode* statement : block->statements) {
		if (statement->type == SyntaxNode::Type::VARIABLE_DECLERATION) {
			stack_size += 8; // assume int
		}
		if (statement->type == SyntaxNode::Type::WHILE_STATEMENT) {
			WhileStatement* while_statement = (WhileStatement*)statement;
			stack_size += calculate_stack_size(while_statement->body);
		}
		if (statement->type == SyntaxNode::Type::IF_STATEMENT) {
			IfStatement* if_statement = (IfStatement*)statement;
			stack_size += calculate_stack_size(if_statement->body);
		}
	}
	return stack_size;
}

int calculate_stack_size(Procedure* procedure) {
	int stack_size = 0;
	int int_size = 8;

	stack_size += 32; // shadow space

	
	for (SyntaxNode* input : procedure->inputs) {
		stack_size += 8; // assume int
	}
	stack_size += calculate_stack_size(procedure->body);

	const int stack_alignment_bytes = 16;
	int stack_multiple = stack_size / stack_alignment_bytes;
	int stack_remainder = stack_size % stack_alignment_bytes;
	if (stack_remainder != 0) stack_multiple += 1;

	return stack_multiple * 16;
}

void compile(Block* node) {
	std::ofstream out("compiled.asm");

	std::map<std::string, std::string> scope;

	program_header(out);
	for (SyntaxNode* statement : node->statements) {
		if (statement->type == SyntaxNode::Type::PROCEDURE_DECLERATION) {
			ProcedureDecleration* procedure_decl = (ProcedureDecleration*)statement;
			declare_procedure(out, procedure_decl, scope);
		}
	}
	data_segment(out);

	out.close();
	const char* compile_command = "nasm -f win64 -o compiled.obj compiled.asm";
	const char* link_command = "link compiled.obj /subsystem:console /out:compiled.exe kernel32.lib legacy_stdio_definitions.lib msvcrt.lib";

	system(compile_command);
	system(link_command);


#if false
	int start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	system("compiled.exe");
	int end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::cout << "graph took " << (end - start) / 1e9 << " seconds" << std::endl;

	start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	system("python C:/Users/Thewa/AppData/Local/Programs/Python/Python310/speed_test.py");
	end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::cout << "python took " << (end - start) / 1e9 << " seconds" << std::endl;
#endif
}