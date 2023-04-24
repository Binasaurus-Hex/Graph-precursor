.data
	position REAL8 1.25f
	velocity REAL8 0.35f

.code
SomeFunction proc
	movd xmm0, [position]
	movd xmm1, [velocity]

	mov rax, rsp
	mov rax, rbp

	push 12
	push 13

	ret

SomeFunction endp
end