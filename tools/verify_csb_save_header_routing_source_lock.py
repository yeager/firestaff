#!/usr/bin/env python3
"""Verify the narrow CSB save-header/routing source-lock gate.

This gate is source-lineage evidence only: it locks the ReDMCSB save-header
identity, dungeon-id criteria, and save-file routing that separates DM saves
from CSB game/prison saves. It intentionally does not claim save compatibility
or runtime parity, and it does not require local CSBGAME.DAT sample saves.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_JSON_OUT = Path("parity-evidence/verification/csb_save_header_routing_source_lock.json")


@dataclass(frozen=True)
class SourceCheck:
    id: str
    file: str
    line_range: tuple[int, int]
    function: str
    needles: tuple[str, ...]
    claim: str


SOURCE_CHECKS = (
    SourceCheck(
        id="save-header-format-identity",
        file="DEFS.H",
        line_range=(468, 498),
        function="DM_SAVE_HEADER / CSB_SAVE_HEADER definitions",
        needles=(
            "} DM_SAVE_HEADER;",
            "} CSB_SAVE_HEADER;",
            "unsigned int16_t DungeonID;",
            "#define C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER     0x01",
            "#define C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK 0x02",
        ),
        claim="DM and CSB save headers are distinct source structures with explicit format IDs.",
    ),
    SourceCheck(
        id="dungeon-id-namespace",
        file="DEFS.H",
        line_range=(519, 523),
        function="Dungeon ID constants",
        needles=(
            "#define C10_DUNGEON_DM         10",
            "#define C12_DUNGEON_CSB_PRISON 12",
            "#define C13_DUNGEON_CSB_GAME   13",
        ),
        claim="DM, CSB prison, and CSB game dungeons have separate source constants.",
    ),
    SourceCheck(
        id="save-file-id-routing-namespace",
        file="DEFS.H",
        line_range=(5001, 5019),
        function="file IDs for load/save members",
        needles=(
            "#define M741_FILE_ID_LOAD_DMSAVE_DAT",
            "#define M742_FILE_ID_LOAD_CSBGAME_DAT",
            "#define M745_FILE_ID_SAVE_DMSAVE_DAT",
            "#define M746_FILE_ID_SAVE_CSBGAME_DAT",
        ),
        claim="DM and CSB save-file IDs are separate in the source namespace.",
    ),
    SourceCheck(
        id="is-dungeon-valid-csb-save-header-read",
        file="CEDTINCU.C",
        line_range=(18, 33),
        function="F7272_IsDungeonValid",
        needles=(
            "L4102_ui_GameType = P4043_ps_->SaveHeaderFormat;",
            "case C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER:",
            "case C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK:",
            "L4108_ps_ = (CSB_SAVE_HEADER*)&P4043_ps_->SaveHeader;",
            "L4109_i_ = L4108_ps_->DungeonID;",
        ),
        claim="Dungeon validation reads DungeonID through the CSB header layout when the save format is CSB.",
    ),
    SourceCheck(
        id="is-dungeon-valid-criteria",
        file="CEDTINCU.C",
        line_range=(36, 72),
        function="F7272_IsDungeonValid",
        needles=(
            "case 2:",
            "(L4109_i_ == C13_DUNGEON_CSB_GAME)",
            "case 3:",
            "((L4109_i_ == C12_DUNGEON_CSB_PRISON) || (L4109_i_ == C10_DUNGEON_DM))",
        ),
        claim="Criteria 2 accepts CSB game only; criteria 3 accepts CSB prison or DM, not CSB game.",
    ),
    SourceCheck(
        id="new-adventure-source-gate",
        file="CEDTINCH.C",
        line_range=(38, 45),
        function="F7086_IsReadyToMakeNewAdventure",
        needles=(
            "if (F7272_IsDungeonValid(&G7111_Games[C0_GAME_SOURCE], 3))",
            "return F7076_AreAllChampionNamesUnique();",
            "F7074_DrawDialogErrorMessage",
        ),
        claim="New Adventure creation is gated through F7272 criteria 3.",
    ),
    SourceCheck(
        id="su1e-csb-game-only-gate",
        file="CEDTINCH.C",
        line_range=(49, 64),
        function="F1996_",
        needles=(
            "P5909_->SaveHeaderFormat == C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK",
            "L5768_->DungeonID == C13_DUNGEON_CSB_GAME",
            "return C1_TRUE;",
        ),
        claim="The SU1E-specific gate accepts only CSB-format saves whose DungeonID is CSB game.",
    ),
    SourceCheck(
        id="save-routing-by-dungeon-id",
        file="CEDTINC8.C",
        line_range=(101, 118),
        function="save-file selection block",
        needles=(
            "L3940_i_ = M746_FILE_ID_SAVE_CSBGAME_DAT;",
            "if (L3938_ps_Game->SaveHeaderFormat == C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER)",
            "L3949_i_DungeonID = L3943_ps_->DungeonID;",
            "if (L3949_i_DungeonID == C10_DUNGEON_DM)",
            "L3940_i_ = M745_FILE_ID_SAVE_DMSAVE_DAT;",
            "(L3949_i_DungeonID == C13_DUNGEON_CSB_GAME) || (L3949_i_DungeonID == C12_DUNGEON_CSB_PRISON)",
        ),
        claim="Save output routes DM to DMSAVE.DAT and CSB game/prison to CSBGAME.DAT by DungeonID.",
    ),
    SourceCheck(
        id="load-game-save-header-dungeon-recovery",
        file="LOADSAVE.C",
        line_range=(2873, 2920),
        function="F0435_STARTEND_LoadGame saved-game block",
        needles=(
            "G0526_ui_DungeonID = L1372_s_SaveHeader.DungeonID;",
            "Broken detection of the original dungeon of Chaos Strikes Back because the actual CSB dungeon has 11 maps and its seed is 13",
            "F0428_DIALOG_RequireGameDiskInDrive_NoDialogDrawn(C1_DO_NOT_FORCE_DIALOG_CSB",
        ),
        claim="Saved-game load recovers DungeonID from the save header and documents the old fallback mismatch for actual CSB.",
    ),
)


def excerpt(path: Path, line_range: tuple[int, int]) -> str:
    start, end = line_range
    return "\n".join(path.read_text(errors="replace").splitlines()[start - 1 : end])


def run_checks(redmcsb_source: Path) -> dict[str, object]:
    failures: list[str] = []
    rows: list[dict[str, object]] = []
    for check in SOURCE_CHECKS:
        path = redmcsb_source / check.file
        if not path.exists():
            failures.append(f"missing source file: {path}")
            rows.append({"id": check.id, "ok": False, "file": check.file})
            continue
        source_excerpt = excerpt(path, check.line_range)
        missing = [needle for needle in check.needles if needle not in source_excerpt]
        ok = not missing
        if not ok:
            failures.append(f"source check {check.id} missing needles: {missing!r}")
        rows.append({
            "id": check.id,
            "ok": ok,
            "file": check.file,
            "function": check.function,
            "lines": f"{check.line_range[0]}-{check.line_range[1]}",
            "claim": check.claim,
            "missing": missing,
        })
    return {
        "schema": "firestaff.csb_save_header_routing_source_lock.v1",
        "pass": not failures,
        "scope": "CSB save-header identity and DM-vs-CSB save routing source-lineage only.",
        "redmcsb_source": str(redmcsb_source),
        "source_checks": rows,
        "non_claims": [
            "No Firestaff runtime code was changed or validated by this gate.",
            "No CSBGAME.DAT sample save is required or claimed present on N2.",
            "No renderer, gameplay, or binary save compatibility parity is claimed.",
            "No DANNESBURK, external capture, or non-N2 reference is used.",
        ],
        "failures": failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--json-out", type=Path, default=DEFAULT_JSON_OUT)
    args = parser.parse_args()

    result = run_checks(args.redmcsb_source)
    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(result, indent=2) + "\n")
    print(
        f"{'PASS' if result['pass'] else 'FAIL'} csb save-header routing source lock: "
        f"{len(result['source_checks'])} source checks"
    )
    if result["failures"]:
        for failure in result["failures"]:
            print(f"- {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
