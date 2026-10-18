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
INCLUDE uidata.inc
OldVideoMode DB 0           ; modo de video que habia antes de arrancar

.CODE
INCLUDE graphics.inc
INCLUDE input.inc
INCLUDE screens.inc

start:
    mov ax, @data
    mov ds, ax

    call SaveVideoMode      ; anotar como estaba la pantalla
    call SetGraphicsMode
    call LoadPalette

    call GetTicks
    mov HeadTick, ax
    mov PanelTick, ax

    mov al, 1               ; nivel 1
    call DrawHeader
    mov cl, 12              ; nivel 1: 12 filas de 4 fichas
    mov ch, 4
    call DrawBoard
    mov al, 6               ; nivel 1: 6 tipos de ficha
    mov ah, 4
    mov cl, 1               ; intento 1 de 12
    mov ch, 12
    call DrawPanel
    mov si, OFFSET MsgHelp
    mov bl, C_WHITE
    call DrawStatus

main_loop:
    call UpdateHeadSloth
    call UpdatePanelSloth
    call PollAction
    cmp al, ACT_ESC
    jne main_loop           ; por ahora solo ESC hace algo

exit_game:
    call RestoreVideoMode   ; dejar la pantalla como estaba
    mov ah, 4Ch
    mov al, 0
    int 21h

END start
