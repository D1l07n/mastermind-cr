; prueba.asm - Modo prueba en texto: escribir el secreto a mano y adivinar
.MODEL SMALL
.STACK 100h
JUMPS

INCLUDE macros.inc

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
MsgSecret   DB 'Secreto (', '$'
MsgGuess    DB 13, 10, 'Intento: $'
MsgRange    DB ' fichas del 1 al 6): $'
MsgBulls    DB '  toros: $'
MsgCows     DB '  vacas: $'

.CODE
INCLUDE game.inc

start:
    mov ax, @data
    mov ds, ax

    mov al, 0                   ; nivel facil: 4 posiciones, 6 colores
    call SetLevel

    PRINT_STR MsgSecret         ; pedir el secreto
    mov dl, NumPos
    add dl, '0'
    call PrintChar
    PRINT_STR MsgRange
    mov di, OFFSET Secret
    call ReadPieces

    PRINT_STR MsgGuess          ; pedir un intento
    mov di, OFFSET Guess
    call ReadPieces

    call ScoreGuess             ; contar y mostrar
    PRINT_STR MsgBulls
    mov dl, Bulls
    add dl, '0'
    call PrintChar
    PRINT_STR MsgCows
    mov dl, Cows
    add dl, '0'
    call PrintChar

    mov ah, 4Ch
    mov al, 0
    int 21h

; ReadPieces: lee NumPos fichas (teclas 1..NumColors) y las guarda en [DI].
;   Ignora cualquier otra tecla. Toca nada.
ReadPieces PROC
    PUSH_ALL
    mov cl, NumPos
    mov ch, 0
rp_key:
    mov ah, 08h                 ; leer tecla sin mostrarla
    int 21h
    sub al, '0'                 ; '3' -> 3
    cmp al, 1
    jb rp_key                   ; menor que 1: no sirve
    cmp al, NumColors
    ja rp_key                   ; mayor que NumColors: no sirve
    mov [di], al                ; guardar la ficha
    inc di
    mov dl, al                  ; mostrarla en pantalla
    add dl, '0'
    call PrintChar
    loop rp_key
    POP_ALL
    ret
ReadPieces ENDP

; PrintChar: imprime el caracter de DL. Toca nada.
PrintChar PROC
    push ax
    mov ah, 02h
    int 21h
    pop ax
    ret
PrintChar ENDP

END start