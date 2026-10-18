; ==========================================================
; uidata.inc - Colores y textos de las pantallas
; Va dentro de .DATA. Los colores usan numeros fijos (200+)
; para no chocar con los de los sprites, que cambian solos.
; ==========================================================

UI_START    EQU 200
UI_COUNT    EQU 12

C_WOOD      EQU 200
C_WOOD_DK   EQU 201
C_HOLE      EQU 202
C_BORDER    EQU 203
C_GOLD      EQU 204
C_BLUE      EQU 205
C_WHITE     EQU 206
C_RED       EQU 207
C_ACTIVE    EQU 208
C_CURSOR    EQU 209
C_SLOT      EQU 210
C_SELECT    EQU 211

UiPalette   LABEL BYTE
            db 38,26,14         ; 200 madera
            db 30,19,10         ; 201 madera oscura
            db 14, 9, 5         ; 202 hueco
            db 48,32,16         ; 203 borde cafe
            db 58,48,16         ; 204 dorado
            db  8,20,56         ; 205 azul bandera
            db 63,63,63         ; 206 blanco
            db 56,10,10         ; 207 rojo bandera
            db 50,48,44         ; 208 fila activa
            db 63,63,16         ; 209 marco amarillo
            db 12,12,12         ; 210 cuadro de ficha
            db 14,30,56         ; 211 ficha seleccionada

MsgMaster   DB 'MASTERMIND', 0
MsgTico     DB 'T', 0, 'I', 0, 'C', 0, 'O', 0
TicoColors  DB C_BLUE, C_WHITE, C_RED, C_BLUE
MsgNivel    DB 'Nivel ', 0
BoardStripes DB C_BLUE, C_WHITE, C_RED, C_WHITE, C_BLUE

HeadFrame   DB 0                ; 0 = perezoso normal, 1 = victoria
HeadTick    DW 0                ; tic del ultimo cambio
