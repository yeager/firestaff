#!/usr/bin/env python3
"""Pass1057: DM1 Amiga 2.2 English DUNGEONB.DAT asset lock.

This is a data-provenance gate, not a runtime parity claim. It verifies the
newly extracted Amiga 2.2 English "kid" dungeon sidecar file when local game
data is present, and always verifies that the SHA256/size are documented in
the checked-in registry.
"""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1057_dm1_amiga22_dungeonb_asset_lock"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

DATA_DIR = Path(os.environ.get("FIRESTAFF_DATA", Path.home() / ".firestaff" / "data"))
EXPECTED_REL = Path("dm1-extras/amiga-2.2-en/DUNGEONB.DAT")
EXPECTED_SIZE = 4806
EXPECTED_SHA256 = "9bac133b4d8d6ca88abad70ff4a3a6436f264e3ae3a7503e0b40a8a6b4007730"
EXPECTED_MD5 = "d42915cf346494efa0ed78cfbbb4c2b5"


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def md5_file(path: Path) -> str:
    h = hashlib.md5()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def registry_status() -> dict[str, Any]:
    text = (ROOT / "docs/VERIFIED_HASHES.md").read_text(encoding="utf-8")
    return {
        "has_expected_path": f"`{EXPECTED_REL}`" in text,
        "has_expected_sha256": EXPECTED_SHA256 in text,
        "has_expected_size": "4,806" in text or "4806" in text,
    }


def docs_status() -> dict[str, Any]:
    bonus = (ROOT / "docs/source-lock/variants_bonus.md").read_text(encoding="utf-8")
    checklist = (ROOT / "docs/DATA_ACQUISITION_CHECKLIST.md").read_text(encoding="utf-8")
    return {
        "variants_bonus_mentions_amiga22_candidate": "Amiga 2.2 English" in bonus
        and EXPECTED_SHA256 in bonus,
        "checklist_marks_amiga22_ready": "Amiga 2.2 English (kid) | ✅ | ✅" in checklist
        and "DUNGEONB.DAT" in checklist,
    }


def coverage_status() -> dict[str, Any]:
    tool = ROOT / "tools/asset-validate/coverage_by_game.py"
    proc = subprocess.run(
        [sys.executable, str(tool), "--json"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=30,
    )
    out: dict[str, Any] = {
        "returncode": proc.returncode,
        "ok": False,
        "dm1_ready": None,
        "row": None,
        "stderr": proc.stderr.strip(),
    }
    if proc.returncode != 0:
        return out
    data = json.loads(proc.stdout)
    out["dm1_ready"] = data.get("summary", {}).get("dm1", {}).get("ready")
    for row in data.get("rows", []):
        if row.get("game") == "dm1" and row.get("variant") == "2.2 English (kid)":
            out["row"] = row
            out["ok"] = (
                row.get("status") == "READY"
                and row.get("have_files") == 3
                and row.get("need_files") == 3
                and row.get("in_registry") == 3
            )
            break
    return out


def local_asset_status() -> dict[str, Any]:
    path = DATA_DIR / EXPECTED_REL
    if not DATA_DIR.exists():
        return {"status": "SKIP", "reason": f"data dir missing: {DATA_DIR}", "path": str(path)}
    if not path.exists():
        return {"status": "FAIL", "reason": f"missing {path}", "path": str(path)}
    size = path.stat().st_size
    sha = sha256_file(path)
    md5 = md5_file(path)
    case_variants: list[dict[str, Any]] = []
    for name in ("DUNGEONB.DAT", "DungeonB.dat", "dungeonb.dat"):
        p = path.parent / name
        case_variants.append(
            {
                "name": name,
                "exists": p.exists(),
                "size": p.stat().st_size if p.exists() else None,
                "sha256": sha256_file(p) if p.exists() else None,
            }
        )
    return {
        "status": "PASS" if size == EXPECTED_SIZE and sha == EXPECTED_SHA256 and md5 == EXPECTED_MD5 else "FAIL",
        "path": str(path),
        "size": size,
        "sha256": sha,
        "md5": md5,
        "case_variants": case_variants,
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    local = result["local_asset"]
    lines = [
        "# Pass1057 DM1 Amiga 2.2 English DUNGEONB.DAT asset lock",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate locks the local DM1 Amiga 2.2 English kid-dungeon sidecar",
        "as data provenance only. It does not claim DM1 PC34 runtime parity and",
        "does not route the Amiga dungeon through the DM1 V1 runtime.",
        "",
        "## Locked file",
        "",
        f"- Relative path: `{EXPECTED_REL}`",
        f"- Size: `{EXPECTED_SIZE}` bytes",
        f"- SHA256: `{EXPECTED_SHA256}`",
        f"- MD5: `{EXPECTED_MD5}`",
        f"- Local check: `{local['status']}`",
        "",
        "## Coverage",
        "",
        f"- Registry row present: `{result['registry']['has_expected_path']}`",
        f"- DM1 Amiga 2.2 coverage row ready: `{result['coverage']['ok']}`",
        "",
        "## Non-claims",
        "",
        "- This is not the 2,098-byte CSB dungeon hash used by the Greatstone kid map cross-link.",
        "- This does not contradict the PC 3.4 DM1 V1 no-bonus-content runtime claim.",
        "- This does not promote Amiga 2.2 gameplay parity.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    registry = registry_status()
    docs = docs_status()
    coverage = coverage_status()
    local = local_asset_status()
    local_ok = local["status"] in {"PASS", "SKIP"}
    ok = (
        registry["has_expected_path"]
        and registry["has_expected_sha256"]
        and registry["has_expected_size"]
        and docs["variants_bonus_mentions_amiga22_candidate"]
        and docs["checklist_marks_amiga22_ready"]
        and coverage["ok"]
        and local_ok
    )
    result: dict[str, Any] = {
        "schema": "firestaff.parity.pass1057_dm1_amiga22_dungeonb_asset_lock.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "PASS" if ok else "FAIL",
        "registry": registry,
        "docs": docs,
        "coverage": coverage,
        "local_asset": local,
    }
    write_outputs(result)
    if ok:
        print(f"PASS {PASS} status={local['status']}")
        return 0
    print(f"FAIL {PASS}", file=sys.stderr)
    print(json.dumps(result, indent=2), file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
