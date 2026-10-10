#!/usr/bin/env python3
"""Lock US dungeon labels and retrieval text to authentic Track 02 bytes."""

from __future__ import annotations

import ast
import hashlib
import os
from pathlib import Path
import re
import sys


MEDIA_FILENAME = "TQUS02.bin"
MEDIA_MD5 = "f23601102138f87c33025877767ebf76"
MEDIA_SHA256 = "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565"
MEDIA_SIZE = 8_104_992
RAW_SECTOR_BYTES = 2352
USER_DATA_OFFSET = 16
USER_DATA_BYTES = 2048
SOURCE_PATH = (
    Path(__file__).resolve().parents[1]
    / "src/theron/theron_v1_track02_dungeon_text.c"
)
DUNGEON_NAMES = (
    ("AKUTUBA", 0x2741EF, b"\x01"),
    ("DRATOR", 0x2741F7, b"\x01"),
    ("FORMIC", 0x2741FF, b"\x01"),
    ("SARMON", 0x274207, b"\x01"),
    ("SHADO", 0x27420F, b"\x01"),
    ("THIEF", 0x274217, b"\x01"),
    ("DEMON", 0x27421F, b"\x00"),
)
TREASURE_NAMES = (
    ("Shield Defiant", 0x27715B),
    ("Taza Boots", 0x27718E),
    ("Taza Poleyn", 0x2771BD),
    ("Soulcage", 0x2771ED),
    ("Taza Armour", 0x27721A),
    ("Tazahelm", 0x27724A),
    ("Retaliator", 0x277277),
)
RETRIEVAL_OFFSETS = (0x27713F, 0x277172, 0x2771A1, 0x2771D1,
                     0x2771FE, 0x27722E, 0x27725B)
GAME_SPEED_LABEL = (b"GAME SPEED", 0x274227)


def c_table_entries(source: str, table_name: str) -> tuple[tuple[int, bytes], ...]:
    table = re.search(
        rf"static const char \*const {re.escape(table_name)}\[[^\]]+\] = \{{(.*?)\n\}};",
        source,
        re.DOTALL,
    )
    if table is None:
        raise AssertionError(f"could not find the compiled {table_name} table")

    entries = []
    for line in table.group(1).splitlines():
        offset = re.search(r"UD 0x([0-9a-fA-F]+)", line)
        literals = re.findall(r'"(?:[^"\\]|\\.)*"', line)
        if offset is None or not literals:
            continue
        try:
            value = "".join(ast.literal_eval(literal) for literal in literals).encode(
                "ascii"
            )
        except (SyntaxError, UnicodeError, ValueError) as error:
            raise AssertionError(f"invalid ASCII literal in {table_name}") from error
        entries.append((int(offset.group(1), 16), value))
    return tuple(entries)


def main() -> int:
    media_root = Path(
        os.environ.get(
            "FIRESTAFF_THERON_TEST_DATA_DIR",
            Path.home() / ".firestaff/data/theron",
        )
    )
    media_path = media_root / MEDIA_FILENAME
    if not media_path.is_file():
        print("SKIP: authentic US Theron Track 02 media is unavailable")
        return 77

    if media_path.stat().st_size != MEDIA_SIZE:
        raise AssertionError(
            f"expected {MEDIA_SIZE} raw bytes, got {media_path.stat().st_size}"
        )
    with media_path.open("rb") as media_file:
        image = media_file.read(MEDIA_SIZE + 1)
    if len(image) != MEDIA_SIZE:
        raise AssertionError(f"bounded media read returned {len(image)} bytes")
    actual_md5 = hashlib.md5(image).hexdigest()
    actual_sha256 = hashlib.sha256(image).hexdigest()
    if actual_md5 != MEDIA_MD5 or actual_sha256 != MEDIA_SHA256:
        raise AssertionError(
            "US Track 02 identity mismatch: "
            f"MD5/SHA-256 {actual_md5}/{actual_sha256}"
        )
    if MEDIA_SIZE % RAW_SECTOR_BYTES:
        raise AssertionError("US Track 02 is not a complete MODE1/2352 stream")

    source = SOURCE_PATH.read_text(encoding="utf-8")
    user_data = b"".join(
        image[offset + USER_DATA_OFFSET : offset + USER_DATA_OFFSET + USER_DATA_BYTES]
        for offset in range(0, MEDIA_SIZE, RAW_SECTOR_BYTES)
    )

    names = c_table_entries(source, "g_dungeon_names")
    if len(names) != len(DUNGEON_NAMES):
        raise AssertionError(f"expected seven dungeon names, got {len(names)}")
    for (offset, value), (expected, expected_offset, observed_suffix) in zip(
        names, DUNGEON_NAMES
    ):
        if value != expected.encode("ascii") or offset != expected_offset:
            raise AssertionError(f"dungeon-name metadata changed for {expected}")
        record = user_data[offset : offset + 8]
        expected_record = value.ljust(7, b" ") + observed_suffix
        if record != expected_record:
            raise AssertionError(
                f"{expected}: 8-byte source record differs at {offset:#x}"
            )
        print(f"PASS: authentic US dungeon-name record {expected} at {offset:#x}")

    treasures = c_table_entries(source, "g_treasure_names")
    if len(treasures) != len(TREASURE_NAMES):
        raise AssertionError(f"expected seven treasure names, got {len(treasures)}")
    for (offset, value), (expected, expected_offset) in zip(treasures, TREASURE_NAMES):
        if value != expected.encode("ascii") or offset != expected_offset:
            raise AssertionError(f"treasure-name metadata changed for {expected}")
        if user_data[offset : offset + len(value)] != value:
            raise AssertionError(
                f"{expected}: compiled label differs from Track 02 at {offset:#x}"
            )
        print(f"PASS: authentic US treasure label {expected} at {offset:#x}")

    retrieval = c_table_entries(source, "g_retrieval")
    if len(retrieval) != len(RETRIEVAL_OFFSETS):
        raise AssertionError(f"expected seven retrieval messages, got {len(retrieval)}")
    for index, ((offset, value), expected_offset) in enumerate(
        zip(retrieval, RETRIEVAL_OFFSETS)
    ):
        if offset != expected_offset:
            raise AssertionError(f"retrieval message {index}: source offset changed")
        if user_data[offset : offset + len(value)] != value:
            raise AssertionError(
                f"retrieval message {index}: compiled bytes differ from Track 02"
            )
        print(
            f"PASS: authentic US retrieval message {index} at {offset:#x} "
            f"({len(value)} bytes)"
        )

    label_match = re.search(
        r'return\s+"([^"]+)";\s*/\*\s*UD 0x([0-9a-fA-F]+)\s*\*/',
        source,
    )
    if label_match is None:
        raise AssertionError("could not find the source-annotated game-speed label")
    label = label_match.group(1).encode("ascii")
    label_offset = int(label_match.group(2), 16)
    if (label, label_offset) != GAME_SPEED_LABEL:
        raise AssertionError("game-speed label source metadata changed")
    if user_data[label_offset : label_offset + len(label)] != label:
        raise AssertionError("game-speed label differs from authentic Track 02")
    print(f"PASS: authentic US game-speed label at {label_offset:#x}")

    print(
        "LIMIT: byte provenance and observed record layout only; text-control "
        "semantics and runtime presentation remain unproven"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
