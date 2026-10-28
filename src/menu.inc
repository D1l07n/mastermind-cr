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
MENU_ROW    EQU 8           ; fila de texto de la primera opcion
MENU_T      EQU 5           ; tics entre cada vuelta del trompo

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
MsgMenuAyuda DB 'Flechas elegir   ENTER aceptar', 0

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
    call UpdateMenuCursor   ; el trompo gira mientras se espera
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
    jne rm_loop             ; otra tecla: no hace nada
    mov al, MENU_EXIT       ; ESC = salir
    jmp rm_end

rm_up:
    mov bl, MenuSel
    cmp bl, 0
    jne rm_up2
    mov bl, MENU_ITEMS      ; de la primera salta a la ultima
rm_up2:
    dec bl
    jmp rm_move
rm_down:
    mov bl, MenuSel
    inc bl
    cmp bl, MENU_ITEMS
    jb rm_move
    mov bl, 0               ; de la ultima vuelve a la primera
rm_move:
    mov al, 0
    call DrawMenuCursor     ; borrar el marco de la vieja
    mov al, MenuSel
    mov MenuSel, bl
    call DrawMenuItem       ; la vieja vuelve a blanco
    mov al, bl
    call DrawMenuItem       ; la nueva en amarillo
    mov al, 1
    call DrawMenuCursor     ; marco en la nueva
    jmp rm_loop

rm_enter:
    mov al, MenuSel
    cmp al, MENU_LEVEL
    jne rm_end              ; cualquier otra: se la devolvemos a main
rm_right:
    cmp MenuSel, MENU_LEVEL
    jne rm_loop             ; las flechas de lado solo sirven en Nivel
    mov al, Level
    inc al                  ; siguiente nivel
    cmp al, 3
    jb rm_set
    mov al, 0               ; despues de dificil vuelve a facil
    jmp rm_set
rm_left:
    cmp MenuSel, MENU_LEVEL
    jne rm_loop
    mov al, Level
    cmp al, 0
    jne rm_left2
    mov al, 3               ; antes de facil va dificil
rm_left2:
    dec al                  ; nivel anterior
rm_set:
    call SetLevel
    mov al, MENU_LEVEL
    call DrawMenuItem       ; escribir el nombre del nivel nuevo
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
    call LoadPalette        ; por si la partida dejo los colores oscuros
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect           ; pantalla en negro

    mov si, OFFSET MsgMaster
    mov dh, 3
    mov dl, 15
    mov bl, C_GOLD
    call DrawText           ; titulo

    mov di, 0               ; DI = franja (0..4)
    mov bx, 36              ; y de la primera franja
dm_flag:
    mov al, BoardStripes[di]
    mov ah, 0
    mov si, ax              ; color de la franja
    mov ax, 120
    mov cx, 80
    mov dx, 2
    call FillRect           ; bandera: azul, blanco, rojo, blanco, azul
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
    call DrawSpriteScaled   ; perezoso grande a la derecha

    mov al, 0               ; AL = opcion
dm_item:
    call DrawMenuItem
    inc al
    cmp al, MENU_ITEMS
    jb dm_item

    mov si, OFFSET MsgMenuAyuda
    mov dh, 22
    mov dl, 5
    mov bl, C_WHITE
    call DrawText
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
    mov bl, C_CURSOR        ; la elegida va en amarillo
mi_color:
    mov ah, 0
    mov si, ax
    shl si, 1               ; SI = opcion * 2 (cada direccion mide 2 bytes)
    mov si, MenuTexts[si]
    mov dh, al
    shl dh, 1
    add dh, MENU_ROW        ; fila = MENU_ROW + opcion * 2
    mov dl, MENU_COL
    call DrawText
    cmp al, MENU_LEVEL
    jne mi_end
    mov al, Level           ; la opcion del nivel dice cual es
    mov ah, 0
    mov si, ax
    shl si, 1
    mov si, LevelNames[si]
    add dl, 7               ; despues de "Nivel: "
    call DrawText
mi_end:
    POP_ALL
    ret
DrawMenuItem ENDP

; ----------------------------------------------------------
; DrawMenuCursor: dibuja o borra el marco y el trompo de la opcion elegida
;   Recibe: AL = 1 dibujar, 0 borrar (usa MenuSel y MenuFrame)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawMenuCursor PROC
    PUSH_ALL
    push bp
    mov bl, MenuSel
    mov bh, 0
    mov cl, 4
    shl bx, cl              ; BX = opcion * 16 (2 filas de texto)
    add bx, MENU_ROW * 8 - 4    ; y del marco
    mov si, 0               ; color 0 = negro: borra
    cmp al, 0
    je mc_frame
    mov si, C_CURSOR
mc_frame:
    mov ax, MENU_COL * 8 - 6
    mov cx, 15 * 8 + 4
    mov dx, 16
    call DrawFrame
    mov ax, MENU_COL * 8 - 26   ; x del trompo
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    cmp si, 0
    jne mc_top
    call FillRect           ; borrar el trompo (SI ya es 0)
    jmp mc_end
mc_top:
    mov si, OFFSET SprTrompo0
    cmp MenuFrame, 0
    je mc_draw
    mov si, OFFSET SprTrompo1
mc_draw:
    mov bp, 0               ; fondo negro: no quedan restos del otro cuadro
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
    jb um_end               ; todavia no le toca
    mov MenuTick, bx
    xor MenuFrame, 1        ; 0 -> 1 y 1 -> 0
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
    call FillRect           ; pantalla en negro

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
    mov bx, 44
    call DrawExampleRow     ; fila del secreto
    mov bl, C_WHITE
    mov si, OFFSET MsgInsTry
    mov dh, 9
    mov dl, 2
    call DrawText
    mov si, OFFSET ExGuess
    mov bx, 68
    call DrawExampleRow     ; fila del intento

    mov cx, SPRITE_W
    mov dx, SPRITE_H
    mov si, OFFSET SprCompleto
    mov ax, 190
    mov bx, 68
    call DrawSprite         ; resultado: 1 toro...
    mov ax, 16
    mov bx, 92
    call DrawSprite         ; ...y la explicacion de abajo
    mov si, OFFSET SprParcial
    mov ax, 210
    mov bx, 68
    call DrawSprite         ; ...y 1 vaca
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
    POP_ALL
    ret
ShowInstructions ENDP

; ----------------------------------------------------------
; DrawExampleRow: dibuja 4 fichas en fila para el ejemplo
;   Recibe: SI = 4 bytes con las fichas (1..8), BX = y
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawExampleRow PROC
    PUSH_ALL
    mov di, si              ; DI = ficha que sigue
    mov ax, 96              ; x de la primera ficha
    mov cx, 4
er_next:
    push ax
    mov al, [di]
    call GetPieceSprite     ; SI = dibujo de esa ficha
    pop ax
    push cx
    mov cx, SPRITE_W
    mov dx, SPRITE_H
    call DrawSprite
    pop cx
    add ax, 20              ; la siguiente, 20 puntos a la derecha
    inc di
    loop er_next
    POP_ALL
    ret
DrawExampleRow ENDP

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
    call FillRect           ; pantalla en negro

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
    call DrawSpriteScaled   ; perezoso feliz, el doble de grande
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
    int 16h                 ; esperar la tecla
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
    mov RevealPos, 0FFh     ; nada que destapar
    mov PanelMood, 0        ; perezoso normal
    mov TestMode, 0
    mov WarnOn, 0
    mov PieceFrame, 0
    call GetTicks
    mov HeadTick, ax        ; todos los relojes empiezan ahora
    mov PanelTick, ax
    mov CursorTick, ax
    mov GameTick, ax
    mov PieceTick, ax
    call DrawGameScreen
    POP_ALL
    ret
StartGame ENDP