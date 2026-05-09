#!/usr/bin/env python3
"""Pass451 DM1 V1 Hall original capture storage policy gate.

Keeps follow-up original PC34 Hall captures off the internal N1 disk.  This is a
source/text gate: it verifies pass173 honors DOSBox/run-base environment
overrides and defaults local macOS/N1 artifact runs to the external Firestaff
artifact root when that disk is mounted.
"""
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass451_dm1_v1_hall_original_capture_storage_policy"
STATUS = "PASS_PASS451_EXTERNAL_CAPTURE_STORAGE_POLICY_READY"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
TARGET = ROOT / "tools/pass173_source_portrait_route_gate_probe.py"
EXTERNAL_ROOT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe")

REQUIRED_NEEDLES = [
    "import json, os, shutil, subprocess, sys, time",
    "DOSBOX = os.environ.get(\"FIRESTAFF_DOSBOX\", shutil.which(\"dosbox\") or \"/usr/bin/dosbox\")",
    "DEFAULT_EXTERNAL_ARTIFACT_ROOT = Path(\"/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe\")",
    "DEFAULT_RUN_BASE_ROOT = DEFAULT_EXTERNAL_ARTIFACT_ROOT if DEFAULT_EXTERNAL_ARTIFACT_ROOT.parent.exists() else Path.home()/\".openclaw/data/firestaff-n2-runs\"",
    "RUN_BASE_ROOT = Path(os.environ.get(\"FIRESTAFF_PASS173_RUN_BASE\", os.environ.get(\"FIRESTAFF_ARTIFACT_ROOT\", str(DEFAULT_RUN_BASE_ROOT))))",
]


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": proc.returncode, "outputTail": proc.stdout[-4000:]}


def git(args: list[str]) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.strip()


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    text = TARGET.read_text(encoding="utf-8") if TARGET.exists() else ""
    missing = [needle for needle in REQUIRED_NEEDLES if needle not in text]
    py_compile = run(["python3", "-m", "py_compile", str(TARGET)])
    errors: list[str] = []
    if not TARGET.exists():
        errors.append(f"missing target {TARGET}")
    if missing:
        errors.append(f"missing storage policy needles: {missing}")
    if py_compile["returncode"] != 0:
        errors.append("pass173 py_compile failed")
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL_PASS451_CAPTURE_STORAGE_POLICY" if errors else STATUS,
        "repo": str(ROOT),
        "branch": git(["branch", "--show-current"]),
        "head": git(["rev-parse", "HEAD"]),
        "target": str(TARGET.relative_to(ROOT)),
        "externalArtifactRoot": str(EXTERNAL_ROOT),
        "externalParentExists": EXTERNAL_ROOT.parent.exists(),
        "envOverrides": ["FIRESTAFF_DOSBOX", "FIRESTAFF_PASS173_RUN_BASE", "FIRESTAFF_ARTIFACT_ROOT"],
        "fallbackRunBase": "~/.openclaw/data/firestaff-n2-runs only when the N1 external artifact parent is absent and no env override is supplied",
        "pyCompile": py_compile,
        "missing": missing,
        "errors": errors,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{manifest['status']}`",
        f"- target: `{manifest['target']}`",
        f"- external artifact root: `{manifest['externalArtifactRoot']}`",
        f"- external parent exists: `{manifest['externalParentExists']}`",
        "- env overrides: `FIRESTAFF_DOSBOX`, `FIRESTAFF_PASS173_RUN_BASE`, `FIRESTAFF_ARTIFACT_ROOT`",
        "- fallback: internal run base is allowed only when external root is absent and no env override is set.",
        "",
        "## Non-claims",
        "No original Hall candidate screenshot parity is claimed here; this only keeps the next capture attempt reproducible and off the internal disk.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    if errors:
        print("FAIL pass451 capture storage policy")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"PASS {PASS}: {STATUS}")
    print(f"wrote {MANIFEST}")
    print(f"wrote {REPORT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
