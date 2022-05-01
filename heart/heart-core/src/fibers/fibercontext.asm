; Copyright (C) 2022 James Keats
;
; This file is part of Heart, a collection of game engine technologies.
;
; Huge thanks to https://graphitemaster.github.io/fibers/#user-space-context-switching
; and https://andreaspk.github.io/posts/2019-02-16-Windows%20Calling%20Convention.html
; for providing most of what needs to be done here
;
PUBLIC heart_swap_fiber_context

.code

heart_swap_fiber_context PROC 

	; Save the return address
	mov r8, [rsp]
	mov [rcx + 8 * 0], r8

	; RSP
	lea r8, [rsp + 8]
	mov [rcx + 8 * 1], r8

	; Preserved registers
	mov [rcx + 8 * 2], rbx
	mov [rcx + 8 * 3], rbp
	mov [rcx + 8 * 4], r12
	mov [rcx + 8 * 5], r13
	mov [rcx + 8 * 6], r14
	mov [rcx + 8 * 7], r15
	mov [rcx + 8 * 8], rdi
	mov [rcx + 8 * 9], rsi

	; XMM registers
	movups [rcx + 8 * 10 + 16 * 0], xmm6
	movups [rcx + 8 * 10 + 16 * 1], xmm7
	movups [rcx + 8 * 10 + 16 * 2], xmm8
	movups [rcx + 8 * 10 + 16 * 3], xmm9
	movups [rcx + 8 * 10 + 16 * 4], xmm10
	movups [rcx + 8 * 10 + 16 * 5], xmm11
	movups [rcx + 8 * 10 + 16 * 6], xmm12
	movups [rcx + 8 * 10 + 16 * 7], xmm13
	movups [rcx + 8 * 10 + 16 * 8], xmm14
	movups [rcx + 8 * 10 + 16 * 9], xmm15
	
	; Load the target RIP into r8
	mov r8, [rdx + 8 * 0]
	; Load the new (old) stack pointer
	mov rsp, [rdx + 8 * 1]

	; Load the preserved registers
	mov rbx, [rdx + 8 * 2]
	mov rbp, [rdx + 8 * 3]
	mov r12, [rdx + 8 * 4]
	mov r13, [rdx + 8 * 5]
	mov r14, [rdx + 8 * 6]
	mov r15, [rdx + 8 * 7]
	mov rdi, [rdx + 8 * 8]
	mov rsi, [rdx + 8 * 9]
	
	; XMM registers
	movups xmm6, [rdx + 8 * 10 + 16 * 0]
	movups xmm7, [rdx + 8 * 10 + 16 * 1]
	movups xmm8, [rdx + 8 * 10 + 16 * 2]
	movups xmm9, [rdx + 8 * 10 + 16 * 3]
	movups xmm10, [rdx + 8 * 10 + 16 * 4]
	movups xmm11, [rdx + 8 * 10 + 16 * 5]
	movups xmm12, [rdx + 8 * 10 + 16 * 6]
	movups xmm13, [rdx + 8 * 10 + 16 * 7]
	movups xmm14, [rdx + 8 * 10 + 16 * 8]
	movups xmm15, [rdx + 8 * 10 + 16 * 9]

	; Push old RIP onto the stack for ret
	push r8
	xor rax, rax
	ret
heart_swap_fiber_context ENDP

END
