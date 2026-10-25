#!/usr/bin/env python3
"""Verify the hash-pinned Nexus Ymir title-menu VDP evidence bundle."""
from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

BUNDLE = Path(__file__).resolve().parents[1] / "parity-evidence/verification/nexus_v1_ymir_title_menu_vdp_bundle_20261025"
HASHES = {
    "title-menu-paused.png": "e30bfc58f9c8a1bf8485007253dc150ef0091e01b670c1684010ac4ad0a346a9",
    "vdp1-capture-bundle.txt": "76e920d7ecb3efeb15599dd72772789fdcadc8c1604bb5828455dc17ecbd82b9",
    "vdp-register-snapshot.txt": "176d158e6bcdf6ea4a640b95be9f0cbe10532f0b34965f946fe90a5c577dd476",
    "vdp1-command-trace.csv": "47d55ec3a4b45c97a96390c60322edc0f56dd0aad6f0e11677e0b2dbff962d81",
    "vdp1-vram.bin": "38ec97126b500f6c7f4a34b6b4c1b54895ed6113f2b1b32b23b8635e07b79a18",
    "vdp1-registers.bin": "26d485a48f56ddf97c2209343b92f45fdcf4e41937d1e9f198db2b0dc1df26b5",
    "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
    "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
    "vdp2-registers.bin": "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d",
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_receipt(path: Path) -> dict[str, str]:
    return dict(line.split("=", 1) for line in path.read_text().splitlines() if "=" in line)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", required=True, type=Path)
    parser.add_argument("--dump-root", required=True, type=Path,
                        help="local, untracked Ymir VDP dump directory")
    args = parser.parse_args()
    for name, expected in HASHES.items():
        path = BUNDLE / name
        if name.endswith(".bin"):
            path = args.dump_root / name
        actual = digest(path)
        assert actual == expected, f"hash mismatch: {name}"
    receipt = parse_receipt(BUNDLE / "vdp1-capture-bundle.txt")
    registers = parse_receipt(BUNDLE / "vdp-register-snapshot.txt")
    assert receipt["format"] == "ymir-vdp1-capture-bundle-v1"
    assert receipt["capture_frame_index"] == registers["vdp1_frame_index"] == "497"
    assert receipt["capture_next_command_index"] == registers["vdp1_next_command_index"] == "178"
    assert receipt["trace_file_complete"] == "1"
    assert receipt["trace_rows"] == "11896"
    assert receipt["trace_rows_for_capture_frame"] == "178"
    assert receipt["trace_dropped_total"] == "0"
    with (BUNDLE / "vdp1-command-trace.csv").open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    assert len(rows) == 11896
    captured = [row for row in rows if row["frame"] == "497"]
    assert len(captured) == 178 and captured[-1]["index"] == "177"
    menu = (args.data_root / "MENU.BPK").read_bytes()
    palt_offset = menu.rfind(b"PALT")
    assert palt_offset >= 0 and palt_offset + 524 == len(menu)
    assert int.from_bytes(menu[palt_offset + 4:palt_offset + 8], "big") == 524
    assert int.from_bytes(menu[palt_offset + 8:palt_offset + 12], "big") == 256
    palt = menu[palt_offset + 12:palt_offset + 524]
    swapped = b"".join(palt[index:index + 2][::-1] for index in range(0, len(palt), 2))
    for name in ("vdp1-vram.bin", "vdp2-vram.bin", "vdp2-cram.bin"):
        memory = (args.dump_root / name).read_bytes()
        assert palt not in memory and swapped not in memory, f"unexpected contiguous PALT: {name}"
    print("Nexus Ymir title-menu VDP bundle: PASS")


if __name__ == "__main__":
    main()
