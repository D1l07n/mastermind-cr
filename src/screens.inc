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

PCOL        EQU PANEL_X / 8 + 1     ; primera columna de texto del panel
PROW        EQU PANEL_Y / 8         ; primera fila de texto del panel
PSLOTH_X    EQU PANEL_X + PANEL_W - 22
PSLOTH_Y    EQU PANEL_Y + 6
SECRET_X    EQU 8 * (PCOL + 2) - 3  ; cajita del primer "?"
SECRET_Y    EQU 8 * (PROW + 4) - 3
PIECE_X     EQU 8 * (PCOL + 1) - 4  ; primera ficha del panel
PIECE_Y     EQU 8 * (PROW + 16) - 20
PANEL_OPEN_T  EQU 54        ; 54 tics = 3 segundos con los ojos abiertos
PANEL_WINK_T  EQU 9         ; 9 tics = medio segundo de guino

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

; ----------------------------------------------------------
; DrawPanelSloth: dibuja el perezoso del panel
;   Recibe: AL = cara (0 = normal, 1 = feliz, 2 = triste, 3 = guino)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawPanelSloth PROC
    PUSH_ALL
    mov bl, al
    mov bh, 0
    shl bx, 1               ; cada direccion de la tabla mide 2 bytes
    push bx
    mov ax, PSLOTH_X
    mov bx, PSLOTH_Y
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0
    call FillRect           ; borrar la cara anterior
    pop si
    mov si, PanelSloths[si]
    call DrawSprite
    POP_ALL
    ret
DrawPanelSloth ENDP

; ----------------------------------------------------------
; DrawPieceSlot: dibuja una ficha sobre su cuadrito gris
;   Recibe: AX = x, BX = y, SI = direccion del sprite
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawPieceSlot PROC
    PUSH_ALL
    push si
    sub ax, 2
    sub bx, 2
    mov cx, SPRITE_W + 4
    mov dx, SPRITE_H + 4
    mov si, C_SLOT
    call FillRect
    add ax, 2
    add bx, 2
    pop si
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    call DrawSprite
    POP_ALL
    ret
DrawPieceSlot ENDP

; ----------------------------------------------------------
; DrawPanel: dibuja el panel lateral completo
;   Recibe: AL = tipos de ficha (6 u 8), AH = fichas por intento (4 o 5)
;           CL = intento actual, CH = intentos maximos
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawPanel PROC
    PUSH_ALL
    push cx
    push ax

    mov ax, PANEL_X
    mov bx, PANEL_Y
    mov cx, PANEL_W
    mov dx, PANEL_H
    mov si, C_BORDER
    call DrawFrame
    mov al, 0
    call DrawPanelSloth

    mov si, OFFSET MsgSecreto
    mov dh, PROW + 2
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText
    pop ax
    push ax
    mov cl, ah
    mov ch, 0               ; CX = cuantos "?"
    mov ax, SECRET_X
    mov dh, PROW + 4
    mov dl, PCOL + 2
dp_secret:
    push cx
    push dx
    mov bx, SECRET_Y
    mov cx, 14
    mov dx, 14
    mov si, C_BORDER
    call DrawFrame
    pop dx
    mov si, OFFSET MsgQuestion
    mov bl, C_GOLD
    call DrawText
    add ax, 16
    add dl, 2
    pop cx
    loop dp_secret

    mov si, OFFSET MsgIntento
    mov dh, PROW + 7
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText
    pop ax
    pop cx
    push ax
    mov al, cl
    mov dl, PCOL + 8
    cmp al, 10
    jae dp_try
    inc dl                  ; un solo digito: correrlo para que pegue al "/"
dp_try:
    mov bl, C_WHITE
    call DrawNumber
    mov si, OFFSET MsgSlash
    mov dl, PCOL + 10
    call DrawText
    mov al, ch
    mov dl, PCOL + 11
    call DrawNumber

    mov si, OFFSET MsgTiempo
    mov dh, PROW + 9
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText
    mov si, OFFSET MsgTimeZero
    mov dl, PCOL + 7
    mov bl, C_WHITE
    call DrawText

    mov si, OFFSET MsgFichas
    mov dh, PROW + 12
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText

    pop ax
    mov cl, al
    mov ch, 0               ; CX = tipos de ficha
    mov di, OFFSET PieceSprites
    mov ax, PIECE_X
    mov bx, PIECE_Y
    mov dh, PROW + 16
    mov dl, PCOL + 1
dp_piece:
    push cx
    mov si, [di]
    call DrawPieceSlot
    push ax
    push bx
    mov ax, di
    sub ax, OFFSET PieceSprites
    shr ax, 1
    inc ax                  ; AL = numero de esta ficha
    mov bl, C_WHITE
    call DrawNumber
    mov cl, al
    pop bx
    pop ax
    add ax, 24
    add dl, 3
    cmp cl, 4
    jne dp_next             ; despues de la 4 se baja de fila
    mov ax, PIECE_X
    add bx, 32
    mov dl, PCOL + 1
    add dh, 4
dp_next:
    add di, 2
    pop cx
    loop dp_piece

    POP_ALL
    ret
DrawPanel ENDP

; ----------------------------------------------------------
; UpdatePanelSloth: hace guinar al perezoso del panel si ya paso su tiempo
;   Recibe: nada (usa PanelMood, PanelFrame y PanelTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdatePanelSloth PROC
    PUSH_ALL
    cmp PanelMood, 0
    jne up_end              ; feliz o triste: no guina
    call GetTicks
    mov bx, ax              ; BX = tic de ahora
    sub ax, PanelTick       ; AX = tics que pasaron
    mov cx, PANEL_OPEN_T
    cmp PanelFrame, 0
    je up_check
    mov cx, PANEL_WINK_T
up_check:
    cmp ax, cx
    jb up_end               ; todavia no le toca
    mov PanelTick, bx
    xor PanelFrame, 1       ; 0 -> 1, 1 -> 0
    mov al, 0
    cmp PanelFrame, 0
    je up_draw
    mov al, 3               ; cara de guino
up_draw:
    call DrawPanelSloth
up_end:
    POP_ALL
    ret
UpdatePanelSloth ENDP
