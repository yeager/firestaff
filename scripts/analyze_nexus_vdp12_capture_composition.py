#!/usr/bin/env python3
"""Reconstruct one captured Nexus NBG0 frame plus selected VDP1 sprites.

This bounded analyzer accepts only hash-manifested Ymir captures and
unscaled 4bpp VDP1 normal-sprite commands. Its output is a capture comparison,
not a production renderer or a general Saturn VDP implementation.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path

from PIL import Image


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_manifest(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.strip():
            digest, relative = line.split("  ", 1)
            normalized = Path(relative).as_posix().removeprefix("./")
            if normalized in result:
                raise ValueError(f"duplicate manifest path: {normalized}")
            result[normalized] = digest
    return result


def manifest_hash(path: Path, manifest: Path) -> str:
    relative = path.resolve().relative_to(manifest.parent.resolve()).as_posix()
    expected = read_manifest(manifest).get(relative)
    if expected is None:
        raise ValueError(f"{relative} is absent from {manifest}")
    actual = sha256(path.read_bytes())
    if actual != expected:
        raise ValueError(f"{relative} does not match {manifest}")
    return actual


def u16be(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def color555(value: int) -> tuple[int, int, int]:
    return tuple(((value >> shift) & 31) << 3 for shift in (0, 5, 10))


def signed10(value: int) -> int:
    value &= 0x03FF
    return value - 0x400 if value & 0x200 else value


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, help="Ymir capture root with dumps/ and screenshots/")
    parser.add_argument("screenshot", type=Path, help="matching 640x448 Ymir screenshot")
    parser.add_argument("--capture-manifest", required=True, type=Path)
    parser.add_argument("--frame-index", required=True, type=int)
    parser.add_argument("--commands", required=True, help="comma-separated VDP1 command indices")
    parser.add_argument("--output-png", required=True, type=Path)
    parser.add_argument("--output-rgb", required=True, type=Path)
    parser.add_argument("--output-json", required=True, type=Path)
    args = parser.parse_args()

    dump = args.capture / "dumps"
    paths = {
        "vdp2-registers.bin": dump / "vdp2-registers.bin",
        "vdp2-vram.bin": dump / "vdp2-vram.bin",
        "vdp2-cram.bin": dump / "vdp2-cram.bin",
        "vdp1-fbs.bin": dump / "vdp1-fbs.bin",
        "vdp1-vram.bin": dump / "vdp1-vram.bin",
        "vdp1-command-trace.csv": dump / "vdp1-command-trace.csv",
        "vdp1-command-execution-frame13162.json":
            args.capture / "vdp1-command-execution-frame13162.json",
        "vdp-register-snapshot.txt": dump / "vdp-register-snapshot.txt",
        "screenshot.png": args.screenshot,
    }
    try:
        hashes = {name: manifest_hash(path, args.capture_manifest) for name, path in paths.items()}
        regs = paths["vdp2-registers.bin"].read_bytes()
        vdp2_vram = paths["vdp2-vram.bin"].read_bytes()
        cram = paths["vdp2-cram.bin"].read_bytes()
        fbs = paths["vdp1-fbs.bin"].read_bytes()
        vdp1_vram = paths["vdp1-vram.bin"].read_bytes()
        execution = json.loads(
            paths["vdp1-command-execution-frame13162.json"].read_text(encoding="utf-8")
        )
        snapshot = dict(
            line.split("=", 1)
            for line in paths["vdp-register-snapshot.txt"].read_text().splitlines()
            if "=" in line
        )
        screenshot = Image.open(args.screenshot).convert("RGB")
    except (OSError, ValueError) as error:
        parser.error(str(error))

    if (snapshot.get("format") != "ymir-vdp-register-snapshot-v1" or
            snapshot.get("register_byte_order") != "big_endian" or
            snapshot.get("vdp1_frame_index") != str(args.frame_index)):
        parser.error("register snapshot does not attest the requested VDP1 frame")
    if (execution.get("format") != "firestaff-nexus-vdp1-command-execution-audit-v1" or
            execution.get("frame_index") != args.frame_index):
        parser.error("command execution receipt does not attest the requested frame")
    if (len(regs) != 512 or len(vdp2_vram) != 0x80000 or len(cram) != 4096 or
            len(fbs) != 0x80000 or fbs[:0x40000] != fbs[0x40000:] or
            len(vdp1_vram) != 0x80000 or screenshot.size != (640, 448)):
        parser.error("unexpected capture sizes or screenshot dimensions")
    observed = {
        "TVMD": u16be(regs, 0x00), "BGON": u16be(regs, 0x20),
        "CHCTLA": u16be(regs, 0x28), "BMPNA": u16be(regs, 0x2C),
        "CRAOFA": u16be(regs, 0xE4), "CRAOFB": u16be(regs, 0xE6),
    }
    if observed != {"TVMD": 0x8000, "BGON": 1, "CHCTLA": 0x0013,
                    "BMPNA": 0, "CRAOFA": 0, "CRAOFB": 0}:
        parser.error("capture does not establish the observed NBG0 8bpp bitmap lane")
    if (args.frame_index < 0 or not args.commands.strip() or
            any(not item.strip().isdigit() for item in args.commands.split(","))):
        parser.error("frame and command indices must be non-negative decimal integers")
    indices = [int(item) for item in args.commands.split(",")]
    if len(set(indices)) != len(indices):
        parser.error("command indices must be unique")
    executed = set(execution.get("executed_command_indices", []))
    if not set(indices).issubset(executed):
        parser.error("one or more selected VDP1 commands were not executed in the attested frame")
    command_rows: dict[int, dict[str, str]] = {}
    with paths["vdp1-command-trace.csv"].open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            if int(row["frame"]) == args.frame_index and int(row["index"]) in indices:
                index = int(row["index"])
                if index in command_rows:
                    parser.error(f"duplicate command {index} in frame trace")
                command_rows[index] = row
    if set(command_rows) != set(indices):
        parser.error("one or more selected VDP1 commands are absent from the requested frame")

    # Decode the observed 512x256 NBG0 8bpp bitmap over the captured VDP1 FBS.
    output = bytearray(320 * 224 * 3)
    fbs_bytes = fbs[:0x40000]
    for y in range(224):
        for x in range(320):
            index = vdp2_vram[y * 512 + x]
            color = (color555(u16be(cram, index * 2)) if index else
                     color555(u16be(fbs_bytes, (y * 512 + x) * 2) & 0x7FFF))
            offset = (y * 320 + x) * 3
            output[offset : offset + 3] = bytes(color)

    command_reports = []
    for index in indices:
        row = command_rows[index]
        words = tuple(int(row[f"word{i}"], 16) for i in range(16))
        address = int(row["address"], 16)
        if address % 32 or address + 32 > len(vdp1_vram):
            parser.error(f"command {index} has an invalid VDP1 command address")
        if struct.unpack_from(">16H", vdp1_vram, address) != words:
            parser.error(f"command {index} does not match captured VDP1 VRAM")
        control, pmod, colr, source_word, size = words[0], words[2], words[3], words[4], words[5]
        if ((control & 0x000F) != 0 or ((pmod >> 3) & 7) != 0 or
                (pmod & 0x0030) != 0):
            parser.error(f"command {index} is outside the supported normal 4bpp sprite lane")
        width = ((size >> 8) & 0x3F) * 8
        height = size & 0xFF
        if width <= 0 or height <= 0:
            parser.error(f"command {index} has invalid dimensions")
        texture_start = source_word * 8
        texture_bytes = width * height // 2
        if texture_start + texture_bytes > len(vdp1_vram):
            parser.error(f"command {index} texture exceeds captured VDP1 VRAM")
        x0, y0 = signed10(words[6]), signed10(words[7])
        palette_base = colr & 0x07FF
        cram_offset = ((u16be(regs, 0xE6) >> 4) & 7) << 8
        flip_x, flip_y = bool(control & 0x0010), bool(control & 0x0020)
        transparent_zero = (pmod & 0x0040) == 0
        drawn = 0
        for y in range(height):
            sy = height - 1 - y if flip_y else y
            for x in range(width):
                sx = width - 1 - x if flip_x else x
                packed = vdp1_vram[texture_start + (sy * width + sx) // 2]
                color_index = packed >> 4 if sx % 2 == 0 else packed & 0x0F
                if color_index == 0 and transparent_zero:
                    continue
                dx, dy = x0 + x, y0 + y
                if not (0 <= dx < 320 and 0 <= dy < 224):
                    continue
                color_slot = ((palette_base + color_index + cram_offset) & 0x03FF) * 2
                color = color555(u16be(cram, color_slot))
                offset = (dy * 320 + dx) * 3
                output[offset : offset + 3] = bytes(color)
                drawn += 1
        command_reports.append({
            "index": index, "command_address": f"0x{address:05x}",
            "command_words_match_vdp1_vram": True,
            "texture_byte_span": [f"0x{texture_start:05x}", f"0x{texture_start + texture_bytes:05x}"],
            "texture_size": [width, height], "palette_base": f"0x{palette_base:03x}",
            "position": [x0, y0], "flip_x": flip_x, "flip_y": flip_y,
            "transparent_index_zero": transparent_zero, "opaque_pixels_drawn": drawn,
        })

    args.output_png.parent.mkdir(parents=True, exist_ok=True)
    composite = Image.frombytes("RGB", (320, 224), bytes(output))
    composite.save(args.output_png)
    args.output_rgb.write_bytes(output)
    screenshot_pixels = screenshot.load()
    within8 = exact = samples = max_error = 0
    for y in range(224):
        for x in range(320):
            reconstructed = output[(y * 320 + x) * 3 : (y * 320 + x + 1) * 3]
            for dy in (0, 1):
                for dx in (0, 1):
                    actual = screenshot_pixels[x * 2 + dx, y * 2 + dy]
                    error = max(abs(reconstructed[c] - actual[c]) for c in range(3))
                    exact += error == 0
                    within8 += error <= 8
                    samples += 1
                    max_error = max(max_error, error)
    report = {
        "format": "firestaff-nexus-vdp12-capture-composition-v1",
        "frame_index": args.frame_index,
        "capture_input_sha256": hashes,
        "command_execution_audit_sha256": hashes["vdp1-command-execution-frame13162.json"],
        "registers": {key: f"0x{value:04x}" for key, value in observed.items()},
        "composition": ["captured VDP1 FBS", "captured NBG0 8bpp bitmap", *[f"VDP1 command {i}" for i in indices]],
        "commands": command_reports,
        "screenshot_comparison": {
            "samples": samples, "exact": exact, "within_8_rgb_levels_per_channel": within8,
            "maximum_channel_error": max_error,
        },
        "renderer_permitted": False,
        "limitations": [
            "This is a bounded capture reconstruction for the selected command rows, not a general VDP1/VDP2 emulator.",
            "Selected command bytes are bound to captured VDP1 VRAM; retail asset ownership must be verified separately.",
        ],
    }
    args.output_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"frame={args.frame_index} commands={len(indices)} screenshotWithin8={within8}/{samples} maxError={max_error}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
