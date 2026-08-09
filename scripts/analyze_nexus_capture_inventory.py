#!/usr/bin/env python3
"""Inventory external Nexus Saturn raw captures by observed hardware state.

The classifications are deliberately hardware-state labels, not game-screen
labels.  A layer/register combination never proves startup, menu, HUD or
viewport ownership without an asset and consumer join.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window
from nexus_vdp2_registers import detect_byte_order, read_u16


REG = {
    "TVMD": 0x00,
    "BGON": 0x20,
    "CHCTLA": 0x28,
    "BMPNA": 0x2C,
}
STATE_RE = re.compile(r"ptmr:([0-9a-f]+),edsr:([0-9a-f]+)")


def classify(tvmd: int, bgon: int) -> str:
    if tvmd == 0 and bgon == 0:
        return "reset-no-vdp-layer"
    if tvmd == 0x0080 and bgon == 0x0002:
        return "nbg1-only"
    if tvmd == 0x2080 and bgon == 0x1110:
        return "rbg0-only"
    return "other-active-vdp2-state"


def manifest_binding(capture_dir: Path, blob: bytes) -> str:
    """Check the launcher-written raw hash when this run has one.

    Older operator manifests predate the raw hash fields and remain useful
    parameter receipts, but must not be reported as hash-bound captures.
    """
    candidates = [capture_dir / "capture.manifest", capture_dir / "manifest.txt"]
    candidates.extend(sorted(capture_dir.glob("*.manifest")))
    manifest = next((path for path in candidates if path.is_file()), None)
    if manifest is None:
        return "missing"
    try:
        fields: dict[str, str] = {}
        for line in manifest.read_text(encoding="utf-8").splitlines():
            if "=" in line:
                key, value = line.split("=", 1)
                fields[key] = value
    except (OSError, UnicodeError):
        return "mismatch"
    expected_hash = fields.get("raw_sha256", "").lower()
    expected_bytes = fields.get("raw_bytes", "")
    if not expected_hash and not expected_bytes:
        return "missing"
    if (not re.fullmatch(r"[0-9a-f]{64}", expected_hash) or
            not expected_bytes.isdigit() or
            int(expected_bytes) != len(blob) or
            expected_hash != hashlib.sha256(blob).hexdigest()):
        return "mismatch"
    return "verified"


def draw_descriptors(frame: dict[str, bytes], state: str) -> list[str]:
    """Return bounded hardware descriptors, never an inferred screen owner."""
    descriptors: list[str] = []
    for offset, words in command_window(frame["vdp1-vram"], state):
        control = words[0]
        command_type = control & 0x000F
        if control & 0x8000 or command_type > 2:
            continue
        colour_mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = (words[5] >> 8) & 0x00FF
        bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = (width * height * bits_per_pixel) // 8
        source_end = source_offset + source_size
        if source_size <= 0 or source_end > len(frame["vdp1-vram"]):
            continue
        source = frame["vdp1-vram"][source_offset:source_end]
        descriptors.append(
            f"0x{offset:05x}/t{command_type}/m{colour_mode}/"
            f"{width}x{height}/src0x{source_offset:05x}/n{source_size}/"
            f"sha256:{hashlib.sha256(source).hexdigest()}"
        )
    return descriptors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_root", type=Path)
    args = parser.parse_args()
    # Accept both the external corpus layout (run-*/runtime-vdp12.raw) and a
    # single operator capture directory.  The latter is useful for reviewing
    # a fresh run before it is folded into the corpus; do not require a
    # run-directory name to establish capture provenance.
    paths = sorted(
        {
            path
            for path in (
                [args.capture_root / "runtime-vdp12.raw"]
                + list(args.capture_root.glob("run-*/runtime-vdp12.raw"))
            )
            if path.is_file()
        }
    )
    if not paths:
        print("NEXUS_CAPTURE_INVENTORY_EMPTY")
        return 1

    totals: dict[str, int] = {}
    print(f"capture_files={len(paths)}")
    for path in paths:
        try:
            blob = path.read_bytes()
            frame_count = blob.count(b"frame=")
            frames, states = frame_regions(blob, frame_count)
        except (OSError, ValueError) as error:
            print(f"file={path.name} status=invalid reason={error}")
            continue
        labels = []
        active = 0
        vdp1_draw_frames = 0
        vdp1_draw_commands = 0
        vdp1_draw_sources: list[str] = []
        for frame, state in zip(frames, states):
            registers = frame["vdp2-regs"]
            byte_order = detect_byte_order(registers)
            tvmd = read_u16(registers, REG["TVMD"], byte_order)
            bgon = read_u16(registers, REG["BGON"], byte_order)
            chctla = read_u16(registers, REG["CHCTLA"], byte_order)
            bmpna = read_u16(registers, REG["BMPNA"], byte_order)
            label = classify(tvmd, bgon)
            labels.append(label)
            match = STATE_RE.search(state)
            if match and int(match.group(1), 16) != 0 and int(match.group(2), 16) != 0:
                active += 1
            try:
                descriptors = draw_descriptors(frame, state)
            except (ValueError, IndexError):
                descriptors = []
            command_count = len(descriptors)
            if descriptors:
                vdp1_draw_frames += 1
                vdp1_draw_commands += command_count
                vdp1_draw_sources.extend(
                    f"f{len(labels) - 1}:{descriptor}"
                    for descriptor in descriptors
                )
            totals[label] = totals.get(label, 0) + 1
            # Keep the first observation compact and reproducible; later
            # frames are represented by the distinct label/count summary.
            if len(labels) == 1:
                first = (
                    f"tvmd=0x{tvmd:04x},bgon=0x{bgon:04x},"
                    f"chctla=0x{chctla:04x},bmpna=0x{bmpna:04x},"
                    f"register_byte_order={byte_order}"
                )
        distinct = ",".join(sorted(set(labels)))
        print(
            f"file={path.parent.name} frames={len(frames)} "
            f"vdp1_nonidle_state_frames={active} states={distinct} first={first} "
            f"vdp1_draw_command_frames={vdp1_draw_frames} "
            f"vdp1_draw_commands={vdp1_draw_commands} "
            f"vdp1_draw_sources={'|'.join(vdp1_draw_sources) or 'none'} "
            f"manifest_binding={manifest_binding(path.parent, blob)} "
            "asset_consumer_identity=unbound"
        )
    print("frame_state_counts=" + ",".join(f"{key}:{value}" for key, value in sorted(totals.items())))
    print("startup_menu_hud_viewport_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
