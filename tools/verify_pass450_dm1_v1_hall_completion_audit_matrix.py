#!/usr/bin/env python3
"""Pass450 DM1 V1 Hall of Champions completion audit matrix.

Read-only audit gate for the Hall of Champions only. It consolidates existing
source/runtime/framebuffer evidence after the route, runtime, panel,
sensor-disable, and pass449 work, and records exactly what remains blocked.
"""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass450_dm1_v1_hall_completion_audit_matrix"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
N2_HALL_ARTIFACT_ROOT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509")
N2_HALL_ARTIFACT_STATUS = "NARROWED_ORIGINAL_HALL_PANEL_VISIBLE_CANDIDATE_CLICK_NO_TRANSITION"
N2_PROMOTABLE_LABEL = "03_panel_visible_north_front_mirror"

DATA_LOCKS = [
    {
        "label": "dm1_pc34_english_graphics",
        "variant": "DM PC 3.4 English / I34E",
        "file": "GRAPHICS.DAT",
        "path": CANON_DM1 / "GRAPHICS.DAT",
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "bytes": 363417,
    },
    {
        "label": "dm1_pc34_english_dungeon",
        "variant": "DM PC 3.4 English / I34E",
        "file": "DUNGEON.DAT",
        "path": CANON_DM1 / "DUNGEON.DAT",
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "bytes": 33357,
    },
]

MATRIX = [
    {
        "area": "portrait click route",
        "status": "VERIFIED_SOURCE_AND_RUNTIME",
        "evidence": [
            "tools/verify_dm1_v1_hall_of_champions_full_source_lock.py",
            "tools/verify_v1_champion_portrait_click_source_path.py",
            "tools/verify_v1_champion_portrait_click_geometry.py",
            "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c",
        ],
        "redmcsb": ["CLIKVIEW.C:347-349", "CLIKVIEW.C:406-432", "MOVESENS.C:1501-1503", "DUNVIEW.C:525-525", "COORD.C:1693-1749"],
        "claim": "C080 viewport click normalized to the front-wall portrait box reaches C127 and F0280 when the hand/cell/alcove gates allow it.",
    },
    {
        "area": "candidate append",
        "status": "VERIFIED_SOURCE_AND_RUNTIME",
        "evidence": ["tools/verify_dm1_v1_hall_of_champions_full_source_lock.py", "test_dm1_v1_resurrection_pc34_compat.c", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["REVIVE.C:272-294"],
        "claim": "F0280 appends a temporary candidate at previous party count, sets G0299 to previous count + 1, and updates leader/action UI for first champion.",
    },
    {
        "area": "panel active/display",
        "status": "NARROWED_SOURCE_RUNTIME_AND_ORIGINAL_PANEL_VISIBLE_CONTEXT",
        "evidence": ["tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["PANEL.C:1619-1636", "PANEL.C:1654-1656", "DEFS.H:2194-2201", "DEFS.H:3774-3777"],
        "claim": "Candidate state forces graphic 40 into C101 panel with dark-green transparency; runtime panel-active flag is covered. N2 adds a hash-locked original Hall/front-mirror visible frame, but candidate panel transition/pixel parity is still blocked.",
    },
    {
        "area": "cancel removal",
        "status": "VERIFIED_SOURCE_AND_RUNTIME",
        "evidence": ["tools/verify_dm1_v1_hall_of_champions_full_source_lock.py", "test_dm1_v1_resurrection_pc34_compat.c", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["REVIVE.C:744-783"],
        "claim": "C162 closes inventory, clears candidate ordinal, decrements party count, clears status area, and redraws enabled menus.",
    },
    {
        "area": "resurrect confirm",
        "status": "VERIFIED_SOURCE_AND_RUNTIME",
        "evidence": ["tools/verify_dm1_v1_hall_of_champions_full_source_lock.py", "test_dm1_v1_resurrection_pc34_compat.c", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["REVIVE.C:785-807"],
        "claim": "C160 keeps the appended champion, clears candidate mode, unlinks possessions, and disables a mirror-square sensor.",
    },
    {
        "area": "reincarnate effects",
        "status": "VERIFIED_SOURCE_AND_RUNTIME_CORE_EFFECTS",
        "evidence": ["test_dm1_v1_resurrection_pc34_compat.c", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["REVIVE.C:806-835"],
        "claim": "C161 follows confirm semantics plus rename branch and DM1 V1 stat/skill/vital reset/halving effects; runtime covers vitals and first skill clear, pure tests cover stat increment distribution.",
    },
    {
        "area": "mirror/sensor disable semantics",
        "status": "VERIFIED_SOURCE_AND_UNIT",
        "evidence": ["tools/verify_dm1_v1_hall_mirror_sensor_disable_source_lock.py", "test_dm1_v1_resurrection_pc34_compat.c"],
        "redmcsb": ["REVIVE.C:785-799", "REVIVE.C:801-804", "DUNGEON.C:2568-2583", "MOVESENS.C:1390-1395"],
        "claim": "Finalization disables the first C03 sensor thing on the mirror square after skipping non-sensors; it intentionally does not search specifically for C127.",
    },
    {
        "area": "HUD/status text and modal blockers",
        "status": "SOURCE_LOCKED_PARTIAL_RUNTIME",
        "evidence": ["tools/verify_dm1_v1_hall_of_champions_full_source_lock.py", "tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py", "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c"],
        "redmcsb": ["COMMAND.C:2159-2184", "COMMAND.C:2336-2370", "CHAMDRAW.C:536-545", "CHAMDRAW.C:1210-1212", "REVIVE.C:744-783"],
        "claim": "Status/inventory/close/rest/save escapes and unrelated slot redraws are source locked; runtime covers party/count/panel transitions, not text/pixel HUD parity.",
    },
    {
        "area": "graphics/palette/framebuffer parity",
        "status": "BLOCKED_ORIGINAL_PROMOTABLE_FRAMES_MISSING",
        "evidence": ["tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py", "parity-evidence/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.md"],
        "redmcsb": ["PANEL.C:1619-1636", "DEFS.H:2078-2086", "DATA.C:314-319", "BASE.C:1341-1369", "MEMORY.C:2474-2525"],
        "claim": "Panel graphic identity, transparent palette index, zone/box coordinates, and original blit/expand path are source locked; original-vs-Firestaff framebuffer parity is not claimed.",
    },
    {
        "area": "original PC34 frame/crop availability",
        "status": "BLOCKED_PANEL_VISIBLE_ORIGINAL_AVAILABLE_REMAINING_TRUE_STOP_AND_SEMANTIC_FRAMES_MISSING",
        "evidence": ["/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509", "parity-evidence/verification/pass173_source_portrait_route_gate_probe", "parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json"],
        "redmcsb": ["GAMELOOP.C:80-90", "DRAWVIEW.C:709-722"],
        "claim": "N2 DOSBox artifact supplies a hash-locked original Hall/front-mirror visible frame/crop (`03_panel_visible_north_front_mirror`), but candidate_select/cancel/resurrect/reincarnate/HUD true-stop frames remain missing because candidate clicks did not visibly transition.",
    },
]

BLOCKING_STATUSES = {
    "BLOCKED_ORIGINAL_PROMOTABLE_FRAMES_MISSING",
    "BLOCKED_ORIGINAL_TRUE_STOP_AND_SEMANTIC_FRAMES_MISSING",
    "BLOCKED_PANEL_VISIBLE_ORIGINAL_AVAILABLE_REMAINING_TRUE_STOP_AND_SEMANTIC_FRAMES_MISSING",
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": proc.returncode, "outputTail": proc.stdout[-4000:]}


def source_exists(ref: str) -> bool:
    file = ref.split(":", 1)[0]
    return (REDMCSB / file).exists()


def audit_data() -> list[dict[str, Any]]:
    rows = []
    for item in DATA_LOCKS:
        path = item["path"]
        row = {k: (str(v) if isinstance(v, Path) else v) for k, v in item.items()}
        if path.exists():
            row["exists"] = True
            row["actualBytes"] = path.stat().st_size
            row["actualSha256"] = sha256(path)
            row["ok"] = row["actualBytes"] == item["bytes"] and row["actualSha256"] == item["sha256"]
        else:
            row.update({"exists": False, "ok": False})
        rows.append(row)
    return rows


def audit_artifact(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    row: dict[str, Any] = {"path": rel, "exists": path.exists()}
    if path.exists() and path.is_file():
        row["bytes"] = path.stat().st_size
        row["sha256"] = sha256(path)
    return row



def audit_n2_hall_artifact() -> dict[str, Any]:
    root = N2_HALL_ARTIFACT_ROOT
    manifest_path = root / "manifest.json"
    row: dict[str, Any] = {
        "root": str(root),
        "exists": root.is_dir(),
        "manifestPath": str(manifest_path),
        "expectedStatus": N2_HALL_ARTIFACT_STATUS,
        "promotableLabel": N2_PROMOTABLE_LABEL,
        "use": "original Hall/front-mirror visible context only; no full pixel parity claim",
    }
    errors: list[str] = []
    if not root.is_dir():
        row.update({"ok": False, "errors": [f"missing N2 Hall artifact root {root}"]})
        return row
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        manifest = {}
        errors.append(f"manifest read/parse failed: {exc}")
    source = manifest.get("source_provenance", {}) if isinstance(manifest.get("source_provenance"), dict) else {}
    required_source = {
        "DUNGEON.DAT_sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "GRAPHICS.DAT_sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "TITLE_sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    }
    row.update({"status": manifest.get("status"), "host": manifest.get("host"), "created": manifest.get("created"), "entryCount": len(manifest.get("entries", [])) if isinstance(manifest.get("entries"), list) else None, "sourceProvenance": source, "requiredSourceProvenance": required_source})
    for key, expected in required_source.items():
        if source.get(key) != expected:
            errors.append(f"source_provenance.{key} mismatch: {source.get(key)} != {expected}")
    if row.get("status") != N2_HALL_ARTIFACT_STATUS:
        errors.append(f"status mismatch: {row.get('status')} != {N2_HALL_ARTIFACT_STATUS}")
    entries = manifest.get("entries", []) if isinstance(manifest.get("entries"), list) else []
    promotable = next((entry for entry in entries if str(entry.get("pc320", "")).startswith(f"pc320/{N2_PROMOTABLE_LABEL}") or str(entry.get("viewport224x136", "")).startswith(f"viewport224x136/{N2_PROMOTABLE_LABEL}")), None)
    row["promotableEntry"] = promotable
    if not promotable:
        errors.append(f"missing promotable entry {N2_PROMOTABLE_LABEL}")
    else:
        for rel_key, hash_key in (("pc320", "pc320_sha256"), ("viewport224x136", "viewport_sha256")):
            rel_path = promotable.get(rel_key)
            path = root / rel_path if rel_path else root / "__missing__"
            expected_hash = promotable.get(hash_key)
            if not path.is_file() or not expected_hash:
                errors.append(f"missing promotable {rel_key} file/hash")
                continue
            actual = sha256(path)
            row[f"{rel_key}Artifact"] = {"path": str(path), "sha256": actual, "expectedSha256": expected_hash, "bytes": path.stat().st_size, "ok": actual == expected_hash}
            if actual != expected_hash:
                errors.append(f"artifact hash mismatch {rel_path}: {actual} != {expected_hash}")
    row["ok"] = not errors
    row["errors"] = errors
    return row

def build_manifest() -> dict[str, Any]:
    source_errors = []
    for row in MATRIX:
        for ref in row["redmcsb"]:
            if not source_exists(ref):
                source_errors.append(f"missing ReDMCSB file for {ref}")
    data = audit_data()
    n2_hall_artifact = audit_n2_hall_artifact()
    gate_runs = {
        "hallFullSourceLock": run(["python3", "tools/verify_dm1_v1_hall_of_champions_full_source_lock.py"]),
        "hallMirrorSensorDisableSourceLock": run(["python3", "tools/verify_dm1_v1_hall_mirror_sensor_disable_source_lock.py"]),
        "hallWalkaroundSourceLock": run(["python3", "tools/verify_dm1_v1_hall_walkaround_source_lock.py"]),
        "pass449CandidateFramebufferEvidenceGate": run(["python3", "tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py"]),
    }
    errors = source_errors + [f"data hash failed: {d['label']}" for d in data if not d["ok"]] + n2_hall_artifact.get("errors", [])
    errors += [name for name, result in gate_runs.items() if result["returncode"] != 0]
    verified = sum(1 for row in MATRIX if row["status"] not in BLOCKING_STATUSES)
    total = len(MATRIX)
    return {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS_WITH_BLOCKERS" if not errors else "FAIL_AUDIT_PREREQUISITE",
        "repo": str(ROOT),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "redmcsbRoot": str(REDMCSB),
        "scope": "DM1 V1 Hall of Champions only",
        "completionPercentageEvidenceBacked": round((verified / total) * 100, 1),
        "completionNumerator": verified,
        "completionDenominator": total,
        "completionRule": "Rows with BLOCKED_* statuses are excluded from completion; source/runtime-only verified rows count, but no framebuffer/original pixel parity is counted.",
        "originalData": data,
        "matrix": MATRIX,
        "n2HallDosboxArtifact": n2_hall_artifact,
        "availableOriginalScenes": ["panel_visible_north_front_mirror_context"],
        "requiredOriginalScenesStillMissing": [
            "candidate_select",
            "cancel",
            "resurrect_confirm",
            "reincarnate_confirm",
            "hud_status_after",
        ],
        "pass449FramebufferComparatorArtifacts": [
            audit_artifact("parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_manifest_schema.json"),
            audit_artifact("parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_compare.json"),
        ],
        "hallRuntimeProbeArtifacts": [
            audit_artifact("parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/hall_runtime_probe/dm1_v1_hall_walkaround_runtime_probe.json"),
            audit_artifact("parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/hall_runtime_probe/dm1_v1_hall_walkaround_runtime_probe.md"),
        ],
        "gateRuns": gate_runs,
        "errors": errors,
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{manifest['status']}`",
        f"- scope: {manifest['scope']}",
        f"- evidence-backed completion: **{manifest['completionPercentageEvidenceBacked']}%** ({manifest['completionNumerator']}/{manifest['completionDenominator']} matrix rows non-blocked)",
        "- parity claim: **not 100%**; original PC34 Hall candidate framebuffer/HUD parity remains blocked.",
        f"- redmcsb: `{manifest['redmcsbRoot']}`",
        "",
        "## Locked original data",
    ]
    for row in manifest["originalData"]:
        lines.append(f"- `{row['label']}` `{row['variant']}` `{row['file']}` sha256 `{row.get('actualSha256', row['sha256'])}` bytes `{row.get('actualBytes', row['bytes'])}` ok={row['ok']}")
    lines += ["", "## Remaining-gap matrix", "", "| area | status | ReDMCSB locks | evidence | gap |", "|---|---|---|---|---|"]
    for row in manifest["matrix"]:
        gap = "none for stated source/runtime scope"
        if row["status"] == "VERIFIED_SOURCE_AND_RUNTIME_PANEL_STATE_ONLY":
            gap = "original panel pixels/crops blocked"
        elif row["status"] == "NARROWED_SOURCE_RUNTIME_AND_ORIGINAL_PANEL_VISIBLE_CONTEXT":
            gap = "candidate panel transition pixels/crops still blocked; N2 only proves Hall/front-mirror visible context"
        elif row["status"] == "SOURCE_LOCKED_PARTIAL_RUNTIME":
            gap = "HUD/status text pixel parity and original crops blocked"
        elif row["status"] in BLOCKING_STATUSES:
            gap = "panel-visible original context exists; remaining promotable original PC34 true-stop frames/crops missing/no-transition"
        refs = ", ".join(f"`{r}`" for r in row["redmcsb"])
        ev = ", ".join(f"`{e}`" for e in row["evidence"])
        lines.append(f"| {row['area']} | `{row['status']}` | {refs} | {ev} | {gap} |")
    n2 = manifest["n2HallDosboxArtifact"]
    lines += ["", "## N2 DOSBox original Hall artifact"]
    lines.append(f"- root: `{n2['root']}` exists={n2['exists']} ok={n2['ok']}")
    lines.append(f"- status: `{n2.get('status')}` host=`{n2.get('host')}` created=`{n2.get('created')}` entries={n2.get('entryCount')}")
    sp = n2.get("sourceProvenance", {})
    lines.append(f"- DUNGEON.DAT sha256 `{sp.get('DUNGEON.DAT_sha256')}`; GRAPHICS.DAT sha256 `{sp.get('GRAPHICS.DAT_sha256')}`; TITLE sha256 `{sp.get('TITLE_sha256')}`")
    pe = n2.get("promotableEntry") or {}
    if pe:
        lines.append(f"- promotable/narrowed label `{n2['promotableLabel']}` pc320 `{pe.get('pc320')}` sha256 `{pe.get('pc320_sha256')}`")
        lines.append(f"- promotable/narrowed label `{n2['promotableLabel']}` viewport224x136 `{pe.get('viewport224x136')}` sha256 `{pe.get('viewport_sha256')}`")
    lines += ["", "## Pass449 framebuffer comparator artifacts"]
    for row in manifest["pass449FramebufferComparatorArtifacts"]:
        suffix = f" sha256 `{row['sha256']}` bytes `{row['bytes']}`" if row.get("sha256") else ""
        lines.append(f"- `{row['path']}` exists={row['exists']}{suffix}")
    lines += ["", "## Hall runtime probe artifacts"]
    for row in manifest["hallRuntimeProbeArtifacts"]:
        suffix = f" sha256 `{row['sha256']}` bytes `{row['bytes']}`" if row.get("sha256") else ""
        lines.append(f"- `{row['path']}` exists={row['exists']}{suffix}")
    lines += [
        "",
        "## Required original scenes still missing",
    ]
    for scene in manifest["requiredOriginalScenesStillMissing"]:
        lines.append(f"- `{scene}`")
    lines += [
        "",
        "## Bottom line",
        "Hall source/runtime behavior is mostly locked, including portrait route, candidate append, cancel, confirm, reincarnate core effects, and first-sensor disable semantics. N2 now contributes a hash-locked original Hall/front-mirror visible frame/crop, but candidate_select/cancel/resurrect/reincarnate/HUD true-stop frames and comparator JSON remain blocked/no-transition.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = build_manifest()
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    write_report(manifest)
    print(f"{manifest['status']} {PASS}: {manifest['completionPercentageEvidenceBacked']}%")
    print(f"wrote {MANIFEST}")
    print(f"wrote {REPORT}")
    if manifest["errors"]:
        for error in manifest["errors"]:
            print(f"- {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
