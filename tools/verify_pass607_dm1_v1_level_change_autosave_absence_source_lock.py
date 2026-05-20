#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CANON = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
PASS = "pass607_dm1_v1_level_change_autosave_absence_source_lock"
STATUS = "PASS607_DM1_V1_LEVEL_CHANGE_AUTOSAVE_ABSENCE_SOURCE_LOCKED"
OUT = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
DIRECT_SAVE_CALL = "F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF();"


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing {path}")
    return path.read_text(encoding=encoding, errors="replace")


def git(*args: str) -> str:
    proc = subprocess.run(["git", *args], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60, check=False)
    if proc.returncode:
        raise AssertionError(proc.stdout[-2000:])
    return proc.stdout.strip()


def line_at(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def find_line(text: str, needle: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {needle!r}")
    return line_at(text, pos)


def slice_between(text: str, start_needle: str, end_needle: str, label: str) -> tuple[int, int, str]:
    start = text.find(start_needle)
    if start < 0:
        raise AssertionError(f"{label}: missing start {start_needle!r}")
    end = text.find(end_needle, start + len(start_needle))
    if end < 0:
        raise AssertionError(f"{label}: missing end {end_needle!r}")
    return line_at(text, start), line_at(text, max(start, end - 1)), text[start:end]


def slice_to_end(text: str, start_needle: str, label: str) -> tuple[int, int, str]:
    start = text.find(start_needle)
    if start < 0:
        raise AssertionError(f"{label}: missing start {start_needle!r}")
    return line_at(text, start), text.count("\n") + 1, text[start:]


def ordered_span(file_text: str, body: str, base_line: int, needles: list[str], label: str) -> str:
    cur = 0
    positions: list[int] = []
    for needle in needles:
        pos = body.find(needle, cur)
        if pos < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        positions.append(pos)
        cur = pos + len(needle)
    first = base_line + line_at(body, min(positions)) - 1
    last = base_line + line_at(body, max(positions)) - 1
    if first < 1 or last > file_text.count("\n") + 1:
        raise AssertionError(f"{label}: bad span {first}-{last}")
    return f"{first}-{last}"


def sha256_file(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing canonical asset {path}")
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def direct_save_call_locations() -> list[dict[str, int | str]]:
    locations: list[dict[str, int | str]] = []
    for path in sorted(RED.glob("*.C")):
        text = read(path, "latin-1")
        start = 0
        while True:
            pos = text.find(DIRECT_SAVE_CALL, start)
            if pos < 0:
                break
            locations.append({"file": path.name, "line": line_at(text, pos)})
            start = pos + len(DIRECT_SAVE_CALL)
    return locations


def assert_no_save_write_path(label: str, body: str) -> None:
    forbidden = [
        DIRECT_SAVE_CALL,
        "F0776_FILE_Create(G0569_pc_SavedGameFileName)",
        "Fcreate(G0569_pc_SavedGameFileName",
        "F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful",
        "F0420_SAVEUTIL_IsWriteObfuscatedSavePartSuccessful",
        "F0416_SAVEUTIL_IsWriteBytesSuccessful",
        "F0422_SAVEUTIL_IsWriteBytesWithChecksumSuccessful",
        "G2018_ul_LastSaveTime = G0313_ul_GameTime",
        "M571_DeleteFile(G0570_pc_SavedGameBackupFileName)",
        "M570_RenameFile(G0569_pc_SavedGameFileName, G0570_pc_SavedGameBackupFileName)",
    ]
    for needle in forbidden:
        if needle in body:
            raise AssertionError(f"{label}: unexpected save-write path marker {needle!r}")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)

    clik = read(RED / "CLIKMENU.C", "latin-1")
    moves = read(RED / "MOVESENS.C", "latin-1")
    command = read(RED / "COMMAND.C", "latin-1")
    loadsave = read(RED / "LOADSAVE.C", "latin-1")
    dungeon = read(RED / "DUNGEON.C", "latin-1")
    cmake = read(ROOT / "CMakeLists.txt")

    f0364_s, f0364_e, f0364 = slice_between(clik, "STATICFUNCTION void F0364_COMMAND_TakeStairs", "\nvoid F0365_COMMAND_ProcessTypes1To2_TurnParty", "CLIKMENU.C F0364")
    f0365_s, f0365_e, f0365 = slice_between(clik, "void F0365_COMMAND_ProcessTypes1To2_TurnParty", "\n/* When the party moves from one square", "CLIKMENU.C F0365")
    f0366_s, f0366_e, f0366 = slice_between(clik, "void F0366_COMMAND_ProcessTypes3To6_MoveParty", "\n#include \"CLIKCHAM.C\"", "CLIKMENU.C F0366")
    f0267_s, f0267_e, f0267 = slice_between(moves, "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "\nvoid F0268_SENSOR_AddEvent", "MOVESENS.C F0267")
    f0276_s, f0276_e, f0276 = slice_to_end(moves, "void F0276_SENSOR_ProcessThingAdditionOrRemoval", "MOVESENS.C F0276")
    f0380_s, f0380_e, f0380 = slice_between(command, "void F0380_COMMAND_ProcessQueue_CPSC", "\nvoid F1055_Post_F0380_COMMAND_ProcessQueue_CPSC", "COMMAND.C F0380")
    f0433_s, f0433_e, f0433 = slice_between(loadsave, "void F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF", "\nvoid F1059_Post_F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF", "LOADSAVE.C F0433")
    f0154_s, f0154_e, f0154 = slice_between(dungeon, "int16_t F0154_DUNGEON_GetLocationAfterLevelChange", "\nint16_t F0155_DUNGEON_GetStairsExitDirection", "DUNGEON.C F0154")
    f0155_s, f0155_e, f0155 = slice_between(dungeon, "int16_t F0155_DUNGEON_GetStairsExitDirection", "\nunsigned char* F0156_DUNGEON_GetThingData", "DUNGEON.C F0155")
    f0173_s, f0173_e, f0173 = slice_between(dungeon, "void F0173_DUNGEON_SetCurrentMap", "\nvoid F0174_DUNGEON_SetCurrentMapAndPartyMap", "DUNGEON.C F0173")
    f0174_s, f0174_e, f0174 = slice_to_end(dungeon, "void F0174_DUNGEON_SetCurrentMapAndPartyMap", "DUNGEON.C F0174")

    audited_movement = {
        "CLIKMENU.C:F0364_COMMAND_TakeStairs": f0364,
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": f0365,
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f0366,
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": f0267,
        "MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval": f0276,
        "DUNGEON.C:F0154_DUNGEON_GetLocationAfterLevelChange": f0154,
        "DUNGEON.C:F0155_DUNGEON_GetStairsExitDirection": f0155,
        "DUNGEON.C:F0173_DUNGEON_SetCurrentMap": f0173,
        "DUNGEON.C:F0174_DUNGEON_SetCurrentMapAndPartyMap": f0174,
    }
    for label, body in audited_movement.items():
        assert_no_save_write_path(label, body)

    stair_span = ordered_span(clik, f0364, f0364_s, [
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "G0327_i_NewPartyMapIndex = F0154_DUNGEON_GetLocationAfterLevelChange",
        "F0173_DUNGEON_SetCurrentMap(G0327_i_NewPartyMapIndex);",
        "F0284_CHAMPION_SetPartyDirection(F0155_DUNGEON_GetStairsExitDirection",
        "F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex);",
    ], "stairs direct transition")
    turn_stairs_span = ordered_span(clik, f0365, f0365_s, ["if (M034_SQUARE_TYPE", "F0364_COMMAND_TakeStairs", "return;"], "turn-on-stairs dispatch")
    move_stairs_span = ordered_span(clik, f0366, f0366_s, [
        "L1123_B_StairsSquare = (M034_SQUARE_TYPE",
        "if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2))",
        "F0364_COMMAND_TakeStairs",
        "if (L1116_i_SquareType == C03_ELEMENT_STAIRS)",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "F0364_COMMAND_TakeStairs",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
    ], "movement stairs and normal movement dispatch")
    teleporter_span = ordered_span(moves, f0267, f0267_s, [
        "if ((AL0709_i_DestinationSquareType = M034_SQUARE_TYPE(AL0708_i_DestinationSquare)) == C05_ELEMENT_TELEPORTER)",
        "if (!M007_GET(AL0708_i_DestinationSquare, MASK0x0008_TELEPORTER_OPEN))",
        "P0560_i_DestinationMapX = L0712_ps_Teleporter->TargetMapX;",
        "P0561_i_DestinationMapY = L0712_ps_Teleporter->TargetMapY;",
        "F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination = L0712_ps_Teleporter->TargetMapIndex);",
        "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
        "F0284_CHAMPION_SetPartyDirection",
    ], "teleporter level/map transition")
    pit_span = ordered_span(moves, f0267, f0267_s, [
        "if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT)",
        "L0715_ui_MapIndexDestination = F0154_DUNGEON_GetLocationAfterLevelChange",
        "F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);",
        "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
        "F0324_CHAMPION_DamageAll_GetDamagedChampionCount(20",
        "G0402_B_UseRopeToClimbDownPit = C0_FALSE;",
    ], "pit fall level transition")
    copy_protection_span = ordered_span(moves, f0267, f0267_s, [
        "#ifndef NOCOPYPROTECTION",
        "if ((L0715_ui_MapIndexDestination > 2)",
        "F0413_CPSC_GetChecksumEor(F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF",
        "P0560_i_DestinationMapX += P0561_i_DestinationMapY;",
        "P0561_i_DestinationMapY -= P0560_i_DestinationMapX;",
    ], "copy-protection save-symbol reference")
    result_span = ordered_span(moves, f0267, f0267_s, [
        "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
        "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX",
        "G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination;",
    ], "move result and sensor consequence")
    floor_sensor_span = ordered_span(moves, f0276, f0276_s, [
        "case C005_SENSOR_FLOOR_PARTY_ON_STAIRS:",
        "M034_SQUARE_TYPE(L0777_ui_Square) != C03_ELEMENT_STAIRS",
        "break;",
        "case C006_SENSOR_FLOOR_GROUP_GENERATOR:",
    ], "floor party-on-stairs sensor gate")
    location_span = ordered_span(dungeon, f0154, f0154_s, [
        "L0252_i_NewLevel = L0254_ps_Map->A.Level + P0271_i_LevelDelta;",
        "for (L0255_i_TargetMapIndex = 0",
        "*P0273_pi_MapY = L0251_i_NewMapY - L0253_i_Offset;",
        "*P0272_pi_MapX = L0250_i_NewMapX - L0254_ps_Map->OffsetMapX;",
        "return L0255_i_TargetMapIndex;",
    ], "level-delta map lookup")
    map_set_span = ordered_span(dungeon, f0174, f0174_s, [
        "F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex = P0322_i_MapIndex);",
        "G0264_puc_CurrentMapAllowedCreatureTypes = L0316_puc_MapMetaData;",
    ], "party map setter")
    save_dispatch_span = ordered_span(command, f0380, f0380_s, [
        "if (L1160_i_Command == C140_COMMAND_SAVE_GAME)",
        "if ((G0305_ui_PartyChampionCount > 0) && !G0299_ui_CandidateChampionOrdinal)",
        DIRECT_SAVE_CALL,
        "goto T0380042;",
    ], "command C140 save dispatch")
    save_write_span = ordered_span(loadsave, f0433, f0433_s, [
        "M571_DeleteFile(G0570_pc_SavedGameBackupFileName);",
        "M570_RenameFile(G0569_pc_SavedGameFileName, G0570_pc_SavedGameBackupFileName);",
        "F0776_FILE_Create(G0569_pc_SavedGameFileName)",
        "F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful",
        "F0420_SAVEUTIL_IsWriteObfuscatedSavePartSuccessful",
        "G2018_ul_LastSaveTime = G0313_ul_GameTime;",
    ], "save write path")

    locations = direct_save_call_locations()
    expected = [{"file": "COMMAND.C", "line": find_line(command, DIRECT_SAVE_CALL)}]
    if locations != expected:
        raise AssertionError(f"unexpected direct save call locations: {locations}; expected {expected}")
    if PASS not in cmake:
        raise AssertionError(f"CMakeLists.txt does not register {PASS}")

    assets = {
        "DUNGEON.DAT": sha256_file(CANON / "DUNGEON.DAT"),
        "GRAPHICS.DAT": sha256_file(CANON / "GRAPHICS.DAT"),
        "TITLE": sha256_file(CANON / "TITLE"),
    }
    source_audit = {
        "CLIKMENU.C:F0364_COMMAND_TakeStairs": {"lines": [f0364_s, f0364_e], "focusedLines": stair_span, "claim": "stairs level change removes party from source, resolves destination level/map, sets facing, and restores current map; no save/write path"},
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": {"lines": [f0365_s, f0365_e], "focusedLines": turn_stairs_span, "claim": "turning while on stairs delegates to F0364 and returns before any command-save route"},
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": {"lines": [f0366_s, f0366_e], "focusedLines": move_stairs_span, "claim": "movement stairs paths delegate through F0364/F0267; ordinary movement calls F0267; no direct save/write path"},
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": {"lines": [f0267_s, f0267_e], "focusedLines": {"teleporter": teleporter_span, "pit": pit_span, "copyProtectionOnly": copy_protection_span, "resultAndSensors": result_span}, "claim": "teleporter and pit chains update map/position/direction/result/sensors; save function name appears only in checksum/copy-protection expressions, not as a call"},
        "MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval": {"lines": [f0276_s, f0276_e], "focusedLines": floor_sensor_span, "claim": "party-on-stairs sensor filtering has no save/write path"},
        "DUNGEON.C:F0154_DUNGEON_GetLocationAfterLevelChange": {"lines": [f0154_s, f0154_e], "focusedLines": location_span, "claim": "level delta resolves a target map and adjusted coordinates only"},
        "DUNGEON.C:F0155_DUNGEON_GetStairsExitDirection": {"lines": [f0155_s, f0155_e], "focusedLines": f"{f0155_s}-{f0155_e}", "claim": "stairs exit direction probes adjacent square type only"},
        "DUNGEON.C:F0173/F0174 current-map setters": {"lines": [f0173_s, f0174_e], "focusedLines": map_set_span, "claim": "map setters update current/party map metadata only"},
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": {"lines": [f0380_s, f0380_e], "focusedLines": save_dispatch_span, "claim": "C140 command is the only direct caller of F0433"},
        "LOADSAVE.C:F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF": {"lines": [f0433_s, f0433_e], "focusedLines": save_write_span, "claim": "F0433 owns save-file creation/write and last-save-time mutation"},
    }

    manifest = {
        "schema": "firestaff.parity.pass607.level_change_autosave_absence.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "worktree": str(ROOT),
        "branchAtVerification": git("branch", "--show-current"),
        "gitHeadAtVerification": git("rev-parse", "HEAD"),
        "scope": "DM1 V1 ordinary stairs/pit/teleporter level-change autosave absence",
        "canonicalAssetsSha256": assets,
        "directSaveCallLocations": locations,
        "sourceAudit": source_audit,
        "conclusion": "No ordinary DM1 V1 stairs, pit, or teleporter level-change path directly saves the game; the audited save write path is reached through command C140 only.",
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    report = [
        "# Pass607 - DM1 V1 level-change autosave absence source lock",
        "",
        f"- Status: {STATUS}",
        f"- Manifest: {MANIFEST.relative_to(ROOT)}",
        "- Scope: ordinary stairs, pits, teleporters, command C140, and save write path",
        "",
        "## ReDMCSB Source Audit",
        "",
        f"- CLIKMENU.C:{f0364_s}-{f0364_e}, focused {stair_span}: F0364 takes stairs through F0267/F0154/F0155 and current-map setup; no direct save/write path.",
        f"- CLIKMENU.C:{f0365_s}-{f0365_e}, focused {turn_stairs_span}: turning on stairs delegates to F0364 and returns.",
        f"- CLIKMENU.C:{f0366_s}-{f0366_e}, focused {move_stairs_span}: movement stairs branches delegate through F0364/F0267; ordinary accepted movement calls F0267; no direct save/write path.",
        f"- MOVESENS.C:{f0267_s}-{f0267_e}, focused teleporter {teleporter_span}, pit {pit_span}, result {result_span}: teleporter/pit chains mutate map/position/direction/result/sensors only.",
        f"- MOVESENS.C:{f0267_s}-{f0267_e}, focused {copy_protection_span}: save-symbol references in F0267 are checksum/copy-protection expressions only, not direct calls.",
        f"- MOVESENS.C:{f0276_s}-{f0276_e}, focused {floor_sensor_span}: party-on-stairs floor sensor filtering has no save/write path.",
        f"- DUNGEON.C:{f0154_s}-{f0154_e}, focused {location_span}: level-delta lookup returns target map/coordinates only.",
        f"- DUNGEON.C:{f0155_s}-{f0155_e}: stairs exit direction probes square geometry only.",
        f"- DUNGEON.C:{f0173_s}-{f0174_e}, focused {map_set_span}: current/party map setters update map metadata only.",
        f"- COMMAND.C:{f0380_s}-{f0380_e}, focused {save_dispatch_span}: command C140 is the only direct caller of F0433.",
        f"- LOADSAVE.C:{f0433_s}-{f0433_e}, focused {save_write_span}: F0433 owns saved-game file creation/write and G2018 last-save-time mutation.",
        "",
        "## Direct Save Call Search",
        "",
        f"- Exact `{DIRECT_SAVE_CALL}` locations across ReDMCSB Common/Source `*.C`: {locations}",
        "",
        "## Canonical Asset Hashes",
        "",
        f"- DUNGEON.DAT: {assets['DUNGEON.DAT']}",
        f"- GRAPHICS.DAT: {assets['GRAPHICS.DAT']}",
        f"- TITLE: {assets['TITLE']}",
        "",
        "## Conclusion",
        "",
        "ReDMCSB does not show an ordinary DM1 V1 autosave on stairs, pits, teleporters, or chained level-change movement. The source-lock result is absence, not an implementation task: save writes are routed through command C140 only.",
    ]
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")
    print(f"{STATUS}: {REPORT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
