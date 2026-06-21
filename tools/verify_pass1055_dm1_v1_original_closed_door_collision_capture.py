#!/usr/bin/env python3
"""Pass1055: DM1 V1 original closed-door collision capture gate.

This is an evidence gate, not a full collision-parity claim. It verifies that
the tracked original PC 3.4 closed-door capture still contains three identical
closed-door raw frames and three identical 224x136 viewport crops, then runs the
Firestaff semantic pair probe that reaches the matching closed-door block.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1055_dm1_v1_original_closed_door_collision_capture"
EVIDENCE_DIR = ROOT / "verification-screens/pass1055-dm1-original-closed-door-collision"
RAW_HEALTH = EVIDENCE_DIR / "raw_frame_health.json"
CLASSIFIER = EVIDENCE_DIR / "pass80_original_frame_classifier.json"
VIEWPORT_MANIFEST = EVIDENCE_DIR / "original_viewport_224x136_manifest.tsv"
SHOT_LABELS = EVIDENCE_DIR / "original_viewport_shot_labels.tsv"
SCAFFOLD = EVIDENCE_DIR / "pass513_i34e_route_key_transcript_scaffold.json"
REPORT = ROOT / "parity-evidence/pass1055_dm1_v1_original_closed_door_collision_gate.md"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
PROBE = ROOT / "build/firestaff_dm1_v1_pass1055_closed_door_pair_probe"

RAW_START_SHA = "40c678403d8f772822c1301bafa373adb0862915a8239d2bdb15f71fccf4b750"
RAW_CLOSED_SHA = "a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6"
VIEW_START_SHA = "3cffaf384e041c349ea3c2f0d9d4b27be86b41a1c713e7a8698cc3eefe23ffa2"
VIEW_CLOSED_SHA = "93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6"
EXPECTED_LABELS = ["start", "door_before", "after_viewport_click", "after_kp5"]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def parse_tsv(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        return rows
    header = lines[0].split("\t")
    for line in lines[1:]:
        if not line.strip():
            continue
        values = line.split("\t")
        rows.append(dict(zip(header, values)))
    return rows


def run_probe() -> dict[str, Any]:
    if not PROBE.exists():
        return {"exists": False, "ok": False, "returncode": None, "stdout": "", "stderr": "probe executable missing"}
    proc = subprocess.run([str(PROBE)], cwd=ROOT, text=True, capture_output=True, check=False)
    stdout = proc.stdout
    ok = (
        proc.returncode == 0
        and "result=PASS" in stdout
        and "resultCode=2 blocked=1 anyMove=0 pos=(0,6,9,3)" in stdout
        and "square=0x94 type=4 passable=0" in stdout
    )
    return {
        "exists": True,
        "ok": ok,
        "returncode": proc.returncode,
        "stdout": stdout,
        "stderr": proc.stderr,
    }


def check_raw_frames(raw: dict[str, Any], classifier: dict[str, Any]) -> dict[str, Any]:
    captures = raw.get("captures", [])
    hashes = [c.get("sha256") for c in captures]
    classes = [c.get("classification") for c in classifier.get("captures", [])]
    ok = (
        raw.get("pass") is True
        and raw.get("captureCount") == 4
        and all(c.get("width") == 320 and c.get("height") == 200 for c in captures)
        and hashes == [RAW_START_SHA, RAW_CLOSED_SHA, RAW_CLOSED_SHA, RAW_CLOSED_SHA]
        and classes == ["dungeon_gameplay", "wall_closeup", "wall_closeup", "wall_closeup"]
        and classifier.get("duplicate_sha256_counts", {}).get(RAW_CLOSED_SHA) == 3
    )
    return {
        "ok": ok,
        "capture_count": raw.get("captureCount"),
        "hashes": hashes,
        "classes": classes,
        "closed_raw_stasis": hashes[1:] == [RAW_CLOSED_SHA, RAW_CLOSED_SHA, RAW_CLOSED_SHA],
    }


def check_viewport_rows() -> dict[str, Any]:
    rows = parse_tsv(VIEWPORT_MANIFEST)
    labels = parse_tsv(SHOT_LABELS)
    hashes = [r.get("sha256") for r in rows]
    label_names = [r.get("route_label") for r in labels]
    ok = (
        len(rows) == 4
        and len(labels) == 4
        and label_names == EXPECTED_LABELS
        and all(r.get("width") == "224" and r.get("height") == "136" for r in rows)
        and hashes == [VIEW_START_SHA, VIEW_CLOSED_SHA, VIEW_CLOSED_SHA, VIEW_CLOSED_SHA]
        and all((EVIDENCE_DIR / "viewport_224x136" / r.get("filename", "")).exists() for r in rows)
    )
    return {
        "ok": ok,
        "labels": label_names,
        "hashes": hashes,
        "closed_viewport_stasis": hashes[1:] == [VIEW_CLOSED_SHA, VIEW_CLOSED_SHA, VIEW_CLOSED_SHA],
        "rows": rows,
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass1055 DM1 V1 original closed-door collision gate",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate makes the pass1055 closed-door stasis evidence reproducible.",
        "It verifies original raw-frame stasis, original 224x136 viewport stasis,",
        "the pass513 scaffold boundary, and the Firestaff-side semantic closed-door",
        "probe. It is not a Firestaff-vs-original pixel comparison.",
        "",
        "## Result",
        "",
        f"- Raw closed-door stasis: `{result['raw_frames']['closed_raw_stasis']}`",
        f"- Viewport closed-door stasis: `{result['viewport']['closed_viewport_stasis']}`",
        f"- Pass513 remains scaffold-only: `{result['scaffold_ok']}`",
        f"- Firestaff closed-door pair probe: `{result['firestaff_probe']['ok']}`",
        "",
        "## Non-claims",
        "",
        "- This does not prove all wall/door/fakewall collision parity.",
        "- This does not add a Firestaff-vs-original pixel diff.",
        "- This does not unblock the creature-chain capture route.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    raw = load_json(RAW_HEALTH) if RAW_HEALTH.exists() else {}
    classifier = load_json(CLASSIFIER) if CLASSIFIER.exists() else {}
    scaffold = load_json(SCAFFOLD) if SCAFFOLD.exists() else {}
    raw_result = check_raw_frames(raw, classifier)
    viewport_result = check_viewport_rows()
    scaffold_ok = (
        scaffold.get("status") == "SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS"
        and bool(scaffold.get("rows"))
        and scaffold["rows"][0].get("scaffoldOnly") is True
        and scaffold["rows"][0].get("rawCaptureSha256") == RAW_CLOSED_SHA
        and scaffold["rows"][0].get("captureSha256") == VIEW_CLOSED_SHA
    )
    probe_result = run_probe()
    ok = raw_result["ok"] and viewport_result["ok"] and scaffold_ok and probe_result["ok"]
    result = {
        "schema": "firestaff.parity.pass1055_dm1_v1_original_closed_door_collision_capture.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "PASS1055_ORIGINAL_CLOSED_DOOR_COLLISION_GATE" if ok else "FAIL",
        "honesty": "Evidence gate only; no complete collision parity or pixel parity is claimed.",
        "raw_frames": raw_result,
        "viewport": viewport_result,
        "scaffold_ok": scaffold_ok,
        "firestaff_probe": probe_result,
    }
    write_outputs(result)
    if ok:
        print("PASS pass1055 DM1 V1 original closed-door collision gate")
        return 0
    print("FAIL pass1055 DM1 V1 original closed-door collision gate")
    print(f"manifest: {MANIFEST.relative_to(ROOT)}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
