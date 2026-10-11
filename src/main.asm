; ==========================================================
; main.asm - Orquestador del juego Mastermind Pura Vida
; Une todas las partes con INCLUDE y decide el orden.
; ==========================================================
.MODEL SMALL
.STACK 100h
JUMPS                       ; TASM alarga solo los saltos que quedan lejos

INCLUDE macros.inc

.DATA
INCLUDE sprites.inc
OldVideoMode DB 0           ; modo de video que habia antes de arrancar
PlayerX      DW 152         ; posicion del perezoso (prueba de la fase 4)
PlayerY      DW 92
PlayerFrame  DB 0           ; 0 = cara normal, 1 = guino
LastTick     DW 0           ; tic del ultimo cambio de cara
STEP         EQU 4          ; puntos que avanza por tecla
BLINK_TICKS  EQU 9          ; 9 tics = medio segundo

.CODE
INCLUDE graphics.inc
INCLUDE input.inc

start:
    mov ax, @data
    mov ds, ax

    call SaveVideoMode      ; anotar como estaba la pantalla
    call SetGraphicsMode
    call LoadPalette

    call GetTicks
    mov LastTick, ax
    call DrawPlayer

main_loop:
    call GetTicks
    mov bx, ax              ; BX = tic de ahora
    sub ax, LastTick        ; AX = tics que pasaron
    cmp ax, BLINK_TICKS
    jb check_key            ; todavia no toca cambiar de cara
    mov LastTick, bx
    call ErasePlayer
    xor PlayerFrame, 1      ; 0 -> 1, 1 -> 0
    call DrawPlayer

check_key:
    call PollAction
    cmp al, ACT_NONE
    je main_loop            ; nadie toco nada: otra vuelta
    cmp al, ACT_ESC
    je exit_game

    call ErasePlayer
    cmp al, ACT_LEFT
    je move_left
    cmp al, ACT_RIGHT
    je move_right
    cmp al, ACT_UP
    je move_up
    cmp al, ACT_DOWN
    je move_down
    jmp moved               ; otra tecla: queda donde estaba

move_left:
    cmp PlayerX, 0
    je moved                ; ya esta en el borde
    sub PlayerX, STEP
    jmp moved
move_right:
    cmp PlayerX, 320 - 16
    je moved
    add PlayerX, STEP
    jmp moved
move_up:
    cmp PlayerY, 0
    je moved
    sub PlayerY, STEP
    jmp moved
move_down:
    cmp PlayerY, 200 - 16
    je moved
    add PlayerY, STEP
moved:
    call DrawPlayer
    jmp main_loop

exit_game:
    call RestoreVideoMode   ; dejar la pantalla como estaba
    mov ah, 4Ch
    mov al, 0
    int 21h

; DrawPlayer: dibuja el perezoso en PlayerX, PlayerY con la cara PlayerFrame. Toca nada.
DrawPlayer PROC
    PUSH_ALL
    mov si, OFFSET SprPerezoso0
    cmp PlayerFrame, 0
    je dp_draw
    mov si, OFFSET SprPerezoso1
dp_draw:
    mov ax, PlayerX
    mov bx, PlayerY
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    call DrawSprite
    POP_ALL
    ret
DrawPlayer ENDP

; ErasePlayer: pinta de negro el cuadro del perezoso. Toca nada.
ErasePlayer PROC
    PUSH_ALL
    mov ax, PlayerX
    mov bx, PlayerY
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0               ; negro
    call FillRect
    POP_ALL
    ret
ErasePlayer ENDP

END start
