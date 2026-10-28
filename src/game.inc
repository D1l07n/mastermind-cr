; game.inc - Reglas del juego (no dibuja ni lee teclado)
; Fichas: 1..8 (1 = antorcha, 2 = bolinchas, ...), 0 = casilla vacia

MAX_POS     EQU 5
NUM_COLORS  EQU 8

.DATA
NumPos      DB 4
Secret      DB MAX_POS DUP(0)
Guess       DB MAX_POS DUP(0)
Bulls       DB 0
Cows        DB 0
CountS      DB NUM_COLORS + 1 DUP(0)
CountG      DB NUM_COLORS + 1 DUP(0)

.CODE
; ----------------------------------------------------------
; ScoreGuess: compara Guess con Secret
;   Recibe: Secret, Guess y NumPos
;   Devuelve: Bulls y Cows
;   Toca: nada
; ----------------------------------------------------------
ScoreGuess PROC
    PUSH_ALL

    mov bx, 0
    mov cx, NUM_COLORS + 1
sg_clear:
    mov CountS[bx], 0
    mov CountG[bx], 0
    inc bx
    loop sg_clear

    mov Bulls, 0
    mov cl, NumPos
    mov ch, 0
    mov si, 0
sg_pos:
    mov al, Secret[si]
    mov ah, Guess[si]
    cmp al, ah
    jne sg_count
    inc Bulls
sg_count:
    mov bh, 0
    mov bl, al
    inc CountS[bx]
    mov bl, ah
    inc CountG[bx]
    inc si
    loop sg_pos

    mov dl, 0
    mov bx, 1
    mov cx, NUM_COLORS
sg_min:
    mov al, CountS[bx]
    cmp al, CountG[bx]
    jbe sg_add
    mov al, CountG[bx]
sg_add:
    add dl, al
    inc bx
    loop sg_min

    sub dl, Bulls
    mov Cows, dl

    POP_ALL
    ret
ScoreGuess ENDP

; ---------- Niveles ----------
.DATA
Level       DB 0                
NumColors   DB 6               
AllowRepeat DB 0              
MaxTries    DB 12               
LevelTable  DB 4, 6, 0, 12      
            DB 4, 6, 1, 10      
            DB 5, 8, 1, 10      

.CODE
; ----------------------------------------------------------
; SetLevel: copia los datos de un nivel a las variables
;   Recibe: AL = nivel (0 a 2)
;   Devuelve: NumPos, NumColors, AllowRepeat y MaxTries
;   Toca: nada
; ----------------------------------------------------------
SetLevel PROC
    PUSH_ALL
    mov Level, al
    mov ah, 0
    mov bx, ax
    shl bx, 1                   
    shl bx, 1
    mov al, LevelTable[bx]
    mov NumPos, al
    mov al, LevelTable[bx + 1]
    mov NumColors, al
    mov al, LevelTable[bx + 2]
    mov AllowRepeat, al
    mov al, LevelTable[bx + 3]
    mov MaxTries, al
    POP_ALL
    ret
SetLevel ENDP

; ---------- Secreto al azar ----------
.DATA
Seed        DW 0                
PrevSecret  DB MAX_POS DUP(0)    

.CODE
; ----------------------------------------------------------
; InitRandom: toma la semilla del reloj (INT 1Ah), una vez al inicio
;   Recibe: nada
;   Devuelve: Seed
;   Toca: nada
; ----------------------------------------------------------
InitRandom PROC
    PUSH_ALL
    mov ah, 00h
    int 1Ah                   
    mov Seed, dx
    POP_ALL
    ret
InitRandom ENDP

; ----------------------------------------------------------
; Random: saca un numero al azar
;   Recibe: nada (usa Seed y NumColors)
;   Devuelve: AL = numero de 1 a NumColors (cambia Seed)
;   Toca: AX
; ----------------------------------------------------------
Random PROC
    push dx
    mov ax, Seed                
    mov dx, 25173
    mul dx
    add ax, 13849
    mov Seed, ax
    mov al, ah                 
    mov ah, 0
    div NumColors               
    mov al, ah
    inc al                     
    pop dx
    ret
Random ENDP

; ----------------------------------------------------------
; MakeSecret: llena Secret con fichas al azar; si AllowRepeat = 0 no
;   repite fichas, y nunca saca el mismo secreto de la partida anterior
;   Recibe: nada (usa NumPos, AllowRepeat y PrevSecret)
;   Devuelve: Secret y PrevSecret
;   Toca: nada
; ----------------------------------------------------------
MakeSecret PROC
    PUSH_ALL
    mov cl, NumPos
    mov ch, 0                   
ms_start:
    mov si, 0                  
ms_next:
    call Random                
    cmp AllowRepeat, 1
    je ms_store                 
    mov bx, 0                   
ms_check:
    cmp bx, si
    jae ms_store               
    cmp Secret[bx], al
    je ms_next                  
    inc bx
    jmp ms_check
ms_store:
    mov Secret[si], al
    inc si
    cmp si, cx
    jb ms_next

    mov bx, 0                   
ms_same:
    mov al, Secret[bx]
    cmp al, PrevSecret[bx]
    jne ms_save                 
    inc bx
    cmp bx, cx
    jb ms_same
    jmp ms_start          
ms_save:
    mov bx, 0                  
ms_copy:
    mov al, Secret[bx]
    mov PrevSecret[bx], al
    inc bx
    cmp bx, cx
    jb ms_copy
    POP_ALL
    ret
MakeSecret ENDP


MAX_TRIES   EQU 12              

.DATA
TryNum      DB 0                ; intentos ya gastados
GameState   DB 0                ; 0 = jugando, 1 = gano, 2 = perdio
HistGuess   DB MAX_TRIES * MAX_POS DUP(0)   
HistBulls   DB MAX_TRIES DUP(0)             
HistCows    DB MAX_TRIES DUP(0)            

.CODE
; ----------------------------------------------------------
; NewGame: prepara una partida nueva del nivel actual: saca el
;   secreto, borra el intento y pone los contadores en cero
;   Recibe: nada
;   Devuelve: Secret, Guess, TryNum y GameState listos
;   Toca: nada
; ----------------------------------------------------------
NewGame PROC
    PUSH_ALL
    call MakeSecret
    mov TryNum, 0
    mov GameState, 0
    mov bx, 0
ng_clear:
    mov Guess[bx], 0
    inc bx
    cmp bx, MAX_POS
    jb ng_clear
    POP_ALL
    ret
NewGame ENDP

; ----------------------------------------------------------
; SubmitGuess: evalua el intento que esta en Guess
;   Recibe: nada (usa Guess, NumPos y MaxTries)
;   Devuelve: AL = 0 si estaba incompleto (no gasta turno),
;             AL = 1 si se evaluo. Actualiza Bulls, Cows, el historial,
;             TryNum y GameState
;   Toca: AX
; ----------------------------------------------------------
SubmitGuess PROC
    push bx
    push cx
    push si
    push di

    mov cl, NumPos              
    mov ch, 0
    mov si, 0
sub_check:
    cmp Guess[si], 0
    je sub_incomplete
    inc si
    loop sub_check

    call ScoreGuess             

    mov al, TryNum            
    mov ah, 0
    mov di, ax                  
    mov bl, Bulls
    mov HistBulls[di], bl
    mov bl, Cows
    mov HistCows[di], bl
    mov bl, MAX_POS
    mul bl                      
    mov di, ax                 
    mov cl, NumPos
    mov ch, 0
    mov si, 0
sub_copy:
    mov al, Guess[si]
    mov HistGuess[di], al
    inc si
    inc di
    loop sub_copy
    inc TryNum

    mov al, Bulls             
    cmp al, NumPos
    jne sub_not_won
    mov GameState, 1            
    jmp sub_done
sub_not_won:
    mov al, TryNum
    cmp al, MaxTries
    jb sub_done
    mov GameState, 2           
sub_done:
    mov al, 1
    jmp sub_exit
sub_incomplete:
    mov al, 0
sub_exit:
    pop di
    pop si
    pop cx
    pop bx
    ret
SubmitGuess ENDP
