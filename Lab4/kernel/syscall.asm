
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90
_NR_milli_seconds   equ 1;
_NR_print_str 		equ 2;
_NR_P 				equ 3;
_NR_V				equ 4;

; 导出符号
global	get_ticks
global milli_seconds
global print_str
global P
global V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              milli_seconds
; ====================================================================
milli_seconds:
	mov	eax, _NR_milli_seconds
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ========================================================================
;                  void print_str(char * str);
; ========================================================================
print_str:
	mov	eax, _NR_print_str
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ========================================================================
;                  void P(int signal,int proc);
; ========================================================================
P:
	mov	eax, _NR_P
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ========================================================================
;                  void V(int signal,int proc);
; ========================================================================
V:
	mov	eax, _NR_V
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret