; ==========================================================
; intro.inc - Pantalla de presentacion
; Un telon rojo de teatro se abre y queda recogido a los lados. Detras estan el titulo, el perezoso grande, los nombres
; y las tablas del escenario.
; Cualquier tecla sigue, ESC sale.
; ==========================================================

.FARDATA?
IntroBuf    DB 64000 DUP(?)     ; copia de la escena que queda detras del telon
.CODE

CURTAIN_STEP EQU 8          ; puntos que se abre el telon en cada paso
FOLD_W       EQU 4          ; ancho de cada parte de un pliegue del telon
CURTAIN_T    EQU 1          ; tics entre cada paso
INTRO_WAIT   EQU 9          ; tics con el telon cerrado antes de abrir
DRAPE_W      EQU 24         ; ancho del telon recogido (multiplo de CURTAIN_STEP)
CORD_Y       EQU 120        ; altura del cordon que recoge el telon
PINCH_ROWS   EQU 48         ; filas que se angosta el telon arriba y abajo del cordon
STAGE_Y      EQU 176        ; donde empiezan las tablas del escenario
SPOT_Y       EQU 178        ; primera fila de la luz del reflector
SPOT_ROWS    EQU 22
BIG_X        EQU 136        ; perezoso grande
BIG_Y        EQU 52
BIG_SCALE    EQU 3
PRESS_T      EQU 9          ; tics que "Presiona una tecla" se ve o se apaga

; ----------------------------------------------------------
; ShowIntro: muestra la presentacion y espera una tecla
;   Recibe: nada
;   Devuelve: AL = 1 si se apreto ESC, 0 con cualquier otra tecla
;   Toca: AX
; ----------------------------------------------------------
ShowIntro PROC
    push bx
    push cx
    push dx
    push si
    push di
    mov bl, 0
    call PaletteLight
    call DrawIntroScene
    call SaveScreen
    call DrawCurtain
    call LoadPalette
    mov cx, INTRO_WAIT
    call WaitTicks

    mov di, SCREEN_W / 2
in_open:
    sub di, CURTAIN_STEP
    mov ax, di
    mov bx, 0
    mov cx, CURTAIN_STEP
    mov dx, 200
    call RestoreStrip
    mov ax, SCREEN_W - CURTAIN_STEP
    sub ax, di
    call RestoreStrip
    mov bx, 0
    mov cx, 1
    mov dx, 200
    mov si, C_CURT_DK
    mov ax, di
    dec ax
    call FillRect
    mov ax, SCREEN_W
    sub ax, di
    call FillRect
    cmp di, DRAPE_W
    je in_open_end
    mov cx, CURTAIN_T
    call WaitTicks
    jmp in_open
in_open_end:
    call TieCurtain
    call GetTicks
    mov IntroTick, ax
    sub ax, PRESS_T
    mov PressTick, ax
in_loop:
    call UpdateIntroSloth
    call UpdatePress
    mov ah, 01h
    int 16h
    jz in_loop
    mov ah, 00h
    int 16h
    mov bl, 0
    cmp al, 27
    jne in_end
    mov bl, 1
in_end:
    mov al, bl
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    ret
ShowIntro ENDP

; ----------------------------------------------------------
; DrawCurtain: tapa toda la pantalla con el telon rojo cerrado
;   (pliegues de terciopelo, banda dorada arriba y fleco abajo)
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawCurtain PROC
    PUSH_ALL
    mov ax, 0
    mov bx, 0
    mov cx, FOLD_W
    mov dx, 200
    mov di, 0
tl_fold:
    mov si, C_CURT_LT
    cmp di, 0
    je tl_draw
    mov si, C_CURT_DK
    cmp di, 2
    je tl_draw
    mov si, C_CURT
tl_draw:
    call FillRect
    add ax, FOLD_W
    inc di
    and di, 3
    cmp ax, SCREEN_W
    jb tl_fold

    mov ax, 0
    mov cx, SCREEN_W
    mov dx, 6
    mov si, C_GOLD
    call FillRect
    mov bx, 192
    mov dx, 8
    mov cx, 2
tl_fringe:
    mov si, C_GOLD
    test ax, 2
    jz tl_tassel
    mov si, C_CURT_DK
tl_tassel:
    call FillRect
    add ax, 2
    cmp ax, SCREEN_W
    jb tl_fringe

    mov ax, SCREEN_W / 2 - 1
    mov bx, 0
    mov cx, 2
    mov dx, 200
    mov si, C_CURT_DK
    call FillRect
    POP_ALL
    ret
DrawCurtain ENDP

; ----------------------------------------------------------
; TieCurtain: angosta cada lado del telon abierto hacia el cordon
;   y dibuja el cordon dorado con su borla
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
TieCurtain PROC
    PUSH_ALL
    mov di, 0
tc_row:
    mov cx, di
    shr cx, 1
    shr cx, 1
    mov bx, CORD_Y - PINCH_ROWS
    add bx, di
    call PinchRow
    mov bx, CORD_Y + PINCH_ROWS - 1
    sub bx, di
    call PinchRow
    inc di
    cmp di, PINCH_ROWS
    jb tc_row

    mov si, C_GOLD
    mov ax, 0
    mov bx, CORD_Y - 2
    mov cx, DRAPE_W - PINCH_ROWS / 4 + 3
    mov dx, 4
    call FillRect
    mov ax, SCREEN_W - DRAPE_W + PINCH_ROWS / 4 - 3
    call FillRect
    mov ax, DRAPE_W - PINCH_ROWS / 4
    mov bx, CORD_Y + 2
    mov cx, 4
    mov dx, 8
    call FillRect
    mov ax, SCREEN_W - DRAPE_W + PINCH_ROWS / 4 - 4
    call FillRect
    POP_ALL
    ret
TieCurtain ENDP

; ----------------------------------------------------------
; PinchRow: quita CX puntos del borde de adentro de los dos lados
;   del telon en una fila y le pone su borde oscuro
;   Recibe: BX = y, CX = puntos a quitar
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
PinchRow PROC
    PUSH_ALL
    mov dx, 1
    cmp cx, 0
    je pn_edge
    mov ax, DRAPE_W
    sub ax, cx
    call RestoreStrip
    mov ax, SCREEN_W - DRAPE_W
    call RestoreStrip
pn_edge:
    mov si, C_CURT_DK
    mov ax, DRAPE_W - 1
    sub ax, cx
    push cx
    mov cx, 1
    call FillRect
    pop cx
    mov ax, SCREEN_W - DRAPE_W
    add ax, cx
    mov cx, 1
    call FillRect
    POP_ALL
    ret
PinchRow ENDP

; ----------------------------------------------------------
; SaveScreen: copia toda la pantalla a IntroBuf
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
SaveScreen PROC
    PUSH_ALL
    push ds
    push es
    cld
    mov ax, SEG IntroBuf
    mov es, ax
    mov di, OFFSET IntroBuf
    mov ax, VIDEO_SEG
    mov ds, ax
    mov si, 0
    mov cx, 32000
    rep movsw
    pop es
    pop ds
    POP_ALL
    ret
SaveScreen ENDP

; ----------------------------------------------------------
; RestoreStrip: devuelve a la pantalla una franja vertical de IntroBuf
;   Recibe: AX = x, BX = y, CX = ancho, DX = alto
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
RestoreStrip PROC
    PUSH_ALL
    push ds
    push es
    cld
    push dx
    mov di, ax
    mov ax, bx
    mov bx, SCREEN_W
    mul bx
    add di, ax
    pop dx
    mov bx, cx
    mov si, OFFSET IntroBuf
    add si, di
    mov ax, VIDEO_SEG
    mov es, ax
    mov ax, SEG IntroBuf
    mov ds, ax
rs_row:
    push si
    push di
    mov cx, bx
    rep movsb
    pop di
    pop si
    add si, SCREEN_W
    add di, SCREEN_W
    dec dx
    jnz rs_row
    pop es
    pop ds
    POP_ALL
    ret
RestoreStrip ENDP

; ----------------------------------------------------------
; DrawIntroScene: dibuja la presentacion completa sobre el fondo negro
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawIntroScene PROC
    PUSH_ALL
    mov ax, 0
    mov bx, 0
    mov cx, SCREEN_W
    mov dx, 200
    mov si, 0
    call FillRect
    mov si, OFFSET MsgMaster
    mov dh, 3
    mov dl, 15
    mov bl, C_GOLD
    call DrawText
    mov si, OFFSET MsgPura
    mov di, OFFSET TicoColors
    mov dh, 4
    mov dl, 15
    mov cx, 9
sn_pura:
    mov bl, [di]
    call DrawText
    add si, 2
    inc dl
    inc di
    cmp di, OFFSET TicoColors + 4
    jb sn_next
    mov di, OFFSET TicoColors
sn_next:
    loop sn_pura

    mov bl, C_WHITE
    mov si, OFFSET MsgAutor1
    mov dh, 14
    mov dl, 4
    call DrawText
    mov si, OFFSET MsgAutor2
    mov dh, 15
    mov dl, 10
    call DrawText
    mov si, OFFSET MsgCurso
    mov dh, 17
    mov dl, 12
    mov bl, C_BORDER
    call DrawText
    call DrawStage

    mov IntroFrame, 0
    mov si, OFFSET SprPerezoso0
    call DrawIntroSloth
    mov PressOn, 0
    POP_ALL
    ret
DrawIntroScene ENDP

; ----------------------------------------------------------
; DrawStage: dibuja las tablas del escenario con la luz del reflector
;   (un ovalo claro hecho fila por fila con los anchos de SpotHalf)
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawStage PROC
    PUSH_ALL
    mov ax, 0
    mov bx, STAGE_Y
    mov cx, SCREEN_W
    mov dx, 200 - STAGE_Y
    mov si, C_WOOD
    call FillRect

    mov di, OFFSET SpotHalf
    mov bx, SPOT_Y
    mov dx, 1
    mov si, C_STAGE_LT
st_spot:
    mov cl, [di]
    mov ch, 0
    mov ax, SCREEN_W / 2
    sub ax, cx
    shl cx, 1
    call FillRect
    inc di
    inc bx
    cmp bx, SPOT_Y + SPOT_ROWS
    jb st_spot

    mov ax, 0
    mov bx, STAGE_Y
    mov cx, SCREEN_W
    mov si, C_WOOD_DK
st_plank:
    call FillRect
    add bx, 8
    cmp bx, 200
    jb st_plank
    POP_ALL
    ret
DrawStage ENDP

; ----------------------------------------------------------
; DrawIntroSloth: dibuja el perezoso grande
;   Recibe: SI = cara del perezoso (ej. OFFSET SprPerezoso0)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawIntroSloth PROC
    PUSH_ALL
    push bp
    mov ax, BIG_X
    mov bx, BIG_Y
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov bp, BIG_SCALE
    call DrawSpriteScaled
    pop bp
    POP_ALL
    ret
DrawIntroSloth ENDP

; ----------------------------------------------------------
; UpdateIntroSloth: pasa a la siguiente cara de SlothSeq si ya paso su tiempo
;   (mira a los lados sospechoso y guina)
;   Recibe: nada (usa IntroFrame, IntroTick y SlothSeq)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateIntroSloth PROC
    PUSH_ALL
    mov bx, OFFSET IntroFrame
    mov di, OFFSET IntroTick
    call NextSlothFace
    jnc is_end
    call DrawIntroSloth
is_end:
    POP_ALL
    ret
UpdateIntroSloth ENDP

; ----------------------------------------------------------
; UpdatePress: prende o apaga "Presiona una tecla" si ya paso su tiempo
;   Recibe: nada (usa PressOn y PressTick)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdatePress PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
    sub ax, PressTick
    cmp ax, PRESS_T
    jb pr_end
    mov PressTick, bx
    xor PressOn, 1
    mov si, OFFSET MsgPresiona
    cmp PressOn, 1
    je pr_draw
    mov si, OFFSET MsgPresOff
pr_draw:
    mov dh, 20
    mov dl, 11
    mov bl, C_CURSOR
    call DrawText
pr_end:
    POP_ALL
    ret
UpdatePress ENDP

; ----------------------------------------------------------
; WaitTicks: espera que pasen unos tics del reloj
;   Recibe: CX = tics
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
WaitTicks PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
wt_loop:
    call GetTicks
    sub ax, bx
    cmp ax, cx
    jb wt_loop
    POP_ALL
    ret
WaitTicks ENDP
