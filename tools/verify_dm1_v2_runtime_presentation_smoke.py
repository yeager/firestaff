#!/usr/bin/env python3
"""DM1 V2 runtime presentation smoke gate.

Runs the real Firestaff CLI with a temporary launcher config for the launchable
DM1 enhanced presentation modes. This is a runtime/script artifact gate only:
it does not claim original DOS pixel parity, finished V2.2 art, or a completed
modern-rendering pass.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
import hashlib
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "dm1_v2_runtime_presentation_smoke"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

MODES = (
    {"index": 1, "id": "v20_filtered", "label": "V2.0 filtered"},
    {"index": 2, "id": "v21_upscaled", "label": "V2.1 enhanced 2D"},
)


def run(cmd: list[str], *, env: dict[str, str], timeout: int = 20) -> dict[str, Any]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    return {
        "cmd": cmd,
        "returncode": proc.returncode,
        "stdout": proc.stdout.strip(),
        "stderr": proc.stderr.strip(),
        "ok": proc.returncode == 0,
    }


def dm1_data_available(data_dir: Path) -> bool:
    return (data_dir / "GRAPHICS.DAT").exists() and (data_dir / "DUNGEON.DAT").exists()


def write_temp_config(home: Path, data_dir: Path, mode: int) -> None:
    cfg_dir = home / "Library" / "Application Support" / "Firestaff"
    cfg_dir.mkdir(parents=True, exist_ok=True)
    (cfg_dir / "startup-menu.toml").write_text(
        "\n".join(
            [
                f'data_dir = "{data_dir}"',
                f"graphics_index = {mode}",
                f"presentation_mode_index = {mode}",
                "renderer_backend_index = 0",
                "window_mode_index = 1",
                "scale_mode_index = 4",
                "display_aspect_mode = 2",
                "",
            ]
        ),
        encoding="utf-8",
    )


def run_mode(firestaff: Path, data_dir: Path, mode: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix=f"firestaff-{PASS}-{mode['id']}-") as td:
        temp_root = Path(td)
        temp_home = temp_root / "home"
        temp_home.mkdir(parents=True, exist_ok=True)
        write_temp_config(temp_home, data_dir, int(mode["index"]))
        probe_path = temp_root / f"{mode['id']}_probe.json"
        screenshot_dir = temp_root / f"{mode['id']}_screenshots"
        env = os.environ.copy()
        env["HOME"] = str(temp_home)
        env.setdefault("SDL_VIDEODRIVER", "dummy")
        env["FIRESTAFF_FAIL_IF_NO_LAUNCH"] = "1"
        env["FIRESTAFF_EXIT_AFTER_LAUNCH"] = "1"
        env["FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"] = str(probe_path)
        env["FIRESTAFF_AUTOTEST_SCREENSHOT_DIR"] = str(screenshot_dir)
        cmd = [
            str(firestaff),
            "--game",
            "dm1",
            "--data-dir",
            str(data_dir),
            "--duration",
            "1500",
        ]
        proc = run(cmd, env=env)
        proc["stderr"] = proc["stderr"].replace(str(temp_root), "<temp>")
        proc["stdout"] = proc["stdout"].replace(str(temp_root), "<temp>")
        row: dict[str, Any] = {
            "mode": mode,
            "command": proc,
            "probe": None,
            "screenshots": [],
            "ok": False,
        }
        if probe_path.exists():
            row["probe"] = json.loads(probe_path.read_text(encoding="utf-8"))
        if screenshot_dir.exists():
            row["screenshots"] = [
                {
                    "name": path.name,
                    "size": path.stat().st_size,
                    "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                }
                for path in sorted(screenshot_dir.glob("*.bmp"))
            ]
        if proc["ok"] and row["probe"]:
            probe = row["probe"]
            presentation = probe.get("presentation", {})
            screenshots = row["screenshots"]
            row["ok"] = (
                probe.get("schema") == "firestaff_m11_autotest_runtime_probe.v1"
                and probe.get("launchedEver") == 1
                and probe.get("active") == 1
                and probe.get("sourceId") == "dm1"
                and presentation.get("mode") == mode["index"]
                and presentation.get("width") == 640
                and presentation.get("height") == 400
                and len(screenshots) == 1
                and screenshots[0].get("size", 0) > 54
            )
        return row


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    rows = result.get("modes", [])
    lines = [
        "# DM1 V2 runtime presentation smoke",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate runs the real Firestaff CLI with a temporary launcher config",
        "for launchable DM1 enhanced presentation modes. It records M11 runtime",
        "probe JSON as script evidence, without promoting any original-DOS",
        "pixel-parity or finished V2.2 art claim.",
        "",
        "## Mode results",
        "",
        "| Mode | Status | Runtime source | Presentation mode | Screenshot artifacts |",
        "|---|---:|---|---:|---:|",
    ]
    for row in rows:
        probe = row.get("probe") or {}
        presentation = probe.get("presentation") or {}
        lines.append(
            f"| {row['mode']['label']} | {'PASS' if row.get('ok') else 'FAIL'} | "
            f"`{probe.get('sourceId', '')}` | `{presentation.get('mode', '')}` | "
            f"`{len(row.get('screenshots', []))}` |"
        )
    if result.get("skipped"):
        lines += ["", f"Skipped: {result.get('reason', '')}"]
    lines += [
        "",
        "## Non-claims",
        "",
        "- This is not an original PC 3.4 screenshot pairing.",
        "- This is not finished V2.2 real-art material/pixel verification.",
        "- This does not unblock the DOSBox debugger/capture rows in DM1 B1.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument(
        "--data-dir",
        type=Path,
        default=Path.home() / ".firestaff" / "data" / "dm1",
    )
    args = parser.parse_args()
    firestaff = args.build_dir / "firestaff"
    result: dict[str, Any] = {
        "schema": "firestaff.parity.dm1_v2_runtime_presentation_smoke.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL",
        "firestaff": str(firestaff),
        "data_dir": str(args.data_dir),
        "modes": [],
        "non_claims": [
            "No original PC 3.4 screenshot pairing.",
            "No finished V2.2 real-art material or pixel claim.",
            "No promotion of blocked DOSBox/debugger capture rows.",
        ],
    }
    if not firestaff.exists():
        result.update({"status": "FAIL", "reason": f"missing firestaff binary: {firestaff}"})
        write_outputs(result)
        print(f"FAIL {PASS}: missing firestaff binary", file=sys.stderr)
        return 1
    if not dm1_data_available(args.data_dir):
        result.update({"status": "SKIP", "skipped": True, "reason": f"missing DM1 data in {args.data_dir}"})
        write_outputs(result)
        print(f"SKIP {PASS}: missing DM1 data")
        return 0

    result["modes"] = [run_mode(firestaff, args.data_dir, mode) for mode in MODES]
    ok = all(row.get("ok") for row in result["modes"])
    result["status"] = "PASS" if ok else "FAIL"
    write_outputs(result)
    if ok:
        print(f"PASS {PASS} modes={len(result['modes'])}")
        return 0
    print(f"FAIL {PASS}", file=sys.stderr)
    print(json.dumps(result, indent=2)[:6000], file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
