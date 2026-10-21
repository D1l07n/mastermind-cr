; ==========================================================
; uidata.inc - Colores y textos de las pantallas
; Va dentro de .DATA. Los colores usan numeros fijos (200+)
; para no chocar con los de los sprites, que cambian solos.
; ==========================================================

UI_START    EQU 200
UI_COUNT    EQU 19

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
C_FLAG_B    EQU 212         ; franjas: colores propios para poder parpadear
C_FLAG_W    EQU 213
C_FLAG_R    EQU 214
C_CURT_LT   EQU 215         ; telon rojo: claro, medio y oscuro
C_CURT      EQU 216
C_CURT_DK   EQU 217
C_STAGE_LT  EQU 218         ; luz del reflector en las tablas

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
            db  8,20,56         ; 212 franja azul
            db 63,63,63         ; 213 franja blanca
            db 56,10,10         ; 214 franja roja
            db 52, 8,10         ; 215 telon claro
            db 40, 4, 7         ; 216 telon
            db 26, 2, 4         ; 217 telon oscuro
            db 50,38,22         ; 218 tablas iluminadas

MsgMaster   DB 'MASTERMIND', 0
SpotHalf    DB 21,35,44,51,56,61,64,66,68,69,70,70,69,68,66,64,61,56,51,44,35,21
MsgTico     DB 'T', 0, 'I', 0, 'C', 0, 'O', 0
TicoColors  DB C_BLUE, C_WHITE, C_RED, C_BLUE
MsgNivel    DB 'Nivel ', 0
MsgPura     DB 'P', 0, 'U', 0, 'R', 0, 'A', 0, ' ', 0, 'V', 0, 'I', 0, 'D', 0, 'A', 0
MsgAutor1   DB 'Christopher Chavarr', 161, 'a Sol', 162, 'rzano', 0
MsgAutor2   DB 'Dylan D', 161, 'az Quintero', 0
MsgCurso    DB 'UNA Campus Coto', 0
MsgPresiona DB 'Presiona una tecla', 0
MsgPresOff  DB '                  ', 0
BoardStripes DB C_FLAG_B, C_FLAG_W, C_FLAG_R, C_FLAG_W, C_FLAG_B
MsgSecreto  DB 'Secreto', 0
MsgQuestion DB '?', 0
MsgIntento  DB 'Intento', 0
MsgSlash    DB '/', 0
MsgTiempo   DB 'Tiempo', 0
MsgTimeZero DB '00:00', 0
MsgFichas   DB 'Fichas', 0
MsgHelp     DB 'Flechas mover  1-8 ficha  ENTER probar', 0
MsgFaltan   DB 'Faltan fichas en la fila', 0
MsgGano     DB 173, 'Ganaste! Pura vida   ESC salir', 0
MsgPerdio   DB 'Perdiste, mira el secreto  ESC salir', 0
MsgBlank2   DB '  ', 0
MsgColon    DB ':', 0
MsgPrueba   DB 'PRUEBA: pon el secreto y ENTER', 0
MsgPruebaOk DB 'Secreto listo. V lo muestra o tapa', 0

PieceSprites DW OFFSET SprAntorcha0, OFFSET SprBolinchas, OFFSET SprCafe0
             DW OFFSET SprCrema, OFFSET SprFarol, OFFSET SprGuante0
             DW OFFSET SprMoneda, OFFSET SprTrompo0
PieceSprites1 DW OFFSET SprAntorcha1, OFFSET SprBolinchas, OFFSET SprCafe1
             DW OFFSET SprCrema, OFFSET SprFarol, OFFSET SprGuante1
             DW OFFSET SprMoneda, OFFSET SprTrompo1
PanelSloths DW OFFSET SprPerezoso0, OFFSET SprPerezosoVictoria
            DW OFFSET SprPerezosoTriste, OFFSET SprPerezoso1

HeadFrame   DB 0                ; cuadro del trompo del encabezado (0 o 1)
HeadTick    DW 0                ; tic del ultimo cambio
PanelMood   DB 0                ; 0 = normal, 1 = feliz, 2 = triste
PanelFrame  DB 0                ; 0 = ojos abiertos, 1 = guino
PanelTick   DW 0                ; tic del ultimo guino
ActiveRow   DB 0                ; fila donde se esta jugando
CurPos      DB 0                ; casilla elegida (0 = la primera)
CursorOn    DB 0                ; 1 = marco amarillo visible
CursorTick  DW 0                ; tic del ultimo parpadeo
WarnOn      DB 0                ; 1 = la barra muestra un aviso
TestMode    DB 0                ; 1 = se esta escribiendo el secreto a mano
GameTick    DW 0                ; tic en que empezo la partida
ShownSec    DW 0                ; segundo que muestra el reloj
RevealPos   DB 0FFh             ; ficha del secreto que sigue (0FFh = nada)
RevealTick  DW 0                ; tic de la ultima ficha destapada
PieceFrame  DB 0                ; cuadro de las fichas que se mueven (0 o 1)
PieceTick   DW 0                ; tic del ultimo cambio de cuadro
SecretShown DB 0                ; un bit por cada ficha del secreto ya destapada
FxMode      DB 0                ; efecto final: 0 = nada, 1 = bandera, 2 = oscurecer
FxStep      DB 0                ; paso del efecto
FxTick      DW 0                ; tic del ultimo paso
IntroFrame  DB 0                ; perezoso de la presentacion: 0 = abierto, 1 = guino
IntroTick   DW 0
PressOn     DB 0                ; 1 = se ve "Presiona una tecla"
PressTick   DW 0
FlagRGB     DB 8,20,56, 63,63,63, 56,10,10   ; azul, blanco, rojo...
            DB 8,20,56, 63,63,63             ; ...repetidos para poder girarlos
