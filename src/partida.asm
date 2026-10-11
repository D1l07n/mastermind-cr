; partida.asm - Una partida completa en modo texto (nivel facil)
; Teclas: 1..6 = ficha, ENTER = evaluar, ESC = salir
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
MsgTry      DB 13, 10, 'Intento $'
MsgColon    DB ': $'
MsgBulls    DB '  toros: $'
MsgCows     DB '  vacas: $'
MsgIncomp   DB '  incompleto, no cuenta$'
MsgWin      DB 13, 10, 'GANASTE!$'
MsgLose     DB 13, 10, 'Perdiste. El secreto era: $'

.CODE
INCLUDE game.inc

start:
    mov ax, @data
    mov ds, ax

    call InitRandom
    mov al, 0                   ; nivel facil
    call SetLevel
    call NewGame

turn:
    PRINT_STR MsgTry            ; "Intento 1: "
    mov al, TryNum
    inc al
    call PrintNum
    PRINT_STR MsgColon
    call ReadGuess
    cmp al, 27                  ; ESC: salir
    je quit

    call SubmitGuess
    cmp al, 0
    jne evaluated
    PRINT_STR MsgIncomp         ; incompleto: mismo intento otra vez
    jmp turn

evaluated:
    PRINT_STR MsgBulls
    mov al, Bulls
    call PrintNum
    PRINT_STR MsgCows
    mov al, Cows
    call PrintNum

    cmp GameState, 1
    je won
    cmp GameState, 2
    je lost
    jmp turn

won:
    PRINT_STR MsgWin
    jmp quit
lost:
    PRINT_STR MsgLose
    mov cl, NumPos
    mov ch, 0
    mov si, 0
show_secret:
    mov al, Secret[si]
    call PrintNum
    inc si
    loop show_secret
quit:
    mov ah, 4Ch
    mov al, 0
    int 21h

; ReadGuess: lee fichas hasta llenar NumPos o hasta ENTER.
;   Las casillas que no se llenaron quedan en 0.
;   Devuelve: AL = 13 (ENTER) o 27 (ESC). Toca AX.
ReadGuess PROC
    push bx
    push dx
    mov bx, 0
rg_clear:
    mov Guess[bx], 0            ; empezar con todo vacio
    inc bx
    cmp bx, MAX_POS
    jb rg_clear
    mov bx, 0                   ; BX = casilla que toca
rg_key:
    mov ah, 08h                 ; leer tecla sin mostrarla
    int 21h
    cmp al, 13
    je rg_end                   ; ENTER
    cmp al, 27
    je rg_end                   ; ESC
    cmp bl, NumPos
    jae rg_key                  ; ya esta lleno: solo sirve ENTER
    sub al, '0'
    cmp al, 1
    jb rg_key
    cmp al, NumColors
    ja rg_key
    mov Guess[bx], al
    inc bx
    call PrintNum
    jmp rg_key
rg_end:
    pop dx
    pop bx
    ret
ReadGuess ENDP

; PrintNum: imprime AL (0..99) como numero. Toca nada.
PrintNum PROC
    push ax
    push dx
    mov ah, 0
    mov dl, 10
    div dl                      ; AL = decenas, AH = unidades
    mov dh, ah
    cmp al, 0
    je pn_units                 ; sin decenas: solo un digito
    mov dl, al
    add dl, '0'
    mov ah, 02h
    int 21h
pn_units:
    mov dl, dh
    add dl, '0'
    mov ah, 02h
    int 21h
    pop dx
    pop ax
    ret
PrintNum ENDP

END start
