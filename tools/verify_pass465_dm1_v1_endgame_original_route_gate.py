#!/usr/bin/env python3
"""Pass465 gate: source-lock DM1 V1 ENDGAME route and refuse unproven stock runtime promotion.

This verifier is intentionally blocker-safe. It proves the ReDMCSB source anchors
for entering ENDGAME.C:F0444_STARTEND_Endgame and the N2 PC34 original-data
identity, then requires a stock-runtime trace/save-state artifact before any
original-runtime route can be promoted.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source"
DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34"
OUT_DIR = ROOT / "parity-evidence/verification/pass465_dm1_v1_endgame_original_route_gate"
REPORT = ROOT / "parity-evidence/pass465_dm1_v1_endgame_original_route_gate.md"
JSON_OUT = OUT_DIR / "manifest.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
TRACE = ROOT / "parity-evidence/verification/pass465_dm1_v1_endgame_original_runtime_trace/trace.ndjson"

SOURCE_CHECKS = [
    ("ENDGAME.C", r"void\s+F0444_STARTEND_Endgame\s*\(", "defines F0444_STARTEND_Endgame"),
    ("ENDGAME.C", r"F0444_STARTEND_Endgame\(C1_TRUE\);", "F0446 fuse sequence enters F0444(true)"),
    ("PROJEXPL.C", r"F0446_STARTEND_FuseSequence\(\);", "fuse action path calls F0446"),
    ("MENU.C", r"case\s+C043_ACTION_FUSE:", "champion action dispatch handles Fuse"),
    ("MENU.C", r"F0225_GROUP_FuseAction\(", "Fuse dispatch calls group fuse action"),
    ("TIMELINE.C", r"F0444_STARTEND_Endgame\(C1_TRUE\);", "end-game sensor can enter F0444(true)"),
    ("GAMELOOP.C", r"F0444_STARTEND_Endgame\(G0303_B_PartyDead\);", "game loop calls F0444 after party death/win loop"),
    ("STARTUP2.C", r"F0444_STARTEND_Endgame\(C0_FALSE\);", "PC34 startup load-failure route calls F0444(false)"),
    ("TITLE.C", r"void\s+F0437_STARTEND_DrawTitle\s*\(", "TITLE.C defines title draw anchor"),
    ("TITLE.C", r"C001_GRAPHIC_TITLE", "TITLE.C loads/draws C001_GRAPHIC_TITLE"),
]

ORIGINAL_FILES = ["DM.EXE", "FIRES", "VGA", "TITLE", "DATA/DUNGEON.DAT", "DATA/GRAPHICS.DAT"]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def find_line(text: str, pattern: str) -> int | None:
    rx = re.compile(pattern)
    for idx, line in enumerate(text.splitlines(), 1):
        if rx.search(line):
            return idx
    return None


def audit_symbol_map() -> dict:
    audit = {"path": str(SYMBOL_MAP), "exists": SYMBOL_MAP.exists(), "entries": [], "verified_runtime_hit": False}
    if not SYMBOL_MAP.exists():
        return audit
    try:
        data = json.loads(SYMBOL_MAP.read_text())
    except Exception as exc:  # diagnostic gate output
        audit["error"] = str(exc)
        return audit
    for entry in data.get("entries", []):
        haystack = json.dumps(entry, sort_keys=True)
        if "F0444_STARTEND_Endgame" not in haystack and "ENDGAME.C" not in haystack:
            continue
        item = {
            "id": entry.get("id"),
            "file": entry.get("file"),
            "function": entry.get("function"),
            "confidence": entry.get("confidence"),
            "runtime_cs_ip": entry.get("runtime_cs_ip"),
        }
        audit["entries"].append(item)
        if item["function"] == "F0444_STARTEND_Endgame" and item["confidence"] == "verified_runtime_hit":
            audit["verified_runtime_hit"] = True
    return audit


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source_results = []
    ok = True
    for fname, pattern, desc in SOURCE_CHECKS:
        path = RED / fname
        exists = path.exists()
        line = find_line(path.read_text(encoding="latin-1", errors="replace"), pattern) if exists else None
        passed = bool(exists and line)
        ok = ok and passed
        source_results.append({"file": fname, "description": desc, "pattern": pattern, "line": line, "ok": passed})

    original_results = []
    for rel in ORIGINAL_FILES:
        path = DM1 / rel
        exists = path.exists()
        ok = ok and exists
        original_results.append({
            "path": str(path),
            "relative_path": rel,
            "exists": exists,
            "size": path.stat().st_size if exists else None,
            "sha256": sha256(path) if exists else None,
        })

    runtime_artifacts = [
        {"path": str(SYMBOL_MAP), "exists": SYMBOL_MAP.exists()},
        {"path": str(TRACE), "exists": TRACE.exists()},
    ]
    symbol_map_f0444 = audit_symbol_map()
    blocker_missing = [a["path"] for a in runtime_artifacts if not a["exists"]]
    if not symbol_map_f0444["verified_runtime_hit"]:
        blocker_missing.append("symbol_map:F0444_STARTEND_Endgame confidence=verified_runtime_hit")

    status = "BLOCKED_ORIGINAL_RUNTIME_ROUTE_MISSING_F0444_TRACE" if blocker_missing else "PASS_ORIGINAL_RUNTIME_ROUTE_ARTIFACTS_PRESENT"
    promoted = ok and not blocker_missing

    result = {
        "gate": "pass465_dm1_v1_endgame_original_route_gate",
        "status": status,
        "promoted_original_runtime_route": promoted,
        "parity_claim": False,
        "redmcsb_source_root": str(RED),
        "original_pc34_root": str(DM1),
        "source_results": source_results,
        "original_data": original_results,
        "required_runtime_artifacts": runtime_artifacts,
        "symbol_map_f0444": symbol_map_f0444,
        "blocker_missing_artifacts": blocker_missing,
        "required_next_evidence": [
            "stock PC34 FIRES load-segment/runtime address map with F0444 confidence=verified_runtime_hit",
            "trace event proving execution reached ENDGAME.C:F0444_STARTEND_Endgame in original runtime",
            "optional save-state/route file bound to the same trace; screenshots alone do not promote",
        ],
    }
    JSON_OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass465_dm1_v1_endgame_original_route_gate",
        "",
        f"- status: `{status}`",
        "- parity claim: **not made**",
        f"- promoted original runtime route: `{str(promoted).lower()}`",
        f"- manifest: `{JSON_OUT}`",
        "",
        "## ReDMCSB-first source audit anchors",
        "",
    ]
    for r in source_results:
        anchor = f"{r['file']}:{r['line']}" if r["line"] else f"{r['file']}:MISSING"
        lines.append(f"- {anchor} — {r['description']} — ok={r['ok']}")
    lines += ["", "## N2 PC34 original data identity", ""]
    for r in original_results:
        sha = r["sha256"][:16] if r["sha256"] else "missing"
        lines.append(f"- {r['relative_path']} size={r['size']} sha256={sha}… exists={r['exists']}")
    lines += [
        "",
        "## Symbol-map F0444 audit",
        "",
        f"- symbol map: {SYMBOL_MAP}",
        f"- verified_runtime_hit: {symbol_map_f0444['verified_runtime_hit']}",
        f"- matching entries: {len(symbol_map_f0444['entries'])}",
    ]
    for entry in symbol_map_f0444["entries"]:
        lines.append(
            "- function={function} id={id} confidence={confidence} runtime_cs_ip={runtime_cs_ip}".format(**entry)
        )
    lines += [
        "",
        "## Blocker",
        "",
        "The source route to `ENDGAME.C:F0444_STARTEND_Endgame` is locked, including `TITLE.C` title anchors, but this pass did not find a promotable stock PC34 runtime route/save-state because the required runtime evidence is absent or unverified:",
        "",
    ]
    for item in blocker_missing:
        lines.append(f"- missing `{item}`")
    lines += [
        "",
        "Required next evidence: a stock `FIRES` debugger trace or save-state route whose symbol map marks `F0444_STARTEND_Endgame` as `verified_runtime_hit`. Screenshots or Firestaff/ReDMCSB rebuilt execution are insufficient.",
        "",
        "Non-claims: no pixel/video parity claim, no pushed changes, no DANNESBURK data, no raw debugger log promotion.",
        "",
    ]
    REPORT.write_text("\n".join(lines))
    print(json.dumps({"status": status, "promoted": promoted, "report": str(REPORT), "manifest": str(JSON_OUT)}, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
