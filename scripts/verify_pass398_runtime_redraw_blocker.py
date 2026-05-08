#!/usr/bin/env python3
"""Pass398 verifier: queued command dispatch reaches the next runtime redraw."""
from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
DM1_DATA = Path(os.environ.get("FIRESTAFF_DM1_CANONICAL_DATA", "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"))
BUILD_DIR = Path(os.environ.get("FIRESTAFF_PASS398_BUILD_DIR", str(Path.home() / ".openclaw/data/firestaff-builds/pass398-runtime-redraw")))
HOME_DIR = Path(os.environ.get("FIRESTAFF_PASS398_HOME_DIR", str(Path.home() / ".openclaw/data/firestaff-homes/pass398-runtime-redraw")))
PASS = "pass398_runtime_redraw_blocker"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SCRIPT = "enter,down,down,down,down,down,down,enter,right,up"


def read(path: Path, enc: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing {path}")
    return path.read_text(encoding=enc, errors="replace")


def compact(s: str) -> str:
    return " ".join(s.split())


def function_body(path: Path, name: str) -> tuple[int, int, str]:
    text = read(path)
    m = re.search(rf"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char)\s+{re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name} in {path.name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, i) + 1, text[m.start(): i + 1]
    next_m = re.search(r"^(?:STATICFUNCTION\s+)?void\s+F\d+_[A-Za-z0-9_]+\s*\(", text[m.start() + 1 :], re.M)
    if next_m:
        end = m.start() + 1 + next_m.start()
        return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, end) + 1, text[m.start(): end]
    return text.count("\n", 0, m.start()) + 1, text.count("\n") + 1, text[m.start():]


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing/order {needle}")
        pos = hit


def redmcsb_audit() -> dict:
    command = RED / "COMMAND.C"
    clik = RED / "CLIKMENU.C"
    game = RED / "GAMELOOP.C"
    dun = RED / "DUNVIEW.C"
    f0361_s, f0361_e, f0361 = function_body(command, "F0361_COMMAND_ProcessKeyPress")
    f0380_s, f0380_e, f0380 = function_body(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0365_s, f0365_e, f0365 = function_body(clik, "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    f0366_s, f0366_e, f0366 = function_body(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    game_text = read(game)
    f0128_s, f0128_e, f0128 = function_body(dun, "F0128_DUNGEONVIEW_Draw_CPSF")

    require(f0361, "G0432_as_CommandQueue", "F0361 queue array")
    require(f0361, "G2153_i_QueuedCommandsCount", "F0361 queue count")
    require(f0361, "G2153_i_QueuedCommandsCount++;", "F0361 I34 queue increment")
    require_order(f0380, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "G2153_i_QueuedCommandsCount == 0",
        "G0310_i_DisabledMovementTicks",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G2153_i_QueuedCommandsCount--;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 dequeue dispatch")
    require(f0365, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0365 releases wait")
    require(f0365, "F0284_CHAMPION_SetPartyDirection", "F0365 direction mutation")
    require(f0366, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0366 releases wait")
    require(f0366, "F0267_MOVE_GetMoveResult_CPSCE", "F0366 movement state mutation")
    require_order(game_text, [
        "for (;;) { /*_Infinite loop_*/",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0380_COMMAND_ProcessQueue_CPSC();",
        "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
    ], "GAMELOOP draw/wait/dispatch loop")
    require_order(f0128, [
        "void F0128_DUNGEONVIEW_Draw_CPSF(",
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
        "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
    ], "F0128 viewport draw and present")
    return {
        "COMMAND.C:F0361_COMMAND_ProcessKeyPress": [f0361_s, f0361_e],
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": [f0380_s, f0380_e],
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": [f0365_s, f0365_e],
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": [f0366_s, f0366_e],
        "GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF": "F0128 before input wait; F0380 inside wait loop; waits on G0321/G0301",
        "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF": [f0128_s, f0128_e],
    }


def product_audit() -> list[dict]:
    checks = [
        ("main_loop_m11.c", ["inputRedrawAfterViewportDirtyCount", "lastInputRedrawAfterViewportDirty", "M11_GameView_HandleInput(&gameView, input)", "M11_GameView_Draw(&gameView"]),
        ("m11_game_view.c", ["DM1_V1_MovementPipeline_EnqueueCommandPc34Compat", "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat", "lastDm1V1MovementPipelineResult.viewportDirty", "return M11_GAME_INPUT_REDRAW"]),
        ("dm1_v1_movement_pipeline_pc34_compat.c", ["outResult->viewportDirty = outResult->core.viewportRedrawRequested", "provenance.viewportPresent"]),
        ("dm1_v1_movement_command_core_pc34_compat.c", ["outResult->viewportRedrawRequested = 1", "dm1_v1_is_turn_command", "dm1_v1_is_step_command"]),
    ]
    rows = []
    for rel, needles in checks:
        text = read(ROOT / rel, "utf-8")
        missing = [n for n in needles if n not in text]
        rows.append({"file": rel, "ok": not missing, "missing": missing})
    return rows


def run(cmd: list[str], *, env: dict[str, str] | None = None, timeout: int = 180) -> dict:
    p = subprocess.run(cmd, cwd=ROOT, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": cmd, "returncode": p.returncode, "ok": p.returncode == 0, "outputTail": p.stdout.splitlines()[-40:]}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    anchors = redmcsb_audit()
    product = product_audit()
    checks = [{"kind": "product_lock", **r} for r in product]
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    checks.append({"kind": "cmake_configure", **run(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR), "-G", "Ninja"], timeout=180)})
    checks.append({"kind": "build_firestaff", **run(["cmake", "--build", str(BUILD_DIR), "--target", "firestaff", "--parallel", "2"], timeout=900)})
    if HOME_DIR.exists():
        shutil.rmtree(HOME_DIR)
    HOME_DIR.mkdir(parents=True, exist_ok=True)
    probe_json = OUT_DIR / "runtime_redraw_probe.json"
    env = os.environ.copy()
    env.update({"HOME": str(HOME_DIR), "SDL_VIDEODRIVER": "dummy", "FIRESTAFF_AUTOTEST": "1", "FIRESTAFF_FAIL_IF_NO_LAUNCH": "1", "FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON": str(probe_json)})
    checks.append({"kind": "runtime_probe", **run(["timeout", "45s", str(BUILD_DIR / "firestaff"), "--duration", "9000", "--width", "1920", "--height", "1080", "--data-dir", str(DM1_DATA), "--script", SCRIPT], env=env, timeout=60)})
    runtime = {}
    if probe_json.exists():
        runtime = json.loads(probe_json.read_text())
    pipe = runtime.get("pipeline", {})
    redraw = runtime.get("redraw", {})
    semantic_mutation_ok = pipe.get("stepApplied") == 1 or pipe.get("turnApplied") == 1
    runtime_ok = (
        runtime.get("launchedEver") == 1 and runtime.get("active") == 1 and
        pipe.get("dequeued") == 1 and semantic_mutation_ok and
        pipe.get("movementBlocked") == 0 and pipe.get("viewportDirty") == 1 and
        redraw.get("inputRedrawDrawCount", 0) >= 1 and
        redraw.get("inputRedrawAfterViewportDirtyCount", 0) >= 1 and
        redraw.get("lastInputRedrawAfterViewportDirty") == 1
    )
    checks.append({"kind": "runtime_redraw_predicate", "ok": runtime_ok, "observed": runtime})
    ok = all(c.get("ok") for c in checks)
    status = "PASS398_RUNTIME_REDRAW_CHAIN_PROVEN" if ok else "BLOCKED_PASS398_RUNTIME_REDRAW_CHAIN"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "head": run(["git", "rev-parse", "HEAD"]).get("outputTail", [""])[-1],
        "redmcsbRoot": str(RED),
        "dm1Data": str(DM1_DATA),
        "script": SCRIPT,
        "requiredSourceAnchors": anchors,
        "checks": checks,
        "probeJson": str(probe_json.relative_to(ROOT)),
        "provenChain": "queued command dispatch/movement state -> viewportDirty -> M11_GAME_INPUT_REDRAW -> immediate M11_GameView_Draw runtime call",
        "notClaimed": ["pixel parity", "DOSBox debugger CS:IP hit", "raw original framebuffer capture"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    lines = [
        "# Pass398 — runtime redraw blocker",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB-first source anchors",
    ]
    for name, span in anchors.items():
        lines.append(f"- `{name}` — `{span}`")
    lines += [
        "",
        "## Runtime predicate",
        f"- Script: `{SCRIPT}`",
        f"- Probe JSON: `{manifest['probeJson']}`",
        f"- Pipeline: `{json.dumps(pipe, sort_keys=True)}`",
        f"- Redraw: `{json.dumps(redraw, sort_keys=True)}`",
        "",
        "## Verdict",
        "- Proven chain: queued command dispatch/movement state reaches `viewportDirty`, returns `M11_GAME_INPUT_REDRAW`, and the same loop immediately calls `M11_GameView_Draw` after the dirty viewport result.",
        "- Scope guard: no pixel-parity or DOSBox debugger-hit claim.",
    ]
    REPORT.write_text("\n".join(lines) + "\n")
    print(json.dumps({"status": status, "report": str(REPORT.relative_to(ROOT)), "manifest": str(MANIFEST.relative_to(ROOT)), "runtime_ok": runtime_ok}, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
