#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect

ROOT = Path(__file__).resolve().parents[1]
PASS = 'pass330_dm1_v1_direct_pty_code_stop_transition_investigation'
OUT = ROOT / 'parity-evidence/verification' / PASS
REPORT = ROOT / 'parity-evidence' / f'{PASS}.md'
ORIG = Path.home() / '.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SRC = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
F0128 = '23AD:40FE'
F0097 = '2809:1EFF'
LOAD_ROUTE = 'wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200 one wait:3500'
LOAD_PREFIX = 'wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200'
LOAD_SUFFIX = 'one wait:3500'
MOVE_ROUTE = 'kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900'
ANSI_RE = re.compile(r'\x1b\[[0-9;?]*[ -/]*[@-~]')
CODE_LINE_RE = re.compile(r'\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+', re.I)


def clean(text: str) -> str:
    text = ANSI_RE.sub('', text).replace('\r', '\n')
    return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]', '', text)


def run(cmd: list[str], **kw: Any):
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def drain(child: pexpect.spawn, seconds: float) -> str:
    end = time.time() + seconds
    out = ''
    while time.time() < end:
        try:
            out += child.read_nonblocking(8192, timeout=0.05)
        except pexpect.TIMEOUT:
            pass
        except pexpect.EOF:
            out += '<EOF>'
            break
    return out


def write_conf(path: Path, stage: Path) -> None:
    path.write_text('\n'.join([
        '[sdl]', 'fullscreen=false', 'output=surface', 'usescancodes=false',
        '[dosbox]', 'machine=svga_paradise', 'memsize=4',
        '[cpu]', 'core=normal', 'cycles=3000',
        '[mixer]', 'nosound=true',
        '[autoexec]', f'mount c {stage}', 'c:', 'DEBUG DM.EXE -vv -sn -pk', '',
    ]), encoding='utf-8')


def source_audit() -> list[dict[str, Any]]:
    specs = [
        ('DUNVIEW.C', 'F0128', ['void F0128_DUNGEONVIEW_Draw_CPSF', 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)']),
        ('DRAWVIEW.C', 'F0097', ['void F0097_DUNGEONVIEW_DrawViewport', 'VIDRV_09_BlitViewPort']),
        ('COMMAND.C', 'F0380', ['void F0380_COMMAND_ProcessQueue_CPSC', 'F0365_COMMAND_ProcessTypes1To2_TurnParty', 'F0366_COMMAND_ProcessTypes3To6_MoveParty']),
    ]
    rows: list[dict[str, Any]] = []
    for fn, ident, needles in specs:
        lines = (SRC / fn).read_text(encoding='latin-1', errors='replace').splitlines()
        anchors: dict[str, int] = {}
        for needle in needles:
            compact = ' '.join(needle.split())
            for idx, line in enumerate(lines, 1):
                if compact in ' '.join(line.split()):
                    anchors[needle] = idx
                    break
        rows.append({'id': ident, 'file': fn, 'ok': len(anchors) == len(needles), 'anchors': anchors, 'missing': [n for n in needles if n not in anchors]})
    return rows


def xdo(display: str, args: list[str]):
    return run(['xdotool', *args], env={**os.environ, 'DISPLAY': display}, timeout=10)


def find_win(display: str) -> str | None:
    ids = [x.strip() for x in xdo(display, ['search', '--sync', '--class', 'dosbox']).stdout.splitlines() if x.strip()]
    return ids[0] if ids else None


def key_name(tok: str) -> str:
    return {'enter': 'Return', 'return': 'Return', 'one': '1', '1': '1', 'kp4': 'KP_Left', 'kp5': 'KP_Begin', 'kp6': 'KP_Right'}[tok]


def click_at(display: str, win: str, x: int, y: int) -> dict[str, Any]:
    ns: dict[str, Any] = {}
    exec(xdo(display, ['getwindowgeometry', '--shell', win]).stdout, {}, ns)
    gw, gh = float(ns['WIDTH']), float(ns['HEIGHT'])
    aspect = 320 / 200
    cw = gw
    ch = cw / aspect
    if ch > gh:
        ch = gh
        cw = ch * aspect
    px = int(round((gw - cw) / 2 + ((x + .5) / 320) * cw))
    py = int(round((gh - ch) / 2 + ((y + .5) / 200) * ch))
    r = xdo(display, ['mousemove', '--window', win, str(px), str(py), 'click', '1'])
    return {'screen': [px, py], 'rc': r.returncode, 'out': r.stdout[-160:]}


def drive(display: str, win: str, route: str, log: list[dict[str, Any]], stop_event: threading.Event | None = None) -> None:
    xdo(display, ['windowactivate', '--sync', win])
    xdo(display, ['windowfocus', '--sync', win])
    for item in route.split():
        if stop_event and stop_event.is_set():
            break
        low = item.lower()
        row: dict[str, Any] = {'t': time.time(), 'route_item': item}
        if low.startswith('wait:'):
            remaining = int(low.split(':', 1)[1]) / 1000
            while remaining > 0:
                sl = min(.1, remaining)
                time.sleep(sl)
                remaining -= sl
                if stop_event and stop_event.is_set():
                    break
            row['slept'] = True
        elif low.startswith('click:'):
            x, y = map(int, low.split(':', 1)[1].split(','))
            row.update(click_at(display, win, x, y))
        else:
            r = xdo(display, ['key', '--window', win, key_name(low)])
            row.update({'rc': r.returncode, 'out': r.stdout[-160:]})
            time.sleep(.2)
        log.append(row)


def dbg(child: pexpect.spawn, cmd: str, log: list[dict[str, Any]], transcript: list[str]) -> str:
    child.sendline(cmd)
    time.sleep(.22)
    out = drain(child, .55)
    transcript.append(out)
    log.append({'t': time.time(), 'cmd': cmd, 'excerpt': clean(out)[-500:]})
    return out


def code_lines(text: str) -> list[str]:
    return [m.group(0)[:160] for m in CODE_LINE_RE.finditer(clean(text))]


def last_code_addr(text: str) -> str | None:
    matches = [m.group('addr').upper() for m in CODE_LINE_RE.finditer(clean(text))]
    return matches[-1] if matches else None


def wait_prompt_by_pause(child: pexpect.spawn, display: str, win: str, cmdlog: list[dict[str, Any]], transcript: list[str], purpose: str) -> bool:
    for key in ['Alt+Pause', 'Pause']:
        xdo(display, ['key', '--window', win, key])
        cmdlog.append({'t': time.time(), 'control': key, 'purpose': purpose})
        out = drain(child, 5)
        transcript.append(out)
        if '->' in clean(out):
            return True
    return False


def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], stop_event: threading.Event | None = None, arm_f0097: bool = False) -> bool:
    deadline = time.time() + seconds
    buf = ''
    saw_running = False
    f0128_seen = False
    while time.time() < deadline:
        chunk = drain(child, .25)
        if chunk:
            transcript.append(chunk)
            buf += chunk
            c = clean(buf)
            if '(Running)' in c:
                saw_running = True
            if '(Running)' in c and '->' in c.split('(Running)', 1)[-1]:
                post = c.split('(Running)', 1)[-1]
                addr = last_code_addr(post)
                row = {'t': time.time(), 'addr': addr, 'running_marker_seen': True, 'prompt_reappeared_after_running': True, 'post_running_code_lines': code_lines(post)[-8:]}
                stops.append(row)
                if addr == F0128 and not f0128_seen:
                    f0128_seen = True
                    if arm_f0097:
                        dbg(child, f'BP {F0097}', cmdlog, transcript)
                        child.send('\x1bOt')
                        cmdlog.append({'t': time.time(), 'control': 'F5', 'purpose': 'run after F0097 candidate arm'})
                        buf = ''
                        continue
                    if stop_event:
                        stop_event.set()
                    break
                if addr == F0097 and f0128_seen:
                    if stop_event:
                        stop_event.set()
                    break
                child.send('\x1bOt')
                cmdlog.append({'t': time.time(), 'control': 'F5', 'purpose': 'continue non-target stop', 'addr': addr})
                buf = ''
        time.sleep(.05)
    return saw_running


def run_one(strategy: str, seconds: int) -> dict[str, Any]:
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    method = 'arm F0128 at initial debugger prompt before any game route input, run through full load+movement route' if strategy == 'pre_arm_before_route' else 'run to stable load/menu prefix, pause to debugger prompt, arm F0128, then continue into route'
    OUT.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='firestaff-pass330-') as td:
        stage = Path(td) / 'dos'
        shutil.copytree(ORIG, stage)
        conf = Path(td) / 'dosbox.conf'
        write_conf(conf, stage)
        display = f':{80 + (os.getpid() % 15)}'
        xvfb = subprocess.Popen(['Xvfb', display, '-screen', '0', '1024x768x24'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn('dosbox-debug', ['-conf', str(conf), '-exit'], env={**os.environ, 'DISPLAY': display, 'TERM': 'vt100'}, encoding='utf-8', timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(drain(child, 1))
            win = find_win(display)
            if not win:
                return {'strategy': strategy, 'method': method, 'ran': True, 'stage': 'load-ready marker', 'blocker': 'dosbox window not found'}
            if strategy == 'pre_arm_before_route':
                for cmd in ['BPDEL *', f'BP {F0128}', 'BPLIST']:
                    dbg(child, cmd, cmdlog, transcript)
                pre_bplist = clean(cmdlog[-1].get('excerpt', '')).upper()
                child.send('\x1bOt')
                cmdlog.append({'t': time.time(), 'control': 'F5', 'purpose': 'run after initial F0128 arm'})
                stop_event = threading.Event()
                t = threading.Thread(target=drive, args=(display, win, LOAD_ROUTE + ' ' + MOVE_ROUTE, routelog, stop_event), daemon=True)
                t.start()
                saw_running = monitor(child, seconds, transcript, cmdlog, stops, stop_event, arm_f0097=True)
                t.join(timeout=1)
                f0128 = any(s.get('addr') == F0128 for s in stops)
                f0097 = any(s.get('addr') == F0097 for s in stops[1:]) if f0128 else False
                if not f0128:
                    ok = wait_prompt_by_pause(child, display, win, cmdlog, transcript, 'post-route retention check')
                    retained = None
                    pause_addr = last_code_addr(''.join(transcript)) if ok else None
                    if ok:
                        dbg(child, 'BPLIST', cmdlog, transcript)
                        retained = F0128 in clean(cmdlog[-1].get('excerpt', '')).upper()
                    else:
                        retained = F0128 in pre_bplist
                    stage_name = 'breakpoint retention' if retained is False else 'code-stop transition'
                    blocker = 'F0128 breakpoint absent from BPLIST after load/route pause' if retained is False else 'F0128 breakpoint armed/retained but no strict running-to-23AD:40FE prompt transition emitted'
                else:
                    stage_name = None
                    blocker = None
                return {'strategy': strategy, 'method': method, 'ran': True, 'durationSeconds': round(time.time() - start, 3), 'routeLog': routelog, 'commandLog': cmdlog, 'stops': stops, 'sawRunning': saw_running, 'breakpointRetainedPostRoute': retained if not f0128 else True, 'postRoutePauseCodeAddr': (pause_addr if not f0128 else None), 'directHits': {'f0128_23AD_40FE': f0128, 'f0097_2809_1EFF_after_f0128': f0097}, 'stage': stage_name, 'blocker': blocker}
            child.send('\x1bOt')
            cmdlog.append({'t': time.time(), 'control': 'F5', 'purpose': 'run unarmed to load/menu prefix'})
            drive(display, win, LOAD_PREFIX, routelog)
            ok = wait_prompt_by_pause(child, display, win, cmdlog, transcript, 'load/menu prefix debugger pause before arming')
            if not ok:
                return {'strategy': strategy, 'method': method, 'ran': True, 'durationSeconds': round(time.time() - start, 3), 'routeLog': routelog, 'commandLog': cmdlog, 'stops': stops, 'directHits': {'f0128_23AD_40FE': False, 'f0097_2809_1EFF_after_f0128': False}, 'stage': 'load-ready marker', 'blocker': 'no debugger prompt after stable load/menu prefix via Alt+Pause/Pause'}
            for cmd in ['BPDEL *', f'BP {F0128}', 'BPLIST']:
                dbg(child, cmd, cmdlog, transcript)
            arm_time = time.time()
            child.send('\x1bOt')
            cmdlog.append({'t': time.time(), 'control': 'F5', 'purpose': 'run after load/menu F0128 arm'})
            stop_event = threading.Event()
            t = threading.Thread(target=drive, args=(display, win, LOAD_SUFFIX + ' ' + MOVE_ROUTE, routelog, stop_event), daemon=True)
            t.start()
            saw_running = monitor(child, seconds, transcript, cmdlog, stops, stop_event, arm_f0097=True)
            t.join(timeout=1)
            f0128 = any(s.get('addr') == F0128 for s in stops)
            f0097 = any(s.get('addr') == F0097 for s in stops[1:]) if f0128 else False
            route_after_arm = any(r.get('t', 0) > arm_time for r in routelog)
            return {'strategy': strategy, 'method': method, 'ran': True, 'durationSeconds': round(time.time() - start, 3), 'routeLog': routelog, 'commandLog': cmdlog, 'stops': stops, 'sawRunning': saw_running, 'routeInputAfterArming': route_after_arm, 'directHits': {'f0128_23AD_40FE': f0128, 'f0097_2809_1EFF_after_f0128': f0097}, 'stage': None if f0128 else ('route input after arming' if not route_after_arm else 'code-stop transition'), 'blocker': None if f0128 else ('route input not delivered after F0128 arming' if not route_after_arm else 'F0128 breakpoint armed but no strict running-to-23AD:40FE prompt transition emitted')}
        finally:
            try:
                transcript.append(drain(child, .5))
                child.terminate(force=True)
            except Exception:
                pass
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()
            (OUT / f'{strategy}.clean.txt').write_text(clean(''.join(transcript))[-300000:] + '\n', encoding='utf-8')


def build(args: argparse.Namespace) -> dict[str, Any]:
    audit = source_audit()
    missing = [x for x in ['dosbox-debug', 'Xvfb', 'xdotool'] if not shutil.which(x)]
    strategies: list[dict[str, Any]] = []
    if not missing:
        strategies.append(run_one('pre_arm_before_route', args.seconds))
    any_f0128 = any(s.get('directHits', {}).get('f0128_23AD_40FE') for s in strategies)
    any_seq = any(s.get('directHits', {}).get('f0097_2809_1EFF_after_f0128') for s in strategies)
    if any_seq:
        status = 'PASS_DIRECT_PTY_F0128_F0097_SEQUENCE_PROVEN'
    elif any_f0128:
        status = 'PASS_DIRECT_PTY_F0128_CODE_STOP_PROVEN'
    elif missing:
        status = 'BLOCKED_PASS330_ROUTE_NO_VIEWPORT_REDRAW'
    else:
        stages = [s.get('stage') for s in strategies]
        if 'breakpoint retention' in stages:
            status = 'BLOCKED_PASS330_BREAKPOINT_ADDRESS_REBASE_UNRESOLVED'
        elif 'route input after arming' in stages:
            status = 'BLOCKED_PASS330_ROUTE_NO_VIEWPORT_REDRAW'
        elif 'load-ready marker' in stages:
            status = 'BLOCKED_PASS330_ROUTE_NO_VIEWPORT_REDRAW'
        else:
            status = 'BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE'
    return {'schema': PASS + '.v1', 'timestampUtc': datetime.now(timezone.utc).isoformat(), 'status': status, 'sourceAudit': audit, 'addresses': {'F0128_DUNGEONVIEW_Draw_CPSF': F0128, 'F0097_VIDRV_09_BlitViewPort_indirect_call': F0097}, 'runtimeProbe': {'ran': not missing, 'missingTools': missing, 'boundedSecondsPerStrategy': args.seconds, 'strategies': strategies}, 'blocker': None if status.startswith('PASS') else next((s.get('blocker') for s in strategies if s.get('blocker')), 'missing tools: ' + ','.join(missing)), 'notPromotedBy': ['BPLIST', 'BP command echo', 'tmux/capture-pane']}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--seconds', type=int, default=75)
    ap.add_argument('--no-runtime', action='store_true')
    args = ap.parse_args()
    args.seconds = max(10, min(75, args.seconds))
    if args.no_runtime:
        audit = source_audit()
        manifest = {'schema': PASS + '.v1', 'timestampUtc': datetime.now(timezone.utc).isoformat(), 'status': 'BLOCKED_PASS330_ROUTE_NO_VIEWPORT_REDRAW', 'sourceAudit': audit, 'runtimeProbe': {'ran': False, 'blocker': '--no-runtime'}, 'blocker': '--no-runtime'}
    else:
        manifest = build(args)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    for strategy in manifest.get('runtimeProbe', {}).get('strategies', []):
        (OUT / (strategy['strategy'] + '_route_keylog.json')).write_text(json.dumps(strategy.get('routeLog', []), indent=2, sort_keys=True) + '\n', encoding='utf-8')
    REPORT.write_text('\n'.join(['# Pass330 — DM1 V1 direct-PTY code-stop transition investigation', '', f"Status: `{manifest['status']}`", '', '## ReDMCSB anchors', ''] + [f"- `{r['file']}` {r['id']}: `{r['anchors']}`" for r in manifest['sourceAudit']] + ['', '## Runtime decision', '', f"- Strategies: `{[s.get('strategy') for s in manifest.get('runtimeProbe', {}).get('strategies', [])]}`", f"- Direct hits: `{[s.get('directHits') for s in manifest.get('runtimeProbe', {}).get('strategies', [])]}`", f"- Blocker: `{manifest.get('blocker')}`", '', 'Manifest: `parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json`']) + '\n', encoding='utf-8')
    print(json.dumps({'status': manifest['status'], 'manifest': str(OUT / 'manifest.json'), 'blocker': manifest.get('blocker'), 'directHits': [s.get('directHits') for s in manifest.get('runtimeProbe', {}).get('strategies', [])]}, indent=2, sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
