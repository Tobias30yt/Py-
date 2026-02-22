#!/usr/bin/env python3
"""
Generate a project title image and stamp it into all .pypp files.

Usage:
  python tools/add_title_image_to_pypp.py
  python tools/add_title_image_to_pypp.py --force
  python tools/add_title_image_to_pypp.py --image assets/custom_title.ppm
"""

from __future__ import annotations

import argparse
from pathlib import Path


SKIP_DIRS = {
    ".git",
    ".github",
    ".vs",
    "build",
    "out",
    "dist",
    "__pycache__",
    "node_modules",
}


def clamp(v: int) -> int:
    return 0 if v < 0 else (255 if v > 255 else v)


def generate_title_image(path: Path, width: int = 960, height: int = 540) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    pixels = [[(0, 0, 0) for _ in range(width)] for _ in range(height)]

    # Background gradient.
    for y in range(height):
        for x in range(width):
            r = 10 + (x * 30) // max(1, width - 1)
            g = 20 + (y * 40) // max(1, height - 1)
            b = 45 + ((x + y) * 70) // max(1, width + height - 2)
            pixels[y][x] = (clamp(r), clamp(g), clamp(b))

    # Vignette.
    cx = width / 2.0
    cy = height / 2.0
    max_d = (cx * cx + cy * cy) ** 0.5
    for y in range(height):
        for x in range(width):
            dx = x - cx
            dy = y - cy
            d = (dx * dx + dy * dy) ** 0.5
            f = 1.0 - 0.45 * (d / max_d)
            r, g, b = pixels[y][x]
            pixels[y][x] = (clamp(int(r * f)), clamp(int(g * f)), clamp(int(b * f)))

    # Banner plate.
    bx0 = width // 8
    by0 = height // 3
    bw = (width * 3) // 4
    bh = height // 3
    for y in range(by0, by0 + bh):
        for x in range(bx0, bx0 + bw):
            r, g, b = pixels[y][x]
            pixels[y][x] = (
                clamp((r * 35 + 220 * 65) // 100),
                clamp((g * 35 + 170 * 65) // 100),
                clamp((b * 35 + 80 * 65) // 100),
            )

    # 5x7 bitmap font for "PY++".
    glyphs = {
        "P": [
            "11110",
            "10001",
            "10001",
            "11110",
            "10000",
            "10000",
            "10000",
        ],
        "Y": [
            "10001",
            "10001",
            "01010",
            "00100",
            "00100",
            "00100",
            "00100",
        ],
        "+": [
            "00000",
            "00100",
            "00100",
            "11111",
            "00100",
            "00100",
            "00000",
        ],
    }

    def draw_char(ch: str, ox: int, oy: int, scale: int, color: tuple[int, int, int]) -> None:
        rows = glyphs[ch]
        for gy, row in enumerate(rows):
            for gx, bit in enumerate(row):
                if bit != "1":
                    continue
                for sy in range(scale):
                    py = oy + gy * scale + sy
                    if py < 0 or py >= height:
                        continue
                    for sx in range(scale):
                        px = ox + gx * scale + sx
                        if 0 <= px < width:
                            pixels[py][px] = color

    text = "PY++"
    scale = max(8, width // 120)
    char_w = 5 * scale
    char_h = 7 * scale
    spacing = 2 * scale
    total_w = len(text) * char_w + (len(text) - 1) * spacing
    start_x = (width - total_w) // 2
    start_y = (height - char_h) // 2 - scale
    for i, ch in enumerate(text):
        draw_char(ch, start_x + i * (char_w + spacing), start_y, scale, (245, 250, 255))

    # Subtitle stripe.
    sy = start_y + char_h + 2 * scale
    if 0 <= sy < height:
        for y in range(sy, min(height, sy + scale)):
            for x in range(bx0 + 40, bx0 + bw - 40):
                pixels[y][x] = (255, 245, 190)

    with path.open("w", encoding="utf-8", newline="\n") as f:
        f.write(f"P3\n{width} {height}\n255\n")
        for row in pixels:
            f.write(" ".join(f"{r} {g} {b}" for (r, g, b) in row))
            f.write("\n")


def iter_pypp_files(root: Path):
    for p in root.rglob("*.pypp"):
        parts = set(p.parts)
        if parts & SKIP_DIRS:
            continue
        yield p


def stamp_header(root: Path, image_path: Path) -> tuple[int, int]:
    image_rel = image_path.relative_to(root).as_posix()
    header = f"# title_image: {image_rel}"

    changed = 0
    total = 0
    for pypp in iter_pypp_files(root):
        total += 1
        text = pypp.read_text(encoding="utf-8")
        lines = text.splitlines()

        if lines and lines[0].startswith("# title_image:"):
            if lines[0] == header:
                continue
            lines[0] = header
            pypp.write_text("\n".join(lines) + ("\n" if text.endswith("\n") else ""), encoding="utf-8", newline="\n")
            changed += 1
            continue

        lines.insert(0, header)
        pypp.write_text("\n".join(lines) + ("\n" if text.endswith("\n") else ""), encoding="utf-8", newline="\n")
        changed += 1

    return changed, total


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate title image and apply to all .pypp files.")
    parser.add_argument("--root", default=".", help="Project root path (default: .)")
    parser.add_argument("--image", default="assets/pypp_title.ppm", help="Output image path relative to root")
    parser.add_argument("--force", action="store_true", help="Regenerate image even if it already exists")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    image_path = (root / args.image).resolve()
    if not image_path.exists() or args.force:
        generate_title_image(image_path)

    changed, total = stamp_header(root, image_path)
    print(f"[OK] image: {image_path}")
    print(f"[OK] stamped .pypp files: {changed}/{total}")


if __name__ == "__main__":
    main()
