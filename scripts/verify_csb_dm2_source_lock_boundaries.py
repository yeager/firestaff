#!/usr/bin/env python3
"""Verify narrow CSB/DM2 source-lock boundary evidence without building Firestaff."""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_ORIGINAL_DM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
DEFAULT_REPO = Path(__file__).resolve().parents[1]

CHECKS = [
    {
        "id": "redmcsb-save-header-formats",
        "file": "DEFS.H",
        "range": "468-498",
        "needles": [
            "typedef struct {",
            "unsigned int16_t Noise[149];",
            "unsigned int16_t Noise[150];",
            "} DM_SAVE_HEADER;",
            "} CSB_SAVE_HEADER;",
            "#define C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER     0x01",
            "#define C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK 0x02",
        ],
        "why": "DM and CSB save headers are structurally separate; do not infer one from the other.",
    },
    {
        "id": "redmcsb-dungeon-id-namespace",
        "file": "DEFS.H",
        "range": "519-523",
        "needles": [
            "#define C10_DUNGEON_DM         10",
            "#define C12_DUNGEON_CSB_PRISON 12",
            "#define C13_DUNGEON_CSB_GAME   13",
        ],
        "why": "CSB/prison are source IDs 13/12, distinct from DM source ID 10.",
    },
    {
        "id": "redmcsb-csb-validation-criteria",
        "file": "CEDTINCU.C",
        "range": "5-77",
        "needles": [
            "BOOLEAN F7272_IsDungeonValid",
            "case C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER:",
            "case C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK:",
            "case 1:",
            "((L4109_i_ == C13_DUNGEON_CSB_GAME) || (L4109_i_ == C12_DUNGEON_CSB_PRISON) || (L4109_i_ == C10_DUNGEON_DM))",
            "case 2:",
            "(L4109_i_ == C13_DUNGEON_CSB_GAME)",
            "case 3:",
            "((L4109_i_ == C12_DUNGEON_CSB_PRISON) || (L4109_i_ == C10_DUNGEON_DM))",
        ],
        "why": "CSB utility validation has explicit criteria; criteria 2 is CSB-game only, criteria 3 excludes CSB-game.",
    },
    {
        "id": "redmcsb-new-adventure-boundary",
        "file": "CEDTINCH.C",
        "range": "5-47",
        "needles": [
            "BOOLEAN F7086_IsReadyToMakeNewAdventure",
            "if (!G7111_Games[C0_GAME_SOURCE].GameLoaded || !G7114_LoadedChampionCount)",
            "F7272_IsDungeonValid(&G7111_Games[C0_GAME_SOURCE], 3)",
            "return F7076_AreAllChampionNamesUnique();",
        ],
        "why": "New Adventure is source-gated through validation criteria 3, not a generic CSB acceptance path.",
    },
    {
        "id": "redmcsb-su1e-csb-game-only-boundary",
        "file": "CEDTINCH.C",
        "range": "49-64",
        "needles": [
            "BOOLEAN F1996_(GAME* P5909_)",
            "P5909_->SaveHeaderFormat == C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK",
            "L5768_->DungeonID == C13_DUNGEON_CSB_GAME",
            "return C1_TRUE;",
        ],
        "why": "The SU1E-specific gate accepts only CSB-format headers for the CSB game dungeon ID.",
    },
    {
        "id": "redmcsb-save-file-routing-boundary",
        "file": "CEDTINC8.C",
        "range": "101-118",
        "needles": [
            "if (L3949_i_DungeonID == C10_DUNGEON_DM)",
            "L3940_i_ = M745_FILE_ID_SAVE_DMSAVE_DAT;",
            "if ((L3949_i_DungeonID == C13_DUNGEON_CSB_GAME) || (L3949_i_DungeonID == C12_DUNGEON_CSB_PRISON))",
            "L3940_i_ = M746_FILE_ID_SAVE_CSBGAME_DAT;",
        ],
        "why": "Save routing source-lock keeps DM saves and CSB saves on separate file IDs.",
    },
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(x) for x in line_range.split("-")]
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def check_source(source: Path) -> tuple[list[str], list[str]]:
    passed: list[str] = []
    failed: list[str] = []
    for check in CHECKS:
        path = source / check["file"]
        if not path.exists():
            failed.append(f"FAIL {check['id']}: missing {path}")
            continue
        text = path.read_text(errors="replace")
        haystack = line_slice(text, check["range"])
        missing = [needle for needle in check["needles"] if needle not in haystack]
        if missing:
            failed.append(
                f"FAIL {check['id']}: {check['file']}:{check['range']} missing "
                + "; ".join(repr(m) for m in missing)
            )
        else:
            passed.append(f"PASS {check['id']}: {check['file']}:{check['range']} - {check['why']}")
    return passed, failed


def inventory_refs(original_dm: Path) -> list[str]:
    rows: list[str] = []
    candidates = [
        "Game,Chaos_Strikes_Back,Amiga,Software.7z",
        "Game,Chaos_Strikes_Back,Atari_ST,Software.7z",
        "Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z",
        "_extracted/dm2-dos-asm/SKULL.ASM",
    ]
    for rel in candidates:
        path = original_dm / rel
        if path.exists():
            rows.append(f"FOUND {rel} bytes={path.stat().st_size} sha256={sha256(path)}")
        else:
            rows.append(f"MISSING {rel}")
    return rows


def repo_boundary_scan(repo: Path) -> list[str]:
    matrix = repo / "PARITY_MATRIX_DM1_V1.md"
    config = repo / "config_m12.h"
    rows: list[str] = []
    if matrix.exists():
        text = matrix.read_text(errors="replace")
        w10 = re.search(r"## 10\. CSB and DM2 readiness.*?(?=\n---\n)", text, re.S)
        if w10:
            for line in w10.group(0).splitlines():
                if "CSB" in line or "DM2" in line or "DM1 assumptions" in line:
                    rows.append(f"MATRIX {line}")
    if config.exists():
        for i, line in enumerate(config.read_text(errors="replace").splitlines(), start=1):
            if "M12_CONFIG_GAME_COUNT" in line:
                rows.append(f"CONFIG config_m12.h:{i}: {line.strip()}")
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--original-dm", type=Path, default=DEFAULT_ORIGINAL_DM)
    parser.add_argument("--repo", type=Path, default=DEFAULT_REPO)
    args = parser.parse_args()

    print("CSB save-header / dungeon-ID / save-routing source-lock boundary guard")
    print(f"redmcsb_source={args.redmcsb_source}")
    print(f"original_dm={args.original_dm}")
    print(f"repo={args.repo}")
    passed, failed = check_source(args.redmcsb_source)
    print("\n[source checks]")
    for row in passed + failed:
        print(row)
    print("\n[reference inventory]")
    for row in inventory_refs(args.original_dm):
        print(row)
    print("\n[repo boundary scan]")
    for row in repo_boundary_scan(args.repo):
        print(row)
    if failed:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
