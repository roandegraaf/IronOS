#!/usr/bin/env python3
"""Writes mono_128x32/hero_font.cpp (original digit design, no third-party font licence). Usage: gen_hero_font.py [--sheet out.png]"""

import argparse
import os

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_CPP = os.path.join(
    HERE, "..", "..", "source", "Core", "Threads", "UI", "drawing", "mono_128x32", "hero_font.cpp"
)

K = 8


class Face:
    def __init__(self, w: int, h: int, stroke: int, radius: float, cell_h: int, top: int):
        self.w, self.h, self.s, self.r, self.cell_h, self.top = w, h, stroke, radius, cell_h, top
        self.mid = (h - stroke) // 2


TALL = Face(w=16, h=28, stroke=4, radius=5.0, cell_h=32, top=2)
SHORT = Face(w=14, h=24, stroke=4, radius=4.5, cell_h=24, top=0)


def canvas(w: float, h: float) -> np.ndarray:
    return np.zeros((int(h * K), int(w * K)), dtype=bool)


def rect(img: np.ndarray, x0: float, y0: float, x1: float, y1: float) -> None:
    img[int(y0 * K) : int(y1 * K), int(x0 * K) : int(x1 * K)] = True


def rounded_box_mask(w: float, h: float, r: float) -> np.ndarray:
    ys, xs = np.mgrid[0 : int(h * K), 0 : int(w * K)]
    px = (xs + 0.5) / K
    py = (ys + 0.5) / K
    cx = np.clip(px, r, w - r)
    cy = np.clip(py, r, h - r)
    return (px - cx) ** 2 + (py - cy) ** 2 <= r * r


def downsample(img: np.ndarray) -> np.ndarray:
    h, w = img.shape[0] // K, img.shape[1] // K
    cover = img.reshape(h, K, w, K).mean(axis=(1, 3))
    return cover >= 0.5


def segments(f: Face) -> dict:
    W, H, S, MID = f.w, f.h, f.s, f.mid
    return {
        "T": (0, 0, W, S),
        "M": (0, MID, W, MID + S),
        "B": (0, H - S, W, H),
        "UL": (0, 0, S, MID + S),
        "UR": (W - S, 0, W, MID + S),
        "LL": (0, MID, S, H),
        "LR": (W - S, MID, W, H),
    }


def digits(f: Face) -> dict:
    W, H, S, MID = f.w, f.h, f.s, f.mid
    return {
        "0": ["T", "B", "UL", "UR", "LL", "LR"],
        "1": [(W - 2 * S, 0, W - S, H), (W - 3 * S - 1, 0, W - S, S)],
        "2": ["T", "UR", "M", "LL", "B"],
        "3": ["T", "UR", (S + 1, MID, W, MID + S), "LR", "B"],
        "4": ["UL", "M", (W - S, 0, W, H)],
        "5": ["T", "UL", "M", "LR", "B"],
        "6": ["T", "UL", "LL", "M", "LR", "B"],
        "7": ["T", (W - S, 0, W, H)],
        "8": ["T", "M", "B", "UL", "UR", "LL", "LR"],
        "9": ["T", "UL", "UR", "M", "LR", "B"],
        "-": [(0, MID, W, MID + S)],
    }


def render_digit(f: Face, parts) -> np.ndarray:
    segs = segments(f)
    img = canvas(f.w, f.h)
    for part in parts:
        rect(img, *(segs[part] if isinstance(part, str) else part))
    img &= rounded_box_mask(f.w, f.h, f.r)
    cell = np.zeros((f.cell_h, f.w), dtype=bool)
    cell[f.top : f.top + f.h] = downsample(img)
    return cell


def digits_labels():
    return digits(TALL).keys()


def render_digits(f: Face):
    return [render_digit(f, parts) for parts in digits(f).values()]


def render_degree(size: int, stroke: int, outer_r: float, inner_r: float, cell_h: int, top: int) -> np.ndarray:
    img = rounded_box_mask(size, size, outer_r) & ~np.pad(
        rounded_box_mask(size - 2 * stroke, size - 2 * stroke, inner_r), stroke * K
    )
    cell = np.zeros((cell_h, size), dtype=bool)
    cell[top : top + size] = downsample(img)
    return cell


def from_ascii(rows) -> np.ndarray:
    return np.array([[ch == "#" for ch in row] for row in rows], dtype=bool)


BOLT = from_ascii(
    [
        "........",
        "........",
        ".....##.",
        "....###.",
        "...###..",
        "..###...",
        ".#######",
        "#######.",
        "...###..",
        "..###...",
        "..##....",
        ".##.....",
        ".#......",
        "........",
        "........",
        "........",
    ]
)


ARROW_RIGHT = from_ascii(
    [
        "......",
        "..#...",
        "...#..",
        "#####.",
        "...#..",
        "..#...",
        "......",
        "......",
    ]
)

ARROW_UP = from_ascii(
    [
        "..#...",
        ".###..",
        "#####.",
        "..#...",
        "..#...",
        "..#...",
        "..#...",
        "......",
    ]
)


USB_C = from_ascii(
    [
        "........",
        "........",
        "...##...",
        "...##...",
        "..####..",
        ".######.",
        ".######.",
        ".######.",
        ".######.",
        "..####..",
        "..#..#..",
        "..#..#..",
        "..####..",
        "........",
        "........",
        "........",
    ]
)

BATTERY_OUTLINE = from_ascii(
    [
        "........",
        "........",
        "..###...",
        "#######.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#.....#.",
        "#######.",
        "........",
        "........",
        "........",
    ]
)

BOOST = from_ascii(
    [
        "..#...",
        ".#.#..",
        "#...#.",
        "..#...",
        ".#.#..",
        "#...#.",
        "......",
        "......",
    ]
)

HEAT = from_ascii(
    [
        "..#...",
        "..##..",
        ".###..",
        ".####.",
        "##.##.",
        "#...#.",
        ".###..",
        "......",
    ]
)

SLEEP = from_ascii(
    [
        "..##..",
        ".##...",
        "##....",
        "##....",
        "##....",
        ".##...",
        "..##..",
        "......",
    ]
)

ICONS = [
    ("HeroIconUsbC", USB_C, "USB-C power source", "HERO_ICON_W * 2"),
    ("HeroIconBatteryOutline", BATTERY_OUTLINE, "battery outline, level filled at runtime", "HERO_ICON_W * 2"),
    ("HeroIconBoost", BOOST, "boost", "HERO_SMALL_ICON_W"),
    ("HeroIconHeat", HEAT, "heat", "HERO_SMALL_ICON_W"),
    ("HeroIconSleep", SLEEP, "sleep", "HERO_SMALL_ICON_W"),
]


def to_strips(cell: np.ndarray) -> bytes:
    h, w = cell.shape
    out = bytearray()
    for strip in range(h // 8):
        for x in range(w):
            b = 0
            for bit in range(8):
                if cell[strip * 8 + bit, x]:
                    b |= 1 << bit
            out.append(b)
    return bytes(out)


def c_array(name: str, cells, labels) -> str:
    lines = [f"const uint8_t {name}[] = {{"]
    for cell, label in zip(cells, labels):
        data = ", ".join(f"0x{b:02X}" for b in to_strips(cell))
        lines.append(f"    {data}, // {label}")
    lines.append("};")
    return "\n".join(lines)


def write_cpp(digits, degree, short_digits, short_degree, bolt, arrow_right, arrow_up) -> None:
    labels = list(digits_labels())
    body = "\n\n".join(
        [
            c_array("HeroDigitFont", digits, labels),
            c_array("HeroDegreeGlyph", [degree], ["degree sign"]),
            c_array("HeroShortDigitFont", short_digits, labels),
            c_array("HeroShortDegreeGlyph", [short_degree], ["degree sign"]),
            c_array("HeroIconBolt", [bolt], ["bolt"]),
            c_array("HeroIconArrowRight", [arrow_right], ["set point arrow"]),
            c_array("HeroIconArrowUp", [arrow_up], ["boost arrow"]),
        ]
        + [c_array(name, [cell], [label]) for name, cell, label, _ in ICONS]
    )
    icon_asserts = "\n".join(
        f'static_assert(sizeof({name}) == {size}, "{name} size");' for name, _, _, size in ICONS
    )
    src = f"""// Generated by tools/fonts/gen_hero_font.py; edit the generator, not this file.
#include "configuration.h"
#ifdef OLED_128x32_HERO_UI
#include "hero_ui.hpp"

// clang-format off
{body}
// clang-format on

static_assert(sizeof(HeroDigitFont) == HERO_GLYPH_COUNT * HERO_DIGIT_W * (HERO_DIGIT_H / 8), "hero font size");
static_assert(sizeof(HeroDegreeGlyph) == HERO_DEGREE_W * 2, "hero degree size");
static_assert(sizeof(HeroShortDigitFont) == HERO_GLYPH_COUNT * HERO_SHORT_DIGIT_W * (HERO_SHORT_DIGIT_H / 8), "short hero font size");
static_assert(sizeof(HeroShortDegreeGlyph) == HERO_SHORT_DEGREE_W, "short hero degree size");
static_assert(sizeof(HeroIconBolt) == HERO_ICON_W * 2, "hero bolt size");
static_assert(sizeof(HeroIconArrowRight) == HERO_SMALL_ICON_W, "hero arrow size");
static_assert(sizeof(HeroIconArrowUp) == HERO_SMALL_ICON_W, "hero arrow size");
{icon_asserts}
#endif /* OLED_128x32_HERO_UI */
"""
    with open(OUT_CPP, "w") as fh:
        fh.write(src)


def write_sheet(path: str, cells) -> None:
    import zlib
    import struct

    scale, gap = 6, 2
    height = max(c.shape[0] for c in cells)
    width = sum(c.shape[1] + gap for c in cells)
    sheet = np.zeros((height, width), dtype=bool)
    x = 0
    for c in cells:
        sheet[: c.shape[0], x : x + c.shape[1]] = c
        x += c.shape[1] + gap
    big = np.kron(sheet, np.ones((scale, scale), dtype=bool)).astype(np.uint8) * 255
    raw = b"".join(b"\x00" + row.tobytes() for row in big)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data))

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", big.shape[1], big.shape[0], 8, 0, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as fh:
        fh.write(png)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sheet", help="also write an enlarged PNG of every glyph")
    args = parser.parse_args()
    tall = render_digits(TALL)
    degree = render_degree(size=8, stroke=3, outer_r=3.0, inner_r=1.0, cell_h=16, top=TALL.top)
    short = render_digits(SHORT)
    short_degree = render_degree(size=6, stroke=2, outer_r=2.5, inner_r=1.0, cell_h=8, top=SHORT.top)
    write_cpp(tall, degree, short, short_degree, BOLT, ARROW_RIGHT, ARROW_UP)
    if args.sheet:
        write_sheet(args.sheet, tall + [degree] + short + [short_degree, BOLT, ARROW_RIGHT, ARROW_UP] + [cell for _, cell, _, _ in ICONS])


if __name__ == "__main__":
    main()
