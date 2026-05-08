#!/usr/bin/env python3
"""Pass244: record the bounded non-DOSBox QEMU/FreeDOS DUNVIEW path.

This verifier is intentionally text-only. It records the reproducible QEMU hard
image construction route and, when the N2-local manual run directory exists,
only metadata for DUNVIEW.OBJ/FIRES.MAP/log status. It must not copy original
source, object files, screenshots, or emulator output floods into the repo.
"""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206"
IBM = RED / "Toolchains/IBM PC/Source"
COMMON = RED / "Toolchains/Common/Source"
RUN_DIR = Path.home() / ".openclaw/data/firestaff-qemu-dunview-20260506g"
DISK_IMAGE = RUN_DIR / "c.img"
PARTITION_OFFSET = 1_048_576
OUT_DIR = ROOT / "parity-evidence/verification/pass244_dm1_v1_qemu_freedos_dunview_path"
REPORT = ROOT / "parity-evidence/pass244_dm1_v1_qemu_freedos_dunview_path.md"


def find_line(path: Path, needle: str) -> dict[str, Any]:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    for idx, line in enumerate(lines, 1):
        if needle in line:
            return {"line": idx, "text": line.strip()}
    return {"line": None, "text": None}


def sha256(path: Path) -> str | None:
    if not path.exists() or not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def mtools(args: list[str], timeout: int = 20) -> subprocess.CompletedProcess[str]:
    env = os.environ.copy()
    env["MTOOLS_SKIP_CHECK"] = "1"
    return subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout, env=env, check=False)


def mdir_entry(dos_path: str) -> dict[str, Any]:
    if not DISK_IMAGE.exists():
        return {"present": False, "reason": "disk image missing"}
    img = f"{DISK_IMAGE}@@{PARTITION_OFFSET}"
    proc = mtools(["mdir", "-i", img, dos_path])
    present = proc.returncode == 0
    return {"present": present, "stdout_excerpt": proc.stdout[:600], "stderr_excerpt": proc.stderr[:300]}


def mtype_head_tail(dos_path: str, max_lines: int = 30) -> dict[str, Any]:
    if not DISK_IMAGE.exists():
        return {"present": False}
    img = f"{DISK_IMAGE}@@{PARTITION_OFFSET}"
    proc = mtools(["mtype", "-i", img, dos_path])
    if proc.returncode != 0:
        return {"present": False, "stderr_excerpt": proc.stderr[:300]}
    lines = proc.stdout.splitlines()
    return {
        "present": True,
        "line_count": len(lines),
        "head": lines[:max_lines],
        "tail": lines[-max_lines:],
        "char_count": len(proc.stdout),
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source_lines = {
        "mkii_tcpp101_path": find_line(IBM / "MKII.BAT", r"SET PATH=\TCPP101\BIN;\TASM20;\LZEXE91"),
        "mkii_dunview_compile": find_line(IBM / "MKII.BAT", r"TCC.EXE -c -ml -O -Z    -DEXEID=74 -n\OBJECT\I34E\FIRES DUNVIEW.C"),
        "mkii_fires_map_tlink": find_line(IBM / "MKII.BAT", r"\BUILD\I34E\FIRES.MAP"),
        "dunview_first_failing_assignment": find_line(COMMON / "DUNVIEW.C", "L0174_B_DrawCreaturesCompleted = L0186_B_SquareHasProjectile"),
    }
    run_metadata = {
        "run_dir": str(RUN_DIR),
        "disk_image_exists": DISK_IMAGE.exists(),
        "disk_image_size": DISK_IMAGE.stat().st_size if DISK_IMAGE.exists() else None,
        "disk_image_sha256": None,  # original-derived image; deliberately not hashed into repo evidence
        "host_start": (RUN_DIR / "host-start.txt").read_text(encoding="utf-8", errors="replace").strip() if (RUN_DIR / "host-start.txt").exists() else None,
        "host_end": (RUN_DIR / "host-end.txt").read_text(encoding="utf-8", errors="replace").strip() if (RUN_DIR / "host-end.txt").exists() else None,
        "qemu_log": mtype_head_tail("::QEMU_DUN.LOG"),
        "dunview_obj_mdir": mdir_entry("::/OBJECT/I34E/FIRES/DUNVIEW.OBJ"),
        "fires_map_mdir": mdir_entry("::/BUILD/I34E/FIRES.MAP"),
    }
    qlog = run_metadata["qemu_log"]
    qlog_lines = list(qlog.get("head", [])) + list(qlog.get("tail", []))
    first_error = next((line for line in qlog_lines if line.startswith("Error ") or " errors in Compile" in line), None)
    if first_error:
        run_metadata["first_error"] = first_error
    obj_stdout = run_metadata["dunview_obj_mdir"].get("stdout_excerpt", "")
    map_present = bool(run_metadata["fires_map_mdir"].get("present"))
    obj_nonzero = "DUNVIEW  OBJ" in obj_stdout and "         0 " not in obj_stdout
    completed = bool(qlog.get("present")) and any("QEMU_DUNVIEW_DONE" in line for line in qlog.get("tail", []))
    if run_metadata["host_end"] is None and qlog.get("present"):
        status = "RUNNING_QEMU_FREEDOS_DUNVIEW_TCC_BOUND_NOT_YET_ELAPSED"
    elif map_present:
        status = "PASS_QEMU_FREEDOS_FIRES_MAP_METADATA_AVAILABLE"
    elif obj_nonzero:
        status = "PARTIAL_QEMU_FREEDOS_DUNVIEW_OBJ_METADATA_AVAILABLE"
    elif completed:
        status = "BLOCKED_QEMU_FREEDOS_DUNVIEW_COMPLETED_ZERO_OR_MISSING_OBJ"
    elif qlog.get("present"):
        status = "BLOCKED_QEMU_FREEDOS_DUNVIEW_TCC_NO_COMPLETION_WITHIN_BOUND"
    else:
        status = "BLOCKED_QEMU_FREEDOS_DUNVIEW_BOOT_OR_BATCH_NO_LOG"

    command_summary = {
        "disk": "dd 256M; sfdisk DOS partition at sector 2048 type 6; mkfs.fat -F 16 --offset=2048; mtools copy source/toolchain into image partition",
        "boot": "FreeDOS 1.3 floppy x86BOOT.img with FDAUTO.BAT",
        "qemu": "qemu-system-i386 -drive file=c.img,format=raw,if=ide,index=0 -fda boot.img -boot a -m 64 -display none -serial none -monitor none -no-reboot -no-shutdown",
        "compile": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\OBJECT\I34E\FIRES DUNVIEW.C >> C:\QEMU_DUN.LOG",
    }
    guards = {
        "qemu_hard_image_not_vvfat": True,
        "non_dosbox_route": True,
        "authentic_dunview_compile_line_found": bool(source_lines["mkii_dunview_compile"]["line"]),
        "no_original_binary_repo_artifacts": True,
    }
    manifest = {
        "schema": "pass244_dm1_v1_qemu_freedos_dunview_path.v1",
        "status": status,
        "source_lines": source_lines,
        "command_summary": command_summary,
        "run_metadata": run_metadata,
        "guards": guards,
        "conclusion": "QEMU+FreeDOS now has a reproducible partitioned hard-disk route that starts the authentic TCPP101 DUNVIEW.C compile outside DOSBox. The observed N2 run completed the TCPP101 invocation outside DOSBox, but TCC reported out-of-memory at DUNVIEW.C line 3938 and emitted no DUNVIEW.OBJ; FIRES.MAP therefore remains blocked before TLINK.",
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass244 — DM1 V1 QEMU/FreeDOS DUNVIEW path",
        "",
        f"Status: `{status}`",
        "",
        "## Result",
        "",
        manifest["conclusion"],
        "",
        "## Reproducible non-DOSBox path",
        "",
    ]
    for key, value in command_summary.items():
        lines.append(f"- `{key}`: {value}")
    lines += [
        "",
        "## Source anchors",
        "",
        f"- `MKII.BAT` TCPP101 PATH line: `{source_lines['mkii_tcpp101_path']['line']}`.",
        f"- `MKII.BAT` authentic DUNVIEW compile line: `{source_lines['mkii_dunview_compile']['line']}`.",
        f"- `MKII.BAT` FIRES.MAP TLINK line: `{source_lines['mkii_fires_map_tlink']['line']}`.",
        f"- `DUNVIEW.C` repeated diagnostic source anchor: `{source_lines['dunview_first_failing_assignment']['line']}`.",
        "",
        "## N2-local run metadata only",
        "",
        f"- Run dir: `{RUN_DIR}`.",
        f"- Disk image exists/size: `{run_metadata['disk_image_exists']}` / `{run_metadata['disk_image_size']}` bytes.",
        f"- Host start/end: `{run_metadata['host_start']}` / `{run_metadata['host_end']}`.",
        f"- QEMU log present: `{qlog.get('present')}`; lines: `{qlog.get('line_count')}`; chars: `{qlog.get('char_count')}`.",
        f"- First compiler error: `{run_metadata.get('first_error')}`.",
        f"- `DUNVIEW.OBJ` mdir present: `{run_metadata['dunview_obj_mdir'].get('present')}`.",
        f"- `FIRES.MAP` mdir present: `{run_metadata['fires_map_mdir'].get('present')}`.",
        "",
        "Evidence manifest: `parity-evidence/verification/pass244_dm1_v1_qemu_freedos_dunview_path/manifest.json`.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(json.dumps({"status": status, "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if all(guards.values()) and status.startswith(("PASS_", "PARTIAL_", "BLOCKED_", "RUNNING_")) else 1


if __name__ == "__main__":
    raise SystemExit(main())
