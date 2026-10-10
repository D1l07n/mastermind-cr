"""
piskel2db.py - Convierte los sprites de Piskel (.piskel) en tablas para TASM.

Uso (desde la carpeta proyecto):
    python herramientas/piskel2db.py sprites src/sprites.inc

Que hace:
  1. Lee todos los .piskel de la carpeta de sprites (en orden alfabetico).
  2. Arma UNA paleta comun para todos los dibujos (el modo 13h tiene una
     sola paleta para toda la pantalla).
  3. Escribe sprites.inc con:
       - la paleta en formato VGA (valores de 0 a 63) para mandarla por
         los puertos 3C8h/3C9h,
       - una tabla 'db' por cada cuadro de cada sprite (0 = transparente).

Solo necesita Python 3 y Pillow (pip install pillow). El juego en si
sigue siendo 100 % ensamblador; este script es una herramienta aparte.
"""

import base64
import io
import json
import os
import sys

from PIL import Image

# Los colores 0..15 se dejan como los del BIOS (sirven para el texto).
# Los colores de los sprites empiezan en el 16.
PALETTE_START = 16
# Dos colores cuya diferencia (en escala VGA 0..63) sea menor o igual a
# esto se toman como el mismo color. Junta los tonos casi repetidos.
MERGE_TOLERANCE = 1
VALUES_PER_LINE = 16


def read_frames(path):
    """Devuelve (ancho, alto, [cuadros]) de un archivo .piskel.
    Si el dibujo tiene varias capas, las junta en una sola imagen."""
    with open(path, encoding="utf-8") as f:
        piskel = json.load(f)["piskel"]
    width, height = piskel["width"], piskel["height"]
    frames = None
    for layer_text in piskel["layers"]:
        layer = json.loads(layer_text)
        count = layer["frameCount"]
        layer_frames = [None] * count
        for chunk in layer["chunks"]:
            png = chunk["base64PNG"].split(",", 1)[1]
            sheet = Image.open(io.BytesIO(base64.b64decode(png))).convert("RGBA")
            # layout[columna][fila] = numero de cuadro dentro de la hoja
            for col, column in enumerate(chunk["layout"]):
                for row, index in enumerate(column):
                    box = (col * width, row * height, (col + 1) * width, (row + 1) * height)
                    layer_frames[index] = sheet.crop(box)
        if frames is None:
            frames = layer_frames
        else:
            for i, frame in enumerate(layer_frames):
                frames[i] = Image.alpha_composite(frames[i], frame)
    return width, height, frames


def to_vga(rgb):
    """Pasa un color de 0..255 a la escala VGA de 0..63."""
    return tuple(round(c * 63 / 255) for c in rgb)


def label_for(file_name, frame, frame_count):
    """lapa.piskel -> SprLapa (o SprCafe0, SprCafe1 si tiene varios cuadros)."""
    base = os.path.splitext(file_name)[0]
    words = base.replace("-", "_").replace(" ", "_").split("_")
    name = "Spr" + "".join(w[:1].upper() + w[1:] for w in words if w)
    return f"{name}{frame}" if frame_count > 1 else name


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    sprite_dir, out_path = sys.argv[1], sys.argv[2]

    # 1. Leer todos los sprites
    sprites = []
    for file_name in sorted(os.listdir(sprite_dir)):
        if file_name.lower().endswith(".piskel"):
            width, height, frames = read_frames(os.path.join(sprite_dir, file_name))
            sprites.append((file_name, width, height, frames))
    if not sprites:
        sys.exit(f"No hay archivos .piskel en {sprite_dir}")

    # 2. Armar la paleta comun
    palette = []  # lista de colores VGA (r, g, b)

    def color_index(rgb):
        vga = to_vga(rgb)
        for i, known in enumerate(palette):
            if max(abs(a - b) for a, b in zip(vga, known)) <= MERGE_TOLERANCE:
                return PALETTE_START + i
        palette.append(vga)
        return PALETTE_START + len(palette) - 1

    tables = []  # (etiqueta, archivo, ancho, alto, [indices])
    for file_name, width, height, frames in sprites:
        for n, frame in enumerate(frames):
            pixels = []
            for y in range(height):
                for x in range(width):
                    r, g, b, a = frame.getpixel((x, y))
                    # alfa bajo = transparente = 0
                    pixels.append(color_index((r, g, b)) if a >= 128 else 0)
            tables.append((label_for(file_name, n, len(frames)), file_name, width, height, pixels))

    if PALETTE_START + len(palette) > 256:
        sys.exit(f"Demasiados colores: {len(palette)}. Simplifica los dibujos.")

    # 3. Escribir sprites.inc
    lines = [
        "; ==========================================================",
        "; sprites.inc - GENERADO por herramientas/piskel2db.py",
        "; No editar a mano: cambie el dibujo en Piskel y vuelva a",
        "; correr el script.",
        "; ==========================================================",
        "",
        "; ---- Paleta propia ----",
        "; Colores desde el indice PALETTE_START, en escala VGA (0..63).",
        "; Se cargan con OUT 3C8h (indice inicial) y OUT 3C9h (R, G, B).",
        f"PALETTE_START   EQU {PALETTE_START}",
        f"PALETTE_COUNT   EQU {len(palette)}",
        "SpritePalette   LABEL BYTE",
    ]
    for i, (r, g, b) in enumerate(palette):
        lines.append(f"                db {r:2d},{g:2d},{b:2d}        ; color {PALETTE_START + i}")
    lines += ["", "; ---- Sprites ----", "; 0 = transparente (DrawSprite no lo pinta)."]

    sizes = {(w, h) for _, _, w, h, _ in tables}
    if len(sizes) == 1:
        w, h = sizes.pop()
        lines += [f"SPRITE_W        EQU {w}", f"SPRITE_H        EQU {h}"]

    for label, file_name, w, h, pixels in tables:
        lines += ["", f"; {file_name}  ({w}x{h})", f"{label} LABEL BYTE"]
        for start in range(0, len(pixels), VALUES_PER_LINE):
            row = pixels[start:start + VALUES_PER_LINE]
            lines.append("    db " + ",".join(f"{v:3d}" for v in row))

    with open(out_path, "w", encoding="ascii", newline="\r\n") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Listo: {out_path}")
    print(f"  {len(tables)} cuadros, {len(palette)} colores (indices {PALETTE_START}..{PALETTE_START + len(palette) - 1})")
    for label, file_name, w, h, _ in tables:
        print(f"  {label:<14} <- {file_name} ({w}x{h})")


if __name__ == "__main__":
    main()
