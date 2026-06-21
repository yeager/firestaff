#!/usr/bin/env python3
"""DM1 V1 creature-chain original-capture gate.

This is a blocker/contract validator.  It prepares the two original
creature-chain rows needed for later pixel comparison, while explicitly
failing any accidental parity promotion before real original DOS captures
exist.
"""
from __future__ import annotations

import json
import os
import argparse
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")


def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


RED = first_existing(
    "FIRESTAFF_REDMCSB_SOURCE",
    [
        DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    ],
)

PASS = "dm1_v1_creature_chain_original_capture_gate"
STATUS = "BLOCKED_DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_GATE_LOCKED"
CONTRACT = ROOT / "docs/parity/DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_CONTRACT.json"
GAP_DOC = ROOT / "docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md"
RUNBOOK = ROOT / "docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md"
CREATURE_RENDER = ROOT / "src/dm1/dm1_v1_creature_render_pc34_compat.c"
CREATURE_TEST = ROOT / "tests/test_dm1_v1_creature_render_pc34_compat_integration.c"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

EXPECTED_ASSETS = {
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
}

REQUIRED_FIELDS = [
    "runId",
    "routeLabel",
    "originalAssetSet.sha256.GRAPHICS.DAT",
    "originalAssetSet.sha256.DUNGEON.DAT",
    "originalFrame.rawSha256",
    "originalFrame.cropSha256",
    "originalFrame.width",
    "originalFrame.height",
    "originalViewportCrop.x",
    "originalViewportCrop.y",
    "originalViewportCrop.width",
    "originalViewportCrop.height",
    "captureClass",
    "party.mapIndex",
    "party.mapX",
    "party.mapY",
    "party.direction",
    "creature.type",
    "creature.name",
    "creature.viewSquare",
    "creature.nativeBitmapIndex",
    "firestaffFrame.viewportSha256",
]

SOURCE_LOCKS = [
    {
        "id": "f0115_creature_chain_draw_route",
        "file": "DUNVIEW.C",
        "lines": "4547-5586",
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "L0157_i_CreatureDirectionDelta = M021_NORMALIZE",
            "AL0127_i_NativeBitmapIndex = M618_GRAPHIC_FIRST_CREATURE",
            "G0222_auc_Graphic558_PaletteChanges_Creature_D2",
            "G0221_auc_Graphic558_PaletteChanges_Creature_D3",
            "If creature is viewed from the right, the side view must be flipped",
        ],
        "claim": "creature chain evidence must bind to the original F0115 pose/bitmap/palette/flip path",
    },
    {
        "id": "pc_i34e_creature_aspect_table",
        "file": "DUNVIEW.C",
        "lines": "1656-1685",
        "needles": [
            "CREATURE_ASPECT G0219_as_Graphic558_CreatureAspects",
            "{ 51, 0, 0x04, 0x65 }",
        ],
        "claim": "Trolin type 16 aspect row resolves to firstNative 51, coordinate/transparent byte 0x04, and replacement sets 0x65",
    },
    {
        "id": "pc_i34e_first_creature_graphic",
        "file": "DEFS.H",
        "lines": "2392-2392",
        "needles": ["#define M618_GRAPHIC_FIRST_CREATURE                   584"],
        "claim": "front native bitmap index for Trolin is 584 + 51 = 635",
    },
]


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = read_text(path).splitlines()
    out: list[str] = []
    for part in spec.split(","):
        first_s, last_s = part.split("-", 1) if "-" in part else (part, part)
        first, last = int(first_s), int(last_s)
        out.extend(lines[first - 1 : last])
    return "\n".join(out)


def flatten_keys(obj: Any, prefix: str = "") -> set[str]:
    if isinstance(obj, dict):
        keys: set[str] = set()
        for key, value in obj.items():
            child = f"{prefix}.{key}" if prefix else key
            keys.add(child)
            keys.update(flatten_keys(value, child))
        return keys
    return set()


def transcript_template(row: dict[str, Any], contract: dict[str, Any]) -> dict[str, Any]:
    return {
        "runId": "<original-runtime-run-id>",
        "routeLabel": row["label"],
        "originalAssetSet": {"sha256": EXPECTED_ASSETS},
        "originalFrame": {"rawSha256": "<sha256>", "cropSha256": "<sha256>", "width": 320, "height": 200},
        "originalViewportCrop": row["expectedOriginalFrame"]["viewportCrop"],
        "captureClass": row["captureClass"],
        "party": {"mapIndex": row["mapIndex"], "mapX": "<int>", "mapY": "<int>", "direction": "<0..3>"},
        "creature": {
            "type": contract["creature"]["type"],
            "name": contract["creature"]["name"],
            "viewSquare": row["viewSquare"],
            "nativeBitmapIndex": contract["creature"]["frontNativeBitmapIndex"],
        },
        "firestaffFrame": {"viewportSha256": "<known Firestaff viewport_224x136 sha256>"},
    }


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        if not path.exists():
            rows.append({**lock, "ok": False, "missing": [f"missing source file: {path}"]})
            continue
        body = compact(source_window(path, lock["lines"]))
        missing = [needle for needle in lock["needles"] if compact(needle) not in body]
        rows.append({
            "id": lock["id"],
            "file": lock["file"],
            "lines": lock["lines"],
            "ok": not missing,
            "claim": lock["claim"],
            "missing": missing,
        })
    return rows


def audit_contract(contract: dict[str, Any]) -> dict[str, Any]:
    problems: list[str] = []
    if contract.get("schema") != "firestaff.dm1_v1.creature_chain_original_capture_contract.v1":
        problems.append("schema drifted")
    if contract.get("status") != "BLOCKED_ON_REFERENCE":
        problems.append("contract must remain BLOCKED_ON_REFERENCE until original rows exist")
    for name, sha in EXPECTED_ASSETS.items():
        if contract.get("assetSet", {}).get(name) != sha:
            problems.append(f"{name} sha256 drifted")
    creature = contract.get("creature", {})
    if creature.get("type") != 16 or creature.get("name") != "Trolin":
        problems.append("creature target must remain Trolin type 16")
    if creature.get("firstNativeBitmapRelativeIndex") != 51:
        problems.append("Trolin firstNative must remain 51")
    if creature.get("frontNativeBitmapIndex") != 635:
        problems.append("Trolin front native bitmap must remain 635")
    rows = contract.get("requiredRows", [])
    labels = [row.get("label") for row in rows]
    if labels != ["creature_chain_d2c_trolin_front", "creature_chain_d1c_trolin_front"]:
        problems.append("requiredRows must be exactly the D2C then D1C Trolin rows")
    for row in rows:
        raw = row.get("expectedOriginalFrame", {}).get("raw", {})
        crop = row.get("expectedOriginalFrame", {}).get("viewportCrop", {})
        if raw != {"width": 320, "height": 200}:
            problems.append(f"{row.get('label')} raw frame must be 320x200")
        if crop != {"x": 0, "y": 33, "width": 224, "height": 136}:
            problems.append(f"{row.get('label')} viewport crop must be x=0 y=33 w=224 h=136")
        if row.get("captureClass") != "dungeon_gameplay":
            problems.append(f"{row.get('label')} captureClass must be dungeon_gameplay")
        template_keys = flatten_keys(transcript_template(row, contract))
        missing = [field for field in REQUIRED_FIELDS if field not in template_keys]
        if missing:
            problems.append(f"{row.get('label')} template fields missing: {', '.join(missing)}")
    return {"ok": not problems, "problems": problems, "requiredFields": REQUIRED_FIELDS}


def audit_docs(contract: dict[str, Any]) -> dict[str, Any]:
    gap = read_text(GAP_DOC)
    runbook = read_text(RUNBOOK)
    labels = [row["label"] for row in contract["requiredRows"]]
    checks = [
        ("gap_doc_names_missing_original", "No paired original DM1 PC 3.4 screenshot of a creature" in gap),
        ("gap_doc_requires_d2c", "creature in D2C cell" in gap),
        ("gap_doc_requires_d1c", "creature in D1C cell" in gap),
        ("runbook_scope_has_creature_chain", "creature-chain" in runbook),
        ("contract_path_documented", "DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_CONTRACT.json" in runbook),
    ]
    for label in labels:
        checks.append((f"runbook_names_{label}", label in runbook))
    return {"ok": all(ok for _, ok in checks), "checks": [{"id": cid, "ok": ok} for cid, ok in checks]}


def audit_firestaff_source_lock() -> dict[str, Any]:
    render = read_text(CREATURE_RENDER)
    test = read_text(CREATURE_TEST)
    checks = [
        ("render_has_trolin_aspect", "{ 51, 687, 0x04, 0x65, 0x0680 }" in render),
        ("render_cites_f0115", "F0115 creature draw section" in render),
        ("test_checks_creature_render_source_lock", "DM1 V1 Creature Viewport Rendering source lock" in test),
        ("test_checks_trolin_name", '"Trolin"' in render),
    ]
    return {"ok": all(ok for _, ok in checks), "checks": [{"id": cid, "ok": ok} for cid, ok in checks]}


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# DM1 V1 creature-chain original-capture gate",
        "",
        f"Status: {manifest['status']}",
        "",
        "This is a blocker gate. It prepares the two route rows needed for original DOS creature-chain comparison and does not promote pixel parity.",
        "",
        "## Required rows",
    ]
    for row in manifest["contract"]["requiredRows"]:
        lines.append(f"- {row['label']} viewSquare={row['viewSquare']} raw=320x200 viewportCrop=0,33,224,136")
    lines += ["", "## Source audit"]
    for row in manifest["sourceAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']} - {row['claim']}")
    lines += ["", "## Document audit"]
    for row in manifest["documentAudit"]["checks"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['id']}")
    lines += ["", "## Decision", "", manifest["decision"], "", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["contract"]["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems"]
        lines.extend(f"- {item}" for item in manifest["problems"])
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate the DM1 V1 creature-chain original-capture blocker contract.")
    parser.add_argument(
        "--check-only",
        action="store_true",
        help="validate without rewriting the tracked manifest/report artifacts",
    )
    args = parser.parse_args(argv)

    contract = json.loads(CONTRACT.read_text(encoding="utf-8"))
    source = audit_sources()
    contract_audit = audit_contract(contract)
    docs = audit_docs(contract)
    firestaff = audit_firestaff_source_lock()

    problems: list[str] = []
    problems.extend(f"source audit failed: {row['id']}" for row in source if not row["ok"])
    problems.extend(contract_audit["problems"])
    problems.extend(f"document audit failed: {row['id']}" for row in docs["checks"] if not row["ok"])
    problems.extend(f"Firestaff source-lock audit failed: {row['id']}" for row in firestaff["checks"] if not row["ok"])

    status = STATUS if not problems else "FAIL_DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_GATE"
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": not problems,
        "contractPath": str(CONTRACT.relative_to(ROOT)),
        "contract": contract,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "contractAudit": contract_audit,
        "documentAudit": docs,
        "firestaffSourceLockAudit": firestaff,
        "decision": "The creature-chain original comparison route is narrowed to two Trolin viewport rows, D2C then D1C, with canonical PC 3.4 asset hashes and 320x200/raw plus 224x136 viewport-crop requirements. The lane remains blocked on real original DOS screenshots.",
        "problems": problems,
    }
    if not args.check_only:
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
