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

ICON_X      EQU 4           ; trompo del encabezado
ICON_Y      EQU 2
ICON_T      EQU 7           ; 7 tics = el trompo gira unas 2.5 veces por segundo
FLAG_T      EQU 4           ; 4 tics por cada giro de colores de la bandera
DARK_T      EQU 4           ; 4 tics por cada paso del oscurecido
DARK_STEPS  EQU 8           ; 8 pasos x 4 tics = casi 2 segundos, hasta media luz
PIECE_T     EQU 7           ; 7 tics = las fichas cambian unas 2.5 veces por segundo

STRIPES     EQU 5           ; franjas de bandera a cada lado
ROW_X       EQU BOARD_X + STRIPES
ROW_W       EQU BOARD_W - 2 * STRIPES
ROW_H       EQU 14          ; 168 / 12 filas
HOLE_X      EQU BOARD_X + 12
HOLE_SIZE   EQU 10
SLOT_STEP   EQU 18          ; distancia entre fichas de una fila
PIECE_BX    EQU BOARD_X + 9 ; primera ficha de una fila del tablero
FB_X        EQU BOARD_X + 108   ; primer pejibaye de una fila
CURSOR_T    EQU 5           ; 5 tics = el marco cambia unas 4 veces por segundo

PCOL        EQU PANEL_X / 8 + 1     ; primera columna de texto del panel
PROW        EQU PANEL_Y / 8         ; primera fila de texto del panel
PSLOTH_X    EQU PANEL_X + PANEL_W - 22
PSLOTH_Y    EQU PANEL_Y + 6
SECRET_X    EQU 8 * (PCOL + 2) - 3  ; cajita del primer "?"
SECRET_Y    EQU 8 * (PROW + 4) - 3
PIECE_X     EQU 8 * (PCOL + 1) - 4  ; primera ficha del panel
PIECE_Y     EQU 8 * (PROW + 16) - 20
REVEAL_T      EQU 9         ; 9 tics = una ficha del secreto cada medio segundo
SPARK_T       EQU 2         ; 2 tics por cuadro de chispas: medio segundo en total

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
    call FillRect
    pop dx
    push bx
    add bx, dx
    dec bx
    push dx
    mov dx, 1
    call FillRect
    pop dx
    pop bx
    push cx
    mov cx, 1
    call FillRect
    pop cx
    add ax, cx
    dec ax
    mov cx, 1
    call FillRect
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
    mov bl, [di]
    call DrawText
    add si, 2
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

    call DrawHeadIcon
    POP_ALL
    ret
DrawHeader ENDP

; ----------------------------------------------------------
; DrawHeadIcon: dibuja el trompo del encabezado segun HeadFrame
;   Recibe: nada (usa HeadFrame: 0 o 1)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawHeadIcon PROC
    PUSH_ALL
    push bp
    mov ax, ICON_X
    mov bx, ICON_Y
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov bp, 0
    mov si, OFFSET SprTrompo0
    cmp HeadFrame, 0
    je hs_draw
    mov si, OFFSET SprTrompo1
hs_draw:
    call DrawSpriteBg
    pop bp
    POP_ALL
    ret
DrawHeadIcon ENDP

; ----------------------------------------------------------
; UpdateHeadIcon: hace girar el trompo si ya paso su tiempo
;   Recibe: nada (usa HeadFrame y HeadTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateHeadIcon PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
    sub ax, HeadTick
    cmp ax, ICON_T
    jb us_end
    mov HeadTick, bx
    xor HeadFrame, 1
    call DrawHeadIcon
us_end:
    POP_ALL
    ret
UpdateHeadIcon ENDP

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
    mov si, bx
    mov bx, BOARD_Y
    mov cx, 1
    mov dx, BOARD_H
    call FillRect
    push ax
    neg ax
    add ax, 2 * BOARD_X + BOARD_W - 1
    call FillRect
    pop ax
    inc ax
    inc di
    pop cx
    loop bd_stripe

    pop cx
    mov bx, BOARD_Y
    mov di, 0
bd_row:
    push cx
    test di, 1
    jz bd_holes
    mov ax, ROW_X
    mov cx, ROW_W
    mov dx, ROW_H
    mov si, C_WOOD_DK
    call FillRect
bd_holes:
    pop cx
    push cx
    mov cl, ch
    mov ch, 0
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
    shl bx, 1
    mov si, PanelSloths[bx]
    call DrawPanelFace
    POP_ALL
    ret
DrawPanelSloth ENDP

; ----------------------------------------------------------
; DrawPanelFace: borra la cara del perezoso del panel y dibuja otra
;   Recibe: SI = cara (ej. OFFSET SprPerezoso0)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawPanelFace PROC
    PUSH_ALL
    push si
    mov ax, PSLOTH_X
    mov bx, PSLOTH_Y
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0
    call FillRect
    pop si
    call DrawSprite
    POP_ALL
    ret
DrawPanelFace ENDP

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
    mov PanelFrame, 0

    mov si, OFFSET MsgSecreto
    mov dh, PROW + 2
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText
    call DrawSecretBoxes

    mov si, OFFSET MsgIntento
    mov dh, PROW + 7
    mov dl, PCOL
    mov bl, C_GOLD
    call DrawText
    pop ax
    pop cx
    push ax
    mov al, cl
    mov ah, ch
    call DrawTryCount

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
    mov ch, 0
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
    inc ax
    mov bl, C_WHITE
    call DrawNumber
    mov cl, al
    pop bx
    pop ax
    add ax, 24
    add dl, 3
    cmp cl, 4
    jne dp_next
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
; UpdatePanelSloth: pasa el perezoso del panel a la siguiente cara de
;   SlothSeq si ya paso su tiempo (la misma secuencia de la presentacion)
;   Recibe: nada (usa PanelMood, PanelFrame y PanelTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdatePanelSloth PROC
    PUSH_ALL
    cmp PanelMood, 0
    jne up_end
    mov bx, OFFSET PanelFrame
    mov di, OFFSET PanelTick
    call NextSlothFace
    jnc up_end
    call DrawPanelFace
up_end:
    POP_ALL
    ret
UpdatePanelSloth ENDP

; ----------------------------------------------------------
; NextSlothFace: avanza un paso de SlothSeq si ya paso su tiempo
;   (la usan el perezoso de la presentacion y el del panel)
;   Recibe: BX = OFFSET del paso (DB), DI = OFFSET del tic (DW)
;   Devuelve: CF = 1 y SI = cara nueva si cambio; CF = 0 si no
;   Toca: SI
; ----------------------------------------------------------
NextSlothFace PROC
    push ax
    push cx
    push dx
    call GetTicks
    mov dx, ax
    sub ax, [di]
    mov cl, [bx]
    mov ch, 0
    mov si, cx
    shl si, 1
    shl si, 1
    cmp ax, SlothSeq[si+2]
    jb ns_wait
    mov [di], dx
    inc cl
    cmp cl, SLOTH_STEPS
    jb ns_set
    mov cl, 0
ns_set:
    mov [bx], cl
    mov si, cx
    shl si, 1
    shl si, 1
    mov si, SlothSeq[si]
    stc
    jmp ns_end
ns_wait:
    clc
ns_end:
    pop dx
    pop cx
    pop ax
    ret
NextSlothFace ENDP

; ----------------------------------------------------------
; DrawStatus: borra la barra de abajo y escribe un mensaje
;   Recibe: SI = mensaje terminado en 0 (maximo 38 letras), BL = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawStatus PROC
    PUSH_ALL
    push si
    push bx
    mov ax, BAR_X
    mov bx, BAR_Y
    mov cx, BAR_W
    mov dx, BAR_H
    mov si, 0
    call FillRect
    pop bx
    pop si
    mov dh, BAR_Y / 8 + 1
    mov dl, 1
    call DrawText
    POP_ALL
    ret
DrawStatus ENDP

; ----------------------------------------------------------
; DrawTry: dibuja un intento del historial en su fila
;   Recibe: AL = numero de intento (0 = el primero)
;           usa HistGuess, HistBulls, HistCows y NumPos de game.inc
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawTry PROC
    PUSH_ALL
    mov ah, 0
    mov di, ax
    mov bl, ROW_H
    mul bl
    add ax, BOARD_Y
    mov bx, ax

    mov si, C_WOOD
    test di, 1
    jz tr_clear
    mov si, C_WOOD_DK
tr_clear:
    mov ax, ROW_X
    mov cx, ROW_W
    mov dx, ROW_H
    call FillRect

    push di
    mov ax, di
    mov cl, MAX_POS
    mul cl
    mov di, ax
    mov cl, NumPos
    mov ch, 0
    mov ax, PIECE_BX
tr_piece:
    push cx
    push ax
    mov al, HistGuess[di]
    call GetPieceSprite
    pop ax
    add si, SPRITE_W
    mov cx, SPRITE_W
    mov dx, ROW_H
    call DrawSprite
    add ax, SLOT_STEP
    inc di
    pop cx
    loop tr_piece
    pop di

    mov ax, FB_X
    mov dx, ROW_H
    mov si, OFFSET SprCompleto + SPRITE_W
    mov cl, HistBulls[di]
    mov ch, 0
    jcxz tr_cows
tr_bull:
    push cx
    mov cx, SPRITE_W
    call DrawSprite
    add ax, SPRITE_W
    pop cx
    loop tr_bull
tr_cows:
    mov si, OFFSET SprParcial + SPRITE_W
    mov cl, HistCows[di]
    mov ch, 0
    jcxz tr_end
tr_cow:
    push cx
    mov cx, SPRITE_W
    call DrawSprite
    add ax, SPRITE_W
    pop cx
    loop tr_cow
tr_end:
    POP_ALL
    ret
DrawTry ENDP

; ----------------------------------------------------------
; DrawActiveSlot: dibuja una casilla de la fila activa (hueco o ficha)
;   Recibe: AL = fila, AH = posicion (0 = la primera); usa Guess
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawActiveSlot PROC
    PUSH_ALL
    mov bl, ah
    mov bh, 0
    mov ah, 0
    mov cl, ROW_H
    mul cl
    add ax, BOARD_Y
    mov dx, ax
    mov al, SLOT_STEP
    mul bl
    add ax, PIECE_BX - 1
    mov si, bx
    mov bx, dx
    push si
    mov cx, SLOT_STEP
    mov dx, ROW_H
    mov si, C_ACTIVE
    call FillRect
    pop si
    mov dl, Guess[si]
    cmp dl, 0
    jne as_piece
    add ax, HOLE_X - PIECE_BX + 1
    add bx, (ROW_H - HOLE_SIZE) / 2
    mov cx, HOLE_SIZE
    mov dx, HOLE_SIZE
    mov si, C_SLOT
    call FillRect
    jmp as_end
as_piece:
    inc ax
    push ax
    mov al, dl
    call GetPieceSprite
    pop ax
    add si, SPRITE_W
    mov cx, SPRITE_W
    mov dx, ROW_H
    call DrawSprite
as_end:
    POP_ALL
    ret
DrawActiveSlot ENDP

; ----------------------------------------------------------
; DrawActiveRow: pinta la fila donde se esta jugando
;   Recibe: AL = fila; usa Guess y NumPos
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawActiveRow PROC
    PUSH_ALL
    push ax
    mov ah, 0
    mov cl, ROW_H
    mul cl
    add ax, BOARD_Y
    mov bx, ax
    mov ax, ROW_X
    mov cx, ROW_W
    mov dx, ROW_H
    mov si, C_ACTIVE
    call FillRect
    pop ax
    mov cl, NumPos
    mov ch, 0
    mov ah, 0
ar_slot:
    call DrawActiveSlot
    inc ah
    loop ar_slot
    POP_ALL
    ret
DrawActiveRow ENDP

; ----------------------------------------------------------
; DrawCursor: pone o quita el marco amarillo de la casilla elegida
;   Recibe: nada (usa ActiveRow, CurPos y CursorOn)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawCursor PROC
    PUSH_ALL
    mov al, ActiveRow
    mov ah, CurPos
    cmp CursorOn, 0
    jne cu_frame
    call DrawActiveSlot
    jmp cu_end
cu_frame:
    mov ah, 0
    mov cl, ROW_H
    mul cl
    add ax, BOARD_Y
    mov bx, ax
    mov al, SLOT_STEP
    mul CurPos
    add ax, PIECE_BX - 1
    mov cx, SLOT_STEP
    mov dx, ROW_H
    mov si, C_CURSOR
    call DrawFrame
cu_end:
    POP_ALL
    ret
DrawCursor ENDP

; ----------------------------------------------------------
; UpdateCursor: hace parpadear el marco si ya paso su tiempo
;   Recibe: nada (usa CursorOn y CursorTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateCursor PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
    sub ax, CursorTick
    cmp ax, CURSOR_T
    jb uc_end
    mov CursorTick, bx
    xor CursorOn, 1
    call DrawCursor
uc_end:
    POP_ALL
    ret
UpdateCursor ENDP

; ----------------------------------------------------------
; DrawTryCount: escribe "intento/maximo" en el panel
;   Recibe: AL = intento, AH = maximo
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawTryCount PROC
    PUSH_ALL
    push ax
    mov si, OFFSET MsgBlank2
    mov dh, PROW + 7
    mov dl, PCOL + 8
    mov bl, C_WHITE
    call DrawText
    pop ax
    mov dl, PCOL + 8
    cmp al, 10
    jae tc_num
    inc dl
tc_num:
    call DrawNumber
    mov si, OFFSET MsgSlash
    mov dl, PCOL + 10
    call DrawText
    mov al, ah
    mov dl, PCOL + 11
    call DrawNumber
    POP_ALL
    ret
DrawTryCount ENDP

; ----------------------------------------------------------
; DrawSecretPiece: destapa una ficha del secreto en el panel
;   Recibe: AL = posicion (0 = la primera)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSecretPiece PROC
    PUSH_ALL
    mov ah, 0
    mov di, ax
    mov cl, al
    mov bl, 1
    shl bl, cl
    or SecretShown, bl
    mov cl, 4
    shl ax, cl
    add ax, SECRET_X - 1
    mov bx, SECRET_Y - 1
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0
    call FillRect
    push ax
    mov al, Secret[di]
    call GetPieceSprite
    pop ax
    mov dx, SPRITE_H
    call DrawSprite
    POP_ALL
    ret
DrawSecretPiece ENDP

; ----------------------------------------------------------
; DrawSecret: destapa todo el secreto de una vez
;   Recibe: nada (usa NumPos de game.inc)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSecret PROC
    PUSH_ALL
    mov cl, NumPos
    mov ch, 0
    mov al, 0
sc_piece:
    call DrawSecretPiece
    inc al
    loop sc_piece
    POP_ALL
    ret
DrawSecret ENDP

; ----------------------------------------------------------
; UpdateReveal: destapa la siguiente ficha del secreto si ya paso su tiempo
;   y al terminar pone la cara feliz o triste
;   Recibe: nada (usa RevealPos, RevealTick y GameState)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateReveal PROC
    PUSH_ALL
    cmp RevealPos, 0FFh
    je rv_end
    call GetTicks
    mov bx, ax
    sub ax, RevealTick
    cmp ax, REVEAL_T
    jb rv_end
    mov RevealTick, bx
    mov al, RevealPos
    call DrawSecretPiece
    inc al
    mov RevealPos, al
    cmp al, NumPos
    jb rv_end
    mov RevealPos, 0FFh
    mov al, 1
    cmp GameState, 1
    je rv_face
    mov al, 2
rv_face:
    mov PanelMood, al
    call DrawPanelSloth
    mov al, GameState
    mov FxMode, al
    mov FxStep, 0
    call GetTicks
    mov FxTick, ax
rv_end:
    POP_ALL
    ret
UpdateReveal ENDP

; ----------------------------------------------------------
; DrawTime: escribe el tiempo como MM:SS en el panel
;   Recibe: AX = segundos
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawTime PROC
    PUSH_ALL
    mov dx, 0
    mov bx, 60
    div bx
    cmp ax, 99
    jbe ti_ok
    mov ax, 99
    mov dx, 59
ti_ok:
    mov cx, dx
    mov dh, PROW + 9
    mov dl, PCOL + 7
    mov bl, C_WHITE
    call DrawTwoDigits
    mov si, OFFSET MsgColon
    mov dl, PCOL + 9
    call DrawText
    mov al, cl
    mov dl, PCOL + 10
    call DrawTwoDigits
    POP_ALL
    ret
DrawTime ENDP

; ----------------------------------------------------------
; UpdateTimer: actualiza el reloj del panel si cambio el segundo
;   Recibe: nada (usa GameTick y ShownSec)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateTimer PROC
    PUSH_ALL
    call GetTicks
    sub ax, GameTick
    mov bx, 10
    mul bx
    mov bx, 182
    div bx
    cmp ax, ShownSec
    je ut_end
    mov ShownSec, ax
    call DrawTime
ut_end:
    POP_ALL
    ret
UpdateTimer ENDP

; ----------------------------------------------------------
; GetPieceSprite: busca el dibujo de una ficha en el cuadro actual
;   Recibe: AL = ficha (1 a 8)
;   Devuelve: SI = direccion del dibujo
;   Toca: SI
; ----------------------------------------------------------
GetPieceSprite PROC
    push bx
    mov bl, al
    dec bl
    mov bh, 0
    shl bx, 1
    mov si, PieceSprites[bx]
    cmp PieceFrame, 0
    je gp_end
    mov si, PieceSprites1[bx]
gp_end:
    pop bx
    ret
GetPieceSprite ENDP

; ----------------------------------------------------------
; DrawPieceBg: redibuja una ficha que se mueve, con su fondo
;   Recibe: AL = ficha (1 a 8), DX = x, BX = y
;           CX = filas (14 en el tablero, 16 en el panel), BP = fondo
;   Devuelve: nada. Si la ficha tiene un solo cuadro no hace nada.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawPieceBg PROC
    PUSH_ALL
    mov ah, 0
    mov di, ax
    dec di
    shl di, 1
    mov si, PieceSprites[di]
    cmp si, PieceSprites1[di]
    je pb_end
    cmp PieceFrame, 0
    je pb_frame
    mov si, PieceSprites1[di]
pb_frame:
    cmp cx, SPRITE_H
    je pb_draw
    add si, SPRITE_W
pb_draw:
    mov ax, dx
    mov dx, cx
    mov cx, SPRITE_W
    call DrawSpriteBg
pb_end:
    POP_ALL
    ret
DrawPieceBg ENDP

; ----------------------------------------------------------
; AnimPanel: mueve las fichas de la lista del panel
;   Recibe: nada (usa NumColors)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
AnimPanel PROC
    PUSH_ALL
    push bp
    mov bp, C_SLOT
    mov cl, NumColors
    mov ch, 0
    mov al, 1
    mov dx, PIECE_X
    mov bx, PIECE_Y
ap_piece:
    push cx
    mov cx, SPRITE_H
    call DrawPieceBg
    pop cx
    add dx, 24
    cmp al, 4
    jne ap_next
    mov dx, PIECE_X
    add bx, 32
ap_next:
    inc al
    loop ap_piece
    pop bp
    POP_ALL
    ret
AnimPanel ENDP

; ----------------------------------------------------------
; AnimBoard: mueve las fichas del tablero (intentos y fila activa)
;   Recibe: nada (usa TryNum, HistGuess, Guess y GameState)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
AnimBoard PROC
    PUSH_ALL
    push bp
    mov bp, C_WOOD
    mov bx, BOARD_Y
    mov di, 0
    mov cl, TryNum
    mov ch, 0
    jcxz ab_active
ab_row:
    push cx
    push di
    mov dx, PIECE_BX
    mov cl, NumPos
    mov ch, 0
ab_piece:
    push cx
    mov al, HistGuess[di]
    mov cx, ROW_H
    call DrawPieceBg
    pop cx
    add dx, SLOT_STEP
    inc di
    loop ab_piece
    pop di
    add di, MAX_POS
    add bx, ROW_H
    xor bp, C_WOOD XOR C_WOOD_DK
    pop cx
    loop ab_row

ab_active:
    cmp GameState, 0
    jne ab_end
    mov al, ActiveRow
    mov ah, 0
    mov cl, ROW_H
    mul cl
    add ax, BOARD_Y
    mov bx, ax
    mov bp, C_ACTIVE
    mov dx, PIECE_BX
    mov di, 0
    mov cl, NumPos
    mov ch, 0
ab_guess:
    push cx
    mov al, Guess[di]
    cmp al, 0
    je ab_gnext
    mov cx, ROW_H
    call DrawPieceBg
ab_gnext:
    pop cx
    add dx, SLOT_STEP
    inc di
    loop ab_guess
    cmp CursorOn, 0
    je ab_end
    call DrawCursor
ab_end:
    pop bp
    POP_ALL
    ret
AnimBoard ENDP

; ----------------------------------------------------------
; AnimSecret: mueve las fichas del secreto que ya se destaparon
;   Recibe: nada (usa SecretShown y Secret)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
AnimSecret PROC
    PUSH_ALL
    push bp
    mov bp, 0
    mov dx, SECRET_X - 1
    mov bx, SECRET_Y - 1
    mov di, 0
    mov ah, 1
    mov cl, NumPos
    mov ch, 0
an_piece:
    test SecretShown, ah
    jz an_next
    push cx
    mov al, Secret[di]
    mov cx, SPRITE_H
    call DrawPieceBg
    pop cx
an_next:
    add dx, 16
    inc di
    shl ah, 1
    loop an_piece
    pop bp
    POP_ALL
    ret
AnimSecret ENDP

; ----------------------------------------------------------
; UpdatePieces: cambia el cuadro de las fichas que se mueven
;   Recibe: nada (usa PieceFrame y PieceTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdatePieces PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
    sub ax, PieceTick
    cmp ax, PIECE_T
    jb pu_end
    mov PieceTick, bx
    xor PieceFrame, 1
    call AnimPanel
    call AnimBoard
    call AnimSecret
pu_end:
    POP_ALL
    ret
UpdatePieces ENDP

; ----------------------------------------------------------
; UpdateFx: avanza el efecto final (bandera u oscurecido) si ya paso su tiempo
;   Recibe: nada (usa FxMode, FxStep y FxTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateFx PROC
    PUSH_ALL
    cmp FxMode, 0
    je fx_end
    call GetTicks
    mov bx, ax
    sub ax, FxTick
    mov cx, FLAG_T
    cmp FxMode, 1
    je fx_check
    mov cx, DARK_T
fx_check:
    cmp ax, cx
    jb fx_end
    mov FxTick, bx
    inc FxStep
    cmp FxMode, 1
    jne fx_dark
    cmp FxStep, 3
    jb fx_flag
    mov FxStep, 0
fx_flag:
    call FlagStep
    jmp fx_end
fx_dark:
    call DarkStep
fx_end:
    POP_ALL
    ret
UpdateFx ENDP

; ----------------------------------------------------------
; FlagStep: gira los colores de las franjas (azul -> blanco -> rojo)
;   Recibe: nada (usa FxStep)
;   Devuelve: nada. Sigue girando hasta que empiece otra partida.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
FlagStep PROC
    PUSH_ALL
    mov al, FxStep
    mov ah, 0
    mov si, ax
    shl si, 1
    add si, ax
    add si, OFFSET FlagRGB
    mov al, C_FLAG_B
    call SetColor
    add si, 3
    mov al, C_FLAG_W
    call SetColor
    add si, 3
    mov al, C_FLAG_R
    call SetColor
    POP_ALL
    ret
FlagStep ENDP

; ----------------------------------------------------------
; DarkStep: oscurece un poco toda la paleta (sprites y pantallas)
;   Recibe: nada (usa FxStep, SpritePalette y UiPalette)
;   Devuelve: nada. Al llegar a DARK_STEPS queda con media luz.
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DarkStep PROC
    PUSH_ALL
    mov bl, 16
    sub bl, FxStep
    call PaletteLight
    cmp FxStep, DARK_STEPS
    jb dk_end
    mov FxMode, 0
dk_end:
    POP_ALL
    ret
DarkStep ENDP

; ----------------------------------------------------------
; DrawSecretBoxes: tapa el secreto con las cajitas de "?"
;   Recibe: nada (usa NumPos)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSecretBoxes PROC
    PUSH_ALL
    mov SecretShown, 0
    mov cl, NumPos
    mov ch, 0
    mov ax, SECRET_X
    mov dh, PROW + 4
    mov dl, PCOL + 2
dp_secret:
    push cx
    push dx
    push ax
    dec ax
    mov bx, SECRET_Y - 1
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, 0
    call FillRect
    pop ax
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
    POP_ALL
    ret
DrawSecretBoxes ENDP

; ----------------------------------------------------------
; StartSparks: en el nivel facil, si el intento tiene toros, arranca
;   las chispas sobre esas fichas
;   Recibe: AL = intento (0 = el primero)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
StartSparks PROC
    PUSH_ALL
    cmp Level, 0
    jne sk_end
    mov ah, 0
    mov di, ax
    cmp HistBulls[di], 0
    je sk_end
    cmp SparkStep, 0
    je sk_start
    push ax
    mov al, SparkRow
    call DrawTry
    pop ax
sk_start:
    mov SparkRow, al
    mov SparkStep, 1
    call GetTicks
    sub ax, SPARK_T
    mov SparkTick, ax
sk_end:
    POP_ALL
    ret
StartSparks ENDP

; ----------------------------------------------------------
; UpdateSparks: pasa las chispas al siguiente cuadro si ya paso su tiempo
;   Recibe: nada (usa SparkStep, SparkRow y SparkTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateSparks PROC
    PUSH_ALL
    cmp SparkStep, 0
    je us2_end
    call GetTicks
    mov bx, ax
    sub ax, SparkTick
    cmp ax, SPARK_T
    jb us2_end
    mov SparkTick, bx
    mov al, SparkRow
    call DrawTry
    cmp SparkStep, 4
    jb us2_draw
    mov SparkStep, 0
    jmp us2_end
us2_draw:
    call DrawSparkFrame
    inc SparkStep
us2_end:
    POP_ALL
    ret
UpdateSparks ENDP

; ----------------------------------------------------------
; DrawSparkFrame: dibuja un cuadro de chispas en cada toro de la fila
;   Recibe: nada (usa SparkStep, SparkRow, HistGuess, Secret y NumPos)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSparkFrame PROC
    PUSH_ALL
    mov al, SparkRow
    mov ah, 0
    mov cl, ROW_H
    mul cl
    add ax, BOARD_Y
    mov bx, ax
    mov al, SparkRow
    mov ah, 0
    mov cl, MAX_POS
    mul cl
    mov di, ax
    mov dx, PIECE_BX
    mov si, 0
    mov cl, NumPos
    mov ch, 0
sf_pos:
    mov al, HistGuess[di]
    cmp al, Secret[si]
    jne sf_next
    call DrawSparks
sf_next:
    add dx, SLOT_STEP
    inc di
    inc si
    loop sf_pos
    POP_ALL
    ret
DrawSparkFrame ENDP

; ----------------------------------------------------------
; DrawSparks: dibuja las chispas del cuadro actual alrededor de una ficha
;   (en los cuadros 1 y 2 son crucecitas, en el 3 puntitos)
;   Recibe: DX = x de la ficha, BX = y (usa SparkStep)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawSparks PROC
    PUSH_ALL
    push bp
    mov al, SparkStep
    dec al
    mov ah, 0
    mov di, ax
    mov cl, SparkColors[di]
    mov ch, 0
    mov bp, cx
    shl di, 1
    mov di, SparkFrames[di]
    mov cl, [di]
    inc di
dk_one:
    push cx
    push bx
    push dx
    mov al, [di]
    cbw
    add ax, dx
    mov cl, [di + 1]
    mov ch, 0
    add bx, cx
    mov si, bp
    mov cx, 1
    mov dx, 1
    cmp SparkStep, 3
    je dk_dot
    dec ax
    mov cx, 3
    call FillRect
    inc ax
    dec bx
    mov cx, 1
    mov dx, 3
dk_dot:
    call FillRect
    pop dx
    pop bx
    pop cx
    add di, 2
    loop dk_one
    pop bp
    POP_ALL
    ret
DrawSparks ENDP
