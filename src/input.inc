; ==========================================================
; input.inc - Teclado y reloj
; ==========================================================

; Acciones que devuelve ReadAction en AL
ACT_NONE    EQU 0           ; tecla que no usamos
ACT_LEFT    EQU 1           ; <- o A
ACT_RIGHT   EQU 2           ; -> o D
ACT_UP      EQU 3           ; flecha arriba o W
ACT_DOWN    EQU 4           ; flecha abajo o S
ACT_ENTER   EQU 5
ACT_BACK    EQU 6           ; BACKSPACE
ACT_ESC     EQU 7
ACT_NUMBER  EQU 8           ; 1..8, el numero va en AH
ACT_TEST    EQU 9           ; H: modo prueba

; ----------------------------------------------------------
; ReadAction: espera una tecla y la traduce a una accion
;   Recibe:   nada
;   Devuelve: AL = accion (ACT_...), AH = 1..8 si es ACT_NUMBER
;   Toca:     AX
; ----------------------------------------------------------
ReadAction PROC
    mov ah, 00h
    int 16h                 ; AL = ASCII, AH = codigo de la tecla
    cmp al, 0
    je ra_arrow             ; AL = 0: tecla especial (flechas)
    cmp al, 0E0h
    je ra_arrow             ; algunos teclados mandan E0h en vez de 0

    cmp al, 13
    je ra_enter
    cmp al, 8
    je ra_back
    cmp al, 27
    je ra_esc

    cmp al, '1'
    jb ra_letter            ; menor que '1': no es numero
    cmp al, '8'
    ja ra_letter            ; mayor que '8': no es numero
    sub al, '0'             ; '3' -> 3
    mov ah, al
    mov al, ACT_NUMBER
    ret

ra_letter:
    or al, 20h              ; mayuscula -> minuscula ('W' -> 'w')
    cmp al, 'w'
    je ra_up
    cmp al, 's'
    je ra_down
    cmp al, 'a'
    je ra_left
    cmp al, 'd'
    je ra_right
    cmp al, 'h'
    je ra_test
    jmp ra_none

ra_arrow:
    cmp ah, 48h
    je ra_up
    cmp ah, 50h
    je ra_down
    cmp ah, 4Bh
    je ra_left
    cmp ah, 4Dh
    je ra_right
    jmp ra_none

ra_up:    mov al, ACT_UP
          ret
ra_down:  mov al, ACT_DOWN
          ret
ra_left:  mov al, ACT_LEFT
          ret
ra_right: mov al, ACT_RIGHT
          ret
ra_enter: mov al, ACT_ENTER
          ret
ra_back:  mov al, ACT_BACK
          ret
ra_esc:   mov al, ACT_ESC
          ret
ra_test:  mov al, ACT_TEST
          ret
ra_none:  mov al, ACT_NONE
          ret
ReadAction ENDP

; ----------------------------------------------------------
; PollAction: como ReadAction, pero NO espera
;   Recibe:   nada
;   Devuelve: AL = ACT_NONE si no hay tecla; si hay, lo mismo
;             que ReadAction
;   Toca:     AX
; ----------------------------------------------------------
PollAction PROC
    mov ah, 01h
    int 16h                 ; hay tecla? ZF = 1 si no hay
    jz pa_none
    call ReadAction         ; hay: leerla y traducirla
    ret
pa_none:
    mov al, ACT_NONE
    ret
PollAction ENDP

; ----------------------------------------------------------
; GetTicks: lee el reloj del BIOS (sube ~18 veces por segundo)
;   Recibe:   nada
;   Devuelve: AX = contador de tics (parte baja)
;   Toca:     AX
; ----------------------------------------------------------
GetTicks PROC
    push es
    mov ax, 0040h
    mov es, ax              ; ES = zona de datos del BIOS
    mov ax, es:[006Ch]      ; contador en 0040h:006Ch
    pop es
    ret
GetTicks ENDP
