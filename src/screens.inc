; ==========================================================
; screens.inc - Dibujo de las pantallas del juego
; Las zonas usan las medidas del PDF. Para mover una zona
; basta con cambiar su EQU.
; ==========================================================

HEAD_X      EQU 0
HEAD_Y      EQU 0
HEAD_W      EQU 320
HEAD_H      EQU 20
BOARD_X     EQU 4
BOARD_Y     EQU 22
BOARD_W     EQU 196
BOARD_H     EQU 168
PANEL_X     EQU 204
PANEL_Y     EQU 22
PANEL_W     EQU 112
PANEL_H     EQU 168
BAR_X       EQU 0
BAR_Y       EQU 190
BAR_W       EQU 320
BAR_H       EQU 10

HSLOTH_X    EQU 4           ; perezoso del encabezado
HSLOTH_Y    EQU 2
HEAD_NORMAL_T  EQU 36       ; 36 tics = 2 segundos normal
HEAD_VICTORY_T EQU 18       ; 18 tics = 1 segundo de victoria

STRIPES     EQU 5           ; franjas de bandera a cada lado
ROW_X       EQU BOARD_X + STRIPES
ROW_W       EQU BOARD_W - 2 * STRIPES
ROW_H       EQU 14          ; 168 / 12 filas
HOLE_X      EQU BOARD_X + 12
HOLE_SIZE   EQU 10
SLOT_STEP   EQU 18          ; distancia entre fichas de una fila

; ----------------------------------------------------------
; DrawFrame: dibuja el contorno de un rectangulo (1 punto)
;   Recibe: AX = x, BX = y, CX = ancho, DX = alto, SI = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawFrame PROC
    PUSH_ALL
    push dx
    mov dx, 1
    call FillRect           ; arriba
    pop dx
    push bx
    add bx, dx
    dec bx
    push dx
    mov dx, 1
    call FillRect           ; abajo
    pop dx
    pop bx
    push cx
    mov cx, 1
    call FillRect           ; izquierda
    pop cx
    add ax, cx
    dec ax
    mov cx, 1
    call FillRect           ; derecha
    POP_ALL
    ret
DrawFrame ENDP

; ----------------------------------------------------------
; DrawHeader: dibuja el encabezado (marco, titulo y nivel)
;   Recibe: AL = nivel (1 a 3)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawHeader PROC
    PUSH_ALL
    push ax

    mov ax, HEAD_X
    mov bx, HEAD_Y
    mov cx, HEAD_W
    mov dx, HEAD_H
    mov si, C_BORDER
    call DrawFrame

    mov si, OFFSET MsgMaster
    mov dh, 1
    mov dl, 3
    mov bl, C_GOLD
    call DrawText

    mov si, OFFSET MsgTico
    mov di, OFFSET TicoColors
    mov dl, 14
    mov cx, 4
hd_tico:
    mov bl, [di]            ; color de esta letra
    call DrawText
    add si, 2               ; siguiente letra (cada una termina en 0)
    inc di
    inc dl
    loop hd_tico

    mov si, OFFSET MsgNivel
    mov dl, 31
    mov bl, C_WHITE
    call DrawText
    pop ax
    mov dl, 37
    call DrawNumber

    call DrawHeadSloth
    POP_ALL
    ret
DrawHeader ENDP

; ----------------------------------------------------------
; DrawHeadSloth: dibuja el perezoso del encabezado segun HeadFrame
;   Recibe: nada (usa HeadFrame: 0 = normal, 1 = victoria)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawHeadSloth PROC
    PUSH_ALL
    mov ax, HSLOTH_X
    mov bx, HSLOTH_Y
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0
    call FillRect           ; borrar la cara anterior
    mov si, OFFSET SprPerezoso0
    cmp HeadFrame, 0
    je hs_draw
    mov si, OFFSET SprPerezosoVictoria
hs_draw:
    call DrawSprite
    POP_ALL
    ret
DrawHeadSloth ENDP

; ----------------------------------------------------------
; UpdateHeadSloth: cambia el perezoso del encabezado si ya paso su tiempo
;   Recibe: nada (usa HeadFrame y HeadTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateHeadSloth PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax              ; BX = tic de ahora
    sub ax, HeadTick        ; AX = tics que pasaron
    mov cx, HEAD_NORMAL_T
    cmp HeadFrame, 0
    je us_check
    mov cx, HEAD_VICTORY_T
us_check:
    cmp ax, cx
    jb us_end               ; todavia no le toca
    mov HeadTick, bx
    xor HeadFrame, 1        ; 0 -> 1, 1 -> 0
    call DrawHeadSloth
us_end:
    POP_ALL
    ret
UpdateHeadSloth ENDP

; ----------------------------------------------------------
; DrawBoard: dibuja el tablero vacio (madera, bandera y huecos)
;   Recibe: CL = filas (10 o 12), CH = fichas por fila (4 o 5)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawBoard PROC
    PUSH_ALL
    push cx

    mov ax, BOARD_X
    mov bx, BOARD_Y
    mov cx, BOARD_W
    mov dx, BOARD_H
    mov si, C_WOOD
    call FillRect

    mov di, OFFSET BoardStripes
    mov ax, BOARD_X
    mov cx, STRIPES
bd_stripe:
    push cx
    mov bl, [di]
    mov bh, 0
    mov si, bx              ; SI = color de esta franja
    mov bx, BOARD_Y
    mov cx, 1
    mov dx, BOARD_H
    call FillRect           ; franja izquierda
    push ax
    neg ax
    add ax, 2 * BOARD_X + BOARD_W - 1
    call FillRect           ; la misma del lado derecho
    pop ax
    inc ax
    inc di
    pop cx
    loop bd_stripe

    pop cx
    mov bx, BOARD_Y         ; BX = y de la fila
    mov di, 0               ; DI = numero de fila
bd_row:
    push cx
    test di, 1
    jz bd_holes             ; fila par: madera clara
    mov ax, ROW_X
    mov cx, ROW_W
    mov dx, ROW_H
    mov si, C_WOOD_DK
    call FillRect
bd_holes:
    pop cx
    push cx
    mov cl, ch
    mov ch, 0               ; CX = fichas por fila
    mov ax, HOLE_X
    push bx
    add bx, (ROW_H - HOLE_SIZE) / 2
bd_hole:
    push cx
    mov cx, HOLE_SIZE
    mov dx, HOLE_SIZE
    mov si, C_HOLE
    call FillRect
    pop cx
    add ax, SLOT_STEP
    loop bd_hole
    pop bx
    pop cx
    add bx, ROW_H
    inc di
    mov ax, di
    cmp al, cl
    jb bd_row

    POP_ALL
    ret
DrawBoard ENDP
