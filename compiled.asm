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
sub rsp, 48
; move the inputs to stack adresses
xor rbx, rbx
mov rbx,0
mov QWORD[rbp - 40],rbx
.while_head0:
mov rbx,20000000
cmp rbx, QWORD[rbp - 40]
setl al
cmp al, 0
jne .while_end0
xor rbx, rbx
mov rbx,1
add rbx, QWORD[rbp - 40]
mov QWORD[rbp - 40],rbx
jmp .while_head0
.while_end0:
mov rbx,QWORD[rbp - 40]
mov rax, rbx
lea rcx, [msg]
mov rdx, rax
call printf
leave
ret
segment .data
	msg db "%d", 0xd, 0xa, 0
