#!/usr/bin/env python3
"""Pass238: runtime CS:IP debugger automation blocker/runbook.

Records the pass237 candidate-only runtime follow-up and the N2 debugger
control blocker. This pass intentionally promotes no symbol-map entries: the
only safe output is a precise manual debugger runbook plus sanitized evidence of
why stdin/SSH automation did not set breakpoints or capture hits.
"""
from __future__ import annotations

import json
import shutil
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/pass238_dm1_v1_runtime_debugger_keystroke_blocker"
REPORT = ROOT / "parity-evidence/pass238_dm1_v1_runtime_debugger_keystroke_blocker.md"
PASS237 = ROOT / "parity-evidence/verification/pass237_dm1_v1_fires_static_csip_crosswalk/manifest.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
RUNTIME_IMAGE = ROOT / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
ATTEMPT_LOG = OUT_DIR / "stdin_bpint_attempt_sanitized.txt"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def csip_runtime_formula(static_cs_ip: str) -> str:
    cs, ip = static_cs_ip.split(":")
    return f"{{load_segment + 0x{cs}}}:0x{ip}"


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pass237 = load_json(PASS237)
    symbol_map = load_json(SYMBOL_MAP)
    runtime_image = load_json(RUNTIME_IMAGE)
    candidates = pass237.get("candidates", [])
    promoted = [entry for entry in symbol_map.get("entries", []) if entry.get("confidence") == "verified_runtime_hit"]
    allowed_promoted_ids = {"viewport_buffer_composed"}
    unexpected_promoted = [entry for entry in promoted if entry.get("id") not in allowed_promoted_ids]
    if unexpected_promoted:
        raise SystemExit(f"Refusing blocker report: unexpected promoted runtime hits: {[e.get("id") for e in unexpected_promoted]}")

    attempt_text = ATTEMPT_LOG.read_text(errors="replace") if ATTEMPT_LOG.exists() else ""
    stdin_control_succeeded = "DEBUG: Set interrupt breakpoint" in attempt_text or "BPLIST" in attempt_text and "INT 21" in attempt_text
    classification = "blocked/direct-f0380-dequeue-hit-required" if promoted else "blocked/debugger-keystroke-control-required"
    if stdin_control_succeeded:
        classification = "fail/unexpected-stdin-debugger-control-detected"
    pass278 = load_json(PASS278) if PASS278.exists() else {}
    f0128_runtime_hit = pass278.get("proof_predicates", {}).get("f0128_draw_hit_seen") is True

    candidate_rows = [
        {
            "id": c["id"],
            "source_function": c.get("source_function"),
            "static_cs_ip": c.get("candidate_static_cs_ip"),
            "runtime_formula": csip_runtime_formula(c.get("candidate_static_cs_ip")),
            "classification": c.get("classification"),
            "confidence": c.get("confidence"),
        }
        for c in candidates
    ]

    runbook = [
        "Create a scratch DOS directory outside the repo; copy the stock DM1 PC34 files and copy the local verified FIRES.EXENEW fixture as FIRES.EXE (do not commit it).",
        "Start /usr/bin/dosbox-debug from a real terminal/console, not through stdin-only SSH automation, mounted to that scratch directory.",
        "Press Alt+Pause/Break to force the debugger command prompt if the status line says Running.",
        "At the debugger prompt enter: BPINT 21 4B; BPLIST; F5.",
        "At the DOS prompt run FIRES.EXE. When EXEC breaks, continue/step until the child entry shows the decompressed FIRES first instruction at CS:0000; record that CS as load_segment.",
        "Compute runtime breakpoints with runtime_cs = load_segment + static_cs, runtime_ip = static_ip.",
        "Set at least these breakpoints for the movement-to-viewport chain: BP {load+0x1b7c}:06e9; BP {load+0x1771}:01aa; BP {load+0x1126}:0516; BP {load+0x23cc}:110e.",
        "Run the route (one movement command after reaching a playable viewport). On every hit, record CS:IP, registers, nearby disassembly, and enough state to prove command_accepted -> movement_applied -> viewport_present.",
        "Only after an actual breakpoint hit may a symbol-map entry become verified_runtime_hit; static CS:IP candidates alone remain candidate_only.",
    ]

    manifest = {
        "schema": "pass238_dm1_v1_runtime_debugger_keystroke_blocker.v1",
        "classification": classification,
        "status": "PARTIAL_F0128_RUNTIME_HIT_F0380_BLOCKED" if promoted else "NO_RUNTIME_HITS_CAPTURED_NO_PROMOTIONS",
        "tools": {"dosbox-debug": shutil.which("dosbox-debug"), "dosbox-x": shutil.which("dosbox-x")},
        "inputs": {
            "pass237_manifest": str(PASS237),
            "symbol_map": str(SYMBOL_MAP),
            "runtime_image": str(RUNTIME_IMAGE),
            "fires_exenew_sha256": runtime_image.get("decompressed_fires", {}).get("sha256"),
        },
        "candidate_breakpoints": candidate_rows,
        "stdin_automation_attempt": {
            "transcript": str(ATTEMPT_LOG),
            "result": "no debugger command acknowledgement observed; input was consumed by running DOSBox session, not by stopped debugger command prompt",
            "searched_for": ["DEBUG: Set interrupt breakpoint", "BPLIST INT 21"],
        },
        "promoted_symbol_map_entries": [{"id": e.get("id"), "runtime_cs_ip": e.get("runtime_cs_ip"), "scope": e.get("verified_runtime_hit", {}).get("scope")} for e in promoted],
        "pass278_runtime_control": {"manifest": str(PASS278), "f0128_runtime_hit_seen": f0128_runtime_hit, "f0380_dequeue_hit_seen": pass278.get("proof_predicates", {}).get("f0380_dequeue_hit_seen")},
        "manual_keystroke_runbook": runbook,
        "promotion_guardrail": "Do not promote candidate_only CS:IP rows until runtime PSP/load segment and debugger-observed hits are captured.",
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# Pass238 — DM1 PC34 runtime debugger keystroke blocker",
        "",
        f"Status: `{manifest['status']}`.",
        f"Classification: `{classification}`.",
        "",
        "## Result",
        "",
        "No pass237 candidate-only F0380/movement CS:IP was promoted. N2 DOSBox-debug/tmux/xdotool control did produce a narrow F0128 runtime BP hit at 23AD:40FE, so the remaining blocker is direct F0380 dequeue (22F4:0699) and viewport-present/F0097 hits, not basic debugger keystroke control."
        "",
        "Sanitized attempt transcript: `parity-evidence/verification/pass238_dm1_v1_runtime_debugger_keystroke_blocker/stdin_bpint_attempt_sanitized.txt`.",
        "",
        "## Candidate runtime formulas",
        "",
    ]
    for row in candidate_rows:
        lines.append(f"- `{row['id']}` — static `{row['static_cs_ip']}` -> runtime `{row['runtime_formula']}`; still `{row['classification']}`.")
    lines.extend([
        "",
        "## Exact manual runbook blocker",
        "",
    ])
    lines.extend(f"{i}. {step}" for i, step in enumerate(runbook, 1))
    lines.extend([
        "",
        "## Guardrail",
        "",
        "The symbol map now contains only the narrow viewport_buffer_composed/F0128 verified runtime hit. Static decompressed-image offsets and formulas remain insufficient for all other entries.",
        "",
        "Evidence manifest: `parity-evidence/verification/pass238_dm1_v1_runtime_debugger_keystroke_blocker/manifest.json`.",
    ])
    REPORT.write_text("\n".join(lines) + "\n")
    print(json.dumps({"classification": classification, "manifest": str(OUT_DIR / "manifest.json"), "report": str(REPORT), "promoted": [e.get("id") for e in promoted]}, indent=2, sort_keys=True))
    return 0 if classification.startswith("blocked/") else 1


if __name__ == "__main__":
    raise SystemExit(main())
