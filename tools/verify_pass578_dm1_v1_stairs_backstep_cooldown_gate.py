#!/usr/bin/env python3
from __future__ import annotations
import json, re, subprocess
from pathlib import Path
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
PASS = 'pass578_dm1_v1_stairs_backstep_cooldown_gate'
STATUS = 'PASS578_DM1_V1_STAIRS_BACKSTEP_COOLDOWN_GATE_SOURCE_LOCKED'
OUT = ROOT/'parity-evidence'/'verification'/PASS
MANIFEST = OUT/'manifest.json'
REPORT = ROOT/'parity-evidence'/f'{PASS}.md'

def read(p, enc='utf-8'):
    if not Path(p).exists(): raise AssertionError(f'missing {p}')
    return Path(p).read_text(encoding=enc, errors='replace')

def line(t, pos): return t.count('\n', 0, pos) + 1

def fn(t, name, rt=r'(?:STATICFUNCTION\s+)?(?:void|int|int16_t|BOOLEAN|unsigned\s+char|struct\s+\w+)'):
    pat = re.compile(r'\b(?:' + rt + r')\s+' + re.escape(name) + r'\s*\(')
    m = pat.search(t)
    if not m: raise AssertionError(f'missing function {name}')
    b = t.find('{', m.end())
    depth = 0
    for i in range(b, len(t)):
        if t[i] == '{': depth += 1
        elif t[i] == '}':
            depth -= 1
            if depth == 0: return line(t, m.start()), line(t, i), t[m.start():i+1]
    raise AssertionError(f'unterminated {name}')

def order(t, needles, label):
    cur = 0; offs = []
    for n in needles:
        p = t.find(n, cur)
        if p < 0: raise AssertionError(f'{label}: missing {n!r}')
        offs.append(p); cur = p + len(n)
    return offs

def span(base, body, offs):
    return f'{base + line(body, min(offs)) - 1}-{base + line(body, max(offs)) - 1}'

def run(cmd):
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
    if p.returncode: raise AssertionError(f'command failed {cmd}:\n{p.stdout[-3000:]}')
    return p.stdout.strip()

def find_test():
    cwd = Path.cwd()
    cands = [cwd/'test_dm1_v1_movement_pipeline_pc34_compat', cwd/'build-pass578'/'test_dm1_v1_movement_pipeline_pc34_compat', cwd/'build'/'test_dm1_v1_movement_pipeline_pc34_compat', ROOT/'build-pass578'/'test_dm1_v1_movement_pipeline_pc34_compat', ROOT/'build'/'test_dm1_v1_movement_pipeline_pc34_compat']
    cands += sorted(ROOT.glob('build*/test_dm1_v1_movement_pipeline_pc34_compat'))
    for c in cands:
        if c.exists(): return c
    raise AssertionError('missing built pipeline test')

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    command = read(RED/'COMMAND.C', 'latin-1')
    clik = read(RED/'CLIKMENU.C', 'latin-1')
    dungeon = read(RED/'DUNGEON.C', 'latin-1')
    queue = read(ROOT/'src/dm1/dm1_v1_input_command_queue_pc34_compat.c')
    core = read(ROOT/'src/dm1/dm1_v1_movement_command_core_pc34_compat.c')
    test = read(ROOT/'tests/test_dm1_v1_movement_pipeline_pc34_compat.c')
    f0380_s, f0380_e, f0380 = fn(command, 'F0380_COMMAND_ProcessQueue_CPSC')
    f0366_s = line(clik, clik.find('void F0366_COMMAND_ProcessTypes3To6_MoveParty'))
    f0366_end_pos = clik.find('#include \"CLIKCHAM.C\"', clik.find('void F0366_COMMAND_ProcessTypes3To6_MoveParty'))
    f0366_e = line(clik, f0366_end_pos) - 1
    f0366 = clik[clik.find('void F0366_COMMAND_ProcessTypes3To6_MoveParty'):f0366_end_pos]
    f0151_s, f0151_e, f0151 = fn(dungeon, 'F0151_DUNGEON_GetSquare')
    q_s, q_e, q = fn(queue, 'DM1_V1_InputCommandQueue_ProcessOnePc34Compat')
    c_s, c_e, c = fn(core, 'DM1_V1_MovementCommandCore_ProcessOnePc34Compat', rt=r'int')
    red_gate = order(f0380, [
        'L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;',
        'if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks',
        'T0380xxx:',
        'G2153_i_QueuedCommandsCount--;',
        'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);'], 'F0380 gate')
    red_stairs = order(f0366, [
        'AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;',
        'L1123_B_StairsSquare = (M034_SQUARE_TYPE',
        'if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2))',
        'F0364_COMMAND_TakeStairs(M007_GET(AL1115_ui_Square, MASK0x0004_STAIRS_UP));',
        'return;',
        'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement',
        'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;'], 'F0366 stairs')
    red_square = order(f0151, [
        'return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];',
        'return M035_SQUARE(C00_ELEMENT_WALL, 0);'], 'F0151 square')
    fire_q = order(q, [
        'result.command = queue->commands[0].command;',
        'if (is_move_command(result.command) &&',
        'disabledMovementTicks',
        'result.movementDisabledGate = 1;',
        'return result;',
        'queue->count--;',
        'result.dequeued = 1;'], 'queue gate')
    fire_c = order(c, [
        'outResult->queue = DM1_V1_InputCommandQueue_ProcessOnePc34Compat',
        'if (!outResult->queue.dequeued) {',
        'return 1;',
        'dm1_v1_apply_pre_step_stamina_cost',
        'if (action == MOVE_BACKWARD)',
        'F0705_MOVEMENT_ResolveStairsTransition_Compat',
        'DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat'], 'core ordering')
    for label in ['stairs_backward_cooldown_gate','stairs_backward_cooldown_queued','stairs_backward_cooldown_not_dequeued','stairs_backward_cooldown_no_transition','stairs_backward_cooldown_no_stamina','stairs_backward_cooldown_stamina_unchanged','stairs_backward_cooldown_map_unchanged','stairs_backward_cooldown_kept']:
        if f'"{label}"' not in test: raise AssertionError(f'missing test label {label}')
    exe = find_test(); out = run([str(exe)])
    if '0 failed' not in out.splitlines()[-1]: raise AssertionError(out[-1000:])
    manifest = {
        'schema': f'firestaff.parity.{PASS}.v1',
        'status': STATUS,
        'timestampUtc': datetime.now(timezone.utc).isoformat(),
        'branch': run(['git','branch','--show-current']),
        'head': run(['git','rev-parse','HEAD']),
        'worktree': str(ROOT),
        'sourceAudit': {
            'COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC': {'lines':[f0380_s,f0380_e], 'focusedLines': span(f0380_s,f0380,red_gate), 'claim':'G0310/G0311 movement gate precedes dequeue and F0366 dispatch'},
            'CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty': {'lines':[f0366_s,f0366_e], 'focusedLines': span(f0366_s,f0366,red_stairs), 'claim':'backward-on-stairs F0364 shortcut only runs after F0380 dispatch and returns before normal cooldown'},
            'DUNGEON.C:F0151_DUNGEON_GetSquare': {'lines':[f0151_s,f0151_e], 'focusedLines': span(f0151_s,f0151,red_square), 'claim':'current-square stairs classification is map data; invalid reads fall back to wall'},
        },
        'firestaffAudit': {
            'dm1_v1_input_command_queue_pc34_compat.c:DM1_V1_InputCommandQueue_ProcessOnePc34Compat': {'lines':[q_s,q_e], 'focusedLines': span(q_s,q,fire_q), 'claim':'gated movement returns before queue pop'},
            'dm1_v1_movement_command_core_pc34_compat.c:DM1_V1_MovementCommandCore_ProcessOnePc34Compat': {'lines':[c_s,c_e], 'focusedLines': span(c_s,c,fire_c), 'claim':'non-dequeued command returns before stamina/stairs/timing'},
            'testExecutable': str(exe),
            'testOutputLastLine': out.splitlines()[-1],
        },
        'nonClaims': ['does not replace pass571 turn no-step timing', 'does not cover pass571 front-command queue/pending replay', 'no original DOS runtime capture'],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True)+'\n')
    REPORT.write_text('\n'.join([
        '# Pass578 - DM1 V1 stairs backstep cooldown gate','',
        f'- Status: {STATUS}',
        f'- Manifest: {MANIFEST.relative_to(ROOT)}','',
        '## ReDMCSB Source Audit','',
        f"- COMMAND.C:{f0380_s}-{f0380_e}, focused {manifest['sourceAudit']['COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC']['focusedLines']}: movement commands are gated by G0310/G0311 before dequeue and F0366 dispatch.",
        f"- CLIKMENU.C:{f0366_s}-{f0366_e}, focused {manifest['sourceAudit']['CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty']['focusedLines']}: backward-on-stairs takes F0364 only after dispatch reaches F0366 and returns before normal relative-step/cooldown flow.",
        f"- DUNGEON.C:{f0151_s}-{f0151_e}, focused {manifest['sourceAudit']['DUNGEON.C:F0151_DUNGEON_GetSquare']['focusedLines']}: stairs classification comes from current-map square data; out-of-bounds reads are wall fallback.",'',
        '## Firestaff Gate','',
        f"- dm1_v1_input_command_queue_pc34_compat.c:{q_s}-{q_e}, focused {manifest['firestaffAudit']['dm1_v1_input_command_queue_pc34_compat.c:DM1_V1_InputCommandQueue_ProcessOnePc34Compat']['focusedLines']}: queue returns before popping a gated move.",
        f"- dm1_v1_movement_command_core_pc34_compat.c:{c_s}-{c_e}, focused {manifest['firestaffAudit']['dm1_v1_movement_command_core_pc34_compat.c:DM1_V1_MovementCommandCore_ProcessOnePc34Compat']['focusedLines']}: command core returns before stamina/stairs/timing when queue did not dequeue.",
        '- test_dm1_v1_movement_pipeline_pc34_compat asserts a cooldown-gated backward command on stairs remains queued, leaves party/stamina unchanged, and does not apply a stairs transition.','',
        '## Not Claimed','','- does not replace pass571 turn no-step timing','- does not cover pass571 front-command queue/pending replay','- no original DOS runtime capture','']) )
    print(f'{STATUS} manifest={MANIFEST.relative_to(ROOT)}')
if __name__ == '__main__':
    main()
