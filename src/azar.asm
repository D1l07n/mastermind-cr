; azar.asm - muestra 5 secretos al azar de cada nivel
.MODEL SMALL
.STACK 100h
JUMPS
INCLUDE macros.inc
.DATA
.CODE
INCLUDE game.inc
start:
    mov ax, @data
    mov ds, ax
    call InitRandom
    mov al, 0
nivel:
    call SetLevel
    mov cx, 5
uno:
    call MakeSecret
    push cx
    mov cl, NumPos
    mov ch, 0
    mov si, 0
pr:
    mov dl, Secret[si]
    add dl, '0'
    mov ah, 2
    int 21h
    inc si
    loop pr
    mov dl, ' '
    int 21h
    pop cx
    loop uno
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h
    mov al, Level
    inc al
    cmp al, 3
    jb nivel
    mov ax, 4C00h
    int 21h
END start
