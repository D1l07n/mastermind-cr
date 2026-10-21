; ==========================================================
; graphics.inc - Dibujar en la pantalla (modo 13h)
; Pantalla de 320 x 200 puntos, 256 colores.
; Cada punto es 1 byte en la memoria de video (segmento A000h).
; ==========================================================

VIDEO_SEG   EQU 0A000h      ; donde empieza la memoria de la pantalla
SCREEN_W    EQU 320         ; puntos por fila

; ----------------------------------------------------------
; SetGraphicsMode: pone la pantalla en modo grafico 13h
;   Recibe: nada
;   Devuelve: nada
;   Toca: AX
; ----------------------------------------------------------
SetGraphicsMode PROC
    mov ax, 0013h           ; AH = 00h (cambiar modo), AL = 13h (modo grafico)
    int 10h                 ; llamar al BIOS de video
    ret
SetGraphicsMode ENDP

; SaveVideoMode: guarda el modo actual en OldVideoMode (va en .DATA de main.asm)
;   Recibe: nada
;   Devuelve: OldVideoMode
;   Toca: AX, BX
SaveVideoMode PROC
    mov ah, 0Fh             ; BIOS: que modo hay? -> AL
    int 10h
    mov OldVideoMode, al
    ret
SaveVideoMode ENDP

; RestoreVideoMode: vuelve al modo guardado en OldVideoMode
;   Recibe: OldVideoMode
;   Devuelve: nada
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
;   Recibe: nada (usa SpritePalette y UiPalette)
;   Devuelve: nada
;   Toca: AX, CX, DX, SI
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

    mov dx, 3C8h            ; ahora los colores fijos de las pantallas
    mov al, UI_START
    out dx, al
    mov dx, 3C9h
    mov si, OFFSET UiPalette
    mov cx, UI_COUNT * 3
lp_ui:
    mov al, [si]
    out dx, al
    inc si
    loop lp_ui
    ret
LoadPalette ENDP

; ----------------------------------------------------------
; SetColor: cambia un solo color de la paleta
;   Recibe: AL = numero de color, SI = direccion de 3 bytes (rojo, verde, azul)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
SetColor PROC
    PUSH_ALL
    mov dx, 3C8h
    out dx, al              ; que color se va a cambiar
    mov dx, 3C9h
    mov cx, 3
co_rgb:
    mov al, [si]
    out dx, al
    inc si
    loop co_rgb
    POP_ALL
    ret
SetColor ENDP

; ----------------------------------------------------------
; DrawSprite: dibuja un sprite de cualquier tamano
;   Recibe: SI = direccion del sprite (ej. OFFSET SprCafe0)
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
; DrawSpriteBg: dibuja un sprite pintando el fondo donde es transparente
;   Recibe: lo mismo que DrawSprite, y BP = color de fondo (no lo cambia)
;   Devuelve: nada. Cada punto se pinta una sola vez: no parpadea.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSpriteBg PROC
    PUSH_ALL
    push es

    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax              ; DI = y * 320 + x
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx
    mov ax, bp
    mov ah, al              ; AH = color de fondo

db_row:
    mov cx, bx
db_pixel:
    mov al, [si]
    cmp al, 0
    jne db_put
    mov al, ah              ; transparente: va el fondo
db_put:
    mov es:[di], al
    inc si
    inc di
    loop db_pixel

    add di, SCREEN_W
    sub di, bx
    dec dx
    jnz db_row

    pop es
    POP_ALL
    ret
DrawSpriteBg ENDP

; ----------------------------------------------------------
; DrawSpriteScaled: dibuja un sprite agrandado (cada punto es un cuadro)
;   Recibe: lo mismo que DrawSprite, y BP = escala (2 = doble, 3 = triple)
;   Devuelve: nada. Los puntos con color 0 se pintan negros.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSpriteScaled PROC
    PUSH_ALL
    push es
    cld                     ; STOSB avanza hacia adelante

    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax              ; DI = y * 320 + x
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx              ; BX = ancho del dibujo

ss_row:
    push dx
    mov dx, bp              ; cada fila del dibujo se repite BP veces
ss_repeat:
    push si
    push di
    mov cx, bx
ss_pixel:
    mov al, [si]
    push cx
    mov cx, bp
    rep stosb               ; pintar BP puntos seguidos del mismo color
    pop cx
    inc si
    loop ss_pixel
    pop di
    pop si
    add di, SCREEN_W
    dec dx
    jnz ss_repeat
    add si, bx              ; siguiente fila del dibujo
    pop dx
    dec dx
    jnz ss_row

    pop es
    POP_ALL
    ret
DrawSpriteScaled ENDP

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

; ----------------------------------------------------------
; DrawText: escribe una frase con la letra del BIOS
;   Recibe: SI = frase terminada en 0
;           DH = fila (0..24), DL = columna (0..39)
;           BL = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawText PROC
    PUSH_ALL
    mov ah, 02h             ; poner el cursor en DH, DL
    mov bh, 0
    int 10h
dt_next:
    mov al, [si]
    cmp al, 0
    je dt_end               ; 0 = fin de la frase
    mov ah, 0Eh             ; escribir AL con color BL y avanzar
    int 10h
    inc si
    jmp dt_next
dt_end:
    POP_ALL
    ret
DrawText ENDP

; ----------------------------------------------------------
; DrawNumber: escribe un numero de 0 a 99 (sin cero adelante)
;   Recibe: AL = numero, DH = fila, DL = columna, BL = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawNumber PROC
    PUSH_ALL
    mov cl, al              ; guardar el numero
    mov ah, 02h
    mov bh, 0
    int 10h
    mov al, cl
    mov ah, 0
    mov cl, 10
    div cl                  ; AL = decenas, AH = unidades
    mov ch, ah              ; guardar las unidades
    cmp al, 0
    je dn_units             ; sin decenas: solo una cifra
    add al, '0'
    mov ah, 0Eh
    int 10h                 ; decenas
dn_units:
    mov al, ch
    add al, '0'
    mov ah, 0Eh
    int 10h                 ; unidades
    POP_ALL
    ret
DrawNumber ENDP

; ----------------------------------------------------------
; DrawTwoDigits: escribe un numero de 0 a 99 siempre con 2 cifras (07)
;   Recibe: AL = numero, DH = fila, DL = columna, BL = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawTwoDigits PROC
    PUSH_ALL
    mov cl, al
    mov ah, 02h
    mov bh, 0
    int 10h
    mov al, cl
    mov ah, 0
    mov cl, 10
    div cl                  ; AL = decenas, AH = unidades
    mov ch, ah
    add al, '0'
    mov ah, 0Eh
    int 10h
    mov al, ch
    add al, '0'
    mov ah, 0Eh
    int 10h
    POP_ALL
    ret
DrawTwoDigits ENDP
