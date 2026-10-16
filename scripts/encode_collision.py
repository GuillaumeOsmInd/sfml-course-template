"""
Encode un masque de niveau dans les bits de poids faible d'une texture.

Le masque est une seule image. Ses informations sont cachees dans les deux
bits de poids faible des composantes rouge, verte et bleue de la texture.
L'alpha de la texture n'est jamais modifie.

Bit 0 (0b01) : le decor.

    Masque          Signification                      Bit encode dans la texture
    R = 0xFF   ->   collision                          rouge  & 0b01
    G = 0xFF   ->   premier plan (dessine devant)      vert   & 0b01
                    + sortie : passe au niveau suivant
    B = 0xFF   ->   collision uniquement par le haut   bleu   & 0b01
                    (plateforme traversable en saut)

    Les canaux se combinent. Exemple : 0xFF00FF = mur + plateforme traversable.
    Seule la valeur exacte 0xFF compte.

Bit 1 (0b10) : les blocs, des entites creees au chargement du niveau. On les
peint en cases de 16 x 16 ; le jeu lit le coin haut-gauche de chaque case.

    Masque          Bloc                               Bit encode dans la texture
    0xFFFFFF   ->   brique                             rouge  & 0b10
    0x000000   ->   bloc mystere (piece)               vert   & 0b10
    0x808080   ->   piece posee (ramassable)           bleu   & 0b10
    0x804000   ->   Goomba                             rouge & 0b10 + vert & 0b10
    0x008080   ->   bloc champignon                    vert  & 0b10 + bleu & 0b10

    Ces trois couleurs ne creent aucune collision (bit 0).

Les pixels totalement transparents du masque ne portent aucune information.

Principe, pour chaque bit de chaque canal :
  1. Le bit est mis a 0 sur tous les pixels de la texture.
  2. Il est mis a 1 la ou le masque porte l'information.
La difference est de 3/255 au maximum : invisible a l'oeil.

Le script est idempotent : on peut l'appliquer a une texture deja encodee.

Usage :
    python scripts/encode_collision.py texture.png masque.png [sortie.png]

Par defaut, la sortie est ecrite a cote de la texture : <texture>_encoded.png
"""

import argparse
import sys
from pathlib import Path

from PIL import Image, ImageChops

DECOR_BIT = 0b01
BLOCK_BIT = 0b10

BRICK_COLOR = (0xFF, 0xFF, 0xFF)
MYSTERY_COLOR = (0x00, 0x00, 0x00)
COIN_COLOR = (0x80, 0x80, 0x80)
GOOMBA_COLOR = (0x80, 0x40, 0x00)
MUSHROOM_COLOR = (0x00, 0x80, 0x80)


def flag(band: Image.Image, informative: Image.Image) -> Image.Image:
    """255 la ou la composante vaut exactement 0xFF sur un pixel porteur d'information, 0 ailleurs."""
    is_full = band.point(lambda v: 255 if v == 0xFF else 0)
    return ImageChops.multiply(is_full, informative)


def encode_bit(channel: Image.Image, marked: Image.Image, bit: int) -> Image.Image:
    """Met `bit` a 0 partout dans le canal, puis a 1 la ou `marked` vaut 255."""
    cleared = channel.point(lambda v: v & ~bit)
    raised = cleared.point(lambda v: v | bit)
    return Image.composite(raised, cleared, marked)


def encode(texture: Image.Image, mask: Image.Image) -> tuple[Image.Image, dict[str, int]]:
    r, g, b, a = texture.convert("RGBA").split()
    mask_r, mask_g, mask_b, mask_a = mask.convert("RGBA").split()

    # Un pixel transparent du masque ne porte aucune information, meme si
    # l'editeur a laisse des valeurs RGB dessous.
    opaque = mask_a.point(lambda v: 255 if v > 0 else 0)

    def is_color(color: tuple[int, int, int]) -> Image.Image:
        """255 sur les pixels opaques du masque exactement de cette couleur."""
        bands = [band.point(lambda v, c=c: 255 if v == c else 0) for band, c in zip((mask_r, mask_g, mask_b), color)]
        same = ImageChops.multiply(ImageChops.multiply(bands[0], bands[1]), bands[2])
        return ImageChops.multiply(same, opaque)

    # Bit 1 : les blocs.
    bricks = is_color(BRICK_COLOR)
    mysteries = is_color(MYSTERY_COLOR)
    coins = is_color(COIN_COLOR)
    goombas = is_color(GOOMBA_COLOR)
    mushrooms = is_color(MUSHROOM_COLOR)

    # Bit 0 : le decor. Les couleurs de blocs n'en portent pas : sans cette
    # regle, le blanc (FF sur les trois canaux) activerait les trois drapeaux.
    blocks = ImageChops.lighter(ImageChops.lighter(ImageChops.lighter(bricks, mysteries), coins), goombas)
    blocks = ImageChops.lighter(blocks, mushrooms)
    informative = ImageChops.multiply(opaque, ImageChops.invert(blocks))
    solid = flag(mask_r, informative)
    front = flag(mask_g, informative)
    one_way = flag(mask_b, informative)

    # Chaque bit est toujours normalise, meme sans aucun pixel marque : sinon
    # le jeu lirait comme marques les pixels dont le bit vaut 1 par hasard.
    # Goomba : combinaison rouge + vert du bit 1.
    r = encode_bit(encode_bit(r, solid, DECOR_BIT), ImageChops.lighter(bricks, goombas), BLOCK_BIT)
    g = encode_bit(encode_bit(g, front, DECOR_BIT), ImageChops.lighter(ImageChops.lighter(mysteries, goombas), mushrooms), BLOCK_BIT)
    # Bloc champignon : combinaison vert + bleu du bit 1.
    b = encode_bit(encode_bit(b, one_way, DECOR_BIT), ImageChops.lighter(coins, mushrooms), BLOCK_BIT)

    counts = {
        "collision           (R 0b01)": solid.histogram()[255],
        "premier plan/sortie (G 0b01)": front.histogram()[255],
        "collision par haut  (B 0b01)": one_way.histogram()[255],
        "briques             (R 0b10)": bricks.histogram()[255],
        "blocs mysteres      (G 0b10)": mysteries.histogram()[255],
        "pieces posees       (B 0b10)": coins.histogram()[255],
        "goombas         (R+G 0b10)": goombas.histogram()[255],
        "blocs champignon (G+B 0b10)": mushrooms.histogram()[255],
    }
    return Image.merge("RGBA", (r, g, b, a)), counts


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Encode un masque de niveau dans les deux bits de poids faible des composantes d'une texture : "
        "bit 0 = decor (R collision, G premier plan + sortie, B collision par le haut), "
        "bit 1 = blocs et ennemis (blanc brique, noir bloc mystere, gris piece posee, marron Goomba)."
    )
    parser.add_argument("texture", type=Path, help="texture PNG")
    parser.add_argument("mask", type=Path, help="masque PNG de meme taille")
    parser.add_argument("output", type=Path, nargs="?", help="PNG de sortie (defaut : <texture>_encoded.png)")
    args = parser.parse_args()

    texture = Image.open(args.texture)
    mask = Image.open(args.mask)

    if texture.size != mask.size:
        print(
            f"Erreur : tailles differentes, texture {texture.size[0]}x{texture.size[1]} "
            f"et masque {mask.size[0]}x{mask.size[1]}.",
            file=sys.stderr,
        )
        return 1

    result, counts = encode(texture, mask)

    output = args.output or args.texture.with_name(f"{args.texture.stem}_encoded.png")
    result.save(output, "PNG")

    total = result.size[0] * result.size[1]
    print(f"{output} ecrit :")
    for name, count in counts.items():
        print(f"  {name} : {count}/{total} pixels")
    return 0


if __name__ == "__main__":
    sys.exit(main())
