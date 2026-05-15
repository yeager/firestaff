#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass563_dm1_v1_pc34_original_c254_boundary"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ORIGINAL_ROOT = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
PC34 = ORIGINAL_ROOT / "_extracted/dm-pc34/DungeonMasterPC34"
ASSET_MANIFEST = ORIGINAL_ROOT / "_manifests/dm_originals_asset_inventory_20260510.json"

EXPECTED_ORIGINALS = {
    "DM.EXE": {"path": PC34 / "DM.EXE", "sha256": "4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4"},
    "DATA/GRAPHICS.DAT": {"path": PC34 / "DATA/GRAPHICS.DAT", "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"},
    "DATA/DUNGEON.DAT": {"path": PC34 / "DATA/DUNGEON.DAT", "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"},
}

SOURCE_LOCKS = [
    {"id": "pc34_game_loop_consumes_io_before_queue_dispatch", "file": "GAMELOOP.C", "needles": ["while (M527_IsCharacterInKeyboardBuffer())", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "F0380_COMMAND_ProcessQueue_CPSC();"], "claim": "DM1 PC/I34E consumes keyboard-present/read before F0361 enqueue and F0380 dispatch."},
    {"id": "i34e_keyboard_macros_bind_to_io2", "file": "DEFS.H", "needles": ["#define M527_IsCharacterInKeyboardBuffer()                               F0539_INPUT_Cconis()", "#define M528_GetCharacterInKeyboardBuffer()                              F0540_INPUT_Crawcin()", "#define C254_DM_IO_INTERRUPT    254", "int16_t (*IODRV_00_GetKeyboardInput)();", "BOOLEAN (*IODRV_01_IsKeyboardInputPresent)();"], "claim": "I34E keyboard macros are IO2 calls backed by IO_DRIVER slot 0/1 through C254."},
    {"id": "io2_slot0_slot1_dispatch", "file": "IO2.C", "needles": ["int16_t F0540_INPUT_Crawcin(", "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();", "switch (L2944_ui_ - 0x1248)", "BOOLEAN F0539_INPUT_Cconis(", "return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();"], "claim": "F0540/F0539 are downstream of decoded IO_DRIVER slot 0/1, not raw host input."},
    {"id": "ibmio_installs_keyboard_irq09_and_buffers_scancodes", "file": "IBMIO.C", "needles": ["asm     mov     ax, 3509h", "asm     mov     dx, offset S8A_Vector", "asm     mov     ax, 2509h", "asm     S8A_Vector:", "asm     in      al, 60h", "F8087_(G8085_ | (G8062_ & 0x1EFF));", "G8083_[G8077_] = P3278_i_;", "G8082_++;"], "claim": "IBMIO first receives raw IRQ09 scancodes and buffers accepted keys."},
    {"id": "ibmio_publishes_game_facing_c254_io_driver_table", "file": "IBMIO.C", "needles": ["(char*)F8090_, /*  0 */", "(char*)F8091_, /*  1 */", "setvect(C254_DM_IO_INTERRUPT, (void interrupt (*)())&G8101_apc_IOInterruptVector);", "F8088_();", "F8089_();"], "claim": "The game-facing boundary is C254 -> G8101 table -> F8090/F8091."},
    {"id": "game_binds_g2162_from_c254", "file": "CEDT026.C", "needles": ["G2162_IODriver = G2161_IODriver = (IO_DRIVER*)getvect(C254_DM_IO_INTERRUPT);", "return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();"], "claim": "The child game binds G2162_IODriver from C254 before using IO2 keyboard-present/read."},
    {"id": "f0361_f0380_queue_contract", "file": "COMMAND.C", "needles": ["void F0361_COMMAND_ProcessKeyPress", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;", "G2153_i_QueuedCommandsCount++;", "void F0380_COMMAND_ProcessQueue_CPSC", "if (G2153_i_QueuedCommandsCount == 0)", "G2153_i_QueuedCommandsCount--;"], "claim": "F0361 enqueue/count and F0380 pop/count remain downstream proof points after C254 slot decode."},
]

PASS_MANIFESTS = {
    "pass514": ROOT / "parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json",
    "pass558": ROOT / "parity-evidence/verification/pass558_dm1_v1_pass514_io_blocker_classifier/manifest.json",
    "pass560": ROOT / "parity-evidence/verification/pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding/manifest.json",
    "pass562": ROOT / "parity-evidence/verification/pass562_dm1_v1_pass560_keyboard_next_boundary/manifest.json",
}


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def sha256(path: Path) -> str | None:
    if not path.exists():
        return None
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def find_line(path: Path, needle: str) -> int | None:
    if not path.exists():
        return None
    compact = " ".join(needle.split())
    for idx, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if compact in " ".join(line.split()):
            return idx
    return None


def audit_source() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_LOCKS:
        path = RED / spec["file"]
        hits = {needle: find_line(path, needle) for needle in spec["needles"]}
        present = [line for line in hits.values() if line is not None]
        rows.append({"id": spec["id"], "file": spec["file"], "path": str(path), "ok": path.exists() and len(present) == len(hits), "lineRange": [min(present), max(present)] if present else None, "lineHits": hits, "missing": [needle for needle, line in hits.items() if line is None], "claim": spec["claim"]})
    return rows


def load_json(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def audit_originals() -> dict[str, Any]:
    rows: dict[str, Any] = {}
    for rel, spec in EXPECTED_ORIGINALS.items():
        actual = sha256(spec["path"])
        rows[rel] = {"path": str(spec["path"]), "exists": spec["path"].exists(), "sha256": actual, "expectedSha256": spec["sha256"], "ok": actual == spec["sha256"]}
    manifest = load_json(ASSET_MANIFEST) or {}
    manifest_text = json.dumps(manifest, sort_keys=True)
    rows["manifest"] = {
        "path": str(ASSET_MANIFEST),
        "exists": ASSET_MANIFEST.exists(),
        "mentionsCanonicalPc34": "_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT" in manifest_text and "_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT" in manifest_text,
        "mentionsN2CanonicalGraphicsHash": EXPECTED_ORIGINALS["DATA/GRAPHICS.DAT"]["sha256"] in manifest_text,
        "mentionsN2CanonicalDungeonHash": EXPECTED_ORIGINALS["DATA/DUNGEON.DAT"]["sha256"] in manifest_text,
        "usesDannesburkAsInput": False,
    }
    rows["manifest"]["ok"] = rows["manifest"]["exists"] and rows["manifest"]["mentionsCanonicalPc34"] and rows["manifest"]["mentionsN2CanonicalGraphicsHash"] and rows["manifest"]["mentionsN2CanonicalDungeonHash"] and not rows["manifest"]["usesDannesburkAsInput"]
    return rows


def audit_pass_chain() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for name, path in PASS_MANIFESTS.items():
        payload = load_json(path)
        row: dict[str, Any] = {"path": str(path.relative_to(ROOT)), "present": payload is not None}
        if payload:
            row["status"] = payload.get("status")
            row["decision"] = payload.get("decision")
            row["blocker"] = payload.get("blocker")
            row["proofPredicates"] = payload.get("proofPredicates")
        out[name] = row
    return out


def classify(source: list[dict[str, Any]], originals: dict[str, Any], chain: dict[str, Any]) -> tuple[str, str, dict[str, bool]]:
    p562_decision = chain.get("pass562", {}).get("decision") or {}
    next_contract = p562_decision.get("nextProbeContract") if isinstance(p562_decision, dict) else []
    predicates = {
        "sourceAuditOk": all(row["ok"] for row in source),
        "originalPc34PayloadOk": all(row.get("ok") for row in originals.values()),
        "pass514EmptyF0380BlockerPresent": chain.get("pass514", {}).get("status") == "BLOCKED_PASS514_KEYBOARD_INPUT_DELIVERED_BUT_NO_F0361_ENQUEUE_BEFORE_EMPTY_F0380",
        "pass558IoBoundaryClassified": chain.get("pass558", {}).get("status") == "BLOCKED_PASS558_DOSBOX_EVENT_TO_GUEST_IO2_DISPATCH_BOUNDARY_CLASSIFIED",
        "pass560C254RuntimeMissing": chain.get("pass560", {}).get("status") == "BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED",
        "pass562SelectedC254SlotDecode": isinstance(p562_decision, dict) and p562_decision.get("selected") == "C254/IO_DRIVER slot decode",
        "pass562ProbeContractNamesSlots": any("slot 0" in str(item) and "slot 1" in str(item) for item in next_contract),
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS563_REDMCSB_SOURCE_AUDIT_FAILED", "ReDMCSB source audit failed.", predicates
    if not predicates["originalPc34PayloadOk"]:
        return "FAIL_PASS563_PC34_ORIGINAL_PAYLOAD_MISMATCH", "Canonical N2 PC34 original payload did not match expected hashes.", predicates
    if all(predicates.values()):
        decision = ("DM1 V1 original overlay/capture is still blocked at the canonical PC34 C254/IO_DRIVER slot runtime decode: "
                    "the exact N2 PC34 DM.EXE/DATA payload is hash-locked, ReDMCSB routes I34E input through C254 slot0/slot1 "
                    "before IO2/F0361, pass514/pass558 classify the empty-F0380 symptom, pass560 has not captured C254 at runtime, "
                    "and pass562 correctly selects C254 slot decode as the next bounded runtime probe.")
        return "BLOCKED_PASS563_PC34_ORIGINAL_C254_SLOT_RUNTIME_PROBE_REQUIRED", decision, predicates
    return "BLOCKED_PASS563_PC34_C254_CHAIN_INCOMPLETE", "Existing pass514/pass558/pass560/pass562 evidence chain is incomplete in this checkout.", predicates



def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass563 - DM1 V1 PC34 original C254 boundary",
        "",
        f"- Status: {manifest['status']}",
        f"- Manifest: parity-evidence/verification/{PASS}/manifest.json",
        "",
        "## Decision",
        "",
        manifest["decision"],
        "",
        "## Canonical original payload",
    ]
    for name, row in manifest["originalAudit"].items():
        if name == "manifest":
            lines.append(f"- original manifest: {'PASS' if row['ok'] else 'FAIL'} {row['path']}")
        else:
            lines.append(f"- {name}: {'PASS' if row['ok'] else 'FAIL'} sha256 {row['sha256']}")
    lines.extend(["", "## ReDMCSB locks"])
    for row in manifest["sourceAudit"]:
        rng = row["lineRange"] or ["?", "?"]
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{rng[0]}-{rng[1]} {row['id']} - {row['claim']}")
    lines.extend([
        "",
        "## Pass chain",
        "",
        f"- pass514: {manifest['passChain']['pass514'].get('status')}",
        f"- pass558: {manifest['passChain']['pass558'].get('status')}",
        f"- pass560: {manifest['passChain']['pass560'].get('status')}",
        f"- pass562: {manifest['passChain']['pass562'].get('status')}",
        "",
        "## Next bounded runtime probe",
        "",
        "- Read IVT C254 at 0000:03F8 after IBMIO startup in the canonical PC34 run.",
        "- Decode C254 as G8101_apc_IOInterruptVector and table slot 0 as F8090_, slot 1 as F8091_.",
        "- Only then continue to IO2/F0539/F0540, F0361 enqueue/G2153 increment, and F0380 pop/dispatch.",
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    source = audit_source()
    originals = audit_originals()
    chain = audit_pass_chain()
    status, decision, predicates = classify(source, originals, chain)
    manifest = {"schema": f"{PASS}.v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "decision": decision, "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"]), "head": run(["git", "rev-parse", "HEAD"]), "sourceRoot": str(RED), "originalRoot": str(ORIGINAL_ROOT), "sourceAudit": source, "originalAudit": originals, "passChain": chain, "proofPredicates": predicates, "nonClaims": ["no new original runtime capture", "no pixel parity promotion", "no movement behavior change", "no DANNESBURK input touched", "no push"]}
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(f"{status}: {decision}")
    return 0 if status.startswith("BLOCKED_PASS563_PC34_ORIGINAL_C254_SLOT_RUNTIME_PROBE_REQUIRED") else 1


if __name__ == "__main__":
    raise SystemExit(main())
