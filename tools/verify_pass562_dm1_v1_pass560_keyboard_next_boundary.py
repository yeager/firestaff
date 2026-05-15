#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass562_dm1_v1_pass560_keyboard_next_boundary"
OUT = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED_COMMON = Path(
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
CANONICAL_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
PASS560_MANIFEST = (
    ROOT
    / "parity-evidence/verification/pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding/manifest.json"
)

EXPECTED_DM1_HASHES = {
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
}

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "c254_table_slots_zero_one_are_keyboard_api",
        "file": "IBMIO.C",
        "lines": "2378-2381",
        "boundary": "C254/IO_DRIVER slot decode",
        "needles": [
            "char* G8101_apc_IOInterruptVector[31] = {",
            "(char*)F8090_, /*  0 */",
            "(char*)F8091_, /*  1 */",
        ],
        "claim": "C254 publishes an IO_DRIVER table whose slots 0 and 1 are the keyboard read/present API.",
    },
    {
        "id": "c254_vector_is_installed_before_child_game",
        "file": "IBMIO.C",
        "lines": "2550-2552",
        "boundary": "C254/IO_DRIVER slot decode",
        "needles": [
            "setvect(C254_DM_IO_INTERRUPT, (void interrupt (*)())&G8101_apc_IOInterruptVector);",
            "F8088_();",
        ],
        "claim": "The IO driver installs C254 before the child game runs.",
    },
    {
        "id": "game_binds_g2162_from_c254",
        "file": "CEDT026.C",
        "lines": "235-238",
        "boundary": "C254/IO_DRIVER slot decode",
        "needles": [
            "void F0713_InitIOInterrupt(",
            "G2162_IODriver = G2161_IODriver = (IO_DRIVER*)getvect(C254_DM_IO_INTERRUPT);",
        ],
        "claim": "The game binds G2162_IODriver from interrupt C254.",
    },
    {
        "id": "f0540_reads_slot0_then_maps_pc34_arrows",
        "file": "IO2.C",
        "lines": "27-61",
        "boundary": "IO2/F0540 dispatch",
        "needles": [
            "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();",
            "switch (L2944_ui_ - 0x1248)",
            "L2944_ui_ = 'L';",
            "L2944_ui_ = 'P';",
            "L2944_ui_ = 'K';",
            "L2944_ui_ = 'M';",
            "return L2944_ui_;",
        ],
        "claim": "F0540 is downstream of slot 0 and normalizes PC34 arrow scancodes to movement-table codes.",
    },
    {
        "id": "f0539_reads_slot1_keyboard_presence",
        "file": "IO2.C",
        "lines": "179-184",
        "boundary": "IO2/F0540 dispatch",
        "needles": [
            "BOOLEAN F0539_INPUT_Cconis(",
            "return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();",
        ],
        "claim": "F0539 keyboard-present is downstream of IO_DRIVER slot 1.",
    },
    {
        "id": "main_loop_orders_io2_before_f0361_then_f0380",
        "file": "GAMELOOP.C",
        "lines": "164-168,215-219",
        "boundary": "F0361 enqueue",
        "needles": [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ],
        "claim": "The game loop calls F0361 only after M527/M528 have consumed IO_DRIVER input.",
    },
    {
        "id": "f0361_writes_keyboard_command_queue",
        "file": "COMMAND.C",
        "lines": "1753-1768",
        "boundary": "F0361 enqueue",
        "needles": [
            "while (L1111_i_Command = L1112_ps_KeyboardInput->Command)",
            "if (P0728_KeyCode == L1112_ps_KeyboardInput->Code)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
        ],
        "claim": "F0361 is the enqueue boundary after F0540 has returned a movement-table code.",
    },
]


def compact(text: str) -> str:
    return " ".join(text.split())


def run(cmd: list[str]) -> str:
    return subprocess.run(
        cmd,
        cwd=ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    ).stdout.strip()


def read_source_window(path: Path, ranges: str) -> tuple[str, list[int]]:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    selected: list[str] = []
    numbers: list[int] = []
    for part in ranges.split(","):
        start, end = [int(value) for value in part.split("-", 1)]
        for line_no in range(start, end + 1):
            if 1 <= line_no <= len(lines):
                selected.append(lines[line_no - 1])
                numbers.append(line_no)
    return "\n".join(selected), numbers


def audit_source() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = RED_COMMON / lock["file"]
        text, numbers = read_source_window(path, lock["lines"]) if path.exists() else ("", [])
        missing = [needle for needle in lock["needles"] if compact(needle) not in compact(text)]
        rows.append(
            {
                "id": lock["id"],
                "file": lock["file"],
                "path": str(path),
                "lines": lock["lines"],
                "lineRangeObserved": [min(numbers), max(numbers)] if numbers else None,
                "boundary": lock["boundary"],
                "claim": lock["claim"],
                "ok": path.exists() and not missing,
                "missing": missing,
            }
        )
    return rows


def sha256(path: Path) -> str | None:
    if not path.exists():
        return None
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def audit_dm1_hashes() -> dict[str, Any]:
    files: dict[str, Any] = {}
    for name, expected in EXPECTED_DM1_HASHES.items():
        path = CANONICAL_DM1 / name
        observed = sha256(path)
        files[name] = {
            "path": str(path),
            "expectedSha256": expected,
            "observedSha256": observed,
            "ok": observed == expected,
        }
    return {"root": str(CANONICAL_DM1), "files": files, "ok": all(row["ok"] for row in files.values())}


def load_pass560() -> dict[str, Any]:
    if not PASS560_MANIFEST.exists():
        return {"ok": False, "exists": False, "path": str(PASS560_MANIFEST)}
    data = json.loads(PASS560_MANIFEST.read_text(encoding="utf-8"))
    status = data.get("status")
    runtime_binding = data.get("runtimeBinding", {})
    next_required = data.get("nextRequiredRuntimeProbe", [])
    blocker = data.get("blocker", "")
    return {
        "ok": status == "BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED"
        and runtime_binding.get("provided") is False
        and any("C254" in item and "decode" in item for item in next_required)
        and any("slot 0" in item and "slot 1" in item for item in next_required),
        "exists": True,
        "path": str(PASS560_MANIFEST.relative_to(ROOT)),
        "status": status,
        "runtimeBinding": runtime_binding,
        "nextRequiredRuntimeProbe": next_required,
        "blocker": blocker,
    }


def decide_boundary(pass560: dict[str, Any]) -> dict[str, Any]:
    candidates = [
        {
            "name": "C254/IO_DRIVER slot decode",
            "sourceLocked": True,
            "blockedByMissingRuntimeProof": pass560.get("ok") is True,
            "reason": "Pass560 source-locks C254 publication and reports no runtime C254/vector/slot artifact; IO2 cannot be interpreted until slot 0/1 are decoded.",
        },
        {
            "name": "IO2/F0540 dispatch",
            "sourceLocked": True,
            "blockedByMissingRuntimeProof": True,
            "reason": "F0540 and F0539 are downstream consumers of G2162_IODriver slot 0/1, so this is the next boundary only after C254 slot decode succeeds.",
        },
        {
            "name": "F0361 enqueue",
            "sourceLocked": True,
            "blockedByMissingRuntimeProof": True,
            "reason": "F0361 receives the M528 key after IO2 has read and normalized it; probing enqueue before C254/IO2 would collapse two boundaries.",
        },
    ]
    return {
        "selected": "C254/IO_DRIVER slot decode",
        "selectedStatus": "PASS562_DM1_V1_NEXT_BOUNDARY_C254_IO_DRIVER_SLOT_DECODE_LOCKED",
        "candidates": candidates,
        "nextProbeContract": [
            "in one bounded N2 runtime, after IO startup, read IVT C254 at 0000:03F8",
            "decode C254 as non-null G8101_apc_IOInterruptVector",
            "decode table slot 0 as F8090_ and slot 1 as F8091_ before any IO2/F0540 claim",
            "only after that, continue to F0539/F0540 hit/value and then F0361 enqueue/G2153 increment",
        ],
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass562 - DM1 V1 keyboard next boundary after pass560",
        "",
        f"- Status: {manifest['status']}",
        f"- Selected next boundary: {manifest['decision']['selected']}",
        f"- Pass560 status: {manifest['pass560']['status']}",
        f"- Manifest: {MANIFEST.relative_to(ROOT)}",
        "",
        "## Canonical DM1 anchors",
    ]
    for name, row in manifest["canonicalDm1"]["files"].items():
        result = "PASS" if row["ok"] else "FAIL"
        lines.append(f"- {name}: sha256 {row['observedSha256']} ({result})")
    lines.extend(["", "## ReDMCSB source locks"])
    for row in manifest["sourceAudit"]:
        result = "PASS" if row["ok"] else "FAIL"
        lines.append(
            f"- {result} {row['boundary']}: {row['file']} {row['lines']} - {row['claim']}"
        )
    lines.extend(
        [
            "",
            "## Decision",
            "",
            "Choose C254/IO_DRIVER slot decode next. IO2/F0540 dispatch and F0361 enqueue are both source-locked, but they are downstream of the C254 table and slot 0/1 binding. A runtime probe that jumps straight to F0540 or F0361 would merge boundaries and would not explain whether pass560 failed at the vector/table binding or later input dispatch.",
            "",
            "## Next probe contract",
        ]
    )
    for item in manifest["decision"]["nextProbeContract"]:
        lines.append(f"- {item}")
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    source = audit_source()
    canonical = audit_dm1_hashes()
    pass560 = load_pass560()
    decision = decide_boundary(pass560)
    ok = all(row["ok"] for row in source) and canonical["ok"] and pass560["ok"]
    status = decision["selectedStatus"] if ok else "FAIL_PASS562_DM1_V1_KEYBOARD_NEXT_BOUNDARY_AUDIT"
    manifest = {
        "schema": f"{PASS}.v1",
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "sourceRoot": str(RED_COMMON),
        "sourceAudit": source,
        "canonicalDm1": canonical,
        "pass560": pass560,
        "decision": decision,
        "blocker": None if ok else "source, canonical hash, or pass560 prerequisite audit failed",
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(status)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
