; Copyright (C) 2022 James Keats
;
; You may use, distribute, and modify this code under the terms of its modified
; BSD-3-Clause license. Use for any commercial purposes is prohibited.
;
; You should have received a copy of the license with this file. If not, please visit:
; https://github.com/growlitheharpo/heart-engine-playground
;
PUBLIC manually_set_rsp
PUBLIC manually_restore_rsp

.code

; manually_set_rsp PROC
; 	push r12
; 	sub rsp, 8
; 
; 	mov r12, rsp
; 	mov rsp, rdx
; 	call rcx
; 	mov rsp, r12
; 
; 	add rsp, 8
; 	pop r12
; 	ret
; manually_set_rsp ENDP

manually_set_rsp PROC
	mov rax, rsp
	mov r8, [rsp]
	mov rsp, rcx
	jmp r8

	int 3

	ret
manually_set_rsp ENDP

manually_restore_rsp PROC
	mov rax, [rsp]
	mov rsp, rcx
	mov [rsp], rax
	ret
manually_restore_rsp ENDP

END
