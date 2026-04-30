#!/usr/bin/env python3
"""Pass162 C080 gdb/DOSBox-X debugger gate for original DM PC lane.

Runs the narrow debugger prerequisite now that gdb is installed. This does not
try new coordinates; it verifies whether host gdb can bind ReDMCSB source
symbols to the stock original DOS executable, and emits the DOSBox-X command
needed for the runtime gate when a DOS real-mode/source-symbol bridge is added.
"""
from __future__ import annotations

import json
import shutil
import subprocess
from dataclasses import asdict
from pathlib import Path

from pass162_c080_queue_trace_probe import CITATIONS, SOURCE_ROOT, OUT_DIR, read_lines

REPO = Path(__file__).resolve().parents[1]
STAGE = Path('/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34')
DM_EXE = STAGE / 'DM.EXE'
GDB_SCRIPT = OUT_DIR / 'pass162_dm_exe_symbol_gate.gdb'
DOSBOX_CONF = OUT_DIR / 'dosbox-x-pass162-runtime-gate.conf'


def run_cmd(args: list[str], timeout: int = 20) -> dict[str, object]:
    try:
        proc = subprocess.run(args, cwd=REPO, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        return {'args': args, 'returncode': proc.returncode, 'output': proc.stdout.splitlines()}
    except Exception as exc:
        return {'args': args, 'error': repr(exc), 'output': []}


def audit_required_redmcsb() -> list[dict[str, object]]:
    required = [
        c for c in CITATIONS
        if (c.file, c.symbol) in {
            ('COMMAND.C', 'F0359_COMMAND_ProcessClick_CPSC'),
            ('COMMAND.C', 'F0380_COMMAND_ProcessQueue_CPSC dequeue'),
            ('COMMAND.C', 'F0380 -> F0377 dispatch'),
            ('CLIKVIEW.C', 'F0377_COMMAND_ProcessType80_ClickInDungeonView'),
            ('CLIKVIEW.C', 'F0377 empty-hand front-wall hit'),
            ('CLIKVIEW.C', 'F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor'),
            ('MOVESENS.C', 'C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280'),
            ('REVIVE.C', 'F0280_CHAMPION_AddCandidateChampionToParty'),
        }
    ]
    out = []
    for c in required:
        excerpt = read_lines(SOURCE_ROOT / c.file, c.lines)
        missing = [token for token in c.must_contain if token not in excerpt]
        out.append({**asdict(c), 'path': str(SOURCE_ROOT / c.file), 'ok': not missing, 'missing': missing})
    return out


def write_runners() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    GDB_SCRIPT.write_text(f"""set pagination off
set confirm off
file {DM_EXE}
break F0359_COMMAND_ProcessClick_CPSC
break F0380_COMMAND_ProcessQueue_CPSC
break F0377_COMMAND_ProcessType80_ClickInDungeonView
break F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor
break F0280_CHAMPION_AddCandidateChampionToParty
info files
info breakpoints
""")
    DOSBOX_CONF.write_text(f"""[sdl]
fullscreen=false
output=opengl
[dosbox]
machine=svga_paradise
memsize=4
captures={OUT_DIR}
[cpu]
core=normal
cputype=386
cpu_cycles=3000
[mixer]
nosound=true
[speaker]
pcspeaker=false
tandy=off
[capture]
capture_dir={OUT_DIR}
default_image_capture_formats=raw
[autoexec]
mount c \"{STAGE}\"
c:
DM -vv -sn
""")


def main() -> int:
    write_runners()
    bins = {name: shutil.which(name) for name in ('gdb', 'dosbox-x', 'dosbox', 'file', 'python3')}
    commands = {
        'gdb_version': run_cmd([bins['gdb'] or 'gdb', '--version']),
        'dosbox_x_version': run_cmd([bins['dosbox-x'] or 'dosbox-x', '-version']),
        'dm_exe_file': run_cmd([bins['file'] or 'file', str(DM_EXE)]),
        'gdb_stock_dm_symbol_gate': run_cmd([bins['gdb'] or 'gdb', '--batch', '-x', str(GDB_SCRIPT)]),
    }
    gdb_out = '\n'.join(commands['gdb_stock_dm_symbol_gate'].get('output', []))
    if 'not in executable format' in gdb_out or 'No symbol table is loaded' in gdb_out:
        classification = 'blocked/gdb-cannot-bind-stock-dos-exe-or-redmcsb-symbols'
        first_missing_gate = 'debugger/source-symbol binding prerequisite; C080 mouse/queue/front-wall gates were not reached'
    else:
        classification = 'ready/gdb-symbol-gate-unexpectedly-passed-review-manually'
        first_missing_gate = 'unknown; inspect gdb output before claiming any C080 runtime gate'
    manifest = {
        'schema': 'pass162_c080_gdb_gate.v1',
        'classification': classification,
        'first_missing_gate': first_missing_gate,
        'repo': str(REPO),
        'source_root': str(SOURCE_ROOT),
        'original_stage': str(STAGE),
        'dm_exe': str(DM_EXE),
        'forbidden_roots_note': 'DANNESBURK not used.',
        'runners': {
            'gdb_script': str(GDB_SCRIPT),
            'gdb_command': f"gdb --batch -x {GDB_SCRIPT}",
            'dosbox_x_conf': str(DOSBOX_CONF),
            'dosbox_x_command': f"dosbox-x -conf {DOSBOX_CONF}",
        },
        'tools': bins,
        'redmcsb_audit': audit_required_redmcsb(),
        'commands': commands,
        'non_claims': [
            'does not prove stock original binary reached C080/F0377/F0280',
            'does not classify mouse translation vs queue dequeue vs C080 dispatch vs F0280 because gdb could not bind symbols to the stock DOS executable',
            'does not do coordinate guessing',
        ],
        'next_step': 'Use DOSBox-X built with/started in its debugger, a DOS real-mode gdbstub, or an address map from ReDMCSB symbols to the loaded DM.EXE image, then apply the emitted breakpoint order.',
    }
    (OUT_DIR / 'gdb_gate_manifest.json').write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n')
    lines = [
        '# Pass162 C080 gdb/debugger gate',
        '',
        f"Classification: `{classification}`",
        f"First missing gate: {first_missing_gate}",
        '',
        '## Exact commands run',
    ]
    for name, result in commands.items():
        lines.append(f"- `{name}` rc={result.get('returncode', 'n/a')}: `{' '.join(result['args'])}`")
        for line in result.get('output', [])[:12]:
            if line.strip():
                lines.append(f"  - {line}")
    lines += [
        '',
        '## Runnable artifacts',
        f"- gdb: `{manifest['runners']['gdb_command']}`",
        f"- DOSBox-X: `{manifest['runners']['dosbox_x_command']}`",
        '',
        '## Source citations audited',
    ]
    for item in manifest['redmcsb_audit']:
        lines.append(f"- {'PASS' if item['ok'] else 'FAIL'} `{item['file']}:{item['lines']}` `{item['symbol']}` — {item['point']}")
    lines += ['', '## Non-claims'] + [f"- {x}" for x in manifest['non_claims']] + ['', f"Next step: {manifest['next_step']}", '']
    (OUT_DIR / 'gdb_gate_README.md').write_text('\n'.join(lines))
    print(json.dumps({'classification': classification, 'out': str(OUT_DIR), 'gdb_script': str(GDB_SCRIPT)}, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
