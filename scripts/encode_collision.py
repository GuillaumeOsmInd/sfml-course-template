"""
Encode un masque de niveau dans les bits de poids faible d'une texture.

Le masque est une seule image, chaque canal porte une information, encodee
dans le MEME canal de la texture. L'alpha de la texture n'est jamais modifie.

    Masque          Signification                      Bit encode dans la texture
    R = 0xFF   ->   collision                          rouge impair
    G = 0xFF   ->   premier plan (dessine devant)      vert impair
    B = 0xFF   ->   collision uniquement par le haut   bleu impair
                    (plateforme traversable en saut)

Les canaux se combinent. Exemple : 0x00FFFF = premier plan + plateforme
traversable, la texture est dessinee par dessus le joueur et le porte
quand il est dessus.

Seule la valeur exacte 0xFF compte. Ne portent aucune information :
  - les pixels totalement transparents du masque ;
  - les pixels blancs (0xFFFFFF) et noirs (0x000000) : on peut s'en servir
    comme fond ou pour annoter le masque sans creer de collisions.

Principe, pour chaque canal :
  1. Tous les pixels de la texture recoivent une composante PAIRE.
  2. La ou le masque porte l'information, la composante devient IMPAIRE.

Dans le jeu, il suffit de tester (rouge & 1), (vert & 1) ou (bleu & 1).
La difference est de 1/255 au maximum : invisible a l'oeil.

Usage :
    python scripts/encode_collision.py texture.png masque.png [sortie.png]

Par defaut, la sortie est ecrite a cote de la texture : <texture>_encoded.png
"""

import argparse
import sys
from pathlib import Path

from PIL import Image, ImageChops


def flag(band: Image.Image, informative: Image.Image) -> Image.Image:
    """255 la ou la composante vaut exactement 0xFF sur un pixel porteur d'information, 0 ailleurs."""
    is_full = band.point(lambda v: 255 if v == 0xFF else 0)
    return ImageChops.multiply(is_full, informative)


def encode_lsb(channel: Image.Image, marked: Image.Image) -> Image.Image:
    """Rend le canal pair partout, puis impair la ou `marked` vaut 255."""
    even = channel.point(lambda v: v & ~1)
    odd = even.point(lambda v: v | 1)
    return Image.composite(odd, even, marked)


def encode(texture: Image.Image, mask: Image.Image) -> tuple[Image.Image, dict[str, int]]:
    r, g, b, a = texture.convert("RGBA").split()
    mask_r, mask_g, mask_b, mask_a = mask.convert("RGBA").split()

    # Un pixel transparent du masque ne porte aucune information, meme si
    # l'editeur a laisse des valeurs RGB dessous.
    opaque = mask_a.point(lambda v: 255 if v > 0 else 0)

    # Le blanc et le noir non plus : sans cette regle, un pixel blanc
    # (FF sur les trois canaux) activerait les trois drapeaux a la fois.
    def all_equal(value: int) -> Image.Image:
        bands = [band.point(lambda v: 255 if v == value else 0) for band in (mask_r, mask_g, mask_b)]
        return ImageChops.multiply(ImageChops.multiply(bands[0], bands[1]), bands[2])

    ignored = ImageChops.lighter(all_equal(0xFF), all_equal(0x00))   # blanc OU noir
    informative = ImageChops.multiply(opaque, ImageChops.invert(ignored))

    solid = flag(mask_r, informative)
    front = flag(mask_g, informative)
    one_way = flag(mask_b, informative)

    # Chaque canal du masque est encode dans le meme canal de la texture.
    # Les trois sont toujours normalises, meme sans aucun pixel marque :
    # sinon le jeu lirait comme marques les pixels impairs par hasard.
    # L'alpha n'est pas touche.
    r = encode_lsb(r, solid)
    g = encode_lsb(g, front)
    b = encode_lsb(b, one_way)

    counts = {
        "collision          (R)": solid.histogram()[255],
        "premier plan       (G)": front.histogram()[255],
        "collision par haut (B)": one_way.histogram()[255],
        "ignores (blanc)       ": ImageChops.multiply(opaque, all_equal(0xFF)).histogram()[255],
    }
    return Image.merge("RGBA", (r, g, b, a)), counts


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Encode un masque de niveau (R=collision, G=premier plan, B=collision par le haut) "
        "dans les bits de poids faible des memes canaux d'une texture."
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
