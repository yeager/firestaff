#!/usr/bin/env python3
"""Verify the CSB V1 Phase 0 provenance/source gate.

This is an evidence-only gate. It pins the exact N2-local source/reference and
original-data inputs that future CSB V1 work may consume, while keeping launch,
runtime, rendering, save compatibility, and gameplay claims blocked until later
CSB-specific gates supply runtime proof.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_phase0_provenance_gate.json"
REPORT = ROOT / "parity-evidence/csb_v1_phase0_provenance_gate_20260520.md"

REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM"
CANON = ORIG / "_canonical/csb"
ATARI_HD = ORIG / "_extracted/csb-atari/HardDisk/2009-02-22 PP"

EXPECTED_GIT = {
    "secondary_csb_lineage": {
        "role": "secondary_source",
        "path": CSB_SRC,
        "expected_head": "dda570585abb4c8113a3298d21c0b599e6cac4f9",
        "why": "CSB lineage reference for workflow/payload names only; ReDMCSB remains primary where it has CSB coverage.",
    },
    "secondary_csbwin_lineage": {
        "role": "secondary_source",
        "path": CSBWIN,
        "expected_head": "2f63d10d9b8c155e0be17888271d394255ce1bac",
        "why": "CSBWin reference for workflow/load behavior only; not a substitute for original Atari/Amiga assets.",
    },
}

EXPECTED_FILES = {
    "redmcsb_defs_h": ("primary_source", REDMCSB / "DEFS.H", 479889, "33b3160a5dd7d9c62b9ad0f3d26ac09ddacd6328c9717656fe2bbe8786625728"),
    "redmcsb_new_adventure_gate": ("primary_source", REDMCSB / "CEDTINCH.C", 2688, "a54c7be10052f89f923597a22a19ca6441ca005ddfef4adb651b56d85c471690"),
    "redmcsb_dungeon_validation": ("primary_source", REDMCSB / "CEDTINCU.C", 2962, "67169cab3c30b2acd18481a867c5692c333747966602bbc4027a355bdcf5e219"),
    "redmcsb_atari_csb_loader": ("primary_source", REDMCSB / "HINTLOAD.C", 22631, "74bee0f78640d6f9a2dec64d66285e4e96942d16606ac3e1b70ff18b8ecf9723"),
    "redmcsb_save_routing": ("primary_source", REDMCSB / "CEDTINC8.C", 20504, "f6acc8c39b5525227b0ece435e87eb787ec054228164e9c6af51953b376d1b07"),
    "csb_lineage_readme": ("secondary_source", CSB_SRC / "README", 972, "ac62c130353221fc2599bf6e045d09910ab498d4055c8e35893a22153421788a"),
    "csb_lineage_chaos_cpp": ("secondary_source", CSB_SRC / "Chaos.cpp", 180270, "e79895b6ed1ad5620de790ea80f2a0ad197fa32881a7da41d30db3740b6d9891"),
    "csb_lineage_graphics_cpp": ("secondary_source", CSB_SRC / "Graphics.cpp", 82105, "938e6b058fcd9449ece41789df880e1466e1cf3a2083a44a68b1d5b4a8f96430"),
    "csbwin_game_readme": ("secondary_source", CSBWIN / "Game/readme.txt", 714, "0e3943251e85c248de5c560dc96d2e6a7f6fc5c1a175331210c76da386d76966"),
    "csbwin_csbwin_cpp": ("secondary_source", CSBWIN / "CSBwin.cpp", 48281, "89418e01b0a8eef330451320d19078a3510cbc699f635c8af22820365e4ceb23"),
    "csbwin_savegame_cpp": ("secondary_source", CSBWIN / "SaveGame.cpp", 87215, "f0009ba8235509e25d9dbea439daf0b68ac29cfa0dcd6241bba876b1f69cd0f2"),
    "canonical_csb_readme": ("original_provenance", CANON / "README.md", 2420, "5a810e1f971223668a772d99f5e925859580a8e5a4c05997971fbc48b3ca3134"),
    "canonical_atari_graphics": ("original_asset", CANON / "atari-GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"),
    "canonical_atari_dungeon": ("original_asset", CANON / "atari-DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
    "canonical_amiga_graphics": ("original_asset_split_reference", CANON / "amiga-Graphics.DAT", 435076, "3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942"),
    "canonical_amiga_dungeon": ("original_asset_split_reference", CANON / "amiga-Dungeon.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
    "selected_atari_hcsb_dat": ("original_runtime_support", ATARI_HD / "HCSB.DAT", 30793, "5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc"),
    "selected_atari_hcsb_htc": ("original_runtime_support", ATARI_HD / "HCSB.HTC", 66172, "1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38"),
    "selected_atari_mini_dat": ("original_runtime_support", ATARI_HD / "MINI.DAT", 42815, "61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9"),
}

EXPECTED_LINKS = {
    "canonical_atari_graphics": (CANON / "atari-GRAPHICS.DAT", ATARI_HD / "GRAPHICS.DAT"),
    "canonical_atari_dungeon": (CANON / "atari-DUNGEON.DAT", ATARI_HD / "DUNGEON.DAT"),
    "canonical_amiga_graphics": (CANON / "amiga-Graphics.DAT", ORIG / "_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT"),
    "canonical_amiga_dungeon": (CANON / "amiga-Dungeon.DAT", ORIG / "_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Dungeon.DAT"),
}

SOURCE_ANCHORS = [
    ("redmcsb_primary_csb_save_and_dungeon_ids", REDMCSB / "DEFS.H", "468-523", ["CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    ("redmcsb_primary_new_adventure_gate", REDMCSB / "CEDTINCH.C", "5-63", ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "C13_DUNGEON_CSB_GAME"]),
    ("redmcsb_primary_dungeon_validation", REDMCSB / "CEDTINCU.C", "5-77", ["F7272_IsDungeonValid", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    ("redmcsb_primary_atari_csb_loader", REDMCSB / "HINTLOAD.C", "11-18", ["0HCSB.HTC", "0HCSB.DAT", "1CSBGAME.DAT", "1CSBGAME.BAK"]),
    ("redmcsb_primary_save_routing", REDMCSB / "CEDTINC8.C", "101-118", ["M745_FILE_ID_SAVE_DMSAVE_DAT", "M746_FILE_ID_SAVE_CSBGAME_DAT", "C12_DUNGEON_CSB_PRISON"]),
    ("csb_secondary_required_payloads", CSB_SRC / "README", "14-21", ["dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"]),
    ("csb_secondary_graphics_boundary", CSB_SRC / "Graphics.cpp", "1814-1915", ["openGraphicsFile", "graphics.dat", "OpenCSBgraphicsFile", "CSBgraphics.dat"]),
    ("csb_secondary_new_adventure_slots", CSB_SRC / "Chaos.cpp", "507-623", ["CSBGAME", "csbgame.dat", "csbgame.bak", "SAVING NEW ADVENTURE", "MINI.DAT"]),
    ("csbwin_secondary_workflow", CSBWIN / "Game/readme.txt", "1-30", ["Enter the dungeon", "choose prison", "Make New Adventure", "Restore the newly created save game"]),
]

EXPECTED_ABSENT = {
    "selected_atari_config_txt": (ATARI_HD / "CONFIG.TXT", "CSB lineage names config.txt, but the selected original-data anchor does not provide it."),
    "selected_atari_csbgame_dat": (ATARI_HD / "CSBGAME.DAT", "No curated CSB saved-game/New Adventure fixture exists in the selected original-data anchor."),
    "selected_atari_csbgame_bak": (ATARI_HD / "CSBGAME.BAK", "No curated CSB saved-game backup fixture exists in the selected original-data anchor."),
}

NON_CLAIMS = [
    "No CSB launch/runtime/render/gameplay/save-compatibility/pixel-parity support is enabled or claimed.",
    "ReDMCSB is the primary source wherever it has CSB-specific coverage; CSB and CSBWin are secondary references only.",
    "Atari ST and Amiga graphics are both hash-locked because they differ; they must not be substituted for each other.",
    "The selected Atari ST asset/support set is necessary provenance for future work, not launch clearance.",
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def git_head(path: Path) -> str | None:
    try:
        return subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()
    except Exception:
        return None


def line_window(path: Path, span: str) -> str:
    start, end = [int(part) for part in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])


def relabel(path: Path) -> str:
    replacements = (
        (REDMCSB, "<redmcsb-primary>/Toolchains/Common/Source"),
        (CSB_SRC, "<csb-secondary>/CSB/src"),
        (CSBWIN, "<csbwin-secondary>/CSBWin"),
        (ORIG, "<original-games>/DM"),
        (ROOT, "<firestaff-repo>"),
    )
    resolved = path.absolute()
    for base, label in replacements:
        try:
            rel = resolved.relative_to(base.absolute())
        except ValueError:
            continue
        return label if str(rel) == "." else f"{label}/{rel}"
    return str(path)


def markdown_report(result: dict) -> str:
    lines = [
        "# CSB V1 Phase 0 provenance gate - 2026-05-20",
        "",
        "Scope: evidence-only source/provenance gate for future CSB V1 work on N2. This does not enable launch, runtime, rendering, gameplay, save compatibility, or pixel parity.",
        "",
        f"Status: {'PASS' if result['pass'] else 'FAIL'}",
        "",
        "## Source roles",
        "",
        "- Primary: ReDMCSB `Toolchains/Common/Source` for CSB save headers, dungeon IDs, dungeon validation, New Adventure readiness, and Atari CSB loader/save routing.",
        "- Secondary: CSB lineage `CSB/src` and CSBWin for workflow/payload/load boundaries only.",
        "- Original data: N2 `firestaff-original-games/DM` canonical CSB anchors plus curated Atari extraction support payloads.",
        "",
        "## Locked source trees",
        "",
    ]
    for row in result["git_refs"]:
        lines.append(f"- `{row['id']}`: `{row['actual_head']}` ok={row['ok']} ({row['path']})")
    lines.extend(["", "## Locked files", ""])
    for row in result["file_refs"]:
        lines.append(f"- `{row['id']}` `{row['role']}` bytes={row['actual_size']} sha256=`{row['actual_sha256']}` ok={row['ok']} path=`{row['path']}`")
    lines.extend(["", "## Canonical symlinks", ""])
    for row in result["link_refs"]:
        lines.append(f"- `{row['id']}` ok={row['ok']} link=`{row['path']}` target=`{row['actual_target']}`")
    lines.extend(["", "## Source anchors", ""])
    for row in result["source_anchors"]:
        lines.append(f"- `{row['id']}` {row['path']}:{row['lines']} ok={row['ok']}")
    lines.extend(["", "## Blockers recorded, not guessed", ""])
    for row in result["expected_absent"]:
        lines.append(f"- `{row['id']}` absent={row['absent']} path=`{row['path']}` - {row['blocker']}")
    lines.extend(["", "## Non-claims", ""])
    for claim in result["non_claims"]:
        lines.append(f"- {claim}")
    lines.extend(["", "## Gate", "", "```sh", "python3 -m py_compile tools/verify_csb_v1_phase0_provenance_gate.py", "python3 tools/verify_csb_v1_phase0_provenance_gate.py", "python3 -m json.tool parity-evidence/verification/csb_v1_phase0_provenance_gate.json >/dev/null", "```", "", "JSON output: `parity-evidence/verification/csb_v1_phase0_provenance_gate.json`.", ""])
    return "\n".join(lines)


def main() -> int:
    default_roots_ready = (
        all(spec["path"].is_dir() for spec in EXPECTED_GIT.values())
        and all(path.exists() for _, path, _, _ in EXPECTED_FILES.values())
        and all(link.is_symlink() and expected.exists() for link, expected in EXPECTED_LINKS.values())
    )
    if not default_roots_ready:
        print("SKIP default CSB provenance roots are not available on this host")
        return 0

    failures: list[str] = []

    git_rows = []
    for ident, spec in EXPECTED_GIT.items():
        actual = git_head(spec["path"])
        ok = actual == spec["expected_head"]
        if not ok:
            failures.append(f"{ident} HEAD mismatch: expected {spec['expected_head']}, actual {actual}")
        git_rows.append({"id": ident, "role": spec["role"], "path": relabel(spec["path"]), "expected_head": spec["expected_head"], "actual_head": actual, "why": spec["why"], "ok": ok})

    file_rows = []
    for ident, (role, path, size, digest) in EXPECTED_FILES.items():
        exists = path.exists()
        actual_size = path.stat().st_size if exists else None
        actual_digest = sha256(path) if exists else None
        ok = exists and actual_size == size and actual_digest == digest
        if not ok:
            failures.append(f"{ident} mismatch: exists={exists} size={actual_size} sha256={actual_digest}")
        file_rows.append({"id": ident, "role": role, "path": relabel(path), "expected_size": size, "actual_size": actual_size, "expected_sha256": digest, "actual_sha256": actual_digest, "ok": ok})

    link_rows = []
    for ident, (link, expected_target) in EXPECTED_LINKS.items():
        actual_target = link.resolve() if link.exists() else None
        ok = link.is_symlink() and actual_target == expected_target
        if not ok:
            failures.append(f"{ident} link mismatch: is_symlink={link.is_symlink()} actual_target={actual_target}")
        link_rows.append({"id": ident, "path": relabel(link), "expected_target": relabel(expected_target), "actual_target": relabel(actual_target) if actual_target else None, "ok": ok})

    source_rows = []
    for ident, path, span, needles in SOURCE_ANCHORS:
        haystack = line_window(path, span)
        missing = [needle for needle in needles if needle not in haystack]
        ok = path.exists() and not missing
        if not ok:
            failures.append(f"{ident} missing {missing} path_exists={path.exists()}")
        source_rows.append({"id": ident, "path": relabel(path), "lines": span, "needles": needles, "missing": missing, "ok": ok})

    absent_rows = []
    for ident, (path, blocker) in EXPECTED_ABSENT.items():
        absent = not path.exists()
        if not absent:
            failures.append(f"{ident} unexpectedly exists at {path}; update blocker/provenance before using it")
        absent_rows.append({"id": ident, "path": relabel(path), "absent": absent, "blocker": blocker})

    result = {
        "schema": "firestaff.csb_v1_phase0_provenance_gate.v1",
        "pass": not failures,
        "scope": "CSB V1 Phase 0 provenance/source/reference gate only.",
        "selected_lane": "csb_atari_st_v2x_harddisk_2009_02_22_pp",
        "primary_source": relabel(REDMCSB),
        "secondary_sources": [relabel(CSB_SRC), relabel(CSBWIN)],
        "original_data_root": relabel(ORIG),
        "launch_runtime_claim_allowed": False,
        "git_refs": git_rows,
        "file_refs": file_rows,
        "link_refs": link_rows,
        "source_anchors": source_rows,
        "expected_absent": absent_rows,
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    REPORT.write_text(markdown_report(result), encoding="utf-8")

    status = "PASS" if result["pass"] else "FAIL"
    print(f"{status} csb v1 phase0 provenance gate: files={len(file_rows)} source_anchors={len(source_rows)} blockers={len(absent_rows)} launch_runtime_claim_allowed=false")
    for failure in failures:
        print("- " + failure)
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
