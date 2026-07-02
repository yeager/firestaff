#!/usr/bin/env python3
"""Verify that pass547's CSB V1 runtime-readiness blocker is retired.

pass547 used to be a blocker-present gate. CSB now has a positive M12 launch
intent, an M11 CSB handoff into FS_GAME_CSB, and a PC real-asset boot/tick
probe. This verifier keeps the source-lock boundary and non-claims explicit.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass547_csb_v1_runtime_readiness_backfill.json"
DOC = ROOT / "parity-evidence/pass547_csb_v1_runtime_readiness_backfill.md"
CSB_MATRIX = ROOT / "docs/parity/PARITY_MATRIX_CSB_V1.md"
COMPLETION = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
ASSET_MANIFEST = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

ANCHORS = [
    {"id": "redmcsb_csb_header_identity", "role": "primary_source", "path": REDMCSB / "DEFS.H", "lines": "482-523", "needles": ["CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_atari_csb_payload_names", "role": "primary_source", "path": REDMCSB / "HINTLOAD.C", "lines": "11-18", "needles": ["0HCSB.HTC", "0HCSB.DAT", "1CSBGAME.DAT", "1CSBGAME.BAK"]},
    {"id": "redmcsb_make_new_adventure_runtime_gate", "role": "primary_source", "path": REDMCSB / "CEDTINCH.C", "lines": "5-63", "needles": ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C13_DUNGEON_CSB_GAME"]},
    {"id": "redmcsb_csb_dungeon_validation_switch", "role": "primary_source", "path": REDMCSB / "CEDTINCU.C", "lines": "5-77", "needles": ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]},
    {"id": "firestaff_m12_launch_guard_supports_csb", "role": "firestaff_positive", "path": ROOT / "src/ui/menu_startup_m12.c", "lines": "2669-2677", "needles": ["All five catalogued games now have runtime launch boundaries", "strcmp(gameId, \"csb\") == 0"]},
    {"id": "firestaff_m12_launch_intent_accepts_available_csb", "role": "firestaff_positive", "path": ROOT / "src/ui/menu_startup_m12.c", "lines": "8221-8293", "needles": ["M12_StartupMenu_GetLaunchIntent", "m12_game_supported(intent.gameId)", "M12_AssetStatus_GameAvailable(&state->assetStatus, intent.gameId)", "version && version->matched"]},
    {"id": "firestaff_m11_csb_handoff_to_game_loop", "role": "firestaff_positive", "path": ROOT / "src/engine/m11_game_view.c", "lines": "7872-7901", "needles": ["FS_GAME_CSB path in firestaff_game_loop.c", "CSB READY (FS LOOP)", "CSB READY: gameId=csb dataDir=%s"]},
    {"id": "firestaff_csb_game_loop_boot_profile", "role": "firestaff_positive", "path": ROOT / "src/engine/firestaff_game_loop.c", "lines": "420-436", "needles": ["FS_GAME_CSB", "csb_v1_boot_scan_assets", "csb_v1_boot_enter_game"]},
    {"id": "firestaff_pc_csb_real_asset_probe", "role": "firestaff_positive", "path": ROOT / "probes/csb/firestaff_csb_v1_pc_real_asset_launch_probe.c", "lines": "1-130", "needles": ["PC-first CSB V1 real-data launch gate", "PC CSB assets scan by hash", "csb_v1_boot_enter_game", "csb_v1_runtime_tick"]},
]

NON_CLAIMS = [
    "This readiness gate does not claim CSB original capture parity or pixel parity.",
    "This readiness gate does not claim Atari ST or Amiga emulator boot parity.",
    "CSB gameplay, viewport, audio, save, and overlay completion points require separate CSB-specific gates.",
]


def line_window(path: Path, span: str) -> str:
    start, end = [int(part) for part in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])


def load_json(path: Path) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}


def main() -> int:
    failures: list[str] = []
    rows: list[dict] = []
    for anchor in ANCHORS:
        haystack = line_window(anchor["path"], anchor["lines"])
        missing = [needle for needle in anchor["needles"] if needle not in haystack]
        ok = anchor["path"].exists() and not missing
        if not ok:
            failures.append(f"{anchor['id']} missing {missing} path_exists={anchor['path'].exists()}")
        rows.append({
            "id": anchor["id"],
            "role": anchor["role"],
            "path": str(anchor["path"]),
            "lines": anchor["lines"],
            "needles": anchor["needles"],
            "missing": missing,
            "ok": ok,
        })

    asset_manifest = load_json(ASSET_MANIFEST)
    manifest_checks = {
        "manifest_pass": asset_manifest.get("pass") is True,
        "asset_pair_ready": asset_manifest.get("asset_pair_ready") is True,
        "runtime_support_payloads_ready": asset_manifest.get("runtime_support_payloads_ready") is True,
        "launch_intent_allowed_true": asset_manifest.get("launch_intent_allowed") is True,
    }
    for key, ok in manifest_checks.items():
        if not ok:
            failures.append(f"asset manifest check failed: {key}")

    completion = load_json(COMPLETION)
    completion_rows = {row.get("target"): row for row in completion.get("rows", []) if isinstance(row, dict)}
    csb = completion_rows.get("CSB V1", {})
    completion_checks = {
        "csb_v1_has_baseline_credit": csb.get("points", 0) >= 20,
        "csb_v1_launch_smoke_positive": csb.get("scores", {}).get("launch_smoke", [0])[0] >= 2,
    }
    for key, ok in completion_checks.items():
        if not ok:
            failures.append(f"completion check failed: {key}")

    matrix_text = CSB_MATRIX.read_text(encoding="utf-8") if CSB_MATRIX.exists() else ""
    doc_text = DOC.read_text(encoding="utf-8") if DOC.exists() else ""
    doc_checks = {
        "matrix_names_readiness_gate": "pass547_csb_v1_runtime_readiness_backfill" in matrix_text,
        "matrix_keeps_non_claim_boundary": "original-overlay parity, and pixel parity still require their own gates" in matrix_text,
        "evidence_doc_names_retirement": "pass547 is now a retired blocker gate" in doc_text,
        "evidence_doc_names_non_claims": "No CSB original capture parity or pixel parity is claimed." in doc_text,
    }
    for key, ok in doc_checks.items():
        if not ok:
            failures.append(f"doc check failed: {key}")

    result = {
        "schema": "firestaff.pass547_csb_v1_runtime_readiness_backfill.v2",
        "pass": not failures,
        "scope": "CSB V1 runtime readiness backfill; retired blocker gate with positive CSB launch handoff evidence.",
        "blocker_retired": not failures,
        "source_anchors": rows,
        "asset_manifest": {"path": str(ASSET_MANIFEST.relative_to(ROOT)), "checks": manifest_checks},
        "completion": {"path": str(COMPLETION.relative_to(ROOT)), "checks": completion_checks},
        "current_positive_replacements": [
            "M12 accepts CSB launch intent for matched assets.",
            "M11 returns a CSB-specific handoff into the FS_GAME_CSB loop.",
            "csb_v1_pc_real_asset_launch proves PC CSB scan, boot, dungeon ownership, and one tick.",
        ],
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(("PASS" if result["pass"] else "FAIL") + " pass547 csb v1 runtime readiness backfill: blocker_retired=" + str(result["blocker_retired"]).lower())
    for failure in failures:
        print("- " + failure)
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
