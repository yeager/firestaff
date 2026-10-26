#!/usr/bin/env python3
"""Verify an authentic Ymir Nexus title-state VDP2 checkpoint.

The retail disc, BIOS, save state, and Ymir dump are supplied at invocation
time.  No game bytes are stored in this repository.  This deliberately checks
only the static VDP2 title checkpoint; it does not claim VDP1 menu parity.
"""

from __future__ import annotations

import argparse
import hashlib
import shlex
from pathlib import Path


EXPECTED = {
    "cue": "878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699",
    "bios": "dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac",
    "state": "11d88ef517c408b3bca5c078c0ccb8081dbf3342f308239dd73403c73f0c3774",
    "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
    "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
    "vdp2-registers.bin": "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d",
}

EXPECTED_TRACKS = {
    "Dungeon Master Nexus (Japan) (Track 1).bin": "91a6e0bbed515475be57bf073aa9ea613b49eb65fb4b85ff06472025f9f3676d",
    "Dungeon Master Nexus (Japan) (Track 2).bin": "a447566895c1b1bc44883b097860f3be16aefdd5d8a32546ff7463fd1ccc7369",
    "Dungeon Master Nexus (Japan) (Track 3).bin": "9f4903d57ad89b4efef5b296b1512eceeb987139397fd1b97876da866b31a8f8",
    "Dungeon Master Nexus (Japan) (Track 4).bin": "62a6faaac97b746d787eba5655701d632908484e7bf49951122dde996d96d21c",
    "Dungeon Master Nexus (Japan) (Track 5).bin": "ba03c2e897ece090d51430dfb6728dbc4991983c62dd37cc8fca5c94a330ed37",
    "Dungeon Master Nexus (Japan) (Track 6).bin": "5e9bc4154cdfbaa3bddee629b98985bab86b15ce66c4a61f96acd53bd938fc41",
    "Dungeon Master Nexus (Japan) (Track 7).bin": "f7d55a91ea15234f09cb4bcc1616cef1a136b8e3ae80a737a45a2371f8313843",
    "Dungeon Master Nexus (Japan) (Track 8).bin": "26498f676f2e2945e88707e6efc1c1afb16bd5fe1ca2508edd79138ca0158591",
    "Dungeon Master Nexus (Japan) (Track 9).bin": "6dfd7b3551b1b701af5fb17b40ff608a5e186b86f50a6acb70a73a9297d8a3f4",
}

EXPECTED_SIZES = {
    "vdp2-registers.bin": 512,
}


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def require_digest(label: str, path: Path) -> None:
    actual = digest(path)
    expected = EXPECTED[label]
    if actual != expected:
        raise ValueError(f"{label} SHA-256 mismatch: {actual} != {expected}")


def cue_track_paths(cue_path: Path) -> list[Path]:
    """Resolve every FILE directive while keeping tracks beside the retail CUE."""
    cue_path = cue_path.resolve(strict=True)
    tracks: list[Path] = []
    for line_number, line in enumerate(cue_path.read_text(encoding="utf-8-sig").splitlines(), 1):
        first_field = line.lstrip().split(maxsplit=1)
        if not first_field or first_field[0].upper() != "FILE":
            continue
        try:
            fields = shlex.split(line, posix=True)
        except ValueError as exc:
            raise ValueError(f"{cue_path}:{line_number}: invalid FILE directive: {exc}") from exc
        if len(fields) != 3 or fields[0].upper() != "FILE":
            raise ValueError(f"{cue_path}:{line_number}: malformed FILE directive")
        name, file_type = fields[1], fields[2].upper()
        expected = EXPECTED_TRACKS.get(name)
        if expected is None:
            raise ValueError(f"{cue_path}:{line_number}: unpinned retail track {name!r}")
        if file_type not in {"BINARY", "MOTOROLA", "WAVE"}:
            raise ValueError(f"{cue_path}:{line_number}: unsupported track type {file_type!r}")
        track_path = (cue_path.parent / name).resolve(strict=True)
        try:
            track_path.relative_to(cue_path.parent)
        except ValueError as exc:
            raise ValueError(f"{cue_path}:{line_number}: track escapes the CUE directory") from exc
        if not track_path.is_file():
            raise ValueError(f"{cue_path}:{line_number}: track is not a file: {name}")
        actual = digest(track_path)
        if actual != expected:
            raise ValueError(f"{name} SHA-256 mismatch: {actual} != {expected}")
        tracks.append(track_path)
    if not tracks:
        raise ValueError(f"{cue_path}: no FILE directives")
    if len({path.name for path in tracks}) != len(tracks):
        raise ValueError(f"{cue_path}: duplicate FILE track reference")
    if {path.name for path in tracks} != set(EXPECTED_TRACKS):
        raise ValueError(f"{cue_path}: CUE does not reference the complete pinned retail track set")
    return tracks


def validate_register_snapshot(path: Path) -> dict[str, str]:
    """Validate the captured snapshot schema and register/frame metadata."""
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise ValueError(f"cannot read VDP register snapshot {path}: {exc}") from exc
    fields: dict[str, str] = {}
    for line_number, line in enumerate(lines, 1):
        if not line:
            continue
        if "=" not in line:
            raise ValueError(f"{path}:{line_number}: malformed snapshot field")
        key, value = line.split("=", 1)
        if not key or not value or key in fields:
            raise ValueError(f"{path}:{line_number}: empty or duplicate snapshot field")
        fields[key] = value

    expected = {
        "format": "ymir-vdp-register-snapshot-v1",
        "register_byte_order": "big_endian",
        "register_word_bytes": "2",
        "vdp1_register_address_base": "0x00000000",
        "vdp1_register_bytes": "32",
        "vdp2_register_address_base": "0x00000000",
        "vdp2_register_bytes": "512",
    }
    for key, value in expected.items():
        if fields.get(key) != value:
            raise ValueError(f"{path}: {key} mismatch: {fields.get(key)!r} != {value!r}")

    integer_fields = (
        "vdp1_frame_index", "vdp1_next_command_index",
        "vdp1_current_command_address", "vdp1_local_x", "vdp1_local_y",
    )
    numbers: dict[str, int] = {}
    for key in integer_fields:
        try:
            value = fields[key]
            numbers[key] = int(value, 0 if value.lower().startswith("0x") else 10)
        except (KeyError, ValueError) as exc:
            raise ValueError(f"{path}: missing or invalid integer field {key}") from exc
    if not 0 <= numbers["vdp1_frame_index"] <= 0xFFFFFFFF:
        raise ValueError(f"{path}: vdp1_frame_index is outside unsigned 32-bit range")
    if not 0 <= numbers["vdp1_next_command_index"] <= 0xFFFF:
        raise ValueError(f"{path}: vdp1_next_command_index is outside unsigned 16-bit range")
    address = numbers["vdp1_current_command_address"]
    if address < 0 or address > 0x7FFFFFFF or address % 4:
        raise ValueError(f"{path}: invalid vdp1_current_command_address")
    for key in ("vdp1_local_x", "vdp1_local_y"):
        if not -32768 <= numbers[key] <= 32767:
            raise ValueError(f"{path}: {key} is outside signed 16-bit range")
    return fields


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cue", required=True, type=Path,
                        help="hash-pinned Japanese retail Nexus CUE")
    parser.add_argument("--bios", required=True, type=Path,
                        help="hash-pinned Japanese Saturn IPL ROM")
    parser.add_argument("--state", required=True, type=Path,
                        help="hash-pinned Ymir title-prompt save state")
    parser.add_argument("--dump-root", required=True, type=Path,
                        help="Ymir DumpMemory output after loading CLI slot 2")
    args = parser.parse_args()

    try:
        for label, path in (("cue", args.cue), ("bios", args.bios), ("state", args.state)):
            require_digest(label, path)
        cue_track_paths(args.cue)
        for name in ("vdp2-vram.bin", "vdp2-cram.bin", "vdp2-registers.bin"):
            require_digest(name, args.dump_root / name)
        for name, size in EXPECTED_SIZES.items():
            actual = (args.dump_root / name).stat().st_size
            if actual != size:
                raise ValueError(f"{name} size mismatch: {actual} != {size}")
        validate_register_snapshot(args.dump_root / "vdp-register-snapshot.txt")
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    print("Nexus Ymir title-state VDP2 checkpoint: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
