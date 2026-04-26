#!/usr/bin/env python3
"""CSB V1 bootstrap scout.

Static, evidence-only probe for the first CSB V1 parity pass. It intentionally
checks source files and documentation markers rather than rendering or changing
runtime semantics.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Check:
    check_id: str
    ok: bool
    detail: str


def read_rel(rel: str) -> str:
    path = ROOT / rel
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="latin-1")


def exists_rel(rel: str) -> bool:
    return (ROOT / rel).exists()


def grep_files(pattern: str, rels: list[str]) -> list[str]:
    rx = re.compile(pattern)
    hits: list[str] = []
    for rel in rels:
        path = ROOT / rel
        if not path.exists():
            continue
        for idx, line in enumerate(read_rel(rel).splitlines(), 1):
            if rx.search(line):
                hits.append(f"{rel}:{idx}:{line.strip()}")
    return hits


def main() -> int:
    checks: list[Check] = []

    menu = read_rel("menu_startup_m12.c")
    asset = read_rel("asset_status_m12.c")
    config_h = read_rel("config_m12.h")

    checks.append(Check(
        "CSB_SLOT_PRESENT",
        "M12_CONFIG_GAME_COUNT = 3  /* DM1, CSB, DM2 */" in config_h and "strcmp(gameId, \"csb\") == 0" in menu,
        "startup/config model carries a CSB game slot",
    ))
    checks.append(Check(
        "CSB_LAUNCH_GATED",
        "return gameId && strcmp(gameId, \"dm1\") == 0;" in menu,
        "m12_game_supported() still gates launch support to DM1 only",
    ))
    checks.append(Check(
        "CSB_GRAPHICS_HASHES_ONLY",
        all(token in asset for token in ["g_csbVersions", "CSBGRAPH.DAT", "M12_AssetStatus_GameRequiredFileCount", "return spec && spec->versionCount > 0U ? 1U : 0U;"]),
        "asset status recognizes CSB graphics hashes but required-file count is one file, not graphics+dungeon",
    ))
    checks.append(Check(
        "DM1_SOURCE_LOCK_TOOL_ONLY",
        exists_rel("tools/greatstone_dm1_source_lock_check.py") and not exists_rel("tools/greatstone_csb_source_lock_check.py"),
        "DM1 has a source-lock helper; CSB does not yet have an equivalent helper",
    ))
    checks.append(Check(
        "DM1_CAPTURE_ONLY",
        exists_rel("scripts/dosbox_dm1_capture.sh") and not exists_rel("scripts/dosbox_csb_capture.sh"),
        "DOSBox/original capture tooling is DM1-only today",
    ))
    checks.append(Check(
        "NO_CSB_RENDERING_PROBE",
        not exists_rel("probes/v1/firestaff_csb_v1_runtime_probe.c"),
        "no CSB V1 rendering/runtime probe exists yet, so first evidence step should stay asset/source-only",
    ))

    source_hits = grep_files(r"CSB|csb|Chaos Strikes Back|CSBGRAPH|CSB\.DAT", [
        "menu_startup_m12.c",
        "asset_status_m12.c",
        "probes/m12/firestaff_m12_startup_menu_probe.c",
        "vga_palette_pc34_compat.h",
    ])

    report = {
        "scope": "CSB V1 bootstrap scout, static evidence only",
        "checks": [check.__dict__ for check in checks],
        "recommended_first_safe_step": "Add a CSB source-lock/asset-manifest probe that verifies CSBGRAPH.DAT plus the matching dungeon/runtime data by hash before enabling launch or rendering paths.",
        "source_hits": source_hits,
    }

    print(json.dumps(report, indent=2, sort_keys=True))
    failed = [check for check in checks if not check.ok]
    if failed:
        print("CSB_V1_BOOTSTRAP_SCOUT: FAIL", file=sys.stderr)
        return 1
    print("CSB_V1_BOOTSTRAP_SCOUT: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
