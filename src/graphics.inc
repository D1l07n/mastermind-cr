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
    mov ax, 0013h
    int 10h
    ret
SetGraphicsMode ENDP

; SaveVideoMode: guarda el modo actual en OldVideoMode (va en .DATA de main.asm)
;   Recibe: nada
;   Devuelve: OldVideoMode
;   Toca: AX, BX
SaveVideoMode PROC
    mov ah, 0Fh
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
; LoadPalette: pone los colores de los sprites y de las pantallas
;   con toda la luz
;   Recibe: nada (usa SpritePalette y UiPalette)
;   Devuelve: nada
;   Toca: nada
; ----------------------------------------------------------
LoadPalette PROC
    push bx
    mov bl, 16
    call PaletteLight
    pop bx
    ret
LoadPalette ENDP

; ----------------------------------------------------------
; PaletteLight: pone toda la paleta (sprites y pantallas) con cierta luz
;   Recibe: BL = luz (16 = toda, 8 = la mitad, 0 = todo negro)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
PaletteLight PROC
    PUSH_ALL
    mov si, OFFSET SpritePalette
    mov al, PALETTE_START
    mov cx, PALETTE_COUNT
    call DarkColors
    mov si, OFFSET UiPalette
    mov al, UI_START
    mov cx, UI_COUNT
    call DarkColors
    POP_ALL
    ret
PaletteLight ENDP

; ----------------------------------------------------------
; DarkColors: manda a la tarjeta una tabla de colores con menos luz
;   Recibe: SI = tabla (rojo, verde, azul), AL = primer color,
;           CX = cuantos colores, BL = luz (16 = toda, 8 = la mitad)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DarkColors PROC
    PUSH_ALL
    mov dx, 3C8h
    out dx, al
    mov dx, 3C9h
    mov ax, cx
    shl cx, 1
    add cx, ax
dc_next:
    mov al, [si]
    mul bl
    shr ax, 1
    shr ax, 1
    shr ax, 1
    shr ax, 1
    out dx, al
    inc si
    loop dc_next
    POP_ALL
    ret
DarkColors ENDP

; ----------------------------------------------------------
; SetColor: cambia un solo color de la paleta
;   Recibe: AL = numero de color, SI = direccion de 3 bytes (rojo, verde, azul)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
SetColor PROC
    PUSH_ALL
    mov dx, 3C8h
    out dx, al
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

    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx

ds_row:
    mov cx, bx
ds_pixel:
    mov al, [si]
    cmp al, 0
    je ds_skip
    mov es:[di], al
ds_skip:
    inc si
    inc di
    loop ds_pixel

    add di, SCREEN_W
    sub di, bx
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
    add di, ax
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx
    mov ax, bp
    mov ah, al

db_row:
    mov cx, bx
db_pixel:
    mov al, [si]
    cmp al, 0
    jne db_put
    mov al, ah
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
    cld

    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx

ss_row:
    push dx
    mov dx, bp
ss_repeat:
    push si
    push di
    mov cx, bx
ss_pixel:
    mov al, [si]
    push cx
    mov cx, bp
    rep stosb
    pop cx
    inc si
    loop ss_pixel
    pop di
    pop si
    add di, SCREEN_W
    dec dx
    jnz ss_repeat
    add si, bx
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

    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax
    pop dx

    mov ax, VIDEO_SEG
    mov es, ax
    mov bx, cx
    mov ax, si
    cld

fr_row:
    mov cx, bx
    rep stosb
    add di, SCREEN_W
    sub di, bx
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
    mov ah, 02h
    mov bh, 0
    int 10h
dt_next:
    mov al, [si]
    cmp al, 0
    je dt_end
    mov ah, 0Eh
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
    mov cl, al
    mov ah, 02h
    mov bh, 0
    int 10h
    mov al, cl
    mov ah, 0
    mov cl, 10
    div cl
    mov ch, ah
    cmp al, 0
    je dn_units
    add al, '0'
    mov ah, 0Eh
    int 10h
dn_units:
    mov al, ch
    add al, '0'
    mov ah, 0Eh
    int 10h
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
    cmp al, 10
    jae dt_num
    push ax
    mov ah, 02h
    mov bh, 0
    int 10h
    mov al, '0'
    mov ah, 0Eh
    int 10h
    pop ax
    inc dl
dt_num:
    call DrawNumber
    POP_ALL
    ret
DrawTwoDigits ENDP
