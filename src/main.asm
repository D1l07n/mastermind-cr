; ==========================================================
; main.asm - Orquestador del juego Mastermind Pura Vida
; Une todas las partes con INCLUDE y decide el orden.
; ==========================================================
.MODEL SMALL
.STACK 400h
JUMPS                       ; TASM alarga solo los saltos que quedan lejos

INCLUDE macros.inc

.DATA
INCLUDE sprites.inc
INCLUDE uidata.inc
OldVideoMode DB 0           ; modo de video que habia antes de arrancar

.CODE
INCLUDE graphics.inc
INCLUDE input.inc
INCLUDE game.inc
INCLUDE screens.inc
INCLUDE intro.inc
INCLUDE menu.inc

start:
    mov ax, @data
    mov ds, ax

    call SaveVideoMode
    call SetGraphicsMode
    call LoadPalette
    call ShowIntro
    cmp al, 1
    je exit_game

    call InitRandom
    mov al, 0
    call SetLevel

menu_loop:
    call RunMenu
    cmp al, MENU_EXIT
    je exit_game
    cmp al, MENU_HELP
    jne ml_cred
    call ShowInstructions
    jmp menu_loop
ml_cred:
    cmp al, MENU_CRED
    jne new_game
    call ShowCredits
    jmp menu_loop

new_game:
    call StartGame

main_loop:
    call UpdateHeadIcon
    call UpdatePieces
    call UpdatePanelSloth
    call UpdateReveal
    call UpdateFx
    call UpdateSparks
    cmp GameState, 0
    jne ml_key
    call UpdateCursor
    call UpdateTimer
ml_key:
    call PollAction
    cmp al, ACT_NONE
    je main_loop
    cmp al, ACT_ESC
    je menu_loop
    cmp GameState, 0
    je ml_play
    cmp al, ACT_ENTER
    je new_game
    cmp al, ACT_TEST
    jne main_loop
ml_play:
    call GameKey
    jmp main_loop

exit_game:
    call RestoreVideoMode
    mov ah, 4Ch
    mov al, 0
    int 21h

; ----------------------------------------------------------
; DrawGameScreen: dibuja la pantalla de juego del nivel actual
;   Recibe: nada (usa Level, NumPos, NumColors, MaxTries y TryNum)
;   Devuelve: nada
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
DrawGameScreen PROC
    PUSH_ALL
    mov FxMode, 0
    mov SparkStep, 0
    call LoadPalette
    mov ax, 0
    mov bx, 0
    mov cx, 320
    mov dx, 200
    mov si, 0
    call FillRect
    mov al, Level
    inc al
    call DrawHeader
    mov cl, MaxTries
    mov ch, NumPos
    call DrawBoard
    mov al, NumColors
    mov ah, NumPos
    mov cl, TryNum
    inc cl
    mov ch, MaxTries
    call DrawPanel
    mov si, OFFSET MsgHelp
    mov bl, C_WHITE
    call DrawStatus
    mov al, TryNum
    mov ActiveRow, al
    mov CurPos, 0
    call DrawActiveRow
    POP_ALL
    ret
DrawGameScreen ENDP

; ----------------------------------------------------------
; GameKey: hace lo que pide una tecla durante la partida
;   Recibe: AL = accion (ACT_...), AH = numero si es ACT_NUMBER
;   Devuelve: nada (cambia Guess, CurPos, el historial y la pantalla)
;   Toca: nada (guarda y restaura todo)
; ----------------------------------------------------------
GameKey PROC
    PUSH_ALL
    push ax
    cmp WarnOn, 0
    je gk_nowarn
    mov WarnOn, 0
    mov si, OFFSET MsgHelp
    mov bl, C_WHITE
    cmp TestMode, 0
    je gk_warn
    mov si, OFFSET MsgPrueba
    mov bl, C_GOLD
gk_warn:
    call DrawStatus
gk_nowarn:
    mov CursorOn, 0
    call DrawCursor
    pop ax
    mov bl, CurPos
    mov bh, 0

    cmp al, ACT_LEFT
    je gk_left
    cmp al, ACT_RIGHT
    je gk_right
    cmp al, ACT_UP
    je gk_up
    cmp al, ACT_DOWN
    je gk_down
    cmp al, ACT_NUMBER
    je gk_number
    cmp al, ACT_BACK
    je gk_back
    cmp al, ACT_ENTER
    je gk_enter
    cmp al, ACT_TEST
    je gk_test
    cmp al, ACT_VIEW
    je gk_view
    jmp gk_done

gk_left:
    cmp CurPos, 0
    je gk_done
    dec CurPos
    jmp gk_done
gk_right:
    inc bl
    cmp bl, NumPos
    jae gk_done
    mov CurPos, bl
    jmp gk_done
gk_up:
    mov al, Guess[bx]
    inc al
    cmp al, NumColors
    jbe gk_set
    mov al, 1
    jmp gk_set
gk_down:
    mov al, Guess[bx]
    dec al
    cmp al, 1
    jge gk_set
    mov al, NumColors
gk_set:
    mov Guess[bx], al
    mov al, ActiveRow
    mov ah, CurPos
    call DrawActiveSlot
    jmp gk_done
gk_number:
    cmp ah, NumColors
    ja gk_done
    mov Guess[bx], ah
    mov al, ActiveRow
    mov ah, CurPos
    call DrawActiveSlot
    jmp gk_right
gk_back:
    mov Guess[bx], 0
    mov al, ActiveRow
    mov ah, CurPos
    call DrawActiveSlot
    jmp gk_done
gk_test:
    cmp TryNum, 0
    je gk_tstart
    call NewGame
    mov RevealPos, 0FFh
    mov PanelMood, 0
    call DrawGameScreen
gk_tstart:
    call GetTicks
    mov GameTick, ax
    mov TestMode, 1
    mov TestSecret, 0
    mov WarnOn, 0
    mov si, OFFSET MsgPrueba
    mov bl, C_GOLD
    call DrawStatus
    jmp gk_done
gk_view:
    cmp TestSecret, 0
    je gk_done
    cmp SecretShown, 0
    je gk_vshow
    call DrawSecretBoxes
    jmp gk_done
gk_vshow:
    call DrawSecret
    jmp gk_done
gk_enter:
    cmp TestMode, 0
    je gk_submit
    call SaveTestSecret
    cmp al, 0
    je gk_missing
    jmp gk_done
gk_submit:
    call SubmitGuess
    cmp al, 0
    jne gk_scored
gk_missing:
    mov WarnOn, 1
    mov si, OFFSET MsgFaltan
    mov bl, C_RED
    call DrawStatus
    jmp gk_done
gk_scored:
    mov al, TryNum
    dec al
    call DrawTry
    call StartSparks
    cmp GameState, 0
    jne gk_over

    mov bx, 0
gk_clear:
    mov Guess[bx], 0
    inc bx
    cmp bx, MAX_POS
    jb gk_clear
    mov al, TryNum
    mov ActiveRow, al
    mov CurPos, 0
    call DrawActiveRow
    inc al
    mov ah, MaxTries
    call DrawTryCount
    jmp gk_done

gk_over:
    mov RevealPos, 0
    call GetTicks
    mov RevealTick, ax
    mov si, OFFSET MsgGano
    mov bl, C_GOLD
    cmp GameState, 1
    je gk_show
    mov si, OFFSET MsgPerdio
    mov bl, C_RED
gk_show:
    call DrawStatus
    POP_ALL
    ret

gk_done:
    mov CursorOn, 1
    call DrawCursor
    call GetTicks
    mov CursorTick, ax
    POP_ALL
    ret
GameKey ENDP

; ----------------------------------------------------------
; SaveTestSecret: en modo prueba, guarda la fila como secreto
;   Recibe: nada (usa Guess y NumPos)
;   Devuelve: AL = 0 si faltan fichas, AL = 1 si se guardo
;   Toca: AX
; ----------------------------------------------------------
SaveTestSecret PROC
    push bx
    push cx
    push si
    mov cl, NumPos
    mov ch, 0
    mov si, 0
st_check:
    cmp Guess[si], 0
    je st_missing
    inc si
    loop st_check
    mov cl, NumPos
    mov si, 0
st_copy:
    mov al, Guess[si]
    mov Secret[si], al
    mov Guess[si], 0
    inc si
    loop st_copy
    mov TestMode, 0
    mov TestSecret, 1
    call DrawSecretBoxes
    mov al, ActiveRow
    call DrawActiveRow
    call GetTicks
    mov GameTick, ax
    mov WarnOn, 1
    mov si, OFFSET MsgPruebaOk
    mov bl, C_GOLD
    call DrawStatus
    mov al, 1
    jmp st_end
st_missing:
    mov al, 0
st_end:
    pop si
    pop cx
    pop bx
    ret
SaveTestSecret ENDP

END start
