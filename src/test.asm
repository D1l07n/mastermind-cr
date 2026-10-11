; test.asm - Prueba los 4 casos de la Tabla 2 (modo texto)
; Compilar: BUILD.bat .\mastermind-cr\src\test
; R=1 V=2 A=3 M=4 N=5 B=6
.MODEL SMALL
.STACK 100h
JUMPS

INCLUDE macros.inc

; PRINT_STR: imprime un texto terminado en '$'
PRINT_STR MACRO msg
    push ax
    push dx
    mov dx, OFFSET msg
    mov ah, 09h
    int 21h
    pop dx
    pop ax
ENDM

.DATA
; cada caso: 4 secreto, 4 intento, toros esperados, vacas esperadas
CaseTable   DB 1,2,3,4,  1,3,2,4,  2,2     ; R V A M / R A V M
            DB 1,1,2,3,  1,2,1,1,  1,2     ; R R V A / R V R R
            DB 1,2,3,4,  5,6,5,6,  0,0     ; R V A M / N B N B
            DB 3,3,3,3,  3,3,1,1,  2,0     ; A A A A / A A R R
NUM_CASES   EQU 4
CASE_SIZE   EQU 10
CaseNum     DB '1'
MsgCase     DB 'Caso $'
MsgColon    DB ': $'
MsgBulls    DB ' toros, $'
MsgCows     DB ' vacas -> $'
MsgOk       DB 'OK', 13, 10, '$'
MsgFail     DB 'FALLA', 13, 10, '$'

.CODE
INCLUDE game.inc

start:
    mov ax, @data
    mov ds, ax

    mov si, OFFSET CaseTable
case_loop:
    mov bx, 0                   ; copiar el caso a Secret y Guess
copy_case:
    mov al, [si + bx]
    mov Secret[bx], al
    mov al, [si + bx + 4]
    mov Guess[bx], al
    inc bx
    cmp bx, 4
    jb copy_case

    call ScoreGuess

    PRINT_STR MsgCase           ; "Caso 1: 2 toros, 2 vacas -> OK"
    mov dl, CaseNum
    call PrintChar
    PRINT_STR MsgColon
    mov dl, Bulls
    add dl, '0'
    call PrintChar
    PRINT_STR MsgBulls
    mov dl, Cows
    add dl, '0'
    call PrintChar
    PRINT_STR MsgCows

    mov al, Bulls               ; comparar con lo esperado
    cmp al, [si + 8]
    jne case_fail
    mov al, Cows
    cmp al, [si + 9]
    jne case_fail
    PRINT_STR MsgOk
    jmp case_next
case_fail:
    PRINT_STR MsgFail
case_next:
    add si, CASE_SIZE
    inc CaseNum
    cmp CaseNum, '1' + NUM_CASES
    jb case_loop

    mov ah, 4Ch
    mov al, 0
    int 21h

; PrintChar: imprime el caracter de DL. Toca nada.
PrintChar PROC
    push ax
    mov ah, 02h
    int 21h
    pop ax
    ret
PrintChar ENDP

END start