; ==========================================================
; graphics.inc - Dibujar en la pantalla (modo 13h)
; Pantalla de 320 x 200 puntos, 256 colores.
; Cada punto es 1 byte en la memoria de video (segmento A000h).
; ==========================================================

VIDEO_SEG   EQU 0A000h      ; donde empieza la memoria de la pantalla
SCREEN_W    EQU 320         ; puntos por fila

; ----------------------------------------------------------
; SetGraphicsMode: pone la pantalla en modo grafico 13h
; ----------------------------------------------------------
SetGraphicsMode PROC
    mov ax, 0013h           ; AH = 00h (cambiar modo), AL = 13h (modo grafico)
    int 10h                 ; llamar al BIOS de video
    ret
SetGraphicsMode ENDP

; SaveVideoMode: guarda el modo actual en OldVideoMode (va en .DATA de main.asm)
;   Toca: AX, BX
SaveVideoMode PROC
    mov ah, 0Fh             ; BIOS: que modo hay? -> AL
    int 10h
    mov OldVideoMode, al
    ret
SaveVideoMode ENDP

; RestoreVideoMode: vuelve al modo guardado en OldVideoMode
;   Toca: AX
RestoreVideoMode PROC
    mov ah, 00h
    mov al, OldVideoMode
    int 10h
    ret
RestoreVideoMode ENDP

; ----------------------------------------------------------
; LoadPalette: le manda a la tarjeta de video los colores
; de nuestros sprites (la tabla SpritePalette de sprites.inc)
; ----------------------------------------------------------
LoadPalette PROC
    mov dx, 3C8h            ; puerto: "voy a cambiar colores desde el..."
    mov al, PALETTE_START   ; ...color 16
    out dx, al
    mov dx, 3C9h            ; puerto: "aqui van rojo, verde, azul"
    mov si, OFFSET SpritePalette
    mov cx, PALETTE_COUNT * 3   ; 3 numeros por color
lp_next:
    mov al, [si]            ; leer un numero de la tabla
    out dx, al              ; mandarlo a la tarjeta
    inc si
    loop lp_next
    ret
LoadPalette ENDP

; ----------------------------------------------------------
; DrawSprite: dibuja un sprite de cualquier tamano
;   Recibe: SI = direccion del sprite (ej. OFFSET SprLapa0)
;           AX = x, BX = y
;           CX = ancho, DX = alto
;   Devuelve: nada. Los puntos con color 0 no se pintan.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSprite PROC
    PUSH_ALL
    push es

    push dx                 ; MUL borra DX: guardar el alto
    mov di, ax              ; DI = x
    mov ax, bx
    mov bx, SCREEN_W
    mul bx                  ; AX = y * 320
    add di, ax              ; DI = y * 320 + x
    pop dx                  ; DX = alto otra vez

    mov ax, VIDEO_SEG
    mov es, ax              ; ES apunta a la pantalla
    mov bx, cx              ; BX = ancho (CX se gasta en cada fila)

ds_row:
    mov cx, bx              ; puntos por fila
ds_pixel:
    mov al, [si]
    cmp al, 0
    je ds_skip              ; 0 = transparente
    mov es:[di], al
ds_skip:
    inc si
    inc di
    loop ds_pixel

    add di, SCREEN_W        ; bajar una fila...
    sub di, bx              ; ...y volver al inicio del sprite
    dec dx
    jnz ds_row

    pop es
    POP_ALL
    ret
DrawSprite ENDP

; ----------------------------------------------------------
; FillRect: pinta un rectangulo de un solo color
;   Recibe: AX = x, BX = y, CX = ancho, DX = alto
;           SI = color (solo se usa la parte baja)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
FillRect PROC
    PUSH_ALL
    push es

    push dx                 ; MUL borra DX: guardar el alto
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax              ; DI = y * 320 + x
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx              ; BX = ancho
    mov ax, si              ; AL = color
    cld                     ; que STOSB avance hacia adelante

fr_row:
    mov cx, bx
    rep stosb               ; pintar CX puntos seguidos con AL
    add di, SCREEN_W
    sub di, bx              ; inicio de la siguiente fila
    dec dx
    jnz fr_row

    pop es
    POP_ALL
    ret
FillRect ENDP
