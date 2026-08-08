#!/usr/bin/env python3
"""Inventory external Nexus Saturn raw captures by observed hardware state.

The classifications are deliberately hardware-state labels, not game-screen
labels.  A layer/register combination never proves startup, menu, HUD or
viewport ownership without an asset and consumer join.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


REG = {
    "TVMD": 0x00,
    "BGON": 0x20,
    "CHCTLA": 0x28,
    "BMPNA": 0x2C,
}
STATE_RE = re.compile(r"ptmr:([0-9a-f]+),edsr:([0-9a-f]+)")


def u16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def classify(tvmd: int, bgon: int) -> str:
    if tvmd == 0 and bgon == 0:
        return "reset-no-vdp-layer"
    if tvmd == 0x0080 and bgon == 0x0002:
        return "nbg1-only"
    if tvmd == 0x2080 and bgon == 0x1110:
        return "rbg0-only"
    return "other-active-vdp2-state"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_root", type=Path)
    args = parser.parse_args()
    paths = sorted(args.capture_root.glob("run-*/runtime-vdp12.raw"))
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
        for frame, state in zip(frames, states):
            registers = frame["vdp2-regs"]
            tvmd = u16(registers, REG["TVMD"])
            bgon = u16(registers, REG["BGON"])
            chctla = u16(registers, REG["CHCTLA"])
            bmpna = u16(registers, REG["BMPNA"])
            label = classify(tvmd, bgon)
            labels.append(label)
            match = STATE_RE.search(state)
            if match and int(match.group(1), 16) != 0 and int(match.group(2), 16) != 0:
                active += 1
            totals[label] = totals.get(label, 0) + 1
            # Keep the first observation compact and reproducible; later
            # frames are represented by the distinct label/count summary.
            if len(labels) == 1:
                first = (
                    f"tvmd=0x{tvmd:04x},bgon=0x{bgon:04x},"
                    f"chctla=0x{chctla:04x},bmpna=0x{bmpna:04x}"
                )
        distinct = ",".join(sorted(set(labels)))
        print(
            f"file={path.parent.name} frames={len(frames)} "
            f"vdp1_nonidle_state_frames={active} states={distinct} first={first} "
            "asset_consumer_identity=unbound"
        )
    print("frame_state_counts=" + ",".join(f"{key}:{value}" for key, value in sorted(totals.items())))
    print("startup_menu_hud_viewport_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
