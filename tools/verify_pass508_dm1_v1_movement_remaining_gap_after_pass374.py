#!/usr/bin/env python3
from __future__ import annotations

import json
import hashlib
import os
import pathlib
import subprocess
import struct
import zlib
from typing import Any
from zipfile import ZipFile, ZIP_DEFLATED, ZIP_STORED
import sys
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass508_dm1_v1_movement_remaining_gap_after_pass374"
OUT_DIR = pathlib.Path(os.environ.get(
    "FIRESTAFF_VERIFICATION_OUTPUT_DIR",
    str(ROOT / "parity-evidence" / "verification" / PASS),
))
MANIFEST = OUT_DIR / "manifest.json"
REPORT = pathlib.Path(os.environ.get(
    "FIRESTAFF_VERIFICATION_REPORT_PATH",
    str(ROOT / "parity-evidence" / (PASS + ".md")),
))
REDMCSB = pathlib.Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source")))
DM1_ARCHIVE = pathlib.Path(os.environ.get(
    "FIRESTAFF_DM1_PC34_ARCHIVE",
    str(pathlib.Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip")))
EXPECTED_STATUS = "BLOCKED_PASS508_DM1_V1_MOVEMENT_REMAINING_ORIGINAL_OVERLAY_GAP_PROVED"


def display_path(path: pathlib.Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT.resolve()))
    except ValueError:
        return str(path.resolve())
EXPECTED_HASHES = {
    "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "f0380-dequeues-and-dispatches-route-keys", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "claim": "original route proof must show keyboard/click queue reaching the turn/move dispatch boundary", "markers": ["void F0380_COMMAND_ProcessQueue_CPSC(", "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]},
    {"id": "f0365-turn-mutates-direction-before-redraw", "file": "CLIKMENU.C", "lines": "142-179", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "claim": "turn overlay proof must bind original capture to source direction mutation", "markers": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty(", "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX", "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]},
    {"id": "f0366-step-legality-move-result-and-cooldown", "file": "CLIKMENU.C", "lines": "180-347", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "claim": "step overlay proof must bind original capture to destination legality, F0267 movement, stamina, cooldown, and input-wait release", "markers": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty(", "F0325_CHAMPION_DecrementStamina", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]},
    {"id": "f0267-commits-party-tuple-and-scent-timing", "file": "MOVESENS.C", "lines": "738-818", "function": "F0267_MOVE_GetMoveResult_CPSCE", "claim": "movement parity must observe committed original party tuple and timing side effects", "markers": ["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;", "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;", "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;", "F0317_CHAMPION_AddScentStrength", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;"]},
    {"id": "f0002-redraws-viewport-from-mutated-party-state", "file": "GAMELOOP.C", "lines": "35-97,215-219", "function": "F0002_MAIN_GameLoop_CPSDF", "claim": "original overlay comparison must use post-command viewport redraw from mutated source party state", "markers": ["STATICFUNCTION void F0002_MAIN_GameLoop_CPSDF(", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]},
]

def compact(value: str) -> str:
    return " ".join(value.split())

def read_text(path: pathlib.Path, encoding: str = "utf-8") -> str:
    if not path.is_file():
        raise AssertionError("missing required file: " + str(path))
    return path.read_text(encoding=encoding, errors="replace")

def source_excerpt(file_name: str, ranges: str) -> str:
    lines = read_text(REDMCSB / file_name, "latin-1").splitlines()
    chunks: list[str] = []
    for span in ranges.split(","):
        start_s, end_s = span.split("-", 1)
        start, end = int(start_s), int(end_s)
        chunks.extend(lines[start - 1:end])
    return "\n".join(chunks)

def audit_source() -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        excerpt = compact(source_excerpt(lock["file"], lock["lines"]))
        missing = [marker for marker in lock["markers"] if compact(marker) not in excerpt]
        item = dict(lock)
        item["ok"] = not missing
        item["missingMarkers"] = missing
        item.pop("markers", None)
        results.append(item)
    return results

def audit_authentic_media() -> list[dict[str, Any]]:
    if not DM1_ARCHIVE.is_file():
        raise AssertionError(f"missing retail PC34 archive: {DM1_ARCHIVE}")
    rows = []
    for name, expected in EXPECTED_HASHES.items():
        raw = DM1_ARCHIVE.read_bytes()
        with ZipFile(DM1_ARCHIVE) as archive:
            info = archive.getinfo(name)
        offset = info.header_offset
        if raw[offset:offset + 4] != b"PK\x03\x04":
            raise AssertionError(f"bad local header for {name}")
        name_len, extra_len = struct.unpack_from("<HH", raw, offset + 26)
        start = offset + 30 + name_len + extra_len
        packed = raw[start:start + info.compress_size]
        if info.compress_type == ZIP_STORED:
            data = packed
        elif info.compress_type == ZIP_DEFLATED:
            data = zlib.decompress(packed, -15)
        else:
            raise AssertionError(f"unsupported compression for {name}")
        if len(data) != info.file_size or (zlib.crc32(data) & 0xffffffff) != info.CRC:
            raise AssertionError(f"retail ZIP integrity failure for {name}")
        actual = hashlib.sha256(data).hexdigest()
        if actual != expected:
            raise AssertionError(f"{name} hash mismatch: {actual} != {expected}")
        rows.append({"member": name, "sha256": actual, "bytes": len(data),
                     "readMode": "in-memory/no-extraction"})
    return rows

def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass508 - DM1 V1 movement remaining gap after pass373/pass374", "", "Status: " + manifest["status"], "", "Scope: movement/forflyttning evidence only. This pass consumes pass373/pass374 and proves the next remaining gap; it does not promote pixel parity.", "", "## ReDMCSB source audit first", ""]
    for item in manifest["redmcsbSourceAudit"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append("- {} {}:{} - {}: {}.".format(state, item["file"], item["lines"], item["function"], item["claim"]))
    lines += ["", "## Authentic PC 3.4 media", ""]
    for item in manifest["authenticMedia"]:
        lines.append("- PASS {} bytes={} sha256={}".format(item["member"], item["bytes"], item["sha256"]))
    lines += ["", "## Remaining movement parity gap", "", "The next remaining gap is not Firestaff's live movement route: pass373/pass374 already credit that route into source-locked wall/door/occlusion redraw. The remaining movement gap is original-backed proof: a DOS PC/I34E keyboard-buffer or route transcript that reaches F0380, F0365/F0366, F0267, then the post-command F0128 viewport redraw, plus representative movement/HUD/viewport overlay captures tied to that tuple.", "", "Promotion requirements:", "", "- materialized original runtime frames or trace records, not ignored/absent capture assets", "- command-specific route labels for turn and step commands", "- post-vblank viewport frame hashes/crops that differ where the command changes direction or position", "- party tuple evidence: direction, map index, X, Y before and after source movement", "- explicit non-claim boundary for Firestaff-only source-equivalent tests until those original artifacts exist", "", "Missing tools/artifacts if blocked: no required executable is missing in this worktree; the missing item is original runtime evidence (DOS PC/I34E keyboard-buffer/F0380 transcript and representative original movement/HUD/viewport overlay captures).", ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    if not REDMCSB.exists():
        raise AssertionError(f"missing repository ReDMCSB source: {REDMCSB}")
    source = audit_source()
    media = audit_authentic_media()
    status = EXPECTED_STATUS if all(i["ok"] for i in source) else "FAIL_PASS508_DM1_V1_MOVEMENT_REMAINING_GAP_AUDIT"
    manifest = {"schema": PASS + ".v2", "status": status, "redmcsbSourceRoot": str(REDMCSB.relative_to(ROOT)), "dm1Archive": str(DM1_ARCHIVE), "remainingGap": {"id": "original_runtime_movement_overlay_and_keyboard_buffer_transcript", "missingArtifacts": ["DOS PC/I34E keyboard-buffer/F0380 route transcript for representative movement keys", "representative original movement/HUD/viewport overlay captures tied to pre/post party tuple"], "missingTools": [], "notClaimed": ["pixel-perfect movement/HUD/viewport parity", "original overlay regression", "binary-level direct F0380 body proof"]}, "redmcsbSourceAudit": source, "authenticMedia": media, "removedStaleDependencies": ["generated completion score", "historical pass manifests", "extracted original-media tree"]}
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": display_path(MANIFEST), "report": display_path(REPORT), "sourceChecks": len(source), "authenticMembers": len(media), "missingTools": []}, indent=2, sort_keys=True))
    return 0 if status == EXPECTED_STATUS else 1

if __name__ == "__main__":
    raise SystemExit(main())
