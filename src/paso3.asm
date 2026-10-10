.MODEL SMALL
.STACK 100h
.DATA
.CODE
inicio:
    mov ax, @data
    mov ds, ax

    ; 1. Pantalla en modo DIBUJO
    mov ax, 0013h
    int 10h

    ; 2. Pintar una raya blanca en el centro
    mov ax, 0A000h      ; A000h = donde vive la pantalla en memoria
    mov es, ax          ; ES apunta al cuaderno de la pantalla
    mov di, 32160       ; DI = la casilla donde empiezo (el centro)
    mov al, 15          ; AL = el color (15 = blanco)
    mov cx, 50          ; 50 cuadritos
raya:
    mov es:[di], al     ; pintar el cuadrito
    inc di              ; pasar al de la derecha
    loop raya           ; repetir

    ; 3. Esperar que toque una tecla
    mov ah, 00h
    int 16h

    ; 4. Pantalla otra vez en modo LETRAS
    mov ax, 0003h
    int 10h

    mov ah, 4Ch
    mov al, 0
    int 21h
END inicio