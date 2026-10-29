# Mastermind Pura Vida

Toros y Vacas en ensamblador x86 de 16 bits (TASM), modo gráfico 13h, para DOSBox.
Proyecto II de Arquitectura de Computadores (EIF205), UNA Sede Regional Brunca, Campus Coto.

Christopher Chavarría Solórzano y Dylan Díaz Quintero.

## Carpetas

| Carpeta | Qué tiene |
|---|---|
| `src` | Código fuente. `main.asm` es el programa principal; los `.inc` se incluyen desde él |
| `bin` | `MAIN.EXE` ya compilado y `dosbox.conf` para jugar sin compilar |
| `sprites` | Dibujos originales en Piskel (`.piskel`) |
| `herramientas` | `piskel2db.py`, que pasa los `.piskel` a bytes para `src/sprites.inc` |
| `doc` | Documento técnico, hoja de sprites y bitácora |

## Ensamblar y correr con el Ensamblador IDE (forma principal)

1. Abrir **Ensamblador IDE**.
2. **Open Folder** y escoger la carpeta del proyecto (`mastermind-cr`) o la carpeta que la contiene.
3. En el explorador de la izquierda, entrar a `src` y abrir `main.asm`.
4. Darle **Run**. El IDE ensambla con TASM, enlaza con TLINK y abre el juego en DOSBox.
   Si solo se quiere ensamblar para ver errores: **Build** (Ctrl+Shift+B); el resultado sale en la terminal de abajo.

Importante: se abre siempre `main.asm`, no los `.inc`. Los `.inc` (macros, graphics, input, game,
screens, intro, menu, uidata, sprites) tienen que estar en la misma carpeta que `main.asm`.

## Ensamblar y correr a mano en DOSBox

Dentro de DOSBox, cambiando las rutas por las de su compu:

```
mount c "C:\ruta\a\mastermind-cr"
mount t "C:\ruta\a\TASM"
c:
cd src
t:\tasm main.asm
t:\tlink main.obj
main.exe
```

- `mount c` = la carpeta del proyecto se ve como el disco C: dentro de DOSBox.
- `mount t` = la carpeta donde están `TASM.EXE` y `TLINK.EXE` se ve como el disco T:.
- Hay que estar dentro de `src` para que TASM encuentre los `.inc`.

## Jugar sin compilar

En la carpeta `bin`, abrir DOSBox con la configuración incluida:

```
dosbox -conf dosbox.conf
```

(Ejecutado desde dentro de `bin`.) La configuración monta esa carpeta y arranca `MAIN.EXE` sola.
También se puede montar `bin` a mano y escribir `MAIN.EXE`.

## Cómo se usa

Al abrir sale la presentación (telón que se abre). Cualquier tecla pasa al menú; Esc sale.

**Menú:** ↑ ↓ eligen la opción y Enter la acepta. En *Nivel*, Enter o ← → cambian entre Fácil, Normal y Difícil.
*Instrucciones* tiene dos páginas: cómo se juega y cómo se cuentan los toros y las vacas.

## Controles del juego

| Tecla | Acción |
|---|---|
| ← → o A D | Mover el cursor entre casillas |
| ↑ ↓ o W S | Cambiar la ficha de la casilla |
| 1 a 8 | Poner esa ficha directamente |
| Retroceso | Borrar la casilla |
| Enter | Probar el intento (con la partida terminada: jugar otra) |
| H | Modo prueba: escribir el secreto a mano y Enter |
| V | Solo en modo prueba: mostrar o tapar el secreto |
| Esc | Volver al menú |

Pejibaye entero = ficha correcta en su lugar (toro). Pejibaye mordido = ficha correcta en otro lugar (vaca).
Los pejibayes dicen cuántos aciertos hay, no en qué casilla: primero salen los toros y después las vacas.
En el nivel Fácil, la ficha que es toro suelta unas chispas como ayuda; en Normal y Difícil no.
