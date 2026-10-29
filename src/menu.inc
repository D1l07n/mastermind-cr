; ==========================================================
; menu.inc - Menu principal, instrucciones y creditos
; Flechas arriba y abajo eligen, ENTER acepta, ESC sale.
; ==========================================================

MENU_PLAY   EQU 0           ; numero de cada opcion
MENU_LEVEL  EQU 1
MENU_HELP   EQU 2
MENU_CRED   EQU 3
MENU_EXIT   EQU 4
MENU_ITEMS  EQU 5           ; cuantas opciones hay
MENU_COL    EQU 14          ; columna de texto de las opciones
MENU_ROW    EQU 7           ; fila de texto de la primera opcion
MENU_T      EQU 5           ; tics entre cada vuelta del trompo
HOW_X       EQU 96          ; segunda pagina de instrucciones: primera ficha
HOW_STEP    EQU 24          ; distancia entre fichas
HOW_SY      EQU 40          ; y del secreto
HOW_GY      EQU 80          ; y del intento

.DATA
MenuSel     DB 0            ; opcion elegida (0..4)
MenuFrame   DB 0            ; cuadro del trompo (0 o 1)
MenuTick    DW 0            ; tic del ultimo cambio del trompo
MsgJugar    DB 'Jugar', 0
MsgNivelM   DB 'Nivel: ', 0
MsgInstr    DB 'Instrucciones', 0
MsgCreditos DB 'Cr', 130, 'ditos', 0
MsgSalir    DB 'Salir', 0
MenuTexts   DW OFFSET MsgJugar, OFFSET MsgNivelM, OFFSET MsgInstr
            DW OFFSET MsgCreditos, OFFSET MsgSalir
MsgFacil    DB 'F', 160, 'cil  ', 0     ; los 3 del mismo largo
MsgNormal   DB 'Normal ', 0             ; para que tapen al anterior
MsgDificil  DB 'Dif', 161, 'cil', 0
LevelNames  DW OFFSET MsgFacil, OFFSET MsgNormal, OFFSET MsgDificil

.CODE
; ----------------------------------------------------------
; RunMenu: muestra el menu y espera hasta que se elija algo
;   Recibe: nada
;   Devuelve: AL = MENU_PLAY, MENU_HELP, MENU_CRED o MENU_EXIT
;             (el nivel se cambia aqui mismo con SetLevel)
;   Toca: AX
; ----------------------------------------------------------
RunMenu PROC
    push bx
    call DrawMenu
    call GetTicks
    mov MenuTick, ax
rm_loop:
    call UpdateMenuCursor
    call PollAction
    cmp al, ACT_NONE
    je rm_loop
    cmp al, ACT_UP
    je rm_up
    cmp al, ACT_DOWN
    je rm_down
    cmp al, ACT_ENTER
    je rm_enter
    cmp al, ACT_LEFT
    je rm_left
    cmp al, ACT_RIGHT
    je rm_right
    cmp al, ACT_ESC
    jne rm_loop
    mov al, MENU_EXIT
    jmp rm_end

rm_up:
    mov bl, MenuSel
    cmp bl, 0
    jne rm_up2
    mov bl, MENU_ITEMS
rm_up2:
    dec bl
    jmp rm_move
rm_down:
    mov bl, MenuSel
    inc bl
    cmp bl, MENU_ITEMS
    jb rm_move
    mov bl, 0
rm_move:
    mov al, 0
    call DrawMenuCursor
    mov al, MenuSel
    mov MenuSel, bl
    call DrawMenuItem
    mov al, bl
    call DrawMenuItem
    mov al, 1
    call DrawMenuCursor
    jmp rm_loop

rm_enter:
    mov al, MenuSel
    cmp al, MENU_LEVEL
    jne rm_end
rm_right:
    cmp MenuSel, MENU_LEVEL
    jne rm_loop
    mov al, Level
    inc al
    cmp al, 3
    jb rm_set
    mov al, 0
    jmp rm_set
rm_left:
    cmp MenuSel, MENU_LEVEL
    jne rm_loop
    mov al, Level
    cmp al, 0
    jne rm_left2
    mov al, 3
rm_left2:
    dec al
rm_set:
    call SetLevel
    mov al, MENU_LEVEL
    call DrawMenuItem
    jmp rm_loop
rm_end:
    pop bx
    ret
RunMenu ENDP

; ----------------------------------------------------------
; DrawMenu: dibuja toda la pantalla del menu
;   Recibe: nada (usa MenuSel y Level)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawMenu PROC
    PUSH_ALL
    push bp
    call LoadPalette
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect

    mov si, OFFSET MsgMaster
    mov dh, 3
    mov dl, 15
    mov bl, C_GOLD
    call DrawText

    mov di, 0
    mov bx, 36
dm_flag:
    mov al, BoardStripes[di]
    mov ah, 0
    mov si, ax
    mov ax, 120
    mov cx, 80
    mov dx, 2
    call FillRect
    add bx, 2
    inc di
    cmp di, 5
    jb dm_flag

    mov si, OFFSET SprPerezoso0
    mov ax, 248
    mov bx, 80
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov bp, 3
    call DrawSpriteScaled

    mov al, 0
    mov si, C_BORDER
dm_item:
    call DrawMenuItem
    call DrawItemBox
    inc al
    cmp al, MENU_ITEMS
    jb dm_item

    mov al, 1
    call DrawMenuCursor
    pop bp
    POP_ALL
    ret
DrawMenu ENDP

; ----------------------------------------------------------
; DrawMenuItem: escribe una opcion del menu
;   Recibe: AL = opcion (0..4). La elegida sale en amarillo.
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawMenuItem PROC
    PUSH_ALL
    mov bl, C_WHITE
    cmp al, MenuSel
    jne mi_color
    mov bl, C_CURSOR
mi_color:
    mov ah, 0
    mov si, ax
    shl si, 1
    mov si, MenuTexts[si]
    mov dh, al
    shl dh, 1
    add dh, al
    add dh, MENU_ROW
    mov dl, MENU_COL
    call DrawText
    cmp al, MENU_LEVEL
    jne mi_end
    mov al, Level
    mov ah, 0
    mov si, ax
    shl si, 1
    mov si, LevelNames[si]
    add dl, 7
    call DrawText
mi_end:
    POP_ALL
    ret
DrawMenuItem ENDP

; ----------------------------------------------------------
; DrawItemBox: dibuja la cajita de una opcion del menu
;   Recibe: AL = opcion (0..4), SI = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawItemBox PROC
    PUSH_ALL
    mov bl, al
    mov bh, 0
    mov cl, 3
    shl bx, cl
    mov dx, bx
    shl bx, 1
    add bx, dx
    add bx, MENU_ROW * 8 - 4
    mov ax, MENU_COL * 8 - 6
    mov cx, 15 * 8 + 4
    mov dx, 16
    call DrawFrame
    POP_ALL
    ret
DrawItemBox ENDP

; ----------------------------------------------------------
; DrawMenuCursor: marca la opcion elegida (cajita amarilla y trompo)
;   o le quita la marca (cajita cafe y sin trompo)
;   Recibe: AL = 1 marcar, 0 quitar (usa MenuSel y MenuFrame)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawMenuCursor PROC
    PUSH_ALL
    push bp
    mov di, ax
    mov si, C_BORDER
    cmp al, 0
    je mc_box
    mov si, C_CURSOR
mc_box:
    mov al, MenuSel
    call DrawItemBox
    mov ah, 0
    mov bx, ax
    mov cl, 3
    shl bx, cl
    mov dx, bx
    shl bx, 1
    add bx, dx
    add bx, MENU_ROW * 8 - 4
    mov ax, MENU_COL * 8 - 26
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    and di, 0FFh
    jnz mc_top
    mov si, 0
    call FillRect
    jmp mc_end
mc_top:
    mov si, OFFSET SprTrompo0
    cmp MenuFrame, 0
    je mc_draw
    mov si, OFFSET SprTrompo1
mc_draw:
    mov bp, 0
    call DrawSpriteBg
mc_end:
    pop bp
    POP_ALL
    ret
DrawMenuCursor ENDP

; ----------------------------------------------------------
; UpdateMenuCursor: hace girar el trompo si ya paso su tiempo
;   Recibe: nada (usa MenuTick y MenuFrame)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
UpdateMenuCursor PROC
    PUSH_ALL
    call GetTicks
    mov bx, ax
    sub ax, MenuTick
    cmp ax, MENU_T
    jb um_end
    mov MenuTick, bx
    xor MenuFrame, 1
    mov al, 1
    call DrawMenuCursor
um_end:
    POP_ALL
    ret
UpdateMenuCursor ENDP

; ---------- Instrucciones ----------
.DATA
MsgInsTit   DB 'INSTRUCCIONES', 0
MsgIns1     DB 'Adivina las fichas escondidas.', 0
MsgInsSec   DB 'Secreto', 0
MsgInsTry   DB 'Intento', 0
MsgIns2     DB '= ficha y lugar correctos', 0
MsgIns3     DB '= ficha correcta, otro lugar', 0
MsgIns4     DB 'Los pejibayes dicen CUANTOS aciertos,', 0
MsgIns5     DB 'no en cual casilla.', 0
MsgIns6     DB 'Flechas mover, 1-8 ficha, ENTER probar', 0
MsgIns7     DB 'BORRAR quita ficha, ESC vuelve al men', 163, 0
ExSecret    DB 3, 8, 7, 5   ; ejemplo: cafe, trompo, moneda, farol
ExGuess     DB 3, 7, 6, 1   ; cafe bien puesto, moneda en otro lugar
; segunda pagina: el toro en la casilla 3
HowSecret   DB 3, 8, 7, 5   ; cafe, trompo, moneda, farol
HowGuess    DB 1, 5, 7, 6   ; antorcha, farol, moneda, guante
MsgHowTit   DB 168, 'C', 162, 'mo se cuentan?', 0
MsgHowRes   DB 'Resultado', 0
MsgHowL1    DB 'verde = TORO: ficha y lugar', 0
MsgHowL2    DB 'amarillo = VACA: ficha, otro lugar', 0
MsgHowL3    DB 'X = no est', 160, ' en el secreto', 0
MsgHowE1    DB 'El toro est', 160, ' en la casilla 3,', 0
MsgHowE2    DB 'pero su pejibaye sale PRIMERO.', 0
MsgHowE3    DB 'Siempre salen primero los toros', 0
MsgHowE4    DB 'y despu', 130, 's las vacas: los pejibayes', 0
MsgHowE5    DB 'NO dicen en cu', 160, 'l casilla.', 0

.CODE
; ----------------------------------------------------------
; ShowInstructions: explica el juego con un ejemplo dibujado
;   Recibe: nada
;   Devuelve: nada (espera una tecla)
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
ShowInstructions PROC
    PUSH_ALL
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect

    mov bl, C_GOLD
    mov si, OFFSET MsgInsTit
    mov dh, 1
    mov dl, 13
    call DrawText
    mov bl, C_WHITE
    mov si, OFFSET MsgIns1
    mov dh, 3
    mov dl, 1
    call DrawText

    mov si, OFFSET MsgInsSec
    mov dh, 6
    mov dl, 2
    call DrawText
    mov si, OFFSET ExSecret
    mov ax, 96
    mov bx, 44
    mov dx, 20
    call DrawExampleRow
    mov bl, C_WHITE
    mov si, OFFSET MsgInsTry
    mov dh, 9
    mov dl, 2
    call DrawText
    mov si, OFFSET ExGuess
    mov ax, 96
    mov bx, 68
    mov dx, 20
    call DrawExampleRow

    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, OFFSET SprCompleto
    mov ax, 190
    mov bx, 68
    call DrawSprite
    mov ax, 16
    mov bx, 92
    call DrawSprite
    mov si, OFFSET SprParcial
    mov ax, 210
    mov bx, 68
    call DrawSprite
    mov ax, 16
    mov bx, 116
    call DrawSprite

    mov bl, C_WHITE
    mov si, OFFSET MsgIns2
    mov dh, 12
    mov dl, 5
    call DrawText
    mov si, OFFSET MsgIns3
    mov dh, 15
    call DrawText
    mov bl, C_GOLD
    mov si, OFFSET MsgIns4
    mov dh, 17
    mov dl, 1
    call DrawText
    mov si, OFFSET MsgIns5
    mov dh, 18
    call DrawText
    mov bl, C_WHITE
    mov si, OFFSET MsgIns6
    mov dh, 20
    call DrawText
    mov si, OFFSET MsgIns7
    mov dh, 22
    call DrawText
    call WaitKeyMsg
    call ShowHowCount
    POP_ALL
    ret
ShowInstructions ENDP

; ----------------------------------------------------------
; DrawExampleRow: dibuja 4 fichas en fila para el ejemplo
;   Recibe: SI = 4 bytes con las fichas (1..8), AX = x de la primera,
;           BX = y, DX = distancia entre fichas
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawExampleRow PROC
    PUSH_ALL
    push bp
    mov bp, dx
    mov di, si
    mov cx, 4
er_next:
    push ax
    mov al, [di]
    call GetPieceSprite
    pop ax
    push cx
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    call DrawSprite
    pop cx
    add ax, bp
    inc di
    loop er_next
    pop bp
    POP_ALL
    ret
DrawExampleRow ENDP

; ----------------------------------------------------------
; ShowHowCount: segunda pagina de instrucciones. Marca en colores
;   cual ficha es toro, cual vaca y cual no esta, y explica que los
;   pejibayes no dicen en cual casilla
;   Recibe: nada
;   Devuelve: nada (espera una tecla)
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
ShowHowCount PROC
    PUSH_ALL
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect

    mov si, OFFSET MsgHowTit
    mov dh, 1
    mov dl, 11
    mov bl, C_GOLD
    call DrawText
    mov al, 1
    mov dh, 3
    mov dl, 13
    mov bl, C_BORDER
hc_num:
    call DrawNumber
    inc al
    add dl, 3
    cmp al, 4
    jbe hc_num

    mov bl, C_WHITE
    mov si, OFFSET MsgInsSec
    mov dh, 5
    mov dl, 2
    call DrawText
    mov si, OFFSET MsgInsTry
    mov dh, 10
    call DrawText
    mov si, OFFSET MsgHowRes
    mov dh, 8
    mov dl, 26
    call DrawText
    mov si, OFFSET HowSecret
    mov ax, HOW_X
    mov bx, HOW_SY
    mov dx, HOW_STEP
    call DrawExampleRow
    mov si, OFFSET HowGuess
    mov bx, HOW_GY
    call DrawExampleRow
    call DrawHowMarks

    mov si, OFFSET MsgHowL1
    mov dh, 13
    mov dl, 1
    mov bl, C_GREEN
    call DrawText
    mov si, OFFSET MsgHowL2
    mov dh, 14
    mov bl, C_CURSOR
    call DrawText
    mov si, OFFSET MsgHowL3
    mov dh, 15
    mov bl, C_RED
    call DrawText
    mov bl, C_WHITE
    mov si, OFFSET MsgHowE1
    mov dh, 17
    call DrawText
    mov si, OFFSET MsgHowE2
    mov dh, 18
    call DrawText
    mov bl, C_GOLD
    mov si, OFFSET MsgHowE3
    mov dh, 20
    call DrawText
    mov si, OFFSET MsgHowE4
    mov dh, 21
    call DrawText
    mov si, OFFSET MsgHowE5
    mov dh, 22
    call DrawText
    call WaitKeyMsg
    POP_ALL
    ret
ShowHowCount ENDP

; ----------------------------------------------------------
; DrawHowMarks: dibuja las marcas del ejemplo de la segunda pagina:
;   toro en verde (casilla 3), vaca en amarillo (farol de la 4 a la 2),
;   X roja en las que no estan y los pejibayes del resultado
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawHowMarks PROC
    PUSH_ALL
    mov cx, 20
    mov dx, 20
    mov si, C_GREEN
    mov ax, HOW_X + 2 * HOW_STEP - 2
    mov bx, HOW_SY - 2
    call DrawFrame
    mov bx, HOW_GY - 2
    call DrawFrame
    mov ax, HOW_X + 2 * HOW_STEP + 8
    mov bx, HOW_SY + 18
    mov cx, 1
    mov dx, HOW_GY - HOW_SY - 21
    call FillRect

    mov cx, 20
    mov dx, 20
    mov si, C_CURSOR
    mov ax, HOW_X + 3 * HOW_STEP - 2
    mov bx, HOW_SY - 2
    call DrawFrame
    mov ax, HOW_X + HOW_STEP - 2
    mov bx, HOW_GY - 2
    call DrawFrame
    mov ax, HOW_X + 3 * HOW_STEP + 8
    mov bx, HOW_SY + 18
    mov cx, 1
    mov dx, 10
    call FillRect
    mov ax, HOW_X + HOW_STEP + 8
    mov bx, HOW_SY + 27
    mov cx, 2 * HOW_STEP + 1
    mov dx, 1
    call FillRect
    mov cx, 1
    mov dx, HOW_GY - HOW_SY - 30
    call FillRect

    mov si, C_RED
    mov ax, HOW_X
    mov bx, HOW_GY
    call DrawCross
    mov ax, HOW_X + 3 * HOW_STEP
    call DrawCross

    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, OFFSET SprCompleto
    mov ax, 216
    call DrawSprite
    mov si, OFFSET SprParcial
    mov ax, 236
    call DrawSprite
    POP_ALL
    ret
DrawHowMarks ENDP

; ----------------------------------------------------------
; DrawCross: dibuja una X de 16x16 encima de una ficha
;   Recibe: AX = x, BX = y, SI = color
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawCross PROC
    PUSH_ALL
    mov di, 0
    mov cx, 1
    mov dx, 1
cr_dot:
    push ax
    push bx
    add ax, di
    add bx, di
    call FillRect
    sub ax, di
    add ax, 15
    sub ax, di
    call FillRect
    pop bx
    pop ax
    inc di
    cmp di, 16
    jb cr_dot
    POP_ALL
    ret
DrawCross ENDP

; ---------- Creditos ----------
.DATA
MsgCredTit  DB 'CR', 144, 'DITOS', 0
MsgHecho    DB 'Hecho por:', 0
MsgCurso2   DB 'Lenguaje Ensamblador', 0
MsgTasm     DB 'Hecho con TASM y DOSBox', 0

.CODE
; ----------------------------------------------------------
; ShowCredits: muestra quien hizo el juego
;   Recibe: nada
;   Devuelve: nada (espera una tecla)
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
ShowCredits PROC
    PUSH_ALL
    push bp
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect

    mov bl, C_GOLD
    mov si, OFFSET MsgCredTit
    mov dh, 2
    mov dl, 16
    call DrawText
    mov bl, C_WHITE
    mov si, OFFSET MsgHecho
    mov dh, 5
    mov dl, 15
    call DrawText
    mov bl, C_CURSOR
    mov si, OFFSET MsgAutor1
    mov dh, 7
    mov dl, 4
    call DrawText
    mov si, OFFSET MsgAutor2
    mov dh, 9
    mov dl, 10
    call DrawText
    mov bl, C_WHITE
    mov si, OFFSET MsgCurso
    mov dh, 11
    mov dl, 12
    call DrawText
    mov si, OFFSET MsgCurso2
    mov dh, 13
    mov dl, 10
    call DrawText
    mov si, OFFSET MsgTasm
    mov dh, 16
    mov dl, 8
    call DrawText

    mov si, OFFSET SprPerezosoVictoria
    mov ax, 144
    mov bx, 156
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov bp, 2
    call DrawSpriteScaled
    call WaitKeyMsg
    pop bp
    POP_ALL
    ret
ShowCredits ENDP

; ----------------------------------------------------------
; WaitKeyMsg: escribe "Presiona una tecla" abajo y espera una
;   Recibe: nada
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
WaitKeyMsg PROC
    PUSH_ALL
    mov si, OFFSET MsgPresiona
    mov dh, 24
    mov dl, 11
    mov bl, C_CURSOR
    call DrawText
    mov ah, 00h
    int 16h
    POP_ALL
    ret
WaitKeyMsg ENDP

; ----------------------------------------------------------
; StartGame: arma una partida nueva y dibuja su pantalla
;   Recibe: nada (usa el nivel elegido)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
StartGame PROC
    PUSH_ALL
    call NewGame
    mov RevealPos, 0FFh
    mov PanelMood, 0
    mov TestMode, 0
    mov TestSecret, 0
    mov WarnOn, 0
    mov PieceFrame, 0
    call GetTicks
    mov HeadTick, ax
    mov PanelTick, ax
    mov CursorTick, ax
    mov GameTick, ax
    mov PieceTick, ax
    call DrawGameScreen
    POP_ALL
    ret
StartGame ENDP
