bits 64
default rel
segment .text
global main
extern ExitProcess
extern printf
mul:
push rbp
mov rbp, rsp
sub rsp, 48
; move the inputs to stack adresses
mov QWORD[rbp - 24], rcx
mov QWORD[rbp - 16], rdx
xor rbx, rbx
mov rbx,QWORD[rbp - 16]
imul rbx, QWORD[rbp - 24]
mov rax, rbx
leave
ret
main:
push rbp
mov rbp, rsp
sub rsp, 64
; move the inputs to stack adresses
xor rbx, rbx
; procedure mul start 
mov rcx, 2

mov rdx, 2

call mul
mov rbx, rax
mov QWORD[rbp - 40],rbx
mov rbx,QWORD[rbp - 40]
mov QWORD[rbp - 48],rbx
; procedure mul start 
mov rcx, 2

mov rdx, QWORD[rbp - 48]

call mul
mov rbx, rax
mov QWORD[rbp - 56],rbx
mov rbx,QWORD[rbp - 56]
add rbx, 3
mov rax, rbx
lea rcx, [msg]
mov rdx, rax
call printf
leave
ret
segment .data
	msg db "%d", 0xd, 0xa, 0
