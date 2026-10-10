.MODEL SMALL
.STACK 100h
.DATA
.CODE
inicio:
    mov ax, @data
    mov ds, ax

     ;Pantallita en modo DIBUJO
    mov ax, 0013h
    int 10h

    ; aquie esperamos el bicho toque una tecla
    mov ah, 00h
    int 16h

    ; pantalla en modo letras
    mov ax, 0003h
    int 10h

    mov ah, 4Ch
    mov al, 0
    int 21h
END inicio