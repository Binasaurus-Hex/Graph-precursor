bits 64
default rel
segment .text
global main
extern ExitProcess
extern printf
extern _CRT_INIT
main:
push rbp
mov rbp, rsp
sub rsp, 64
call    _CRT_INIT
; move the inputs to stack adresses
xor rbx, rbx
mov rbx,5
sub rbx, 10
mov QWORD [rbp - 40],rbx
; procedure printf start 
lea rcx, [string_id0]

mov rdx, QWORD [rbp - 40]

call printf
mov rbx,0
mov rax, rbx
leave
ret
segment .data
msg db "%d", 0xd, 0xa, 0
string_id0 db "%d", 0xd, 0xa, 0
