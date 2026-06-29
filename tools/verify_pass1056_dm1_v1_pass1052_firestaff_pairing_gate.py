#!/usr/bin/env python3
"""Pass1056: DM1 V1 pass1052 original <-> Firestaff pairing gate.

This is an evidence gate, not a renderer test.  It verifies that the clean
pass1052 original DOSBox viewport crops and the pass1054 nearest-neighbour
Firestaff pairing artifacts remain present and internally consistent.

Honesty boundary:
- One wall row is promoted as exact pixel parity: changed_pixels=0 / MAE=0.
- The other rows are scout candidates only; they are useful for route work but
  are not same-state parity claims.
"""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1056_dm1_v1_pass1052_firestaff_pairing_gate"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

PASS1052_DIR = ROOT / "verification-screens/pass1052-dm1-original-route-24h-turncycle"
PASS1052_CROPS = PASS1052_DIR / "viewport_224x136"
PASS1054_DIR = ROOT / "verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff"
PASS1054_MANIFEST = PASS1054_DIR / "manifest.json"
PASS1054_PAIRS = PASS1054_DIR / "pairs"

EXPECTED_ORIGINAL_CROPS = [
    "01_party_hud_original_viewport_224x136.png",
    "02_left_1_wall_original_viewport_224x136.png",
    "03_left_2_view_original_viewport_224x136.png",
    "04_left_3_view_original_viewport_224x136.png",
]
EXPECTED_ORIGINAL_SHA256 = {
    "01_party_hud_original_viewport_224x136.png": (
        "0f71683d923b015831c6e230952226fc4dadaf681ad510b319803973188ad212"
    ),
    "02_left_1_wall_original_viewport_224x136.png": (
        "8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c"
    ),
    "03_left_2_view_original_viewport_224x136.png": (
        "96bd18e5d98be4d36d3d990698d4bfc39ddac5505ce308ccf37ac9ed450c854f"
    ),
    "04_left_3_view_original_viewport_224x136.png": (
        "1cb139bdb0b1fab1d970d80913f3f4e2f0763af8338324d67939361be6c765ed"
    ),
}
EXPECTED_PASS1054_SCHEMA = "pass1054_dm1_v1_original_firestaff_viewport_wall_diff.v1"
EXPECTED_PASS1054_STATUS = "PASS1054_ONE_WALL_CROP_EXACT_MATCH_REST_SCOUT_ONLY"
EXPECTED_HONESTY_PHRASES = (
    "Only rows with exact_pixel_match=true are promoted as pixel-equal evidence",
    "nonzero rows are scout data, not same-state parity claims",
)
EXPECTED_SCOUT_ONLY_ORIGINALS = {
    "01_party_hud_original_viewport_224x136.png",
    "03_left_2_view_original_viewport_224x136.png",
    "04_left_3_view_original_viewport_224x136.png",
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def png_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:24]
    except OSError:
        return None
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    return struct.unpack(">II", data[16:24])


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def check_original_crops() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for name in EXPECTED_ORIGINAL_CROPS:
        path = PASS1052_CROPS / name
        dims = png_dims(path) if path.exists() else None
        digest = sha256(path) if path.exists() else None
        expected_digest = EXPECTED_ORIGINAL_SHA256[name]
        rows.append(
            {
                "name": name,
                "path": str(path.relative_to(ROOT)),
                "exists": path.exists(),
                "dims": list(dims) if dims else None,
                "sha256": digest,
                "expected_sha256": expected_digest,
                "sha256_matches_expected": digest == expected_digest,
                "ok": path.exists() and dims == (224, 136) and digest == expected_digest,
            }
        )
    return rows


def pair_file(label: str, suffix: str) -> Path:
    return PASS1054_PAIRS / f"{label}_{suffix}.png"


def check_pair_manifest() -> dict[str, Any]:
    if not PASS1054_MANIFEST.exists():
        return {"exists": False, "ok": False, "rows": []}
    data = load_json(PASS1054_MANIFEST)
    rows = data.get("rows", [])
    checked: list[dict[str, Any]] = []
    for row in rows:
        label = row.get("label", "")
        original = pair_file(label, "original")
        firestaff = pair_file(label, "firestaff")
        diff = pair_file(label, "diff")
        checked.append(
            {
                "original": row.get("original"),
                "best_firestaff": row.get("best_firestaff"),
                "label": label,
                "mae": row.get("mae"),
                "changed_pixels": row.get("changed_pixels"),
                "max_delta": row.get("max_delta"),
                "exact_pixel_match": bool(row.get("exact_pixel_match")),
                "original_artifact": str(original.relative_to(ROOT)),
                "firestaff_artifact": str(firestaff.relative_to(ROOT)),
                "diff_artifact": str(diff.relative_to(ROOT)),
                "artifacts_exist": original.exists() and firestaff.exists() and diff.exists(),
                "original_sha256_matches": original.exists()
                and sha256(original) == row.get("original_sha256"),
                "firestaff_sha256_matches": firestaff.exists()
                and sha256(firestaff) == row.get("firestaff_sha256"),
            }
        )
    exact = [r for r in checked if r["exact_pixel_match"]]
    exact_wall = [
        r
        for r in exact
        if r["original"] == "02_left_1_wall_original_viewport_224x136.png"
        and r["best_firestaff"] == "hall_1_4_dirE_viewport_224x136.ppm"
        and r["mae"] == 0.0
        and r["changed_pixels"] == 0
    ]
    scout_rows = [r for r in checked if not r["exact_pixel_match"]]
    scout_originals = {str(r["original"]) for r in scout_rows}
    scout_rows_nonzero = all(
        isinstance(r["changed_pixels"], int)
        and r["changed_pixels"] > 0
        and isinstance(r["max_delta"], int)
        and r["max_delta"] > 0
        and isinstance(r["mae"], (float, int))
        and float(r["mae"]) > 0.0
        for r in scout_rows
    )
    scout_only_contract_ok = (
        scout_originals == EXPECTED_SCOUT_ONLY_ORIGINALS
        and len(scout_rows) == 3
        and scout_rows_nonzero
    )
    honesty = str(data.get("honesty", ""))
    manifest_honesty_ok = all(phrase in honesty for phrase in EXPECTED_HONESTY_PHRASES)
    manifest_schema_ok = data.get("schema") == EXPECTED_PASS1054_SCHEMA
    manifest_status_ok = data.get("status") == EXPECTED_PASS1054_STATUS
    return {
        "exists": True,
        "schema": data.get("schema"),
        "expected_schema": EXPECTED_PASS1054_SCHEMA,
        "manifest_schema_ok": manifest_schema_ok,
        "status": data.get("status"),
        "expected_status": EXPECTED_PASS1054_STATUS,
        "manifest_status_ok": manifest_status_ok,
        "honesty": data.get("honesty"),
        "manifest_honesty_ok": manifest_honesty_ok,
        "row_count": len(rows),
        "rows": checked,
        "exact_match_count": len(exact),
        "has_expected_exact_wall_match": len(exact_wall) == 1,
        "scout_only_originals": sorted(scout_originals),
        "expected_scout_only_originals": sorted(EXPECTED_SCOUT_ONLY_ORIGINALS),
        "scout_rows_nonzero": scout_rows_nonzero,
        "scout_only_contract_ok": scout_only_contract_ok,
        "all_artifacts_present": all(r["artifacts_exist"] for r in checked),
        "all_pair_hashes_match": all(
            r["original_sha256_matches"] and r["firestaff_sha256_matches"]
            for r in checked
        ),
        "ok": manifest_schema_ok
        and manifest_status_ok
        and manifest_honesty_ok
        and len(rows) == 4
        and len(exact) == 1
        and len(exact_wall) == 1
        and scout_only_contract_ok
        and all(r["artifacts_exist"] for r in checked)
        and all(r["original_sha256_matches"] and r["firestaff_sha256_matches"] for r in checked),
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    pair = result["pairing"]
    lines = [
        "# Pass1056 DM1 V1 pass1052 original-to-Firestaff pairing gate",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate makes the pass1052/pass1054 capture state reproducible: four",
        "clean original PC 3.4 viewport crops exist, the pass1054 Firestaff pairing",
        "manifest exists, and the one promoted wall row remains an exact 0-pixel",
        "match. Nonzero rows stay scout-only and are not same-state parity claims.",
        "",
        "## Inputs",
        "",
        f"- Original crops: `{PASS1052_CROPS.relative_to(ROOT)}`",
        f"- Pairing artifacts: `{PASS1054_PAIRS.relative_to(ROOT)}`",
        f"- Pairing manifest: `{PASS1054_MANIFEST.relative_to(ROOT)}`",
        "",
        "## Result",
        "",
        f"- Original crops OK: `{result['original_crops_ok']}`",
        f"- Pair rows: `{pair['row_count']}`",
        f"- Exact match count: `{pair['exact_match_count']}`",
        f"- Expected exact wall match present: `{pair['has_expected_exact_wall_match']}`",
        f"- Pass1054 scout-only status pinned: `{pair['manifest_status_ok']}`",
        f"- Non-exact rows remain nonzero scout rows: `{pair['scout_only_contract_ok']}`",
        f"- Pair artifact hashes match manifest: `{pair['all_pair_hashes_match']}`",
        "",
        "| Original crop | Best Firestaff crop | MAE | Changed pixels | Status |",
        "|---|---|---:|---:|---|",
    ]
    for row in pair["rows"]:
        status = "Exact pixel match" if row["exact_pixel_match"] else "Scout only"
        lines.append(
            f"| `{row['original']}` | `{row['best_firestaff']}` | "
            f"{row['mae']} | {row['changed_pixels']} | {status} |"
        )
    lines += [
        "",
        "## Non-claims",
        "",
        "- This does not promote the three nonzero rows as same-state parity.",
        "- This does not close creature-chain or champion-panel capture gaps.",
        "- This does not replace a future debugger-observed original route transcript.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    original_rows = check_original_crops()
    pairing = check_pair_manifest()
    original_ok = all(r["ok"] for r in original_rows)
    ok = original_ok and pairing["ok"]
    result: dict[str, Any] = {
        "schema": "firestaff.parity.pass1056_dm1_v1_pass1052_firestaff_pairing_gate.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "PASS" if ok else "FAIL",
        "scope": (
            "DM1 V1 pass1052 original PC 3.4 viewport crops plus pass1054 "
            "nearest-neighbour Firestaff pairing artifacts. Exactly one wall row "
            "is promoted as 0-pixel parity; nonzero rows are scout-only."
        ),
        "original_crops_ok": original_ok,
        "original_crops": original_rows,
        "pairing": pairing,
    }
    write_outputs(result)
    if ok:
        print("PASS pass1056 DM1 pass1052 Firestaff pairing gate")
        return 0
    print("FAIL pass1056 DM1 pass1052 Firestaff pairing gate")
    print(json.dumps(result, indent=2)[:4000])
    return 1


if __name__ == "__main__":
    sys.exit(main())
