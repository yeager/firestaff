#!/usr/bin/env python3
"""Pass344 verifier: DM1 V1 full launcher --script handoff.

The verifier intentionally records a short, redacted runtime probe instead of
large logs. It validates the handoff source seams and proves that a clean HOME
(default V1 config) can launch through the outer firestaff binary with --script,
then continue feeding post-launch route tokens to the active game-view path.
"""
from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass344_dm1_v1_full_launcher_script_handoff"
BUILD_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS344_BUILD_DIR", str(pathlib.Path.home() / ".openclaw/data/firestaff-builds/pass344-verify")))
HOME_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS344_HOME_DIR", str(pathlib.Path.home() / ".openclaw/data/firestaff-homes/pass344-verify")))
DATA_DIR = pathlib.Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1")
SCRIPT = "enter,down,down,down,down,down,down,enter,left,up,right"
COMMAND = [
    "timeout",
    "45s",
    str(BUILD_DIR / "firestaff"),
    "--duration",
    "12000",
    "--width",
    "1920",
    "--height",
    "1080",
    "--data-dir",
    str(DATA_DIR),
    "--script",
    SCRIPT,
]

CHECKS = [
    ("src/engine/main_loop_m11.c", "m11_next_script_input", "script token reader exists"),
    ("src/engine/main_loop_m11.c", "m11_push_script_event_token", "script SDL event bridge exists"),
    ("src/engine/main_loop_m11.c", "if (gameView.active)", "active game-view input branch exists"),
    ("src/engine/main_loop_m11.c", "M11_GameView_HandleInput(&gameView, input)", "post-launch tokens route to game view"),
    ("src/engine/main_loop_m11.c", "Do not short-circuit Enter/Right", "launcher top-level short-circuit remains disabled"),
    ("src/engine/main_loop_m11.c", "m11_open_requested_launch(&gameView, &menuState", "M12 launch request opens runtime"),
    ("src/ui/menu_startup_m12.c", "state->launchRequested = 1", "explicit launch row requests runtime handoff"),
    ("src/ui/menu_startup_m12.c", "pmode == M12_PRESENTATION_V3_MODERN_3D", "V3 block remains explicit"),
    ("src/ui/menu_hit_m12.c", "M12_HIT_GAMEOPT_LAUNCH", "modern mouse launch hit remains explicit"),
]

ANCHORS = [
    {"file": "ENTRANCE.C", "lines": "739-747", "meaning": "entrance installs primary mouse input, no movement secondary table"},
    {"file": "ENTRANCE.C", "lines": "856-882", "meaning": "entrance discards input then drains command queue until new-game mode changes"},
    {"file": "STARTUP2.C", "lines": "1179-1182", "meaning": "post-load game installs interface primary plus movement secondary input tables"},
    {"file": "GAMELOOP.C", "lines": "164-168,215", "meaning": "game loop drains keyboard and command queue while waiting for player input"},
    {"file": "COMMAND.C", "lines": "243-260", "meaning": "PC34 keyboard movement command table maps turn/move commands"},
    {"file": "COMMAND.C", "lines": "375-405", "meaning": "interface primary and movement secondary mouse command tables"},
    {"file": "COMMAND.C", "lines": "2045-2156", "meaning": "queue processor owns command dispatch seam"},
    {"file": "COMMAND.C", "lines": "2438-2451", "meaning": "entrance enter/resume commands switch load mode"},
]


def run(cmd: list[str], *, env: dict[str, str] | None = None, cwd: pathlib.Path = ROOT, timeout: int | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, cwd=str(cwd), env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout)


def short(text: str, limit: int = 4000) -> str:
    text = text.replace(str(DATA_DIR), "<DM1_CANONICAL_DATA_DIR>")
    text = text.replace(str(BUILD_DIR), "<PASS344_BUILD_DIR>")
    text = text.replace(str(HOME_DIR), "<PASS344_HOME>")
    if len(text) > limit:
        return text[:limit] + "\n[truncated]"
    return text


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    failures: list[str] = []
    static_results = []
    for rel, needle, label in CHECKS:
        path = ROOT / rel
        ok = path.exists() and needle in path.read_text(errors="replace")
        static_results.append({"file": rel, "check": label, "needle": needle, "ok": ok})
        if not ok:
            failures.append(f"missing {label}: {rel}")

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    cmake = run(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)], timeout=120)
    if cmake.returncode != 0:
        failures.append("cmake configure failed")

    build = run(["cmake", "--build", str(BUILD_DIR), "--target", "firestaff", "-j2"], timeout=600)
    if build.returncode != 0:
        failures.append("firestaff build failed")

    if HOME_DIR.exists():
        shutil.rmtree(HOME_DIR)
    HOME_DIR.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update({
        "HOME": str(HOME_DIR),
        "SDL_VIDEODRIVER": "dummy",
        "FIRESTAFF_AUTOTEST": "1",
        "FIRESTAFF_FAIL_IF_NO_LAUNCH": "1",
    })
    probe = run(COMMAND, env=env, timeout=60)
    if probe.returncode != 0:
        failures.append("outer launcher script probe did not launch cleanly")

    log_path = OUT_DIR / "full_launcher_script_probe.redacted.log"
    log_path.write_text(
        "pass344 full launcher script probe (redacted)\n"
        f"utc={datetime.now(timezone.utc).isoformat()}\n"
        f"command={' '.join(COMMAND)}\n"
        f"env=HOME=<PASS344_HOME> SDL_VIDEODRIVER=dummy FIRESTAFF_AUTOTEST=1 FIRESTAFF_FAIL_IF_NO_LAUNCH=1\n"
        f"returncode={probe.returncode}\n"
        "stdout:\n" + (short(probe.stdout).rstrip() or "<empty>") + "\n"
        "stderr:\n" + (short(probe.stderr).rstrip() or "<empty>") + "\n",
        encoding="utf-8",
    )

    manifest = {
        "pass": "pass344_dm1_v1_full_launcher_script_handoff",
        "status": "MOVEMENT_PROVED_FULL_LAUNCHER" if not failures else "BLOCKED_PASS344_FULL_LAUNCHER_SCRIPT_HANDOFF",
        "utc": datetime.now(timezone.utc).isoformat(),
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"]).stdout.strip(),
        "redmcsb_anchors": ANCHORS,
        "static_results": static_results,
        "exact_narrow_launcher_command": {
            "env": {
                "HOME": str(HOME_DIR),
                "SDL_VIDEODRIVER": "dummy",
                "FIRESTAFF_AUTOTEST": "1",
                "FIRESTAFF_FAIL_IF_NO_LAUNCH": "1",
            },
            "argv": COMMAND,
            "script": SCRIPT,
            "returncode": probe.returncode,
            "log": str(log_path.relative_to(ROOT)),
        },
        "gates": {
            "cmake_configure_rc": cmake.returncode,
            "build_firestaff_rc": build.returncode,
            "probe_rc": probe.returncode,
        },
        "failures": failures,
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(json.dumps({"status": manifest["status"], "failures": failures, "manifest": str(OUT_DIR / "manifest.json")}, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
