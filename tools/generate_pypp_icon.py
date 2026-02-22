#!/usr/bin/env python3
"""Generate a custom .ico for pypp without external dependencies."""

from __future__ import annotations

import argparse
import struct
import zlib
from pathlib import Path


def _png_chunk(tag: bytes, data: bytes) -> bytes:
    return (
        struct.pack(">I", len(data))
        + tag
        + data
        + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    )


def _encode_png_rgba(width: int, height: int, rgba: bytes) -> bytes:
    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    stride = width * 4
    raw = bytearray()
    for y in range(height):
        raw.append(0)  # filter type
        start = y * stride
        raw.extend(rgba[start : start + stride])
    compressed = zlib.compress(bytes(raw), level=9)
    return sig + _png_chunk(b"IHDR", ihdr) + _png_chunk(b"IDAT", compressed) + _png_chunk(b"IEND", b"")


def _make_icon_rgba(size: int = 256) -> bytes:
    cx = (size - 1) / 2.0
    cy = (size - 1) / 2.0
    out = bytearray(size * size * 4)
    for y in range(size):
        for x in range(size):
            dx = (x - cx) / size
            dy = (y - cy) / size
            r2 = dx * dx + dy * dy
            t = max(0.0, 1.0 - min(1.0, r2 * 5.0))

            # Blue-cyan base gradient
            br = int(20 + 20 * t + (x / max(1, size - 1)) * 20)
            bg = int(65 + 120 * t + (y / max(1, size - 1)) * 15)
            bb = int(140 + 110 * t)
            a = 255

            # Circular alpha edge
            edge = (r2 ** 0.5) * 2.1
            if edge > 0.95:
                a = int(max(0, 255 * (1.15 - edge) / 0.20))
            if a < 0:
                a = 0

            idx = (y * size + x) * 4
            out[idx] = max(0, min(255, br))
            out[idx + 1] = max(0, min(255, bg))
            out[idx + 2] = max(0, min(255, bb))
            out[idx + 3] = max(0, min(255, a))

    # Draw a blocky "P" and "++" in white-ish.
    def draw_rect(x0: int, y0: int, w: int, h: int, r: int, g: int, b: int, a: int = 255) -> None:
        for yy in range(max(0, y0), min(size, y0 + h)):
            for xx in range(max(0, x0), min(size, x0 + w)):
                i = (yy * size + xx) * 4
                out[i] = r
                out[i + 1] = g
                out[i + 2] = b
                out[i + 3] = a

    s = size // 16
    ox = size // 5
    oy = size // 4
    fg = (245, 250, 255)

    # P
    draw_rect(ox, oy, 2 * s, 8 * s, *fg)
    draw_rect(ox + 2 * s, oy, 4 * s, 2 * s, *fg)
    draw_rect(ox + 2 * s, oy + 3 * s, 4 * s, 2 * s, *fg)
    draw_rect(ox + 6 * s, oy + 1 * s, 2 * s, 2 * s, *fg)

    # + +
    px = ox + 9 * s
    py = oy + 2 * s
    for n in range(2):
        dx = px + n * 4 * s
        draw_rect(dx + s, py, s, 4 * s, *fg)
        draw_rect(dx, py + s + s // 2, 3 * s, s, *fg)

    return bytes(out)


def write_ico(path: Path, png_data: bytes, size: int = 256) -> None:
    # ICO header
    icon_dir = struct.pack("<HHH", 0, 1, 1)
    # width/height byte: 0 means 256
    w = 0 if size >= 256 else size
    h = 0 if size >= 256 else size
    image_offset = 6 + 16
    entry = struct.pack(
        "<BBBBHHII",
        w,
        h,
        0,  # colors in palette
        0,  # reserved
        1,  # planes
        32,  # bpp
        len(png_data),
        image_offset,
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(icon_dir + entry + png_data)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate assets/pypp.ico")
    parser.add_argument("--out", default="assets/pypp.ico", help="Output .ico path")
    parser.add_argument("--size", type=int, default=256, help="Icon size (default: 256)")
    args = parser.parse_args()

    size = max(32, min(256, args.size))
    out_path = Path(args.out).resolve()
    rgba = _make_icon_rgba(size=size)
    png = _encode_png_rgba(size, size, rgba)
    write_ico(out_path, png, size=size)
    print(f"[OK] wrote icon: {out_path}")


if __name__ == "__main__":
    main()
