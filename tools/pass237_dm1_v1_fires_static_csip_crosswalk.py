#!/usr/bin/env python3
"""Pass237: conservative static CS:IP crosswalk for DM1 PC34 FIRES.EXENEW.

This pass does not promote runtime hits. It derives debugger follow-up
breakpoint candidates from the decompressed FIRES.EXENEW image body, MZ header
facts, far-call operands, and nearby opcode/source-constant signatures.
"""
from __future__ import annotations

import hashlib
import json
import os
import struct
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/pass237_dm1_v1_fires_static_csip_crosswalk"
REPORT = ROOT / "parity-evidence/pass237_dm1_v1_fires_static_csip_crosswalk.md"
EXPECTED_SHA256 = "fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94"

# Do not copy this binary into the repo. It is read-only input for text evidence.
FIRES_SEARCH = [
    Path(os.environ.get("FIRES_EXENEW", "")) if os.environ.get("FIRES_EXENEW") else None,
    ROOT / "parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW",
    ROOT.parent / "firestaff-dm1v1-viewport-walls-source-lock-20260506-0237/parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW",
]

CANDIDATES: list[dict[str, Any]] = [
    {
        "id": "command_accepted",
        "source_file": "COMMAND.C",
        "source_function": "F0380_COMMAND_ProcessQueue_CPSC",
        "body_linear": 0x1C2A9,
        "candidate_static_cs_ip": "1b7c:06e9",
        "confidence": "medium_high",
        "classification": "candidate_only",
        "evidence": [
            "function prologue followed by queue-count guard at data 3e78 and queue index at 3ec8",
            "branches compare accepted command in SI against 1/2 and 3..6",
            "dispatch lcall targets at 1c3b6 -> 1771:010d and 1c3ca -> 1771:01aa",
        ],
        "blocker_to_promote": "Capture loaded FIRES PSP/load segment and debugger hit at this queue-dispatch prologue or branch after command dequeuing.",
    },
    {
        "id": "turn_types_1_to_2",
        "source_file": "CLIKMENU.C",
        "source_function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "body_linear": 0x1781D,
        "candidate_static_cs_ip": "1771:010d",
        "confidence": "high_static",
        "classification": "candidate_only",
        "evidence": [
            "exact far-call operand from F0380 branch for command values 1 or 2",
            "prologue sets movement redraw flag at 1a7c then selects zone constants 0x44/0x45 (C068/C069 turn arrows)",
            "updates party direction through 0d06:000d before invoking movement result helper 1126:18ba",
        ],
        "blocker_to_promote": "Runtime hit on 1771:010d with command 1/2 and party-direction before/after values.",
    },
    {
        "id": "move_types_3_to_6",
        "source_file": "CLIKMENU.C",
        "source_function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "body_linear": 0x178BA,
        "candidate_static_cs_ip": "1771:01aa",
        "confidence": "high_static",
        "classification": "candidate_only",
        "evidence": [
            "exact far-call operand from F0380 branch for command values 3..6",
            "prologue sets movement redraw flag at 1a7c then computes command-3 movement index",
            "adds 0x46 to movement index, matching C070_ZONE_MOVE_FORWARD + movement-arrow index family",
        ],
        "blocker_to_promote": "Runtime hit on 1771:01aa with command 3..6 and source/destination party coordinates.",
    },
    {
        "id": "move_get_move_result",
        "source_file": "MOVESENS.C",
        "source_function": "F0267_MOVE_GetMoveResult_CPSCE",
        "body_linear": 0x11776,
        "candidate_static_cs_ip": "1126:0516",
        "confidence": "medium_high",
        "classification": "candidate_only",
        "evidence": [
            "exact far-call target repeatedly used with five pushed arguments including 0xffff party thing sentinel",
            "entry decodes thing cell bits with masks 0x3c00/0xc000 and branches on source/destination map state",
            "called from F0365/F0366 static candidates as the legal step / collision resolver",
        ],
        "blocker_to_promote": "Runtime hit on 1126:0516 plus watchpoints proving G0306/G0307 writes for the party path.",
    },
    {
        "id": "viewport_game_loop_draw_call_site",
        "source_file": "GAMELOOP.C",
        "source_function": "F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF",
        "body_linear": 0x24DCE,
        "candidate_static_cs_ip": "23cc:110e",
        "confidence": "low_medium",
        "classification": "candidate_only",
        "evidence": [
            "function calls input/timeline/viewport-adjacent routines and later conditionally calls 1126:0516 with party coordinates",
            "candidate came from the decompressed-image control-flow cluster around pass234 viewport blocker follow-up",
            "not yet separated into the exact F0098/F0104/F0108/F0109/F0115 draw helpers without FIRES.MAP",
        ],
        "blocker_to_promote": "Need FIRES.MAP or debugger stepping from F0128 through DUNVIEW helpers; current static evidence only identifies a viewport-adjacent loop cluster.",
    },
]

PATTERNS = {
    "command_dispatch_lcall_turn": bytes.fromhex("9a 0d 01 71 17"),
    "command_dispatch_lcall_move": bytes.fromhex("9a aa 01 71 17"),
    "turn_zone_44_45": bytes.fromhex("b8 44 00 eb 03 b8 45 00"),
    "move_zone_46_add": bytes.fromhex("2d 03 00 8b f0 05 46 00"),
    "move_result_far_call": bytes.fromhex("9a 16 05 26 11"),
    "viewport_cluster_entry": bytes.fromhex("55 8b ec 0e e8 ad f1 9a c8 01 e2 13"),
}

SOURCE_NEEDLES = {
    "COMMAND.C": ["F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],
    "CLIKMENU.C": ["C068_ZONE_TURN_LEFT", "C070_ZONE_MOVE_FORWARD", "F0267_MOVE_GetMoveResult_CPSCE"],
    "MOVESENS.C": ["F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX = P0560_i_DestinationMapX"],
    "DUNVIEW.C": ["F0098_DUNGEONVIEW_DrawFloorAndCeiling", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "F0128_DUNGEONVIEW_Draw_CPSF"],
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def find_fires() -> Path:
    for p in FIRES_SEARCH:
        if p and p.is_file():
            return p
    matches = sorted(ROOT.parent.glob("firestaff-*/parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW"))
    if matches:
        return matches[0]
    raise FileNotFoundError("FIRES.EXENEW not found; set FIRES_EXENEW to the decompressed text-input binary path")


def mz_info(blob: bytes) -> dict[str, Any]:
    if blob[:2] != b"MZ":
        raise ValueError("input is not an MZ executable")
    e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc, e_ss, e_sp, e_csum, e_ip, e_cs, e_lfarlc = struct.unpack_from("<HHHHHHHHHHHH", blob, 2)
    return {
        "e_cparhdr": e_cparhdr,
        "header_bytes": e_cparhdr * 16,
        "e_crlc": e_crlc,
        "e_lfarlc": e_lfarlc,
        "entry_cs_ip": f"{e_cs:04x}:{e_ip:04x}",
        "entry_body_linear": e_cs * 16 + e_ip,
        "declared_pages": e_cp,
        "last_page_bytes": e_cblp,
        "minalloc": e_minalloc,
        "maxalloc": e_maxalloc,
        "initial_ss_sp": f"{e_ss:04x}:{e_sp:04x}",
    }


def pattern_hits(body: bytes) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    for name, pat in PATTERNS.items():
        hits = []
        start = 0
        while True:
            idx = body.find(pat, start)
            if idx < 0:
                break
            hits.append(f"0x{idx:05x}")
            start = idx + 1
        out[name] = hits
    return out


def source_audit() -> dict[str, Any]:
    out = {}
    for file_name, needles in SOURCE_NEEDLES.items():
        path = SOURCE_ROOT / file_name
        text = path.read_text(encoding="latin-1", errors="replace") if path.is_file() else ""
        out[file_name] = {"path": str(path), "ok": bool(text) and all(n in text for n in needles), "needles": {n: (n in text) for n in needles}}
    return out


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass237 — DM1 PC34 FIRES static CS:IP crosswalk",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This pass emits debugger follow-up candidates only. Nothing here is a `verified_runtime_hit`.",
        "",
        "## MZ / image facts",
        "",
        f"- FIRES.EXENEW input: `{manifest['fires_input']['path']}`",
        f"- sha256: `{manifest['fires_input']['sha256']}`",
        f"- MZ header bytes skipped for body disassembly: `{manifest['mz']['header_bytes']}`",
        f"- body size: `{manifest['fires_input']['body_size']}`",
        f"- MZ entry CS:IP: `{manifest['mz']['entry_cs_ip']}` (not a source seam)",
        "",
        "## Candidate breakpoints",
        "",
    ]
    for c in manifest["candidates"]:
        lines.append(f"- `{c['id']}` — `{c['source_function']}`")
        lines.append(f"  - static candidate: `{c['candidate_static_cs_ip']}` / body linear `0x{c['body_linear']:05x}`")
        lines.append(f"  - confidence: `{c['confidence']}`; classification: `{c['classification']}`")
        lines.append(f"  - promote blocker: {c['blocker_to_promote']}")
        for ev in c["evidence"]:
            lines.append(f"  - evidence: {ev}")
    lines += ["", "## Pattern guards", ""]
    for name, hits in manifest["pattern_hits"].items():
        hit_text = ", ".join(hits) if hits else "`MISSING`"
        lines.append(f"- `{name}`: {hit_text}")
    lines += ["", "## Promotion blocker", "", manifest["promotion_blocker"], ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    fires = find_fires()
    blob = fires.read_bytes()
    digest = sha256(fires)
    mz = mz_info(blob)
    body = blob[mz["header_bytes"]:]
    hits = pattern_hits(body)
    candidate_offsets_ok = all(0 <= c["body_linear"] < len(body) for c in CANDIDATES)
    required_patterns_ok = all(hits[k] for k in ["command_dispatch_lcall_turn", "command_dispatch_lcall_move", "turn_zone_44_45", "move_zone_46_add", "move_result_far_call", "viewport_cluster_entry"])
    source = source_audit()
    status = "CANDIDATE_ONLY_RUNTIME_HITS_REQUIRED" if digest == EXPECTED_SHA256 and candidate_offsets_ok and required_patterns_ok and all(v["ok"] for v in source.values()) else "FAIL_STATIC_GUARD"
    manifest = {
        "schema": "pass237_dm1_v1_fires_static_csip_crosswalk.v1",
        "status": status,
        "fires_input": {"path": str(fires), "sha256": digest, "size": len(blob), "body_size": len(body), "artifact_policy": "read-only original-derived binary; not copied into repo"},
        "mz": mz,
        "source_audit": source,
        "pattern_hits": hits,
        "candidates": CANDIDATES,
        "promotion_blocker": "Exact blocker to promote any candidate: capture runtime PSP/load segment and debugger-observed hit at the listed static CS:IP (runtime_cs = PSP + 0x10 + static_cs, runtime_ip = static_ip), plus seam-specific state/watchpoint evidence. Until then every row remains candidate_only, not verified_runtime_hit.",
        "artifact_policy": {"no_original_binaries_committed": True, "text_only_repo_artifacts": True},
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json"), "candidate_count": len(CANDIDATES)}, indent=2, sort_keys=True))
    return 0 if status == "CANDIDATE_ONLY_RUNTIME_HITS_REQUIRED" else 1

if __name__ == "__main__":
    raise SystemExit(main())
