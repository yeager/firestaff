#!/usr/bin/env python3
"""Verify compiled US dungeon stories against authenticated Track 02 bytes."""

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
EXPECTED_STORIES = (
    (0, "AKUTUBA", 0x27613E, 388),
    (1, "DRATOR", 0x2762C4, 364),
    (2, "FORMIC", 0x276432, 561),
    (3, "SARMON", 0x276665, 379),
    (4, "SHADO", 0x2767E2, 372),
    (5, "THIEF", 0x276958, 489),
    (6, "DEMON", 0x276B43, 392),
)


def read_story_table(source: str) -> tuple[tuple[int, str, int, int, bytes], ...]:
    table = re.search(
        r"static const char \*const g_stories\[[^\]]+\] = \{(.*?)\n\};",
        source,
        re.DOTALL,
    )
    if table is None:
        raise AssertionError("could not find the compiled g_stories table")

    body = table.group(1)
    entries = tuple(
        re.finditer(
            r"/\*\s*(\d+):\s*([A-Z]+).*?UD 0x([0-9a-fA-F]+),\s*(\d+) bytes\s*\*/",
            body,
        )
    )
    if len(entries) != len(EXPECTED_STORIES):
        raise AssertionError(f"expected seven story source comments, got {len(entries)}")

    stories = []
    for index, entry in enumerate(entries):
        expected_index, expected_name, expected_offset, expected_length = (
            EXPECTED_STORIES[index]
        )
        values = (
            int(entry.group(1)),
            entry.group(2),
            int(entry.group(3), 16),
            int(entry.group(4)),
        )
        if values != (
            expected_index,
            expected_name,
            expected_offset,
            expected_length,
        ):
            raise AssertionError(
                f"story {index}: source metadata changed from the reviewed table"
            )

        next_start = entries[index + 1].start() if index + 1 < len(entries) else len(body)
        literals = re.findall(
            r'"(?:[^"\\]|\\.)*"', body[entry.end() : next_start]
        )
        if not literals:
            raise AssertionError(f"story {expected_name}: no C string literals found")
        try:
            encoded = "".join(ast.literal_eval(literal) for literal in literals).encode(
                "ascii"
            )
        except (SyntaxError, UnicodeError, ValueError) as error:
            raise AssertionError(
                f"story {expected_name}: invalid ASCII C string literal"
            ) from error
        stories.append((*values, encoded))
    return tuple(stories)


def find_all(data: bytes, needle: bytes) -> tuple[int, ...]:
    offsets = []
    cursor = 0
    while True:
        cursor = data.find(needle, cursor)
        if cursor < 0:
            return tuple(offsets)
        offsets.append(cursor)
        cursor += 1


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
    stories = read_story_table(source)
    user_data = b"".join(
        image[offset + USER_DATA_OFFSET : offset + USER_DATA_OFFSET + USER_DATA_BYTES]
        for offset in range(0, MEDIA_SIZE, RAW_SECTOR_BYTES)
    )
    for index, name, source_offset, source_length, compiled_bytes in stories:
        if len(compiled_bytes) != source_length:
            raise AssertionError(
                f"{name}: compiled byte length {len(compiled_bytes)} "
                f"does not match source comment {source_length}"
            )
        if source_offset + source_length > len(user_data):
            raise AssertionError(f"{name}: source span is outside Track 02 user data")
        authentic_bytes = user_data[source_offset : source_offset + source_length]
        if compiled_bytes != authentic_bytes:
            raise AssertionError(
                f"{name}: compiled story differs from authentic bytes at "
                f"user-data offset {source_offset:#x}"
            )
        if find_all(user_data, authentic_bytes) != (source_offset,):
            raise AssertionError(f"{name}: authentic story span is not unique")
        print(
            f"PASS: authentic US {name} story matches {source_length} bytes "
            f"at user-data offset {source_offset:#x} "
            f"(sha256 {hashlib.sha256(authentic_bytes).hexdigest()})"
        )

    print(
        "LIMIT: source bytes and offsets only; game selector, caller, and "
        "runtime presentation remain unproven"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
