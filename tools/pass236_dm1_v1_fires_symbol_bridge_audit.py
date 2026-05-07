#!/usr/bin/env python3
"""Pass236: audit N2-local FIRES public-symbol/source-to-runtime bridge evidence.

This is text-only evidence. Any ReDMCSB build products are created under /tmp
and only bounded text summaries are copied into parity-evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit"
REPORT = ROOT / "parity-evidence/pass236_dm1_v1_fires_symbol_bridge_audit.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source"
WIP = REDMCSB / "ReDMCSB_WIP20210206"
COMMON_SOURCE = WIP / "Toolchains/Common/Source"
IBM_SOURCE = WIP / "Toolchains/IBM PC/Source"
IBM_BASE = WIP / "Toolchains/IBM PC/Base/HARDDISK"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM"
CANON = ORIG / "_canonical/dm1"
EXTRACTED = ORIG / "_extracted/dm-pc34/DungeonMasterPC34"

SEARCH_ROOTS = [
    REDMCSB / "Reference/ReDMCSB/I34E",
    WIP / "Reference/ReDMCSB/I34E",
    REDMCSB / "Reference/Original/I34E",
    WIP / "Reference/Original/I34E",
    IBM_SOURCE,
    COMMON_SOURCE,
    CANON,
    EXTRACTED,
]
TARGET_SYMBOL_NAMES = ["FIRES.MAP", "FIRES.SYM", "FIRES.LST", "FIRES.OBJ", "FIRES.EXE.MAP"]
SEAMS = [
    ("COMMAND.C", "F0380_COMMAND_ProcessQueue_CPSC"),
    ("CLIKMENU.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty"),
    ("CLIKMENU.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    ("MOVESENS.C", "F0267_MOVE_GetMoveResult_CPSCE"),
    ("DUNVIEW.C", "F0098_DUNGEONVIEW_DrawFloorAndCeiling"),
    ("DUNVIEW.C", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"),
    ("DUNVIEW.C", "F0108_DUNGEONVIEW_DrawFloorOrnament"),
    ("DUNVIEW.C", "F0109_DUNGEONVIEW_DrawDoorOrnament"),
    ("DUNVIEW.C", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def rel(path: Path) -> str:
    return str(path)


def find_files(root: Path) -> list[dict[str, Any]]:
    hits: list[dict[str, Any]] = []
    if not root.exists():
        return hits
    for dirpath, dirnames, filenames in os.walk(root):
        # keep searches focused; skip bulky/runtime dependency dirs if pointed too broadly by mistake
        dirnames[:] = [d for d in dirnames if d not in {"node_modules", ".git", "__pycache__"}]
        for name in filenames:
            upper = name.upper()
            suffix = Path(name).suffix.upper()
            if upper in TARGET_SYMBOL_NAMES or suffix in {".MAP", ".SYM", ".LST", ".OBJ"}:
                p = Path(dirpath) / name
                hits.append({"path": rel(p), "size": p.stat().st_size, "sha256": sha256(p) if p.stat().st_size < 2_000_000 else None})
    return sorted(hits, key=lambda x: x["path"])


def source_seam_audit() -> list[dict[str, Any]]:
    out = []
    for file_name, fn in SEAMS:
        p = COMMON_SOURCE / file_name
        lines = p.read_text(encoding="latin-1", errors="replace").splitlines() if p.exists() else []
        hit = next((i + 1 for i, line in enumerate(lines) if fn in line), None)
        out.append({"file": file_name, "function": fn, "source_path": rel(p), "line": hit, "ok": bool(hit)})
    return out


def build_script_audit() -> dict[str, Any]:
    mk = IBM_SOURCE / "MKII.BAT"
    text = mk.read_text(encoding="latin-1", errors="replace")
    lines = text.splitlines()
    fires_lines = []
    for idx, line in enumerate(lines, 1):
        if "I34E\\FIRES" in line or "FIRES.MAP" in line or "I34E.LNK" in line:
            fires_lines.append({"line": idx, "text": line})
    lnk = (IBM_SOURCE / "I34E.LNK").read_text(encoding="latin-1", errors="replace").splitlines()
    return {"mkii_path": rel(mk), "i34e_lnk_path": rel(IBM_SOURCE / "I34E.LNK"), "mkii_fires_lines": fires_lines, "i34e_lnk_objects": [x.strip("+") for x in lnk if x.strip()]}


def run_dosbox_build(timeout_s: int) -> dict[str, Any]:
    if not shutil.which("dosbox-x") or not shutil.which("xvfb-run"):
        return {"attempted": False, "ok": False, "reason": "dosbox-x or xvfb-run missing"}
    with tempfile.TemporaryDirectory(prefix="firestaff-pass236-i34e-") as td:
        tmp = Path(td)
        shutil.copytree(IBM_BASE, tmp, dirs_exist_ok=True)
        (tmp / "SOURCE").mkdir(exist_ok=True)
        (tmp / "OBJECT/I34E/FIRES").mkdir(parents=True, exist_ok=True)
        (tmp / "OBJECT/I34E/STATS.EXE").mkdir(parents=True, exist_ok=True)
        (tmp / "BUILD/I34E").mkdir(parents=True, exist_ok=True)
        shutil.copytree(COMMON_SOURCE, tmp / "SOURCE", dirs_exist_ok=True)
        shutil.copytree(IBM_SOURCE, tmp / "SOURCE", dirs_exist_ok=True)
        (tmp / "SOURCE/EXEID74.TXT").write_text("")
        conf = tmp / "run.conf"
        conf.write_text("\n".join([
            "[sdl]", "usescancodes=false", "[dosbox]", "memsize=32", "[cpu]", "cycles=max", "[autoexec]",
            f"mount c {tmp}", "c:", "call \\SOURCE\\MKII.BAT", "exit", "",
        ]), encoding="utf-8")
        proc = subprocess.run(["xvfb-run", "-a", "dosbox-x", "-nogui", "-silent", "-fastlaunch", "-conf", str(conf), "-exit", "-time-limit", str(timeout_s)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout_s + 30)
        log = tmp / "BUILD/MKII.LOG"
        log_text = log.read_bytes().decode("latin-1", errors="replace") if log.exists() else ""
        map_path = tmp / "BUILD/I34E/FIRES.MAP"
        objs = sorted((p.name, p.stat().st_size) for p in (tmp / "OBJECT/I34E/FIRES").glob("*.OBJ"))
        errors = []
        for m in re.finditer(r"(?:Error|Fatal)[^\r\n]{0,220}", log_text):
            msg = m.group(0)
            if msg.strip().lower() == "error messages:    none":
                continue
            errors.append(msg)
        # Save bounded text summaries only.
        (OUT_DIR / "dosbox_build_stdout_tail.txt").write_text(proc.stdout[-12000:].decode("latin-1", errors="replace"), encoding="utf-8")
        (OUT_DIR / "mkii_log_error_excerpt.txt").write_text("\n".join(errors[:40]) + "\n", encoding="utf-8")
        return {
            "attempted": True,
            "ok": map_path.exists() and map_path.stat().st_size > 128 and not errors,
            "returncode": proc.returncode,
            "tempdir_policy": "temporary build tree removed after run; repo receives text excerpts only",
            "compiled_objects": [{"name": n, "size": s} for n, s in objs],
            "fires_map_size": map_path.stat().st_size if map_path.exists() else 0,
            "dunview_obj_size": next((s for n, s in objs if n.upper() == "DUNVIEW.OBJ"), None),
            "error_excerpt_path": rel(OUT_DIR / "mkii_log_error_excerpt.txt"),
            "stdout_tail_path": rel(OUT_DIR / "dosbox_build_stdout_tail.txt"),
            "errors": errors[:12],
        }


def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass236 — DM1 PC34 FIRES symbol bridge audit", "", "Status: `{}`".format(manifest["status"]), "", "## Result", ""]
    lines.append(manifest["result"])
    lines += ["", "## Searched paths", ""]
    for p in manifest["searched_paths"]:
        lines.append(f"- `{p}`")
    lines += ["", "## Public symbol files found", ""]
    if manifest["symbol_file_hits"]:
        for hit in manifest["symbol_file_hits"]:
            lines.append("- `{}` ({} bytes)".format(hit["path"], hit["size"]))
    else:
        lines.append("- None in the focused N2-local original/ReDMCSB roots.")
    lines += ["", "## ReDMCSB build-script bridge facts", ""]
    bs = manifest["build_script"]
    lines.append("- `MKII.BAT`: `{}`".format(bs["mkii_path"]))
    lines.append("- `I34E.LNK`: `{}`".format(bs["i34e_lnk_path"]))
    lines.append("- `MKII.BAT` has an explicit `TLINK.EXE ... ,\\BUILD\\I34E\\FIRES.EXE,\\BUILD\\I34E\\FIRES.MAP` line for EXEID 74.")
    lines.append("- `I34E.LNK` order includes `MOVESENS.OBJ`, `CLIKMENU.OBJ`, `COMMAND.OBJ`, then `DUNVIEW.OBJ`.")
    lines += ["", "## DOSBox build attempt", ""]
    ba = manifest["dosbox_build_attempt"]
    if ba.get("attempted"):
        lines.append("- Return code: `{}`".format(ba.get("returncode")))
        lines.append("- `DUNVIEW.OBJ` size: `{}`".format(ba.get("dunview_obj_size")))
        lines.append("- `FIRES.MAP` size: `{}`".format(ba.get("fires_map_size")))
        lines.append("- Error excerpt: `{}`".format(ba.get("error_excerpt_path")))
        for err in ba.get("errors", [])[:4]:
            lines.append(f"  - `{err[:180]}`")
    else:
        lines.append("- Not attempted: `{}`".format(ba.get("reason")))
    lines += ["", "## Promotion decision", "", manifest["promotion_decision"], ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--attempt-build", action="store_true")
    ap.add_argument("--timeout", type=int, default=240)
    args = ap.parse_args()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    hits = []
    for root in SEARCH_ROOTS:
        hits.extend(find_files(root))
    build_attempt = run_dosbox_build(args.timeout) if args.attempt_build else {"attempted": False, "reason": "not requested"}
    manifest = {
        "schema": "pass236_dm1_v1_fires_symbol_bridge_audit.v1",
        "status": "BLOCKED_NO_PROMOTED_CSIP",
        "searched_paths": [rel(p) for p in SEARCH_ROOTS],
        "symbol_file_hits": hits,
        "source_seams": source_seam_audit(),
        "build_script": build_script_audit(),
        "dosbox_build_attempt": build_attempt,
        "result": "No existing FIRES.MAP/FIRES.SYM/listing/object bridge was found in focused N2-local roots. ReDMCSB MKII.BAT can in principle emit FIRES.MAP, but the bounded DOSBox-X build did not produce a usable map.",
        "promotion_decision": "CS:IP candidates are still blocked. The build-script TLINK line is actionable evidence for the next unblock path, but it is not a verified public-symbol/runtime bridge until FIRES.MAP is actually produced and matched to FIRES.EXENEW or debugger-observed seam hits.",
        "artifact_policy": {"no_original_binaries_committed": True, "text_only_repo_artifacts": True},
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "report": rel(REPORT), "manifest": rel(OUT_DIR / "manifest.json"), "symbol_hits": len(hits), "build_attempted": build_attempt.get("attempted")}, indent=2, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
