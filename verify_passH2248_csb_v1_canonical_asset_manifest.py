#!/usr/bin/env python3
"""
Pass H2248: CSB V1 Phase 7 — Canonical Asset Manifest

Canonical source-lock manifest for all CSB V1 runtime payloads.
This is the master identity record for the CSB V1 verification suite;
downstream probes consume this file for path and hash anchoring.

Payload roles:
  pair          — GRAPHICS.DAT + DUNGEON.DAT (the minimal launch pair)
  runtime_support — HCSB.DAT, HCSB.HTC, MINI.DAT (named by CSB lineage source)
  forbidden      — files that must NOT be present for valid CSB V1

Schema: firestaff.csb_v1.canonical_asset_manifest.v1
"""
from __future__ import annotations

import json
import hashlib
import os
from dataclasses import asdict, dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/passH2248_csb_v1_canonical_asset_manifest.json"
REPORT = ROOT / "parity-evidence/firestaff_csb_v1_phase7_asset_manifest_H2248.md"

# ReDMCSB source anchor
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

# CSB lineage source anchor (local clone or reference path)
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"

# Local canonical data directory (may be a symlink or copy)
LOCAL_CSB = Path.home() / ".firestaff/data/csb"

# Expected canonical hashes — derived from the verified Atari ST v2.0 HardDisk extraction.
# These are the LOCKED reference values. Any mismatch = invariant failure.
# Source: verify_csb_v1_atari_asset_pair_manifest.py (pass521 equivalent)
LOCKED_CANONICAL_HASHES = {
    # PC 3.4 English — GRAPHICS.DAT 435076 bytes
    "pc34_graphics": {
        "sha256": "3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942",
        "size": 435076,
        "role": "pair",
        "platform": "PC 3.4 English",
        "source_hashes": [
            {"path": "GRAPHICS.DAT", "sha256": "3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942"},
        ],
    },
    # PC 3.4 English — DUNGEON.DAT 2098 bytes
    "pc34_dungeon": {
        "sha256": "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba",
        "size": 2098,
        "role": "pair",
        "platform": "PC 3.4 English",
        "source_hashes": [
            {"path": "DUNGEON.DAT", "sha256": "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"},
        ],
    },
}

# Runtime support payloads — named by CSB lineage README (CSB_SRC/README lines 14-21).
# These exist in the Atari ST extraction but not locally yet.
# sha256 values from the original Atari ST v2.0 HardDisk extraction.
ATARI_ST_RUNTIME_SUPPORT = {
    "hcsb_dat": {
        "sha256": "5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc",
        "size": 30793,
        "role": "runtime_support",
        "platform": "Atari ST v2.0 HardDisk 2009-02-22 PP",
        "source_hashes": [
            {"path": "HCSB.DAT", "sha256": "5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc"},
        ],
    },
    "hcsb_htc": {
        "sha256": "1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38",
        "size": 66172,
        "role": "runtime_support",
        "platform": "Atari ST v2.0 HardDisk 2009-02-22 PP",
        "source_hashes": [
            {"path": "HCSB.HTC", "sha256": "1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38"},
        ],
    },
    "mini_dat": {
        "sha256": "61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9",
        "size": 42815,
        "role": "runtime_support",
        "platform": "Atari ST v2.0 HardDisk 2009-02-22 PP",
        "source_hashes": [
            {"path": "MINI.DAT", "sha256": "61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9"},
        ],
    },
}

# Forbidden files (must not be present in valid CSB V1 data directory)
FORBIDDEN = [
    "DMGAME.DAT",    # DM save slot — CSB uses CSBGAME.DAT
    "dmgame.dat",    # lowercase variant
    "SAVEGAME.DAT",  # DM save slot
    "savegame.dat",  # lowercase variant
]

# Source anchors
SOURCE_ANCHORS = [
    {
        "id": "redmcsb_csb_dungeon_ids",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "DEFS.H"),
        "lines": "519-523",
        "needles": ["C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "Value used in CSB MINI.DAT"],
        "claim": "CSB prison/game dungeon IDs and association with MINI.DAT.",
    },
    {
        "id": "redmcsb_save_header_format",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "SAVEHEAD.C"),
        "lines": "14-63",
        "needles": ["F0417_SAVEUTIL_GetChecksumAndObfuscate", "C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX", "128"],
        "claim": "CSB save header uses key index C29, not the DM key index C10.",
    },
    {
        "id": "csb_lineage_payload_contract",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "README") if CSB_SRC.exists() else "N/A",
        "lines": "14-21",
        "needles": ["dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"],
        "claim": "CSB runtime requires the full payload set: GRAPHICS.DAT, DUNGEON.DAT, HCSB.DAT, HCSB.HTC, MINI.DAT, CONFIG.TXT.",
    },
    {
        "id": "csb_lineage_graphics_open",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Graphics.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "1814-1915",
        "needles": ["OpenCSBgraphicsFile", "graphics.dat", "Cannot find 'graphics.dat'", "CSBgraphics.dat"],
        "claim": "CSB graphics open route is separate from DM; optional CSBgraphics.dat boundary.",
    },
    {
        "id": "csb_lineage_csbgame_save",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Chaos.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "507-623",
        "needles": ["CSBGAME", "csbgame.dat", "csbgame.bak", "SAVING NEW ADVENTURE", "MINI.DAT"],
        "claim": "CSB Utility / Make New Adventure route uses CSBGAME slots and MINI.DAT support.",
    },
    {
        "id": "csb_lineage_disk_type",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Reqdisk.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "1-30",
        "needles": ["GetDiskType_CPSB", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK"],
        "claim": "CSB disk type detection returns a format namespace distinct from DM.",
    },
]


@dataclass(frozen=True)
class AssetEntry:
    id: str
    path_hint: str  # relative to data root
    role: str  # pair | runtime_support | forbidden
    expected_sha256: str
    expected_size: int
    platform: str
    present_local: bool = False
    actual_sha256: str = ""
    actual_size: int = 0
    status: str = "absent"  # present | absent | hash_mismatch | size_mismatch

    def check(self, data_root: Path) -> "AssetEntry":
        """Verify this asset against the local data root."""
        p = data_root / self.path_hint
        if p.exists():
            actual_sha = sha256_file(p)
            actual_size = p.stat().st_size
            if actual_sha == self.expected_sha256 and actual_size == self.expected_size:
                status = "present"
            elif actual_sha != self.expected_sha256:
                status = "hash_mismatch"
            else:
                status = "size_mismatch"
            return AssetEntry(
                id=self.id,
                path_hint=self.path_hint,
                role=self.role,
                expected_sha256=self.expected_sha256,
                expected_size=self.expected_size,
                platform=self.platform,
                present_local=True,
                actual_sha256=actual_sha,
                actual_size=actual_size,
                status=status,
            )
        return self


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def check_forbidden(data_root: Path) -> list[str]:
    """Return list of forbidden filenames found in data root."""
    found = []
    for fn in FORBIDDEN:
        p = data_root / fn
        if p.exists():
            found.append(fn)
    return found


def build_manifest(data_root: Path) -> dict:
    entries = {}

    # Pair assets
    for key, info in LOCKED_CANONICAL_HASHES.items():
        entry = AssetEntry(
            id=key,
            path_hint="GRAPHICS.DAT" if "graphics" in key else "DUNGEON.DAT",
            role=info["role"],
            expected_sha256=info["sha256"],
            expected_size=info["size"],
            platform=info["platform"],
        )
        entries[key] = asdict(entry.check(data_root))

    # Runtime support assets (may not be present locally)
    for key, info in ATARI_ST_RUNTIME_SUPPORT.items():
        path_hint = info["source_hashes"][0]["path"]
        entry = AssetEntry(
            id=key,
            path_hint=path_hint,
            role=info["role"],
            expected_sha256=info["sha256"],
            expected_size=info["size"],
            platform=info["platform"],
        )
        entries[key] = asdict(entry.check(data_root))

    # Forbidden check
    forbidden_found = check_forbidden(data_root)

    return {
        "schema": "firestaff.csb_v1.canonical_asset_manifest.v1",
        "manifest_id": "passH2248_csb_v1_canonical_asset_manifest",
        "data_root": str(data_root),
        "source_anchors": SOURCE_ANCHORS,
        "assets": entries,
        "forbidden_check": {
            "forbidden_files": FORBIDDEN,
            "found": forbidden_found,
            "status": "clear" if not forbidden_found else "VIOLATION",
        },
        "summary": {
            "total_assets": len(entries),
            "present": sum(1 for e in entries.values() if e["status"] == "present"),
            "absent": sum(1 for e in entries.values() if e["status"] == "absent"),
            "hash_mismatch": sum(1 for e in entries.values() if e["status"] == "hash_mismatch"),
            "size_mismatch": sum(1 for e in entries.values() if e["status"] == "size_mismatch"),
            "runtime_support_missing": sum(
                1 for e in entries.values()
                if e["role"] == "runtime_support" and e["status"] == "absent"
            ),
        },
    }


def write_report(manifest: dict, report_path: Path):
    s = manifest["summary"]
    assets = manifest["assets"]

    lines = [
        "# CSB V1 Phase 7 — Canonical Asset Manifest\n",
        f"**Pass:** H2248\n",
        f"**Date:** 2026-05-26\n",
        f"**Schema:** `firestaff.csb_v1.canonical_asset_manifest.v1`\n",
        f"**Data root:** `{manifest['data_root']}`\n",
        "\n## Summary\n",
        f"- Total tracked assets: {s['total_assets']}\n",
        f"- Present: {s['present']}  \n",
        f"- Absent: {s['absent']}  \n",
        f"- Hash mismatch: {s['hash_mismatch']}  \n",
        f"- Size mismatch: {s['size_mismatch']}  \n",
        f"- Runtime support missing: {s['runtime_support_missing']}  \n",
        f"- Forbidden files found: {len(manifest['forbidden_check']['found'])}  \n",
        "\n## Asset Table\n",
        "\n| ID | Role | Platform | Expected SHA256 | Status |\n",
        "|----|------|---------|-----------------|--------|\n",
    ]

    for id_, info in sorted(assets.items()):
        sha = info["expected_sha256"][:16] + "..."
        lines.append(
            f"| `{id_}` | {info['role']} | {info['platform']} | `{sha}` | **{info['status']}** |\n"
        )

    # Source anchors
    lines += [
        "\n## Source Anchors\n",
    ]
    for anchor in manifest["source_anchors"]:
        lines += [
            f"### `{anchor['id']}`\n",
            f"- Source: {anchor['source']}\n",
            f"- File: `{anchor['path']}`  \n",
            f"- Lines: {anchor['lines']}\n",
            f"- Claim: {anchor['claim']}\n",
            f"- Needles: `{'`, `'.join(anchor['needles'])}`\n",
            "\n",
        ]

    report_path.write_text("".join(lines), encoding="utf-8")


def main():
    data_root = LOCAL_CSB

    manifest = build_manifest(data_root)

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote manifest to {OUT}")

    write_report(manifest, REPORT)
    print(f"Wrote report to {REPORT}")

    # Invariant checks
    failed = []
    if manifest["forbidden_check"]["status"] == "VIOLATION":
        failed.append(f"FORBIDDEN_FILES_FOUND: {manifest['forbidden_check']['found']}")

    for id_, info in manifest["assets"].items():
        if info["role"] == "pair" and info["status"] != "present":
            failed.append(f"PAIR_ASSET_MISSING: {id_} is {info['status']}")
        if info["status"] == "hash_mismatch":
            failed.append(f"HASH_MISMATCH: {id_}")

    if failed:
        print("\nINVARIANTS FAILED:")
        for f in failed:
            print(f"  FAIL: {f}")
        return 1
    else:
        print("\nAll invariants PASSED.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())