#!/usr/bin/env python3
"""Verify CSB startup/dungeon-load source-lock boundaries from ReDMCSB.

This is an evidence gate only. It source-locks the sequence and original CSB
Atari dungeon member identity; it does not claim Firestaff runtime parity.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_ORIGINAL_DM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
DEFAULT_JSON_OUT = Path("parity-evidence/verification/csb_startup_dungeon_source_lock.json")


@dataclass(frozen=True)
class SourceCheck:
    id: str
    file: str
    function: str
    lines: tuple[int, int]
    needles: tuple[str, ...]
    note: str


SOURCE_CHECKS = (
    SourceCheck(
        id="csb-start-initialize-sequence",
        file="STARTUP2.C",
        function="F0463_START_InitializeGame_CPSADEF",
        lines=(1303, 1410),
        needles=(
            "void F0463_START_InitializeGame_CPSADEF(",
            "F0448_STARTUP1_InitializeMemoryManager_CPSADEF();",
            "F0479_MEMORY_ReadGraphicsDatHeader();",
            "F0460_START_InitializeGraphicData();",
            "F0094_DUNGEONVIEW_LoadFloorSet(C0_FLOOR_SET_STONE);",
            "F0095_DUNGEONVIEW_LoadWallSet(C0_WALL_SET_STONE);",
            "F0054_TEXT_Initialize();",
        ),
        note="CSB startup initializes memory/graphics/title/floor/wall/text before load-game handoff.",
    ),
    SourceCheck(
        id="load-dungeon-compressed-header-gate",
        file="LOADSAVE.C",
        function="F0434_STARTEND_IsLoadDungeonSuccessful_CPSC",
        lines=(1803, 1944),
        needles=(
            "STATICFUNCTION BOOLEAN F0434_STARTEND_IsLoadDungeonSuccessful_CPSC(",
            "COMPRESSED_DUNGEON_HEADER L1362_s_CompressedDungeonHeader;",
            "G0530_B_LoadingCompressedDungeon = C0_FALSE;",
            "L1362_s_CompressedDungeonHeader.Signature != C0x8104_SIGNATURE_COMPRESSED_DUNGEON",
            "G0526_ui_DungeonID = L1362_s_CompressedDungeonHeader.DungeonID;",
            "G0530_B_LoadingCompressedDungeon = C1_TRUE;",
            "F0455_FLOPPY_DecompressDungeon(",
            "G0278_ps_DungeonHeader = (DUNGEON_HEADER*)M533_F0468_MEMORY_Allocate",
            "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful",
            "G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;",
        ),
        note="Dungeon load distinguishes compressed vs direct files, records compressed DungeonID, reads header with checksum, then seeds initial party location.",
    ),
    SourceCheck(
        id="new-game-dungeon-file-open-boundary",
        file="LOADSAVE.C",
        function="F0435_STARTEND_LoadGame / new-game dungeon open block",
        lines=(2291, 2393),
        needles=(
            "G0526_ui_DungeonID = C10_DUNGEON_DM;",
            "if (G0298_B_NewGame) {",
            "F0428_DIALOG_RequireGameDiskInDrive_NoDialogDrawn(C0_DO_NOT_FORCE_DIALOG_DM_CSB",
            "G0521_i_GameFileHandle = Fopen(\"\\\\DUNGEON.DAT\", C0_READ_ONLY);",
            "G0521_i_GameFileHandle = F0770_FILE_Open(\"\\\\DUNGEON.DAT\");",
            "G0521_i_GameFileHandle = F0770_FILE_Open(\"*:DUNGEON.GAME\");",
            "F0019_MAIN_DisplayErrorAndStop(C50_ERROR_UNABLE_TO_OPEN_DUNGEON_OR_SAVED_GAME);",
        ),
        note="New-game dungeon loading starts from DM default ID, requires the game disk, opens platform-specific dungeon files, and hard-stops on open failure.",
    ),
    SourceCheck(
        id="save-load-csb-dungeon-id-boundary",
        file="LOADSAVE.C",
        function="F0435_STARTEND_LoadGame / saved-game dungeon-id recovery",
        lines=(2826, 2920),
        needles=(
            "if (!F0434_STARTEND_IsLoadDungeonSuccessful_CPSC())",
            "if (L1372_s_SaveHeader.FormatID == C1_FORMAT_DM_ATARI_ST)",
            "if ((AL1363_ui_MapCount == 14) && (G0278_ps_DungeonHeader->OrnamentRandomSeed == 99))",
            "if ((AL1363_ui_MapCount == 10) && (G0278_ps_DungeonHeader->OrnamentRandomSeed == 76))",
            "Broken detection of the original dungeon of Chaos Strikes Back because the actual CSB dungeon has 11 maps and its seed is 13",
            "G0526_ui_DungeonID = L1372_s_SaveHeader.DungeonID;",
            "F0428_DIALOG_RequireGameDiskInDrive_NoDialogDrawn(C1_DO_NOT_FORCE_DIALOG_CSB",
        ),
        note="Saved-game path calls the same dungeon loader; CSB ID comes from the save header, while the old DM-format fallback explicitly does not match actual CSB.",
    ),
    SourceCheck(
        id="dungeon-id-namespace",
        file="DEFS.H",
        function="Dungeon IDs",
        lines=(519, 523),
        needles=(
            "#define C00_DUNGEON_UNKNOWN     0",
            "#define C10_DUNGEON_DM         10",
            "#define C12_DUNGEON_CSB_PRISON 12",
            "#define C13_DUNGEON_CSB_GAME   13",
        ),
        note="DM, CSB prison, and CSB game dungeon IDs are separate source constants.",
    ),
)

ORIGINAL_MEMBER = {
    "id": "canonical-csb-atari-dungeon-dat",
    "relpath": "_canonical/csb/atari-DUNGEON.DAT",
    "size": 2098,
    "sha256": "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba",
    "note": "Canonical Atari ST hard-disk extracted CSB DUNGEON.DAT anchor for the current CSB source-lock lane.",
}


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def excerpt(path: Path, lines: tuple[int, int]) -> str:
    start, end = lines
    return "\n".join(path.read_text(errors="replace").splitlines()[start - 1 : end])


def run_checks(redmcsb_source: Path, original_dm: Path) -> dict[str, object]:
    failures: list[str] = []
    source_rows: list[dict[str, object]] = []
    for check in SOURCE_CHECKS:
        path = redmcsb_source / check.file
        if not path.exists():
            failures.append(f"missing source file: {path}")
            source_rows.append({"id": check.id, "ok": False, "file": check.file})
            continue
        haystack = excerpt(path, check.lines)
        missing = [needle for needle in check.needles if needle not in haystack]
        ok = not missing
        if not ok:
            failures.append(f"source check {check.id} missing needles: {missing!r}")
        source_rows.append(
            {
                "id": check.id,
                "ok": ok,
                "file": check.file,
                "function": check.function,
                "lines": f"{check.lines[0]}-{check.lines[1]}",
                "note": check.note,
                "missing": missing,
            }
        )

    path = original_dm / ORIGINAL_MEMBER["relpath"]
    exists = path.exists()
    actual_size = path.stat().st_size if exists else None
    actual_sha = sha256_file(path) if exists else None
    member_ok = (
        exists
        and actual_size == ORIGINAL_MEMBER["size"]
        and actual_sha == ORIGINAL_MEMBER["sha256"]
    )
    if not member_ok:
        failures.append(
            f"member check {ORIGINAL_MEMBER['id']} failed: "
            f"exists={exists} size={actual_size} sha256={actual_sha}"
        )

    return {
        "schema": "firestaff.csb_startup_dungeon_source_lock.v1",
        "pass": not failures,
        "scope": "CSB startup/dungeon-load source sequence and canonical Atari DUNGEON.DAT identity only; no runtime/render/save compatibility claim.",
        "redmcsb_source": str(redmcsb_source),
        "original_dm_root": str(original_dm),
        "source_checks": source_rows,
        "member_checks": [
            {
                **ORIGINAL_MEMBER,
                "ok": member_ok,
                "size_actual": actual_size,
                "sha256_actual": actual_sha,
            }
        ],
        "non_claims": [
            "Does not execute Firestaff or original CSB.",
            "Does not prove CSB rendering, gameplay, or savegame parity.",
            "Does not use any non-N2 source path.",
        ],
        "failures": failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--original-dm", type=Path, default=DEFAULT_ORIGINAL_DM)
    parser.add_argument("--json-out", type=Path, default=DEFAULT_JSON_OUT)
    args = parser.parse_args()

    result = run_checks(args.redmcsb_source, args.original_dm)
    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(result, indent=2) + "\n")
    print(
        f"{'PASS' if result['pass'] else 'FAIL'} csb startup/dungeon source lock: "
        f"{len(result['source_checks'])} source checks, {len(result['member_checks'])} member checks"
    )
    if result["failures"]:
        for failure in result["failures"]:
            print(f"- {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
