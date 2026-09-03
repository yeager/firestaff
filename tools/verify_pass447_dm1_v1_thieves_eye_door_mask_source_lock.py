#!/usr/bin/env python3
"""Pass447 DM1 V1 thieves-eye door-mask source lock.

Primary evidence is ReDMCSB PC34/I34E: STARTUP2 maps the two special door
ornament masks C15/C16 to M649_GRAPHIC_DOOR_MASK_DESTROYED + {0,1}, and
DUNVIEW F0111 applies C16 only to the D1C door when Thieves Eye is active.

The gate hash-locks local GRAPHICS.DAT variants before asserting that PC34
entry 440 is the thieves-eye mask next to destroyed-door mask 439.
"""
from __future__ import annotations

from pathlib import Path
import hashlib
import re
import subprocess
import sys
import zipfile
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
RED_DEFS = RED_ROOT / "DEFS.H"
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"
RED_STARTUP2 = RED_ROOT / "STARTUP2.C"

GRAPHICS_VARIANTS = [
    (
        "DM PC 3.4 English GRAPHICS.DAT",
        Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip",
        "DATA/GRAPHICS.DAT",
        "FA6B1AA29E191418713BF2CDA93D962E",
        "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        713,
    ),
]


def read_archive_member(archive: Path, member: str) -> bytes:
    if not archive.is_file():
        print(f"SKIP pass447: licensed DM1 archive unavailable: {archive}")
        raise SystemExit(77)
    try:
        with zipfile.ZipFile(archive) as zf:
            return zf.read(member)
    except (KeyError, zipfile.BadZipFile):
        # Some original DOS archives have a DOS backslash in the local
        # header and a slash in the central directory.  Python correctly
        # refuses that inconsistent archive; 7-Zip can stream the member
        # without extracting it, which preserves the no-on-disk-data rule.
        proc = subprocess.run(
            ["7z", "x", "-so", str(archive), member],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        if proc.returncode == 0 and proc.stdout:
            return proc.stdout
        raise AssertionError(f"{archive.name}: cannot read {member}")


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def enum_value(text: str, name: str) -> str:
    m = re.search(r"\b" + re.escape(name) + r"\s*=\s*([^,\n]+)", text)
    if not m:
        raise AssertionError(f"missing enum value {name}")
    return m.group(1).strip()


def find_function(text: str, name: str) -> tuple[int, str]:
    pat = re.compile(r"\b(?:static\s+)?(?:int|unsigned\s+int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pat.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0 or text.find(";", m.end(), brace) >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"missing function {name}")


def le16(data: bytes, off: int) -> int:
    return data[off] | (data[off + 1] << 8)


def entry_info(data: bytes, index: int) -> tuple[int, int, int, int]:
    count = le16(data, 2)
    comp = le16(data, 4 + 2 * index)
    dec = le16(data, 4 + 2 * count + 2 * index)
    width = le16(data, 4 + 4 * count + 4 * index)
    height = le16(data, 4 + 4 * count + 4 * index + 2)
    return comp, dec, width, height


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    defs = RED_DEFS.read_text(encoding="latin-1")
    dunview = RED_DUNVIEW.read_text(encoding="latin-1")
    startup2 = RED_STARTUP2.read_text(encoding="latin-1")

    for needle in [
        "#define M649_GRAPHIC_DOOR_MASK_DESTROYED              439",
        "#define C15_DOOR_ORNAMENT_DESTROYED_MASK   15",
        "#define C16_DOOR_ORNAMENT_THIEVES_EYE_MASK 16",
    ]:
        require(defs, needle, "ReDMCSB special door-mask constants")

    startup_anchor = require(
        startup2,
        "AL1466_i_DoorOrnamentIndex + (M649_GRAPHIC_DOOR_MASK_DESTROYED - C15_DOOR_ORNAMENT_DESTROYED_MASK)",
        "ReDMCSB STARTUP2 C15/C16 mask native-bitmap mapping",
    )
    require(startup2, "G0103_as_CurrentMapDoorOrnamentsInfo[AL1466_i_DoorOrnamentIndex].CoordinateSet = 1;", "ReDMCSB C15/C16 coord-set mapping")
    dunview_anchor = require(
        dunview,
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK), C2_VIEW_DOOR_ORNAMENT_D1LCR);",
        "ReDMCSB F0111 thieves-eye D1C draw",
    )

    for label, archive, member, expected_md5, expected_sha256, expected_count in GRAPHICS_VARIANTS:
        data = read_archive_member(archive, member)
        md5 = hashlib.md5(data).hexdigest().upper()
        sha256 = hashlib.sha256(data).hexdigest()
        if md5 != expected_md5:
            raise AssertionError(f"{label}: MD5 drift {md5} != {expected_md5}")
        if sha256 != expected_sha256:
            raise AssertionError(f"{label}: SHA256 drift {sha256} != {expected_sha256}")
        if le16(data, 2) != expected_count:
            raise AssertionError(f"{label}: item count drift")
        if entry_info(data, 439) != (343, 343, 96, 88):
            raise AssertionError(f"{label}: destroyed-door mask entry 439 drifted")
        if entry_info(data, 440) != (258, 258, 80, 74):
            raise AssertionError(f"{label}: thieves-eye mask entry 440 drifted")
        if entry_info(data, 441)[2:] != (30, 19):
            raise AssertionError(f"{label}: first normal door ornament entry 441 drifted")

    if enum_value(fire, "M11_GFX_DOOR_MASK_DESTROYED") != "439":
        raise AssertionError("Firestaff destroyed-door mask must stay on GRAPHICS.DAT 439")
    if enum_value(fire, "M11_GFX_DOOR_MASK_THIEVES_EYE") != "440":
        raise AssertionError("Firestaff thieves-eye mask must map to GRAPHICS.DAT 440")
    func_start, func = find_function(fire, "m11_draw_dm1_center_thieves_eye_mask")
    for needle in [
        "state->world.lifecycle.status.thievesEyeCount == 0",
        "cells[0][1].elementType != DUNGEON_ELEMENT_DOOR",
        "M11_GFX_DOOR_MASK_THIEVES_EYE",
        "M11_AssetLoader_BlitScaled",
    ]:
        require(func, needle, "Firestaff D1C thieves-eye mask renderer")
    render_match = re.search(
        r"m11_draw_dm1_center_thieves_eye_mask\s*\(\s*state\s*,\s*"
        r"framebuffer\s*,\s*framebufferWidth\s*,\s*framebufferHeight\s*,\s*"
        r"cells\s*\)\s*;",
        fire,
    )
    if not render_match:
        raise AssertionError("Firestaff viewport pass order: missing D1C thieves-eye mask call")
    require(cmake, "NAME pass447_dm1_v1_thieves_eye_door_mask_source_lock", "CMake gate registration")

    print("PASS pass447 DM1 V1 thieves-eye door-mask source lock")
    print(f"- ReDMCSB STARTUP2 C15/C16 mapping: {RED_STARTUP2.name}:{line_no(startup2, startup_anchor)}")
    print(f"- ReDMCSB DUNVIEW F0111 C16 D1C draw: {RED_DUNVIEW.name}:{line_no(dunview, dunview_anchor)}")
    for label, archive, member, _md5, sha256, _count in GRAPHICS_VARIANTS:
        print(f"- {label}: {archive.name}!{member}; sha256={sha256}; entry 439=96x88 destroyed mask; entry 440=80x74 thieves-eye mask")
    print(f"- Firestaff maps C16 to GRAPHICS.DAT 440: {FIRE.name}:{line_no(fire, func_start)}")
    print(f"- Firestaff viewport order calls thieves-eye mask pass: {FIRE.name}:{line_no(fire, render_match.start())}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)
