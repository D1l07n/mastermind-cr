; ==========================================================
; macros.inc - Macros compartidas
; ==========================================================

; PUSH_ALL: guarda los registros de uso general en la pila
PUSH_ALL MACRO
    push ax
    push bx
    push cx
    push dx
    push si
    push di
ENDM

; POP_ALL: los recupera en orden inverso (la pila es LIFO)
POP_ALL MACRO
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
ENDM
