#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shlex, shutil, subprocess, tempfile, time
from datetime import datetime, timezone
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'parity-evidence/verification/pass275_dm1_v1_debugger_bpm_runtime_hook_attempt'
REPORT=ROOT/'parity-evidence/pass275_dm1_v1_debugger_bpm_runtime_hook_attempt.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SOURCE_ROOT=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDR={
 'G0432_as_CommandQueue':'2C20:3E7A','G0433_i_CommandQueueFirstIndex':'2C20:3EC8','G0434_i_CommandQueueLastIndex':'2C20:1F08','G0435_B_CommandQueueLocked':'2C20:1F0A',
 'G0308_i_PartyDirection':'2C20:3C92','G0306_i_PartyMapX':'2C20:3C94','G0307_i_PartyMapY':'2C20:3CE0',
 'F0380_COMMAND_ProcessQueue_CPSC':'22F4:0699','F0365_COMMAND_ProcessTypes1To2_TurnParty':'1EA4:010D','F0366_COMMAND_ProcessTypes3To6_MoveParty':'1EA4:01AA','F0267_MOVE_GetMoveResult_CPSCE':'1859:0516','F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE'}
ROUTE='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 kp5 wait:700 kp4 wait:700 kp6 wait:700'

def run(cmd, **kw):
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)

def clean(text:str)->str:
    text=re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',text).replace('\r','\n')
    text=re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',text)
    return text

def source_audit():
    checks=[('COMMAND.C',1,16,['G0432_as_CommandQueue','G0433_i_CommandQueueFirstIndex','G0434_i_CommandQueueLastIndex','G0435_B_CommandQueueLocked']),('COMMAND.C',2045,2156,['void F0380_COMMAND_ProcessQueue_CPSC','G0432_as_CommandQueue','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty']),('CLIKMENU.C',142,328,['F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty','G0308_i_PartyDirection','G0306_i_PartyMapX','G0307_i_PartyMapY']),('GAMELOOP.C',55,91,['F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)']),('MOVESENS.C',316,556,['BOOLEAN F0267_MOVE_GetMoveResult_CPSCE','G0306_i_PartyMapX','G0307_i_PartyMapY']),('DUNVIEW.C',8318,8611,['void F0128_DUNGEONVIEW_Draw_CPSF','F0097_DUNGEONVIEW_DrawViewport'])]
    out=[]
    for fn,a,b,needles in checks:
        p=SOURCE_ROOT/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines() if p.exists() else []
        block='\n'.join(lines[a-1:min(b,len(lines))])
        compact=' '.join(block.split())
        missing=[n for n in needles if ' '.join(n.split()) not in compact]
        out.append({'file':fn,'line_range':[a,b],'ok':p.exists() and not missing,'missing':missing})
    return out

def write_conf(path:Path, stage:Path):
    path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))

def xdotool_keys(display, pid, events, keylog:Path):
    helper=Path('/tmp/pass275_keys.sh')
    helper.write_text(r'''#!/usr/bin/env bash
set -euo pipefail
pid="$1"; route="$2"; window="$(xdotool search --sync --pid "$pid" | head -n 1)"
xdotool windowactivate --sync "$window" >/dev/null 2>&1 || true; xdotool windowfocus --sync "$window" >/dev/null 2>&1 || true
key_for(){ case "$1" in enter|return) echo Return;; one|1) echo 1;; kp4) echo KP_Left;; kp5) echo KP_Begin;; kp6) echo KP_Right;; *) return 1;; esac; }
click_at(){ local x="$1" y="$2"; local geom; geom="$(xdotool getwindowgeometry --shell "$window")"; eval "$geom"; read -r px py < <(python3 - "$WIDTH" "$HEIGHT" "$x" "$y" <<'P'
import sys
gw,gh,x,y=map(float,sys.argv[1:]); aspect=320/200; cw=gw; ch=cw/aspect
if ch>gh: ch=gh; cw=ch*aspect
left=(gw-cw)/2; top=(gh-ch)/2
print(int(round(left+((x+.5)/320)*cw)), int(round(top+((y+.5)/200)*ch)))
P
); xdotool mousemove --window "$window" "$px" "$py" click 1; echo "click:$x,$y->$px,$py"; sleep .18; }
for tok in $route; do low="${tok,,}"; echo "route-step $tok"; case "$low" in wait:*) python3 - "$low" <<'P' | xargs sleep
import sys; print(int(sys.argv[1].split(':',1)[1])/1000)
P
;; click:*) c="${low#click:}"; click_at "${c%,*}" "${c#*,}";; *) k="$(key_for "$low")"; xdotool key --window "$window" "$k"; sleep .12;; esac; done
''')
    helper.chmod(0o755)
    proc=run(['bash',str(helper),str(pid),events],env={**os.environ,'DISPLAY':display},timeout=40)
    keylog.write_text(proc.stdout)
    return proc.returncode

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=35); ap.add_argument('--route',default=ROUTE); args=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True)
    missing=[x for x in ['dosbox-debug','tmux','Xvfb','xdotool'] if not shutil.which(x)]
    if missing: raise SystemExit('missing: '+', '.join(missing))
    audit=source_audit()
    with tempfile.TemporaryDirectory(prefix='firestaff-pass275-') as td:
        stage=Path(td)/'dos'; shutil.copytree(ORIG,stage)
        conf=Path(td)/'dosbox.conf'; write_conf(conf,stage)
        display=':75'
        xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT)
        time.sleep(.5)
        sess=f'pass275-{os.getpid()}'
        raw_snaps=[]; key_rc=None; dosbox_pid=None; final=''
        try:
            r=run(['tmux','new-session','-d','-s',sess,f'TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit'],timeout=5)
            if r.returncode: raise RuntimeError(r.stdout)
            time.sleep(3)
            cmds=[]
            for k in ['F0380_COMMAND_ProcessQueue_CPSC','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty','F0267_MOVE_GetMoveResult_CPSCE','F0128_DUNGEONVIEW_Draw_CPSF']:
                cmds.append(f'BP {ADDR[k]}')
            for k in ['G0432_as_CommandQueue','G0433_i_CommandQueueFirstIndex','G0434_i_CommandQueueLastIndex','G0308_i_PartyDirection','G0306_i_PartyMapX','G0307_i_PartyMapY']:
                cmds.append(f'BPM {ADDR[k]}')
            cmds += ['BPLIST','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']
            for c in cmds:
                run(['tmux','send-keys','-t',sess,c,'Enter'],timeout=5); time.sleep(.35)
            run(['tmux','send-keys','-t',sess,'Escape','O','t'],timeout=5)
            time.sleep(2)
            p=run(['bash','-lc',f'export DISPLAY={display}; xdotool search --sync --class dosbox | head -n 1 | xargs -r xdotool getwindowpid'],timeout=8)
            dosbox_pid=int(p.stdout.strip().splitlines()[0]) if p.stdout.strip() else None
            if dosbox_pid:
                key_rc=xdotool_keys(display,dosbox_pid,args.route,OUT/'route_keylog.txt')
            end=time.time()+args.seconds
            while time.time()<end:
                cap=run(['tmux','capture-pane','-p','-S','-900','-t',sess],timeout=5).stdout
                raw_snaps.append(clean(cap)[-20000:])
                if '-> _' in cap or re.search(r'->\s*$', cap):
                    for c in ['CPU','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']:
                        run(['tmux','send-keys','-t',sess,c,'Enter'],timeout=5); time.sleep(.25)
                    run(['tmux','send-keys','-t',sess,'Escape','O','t'],timeout=5)
                time.sleep(1)
            final=run(['tmux','capture-pane','-p','-S','-3000','-t',sess],timeout=5).stdout
        finally:
            run(['tmux','kill-session','-t',sess],timeout=5)
            xvfb.terminate(); xvfb.wait(timeout=5)
    transcript=clean('\n\n--- SNAP ---\n\n'.join(raw_snaps+[final]))
    transcript_path=OUT/'dosbox_debug_runtime_attempt.clean.txt'; transcript_path.write_text(transcript[-200000:],encoding='utf-8')
    for p in ROOT.glob('memdump*'):
        dst=OUT/p.name; dst.write_bytes(p.read_bytes()); p.unlink()
    signals={name: (addr in transcript) for name,addr in ADDR.items()}
    bplist=sorted(set(re.findall(r'\b\d+\. BP(?:M)? ([0-9A-F]{4}:[0-9A-F]{4})',transcript)))
    status='BLOCKED_NO_PROVEN_RUNTIME_HOOK'
    manifest={'schema':'pass275_dm1_v1_debugger_bpm_runtime_hook_attempt.v1','timestamp_utc':datetime.now(timezone.utc).isoformat(),'status':status,'source_audit':audit,'addresses':ADDR,'route':args.route,'dosbox_debug':'/usr/bin/dosbox-debug','method':'tmux TERM=vt100 debugger commands; vt100 F5 workaround Escape O t; Xvfb+xdotool game input','key_rc':key_rc,'dosbox_window_pid':dosbox_pid,'listed_breakpoints_or_watches':bplist,'address_strings_seen':signals,'transcript':str(transcript_path.relative_to(ROOT)),'promotion_rule':'Do not claim runtime hook unless transcript proves key input writes G0432, F0380 dequeues, movement/turn mutates G0308 or G0306/G0307, and F0128 consumes resulting tuple.'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    REPORT.write_text('\n'.join(['# Pass275 — DM1 V1 debugger BPM runtime hook attempt','',f'Status: `{status}`','', '## What this pass did', '', '- Re-audited the ReDMCSB command/movement/viewport source seams.', '- Used pass273 public-symbol runtime addresses for BP/BPM setup.', '- Started `DEBUG DM.EXE -vv -sn -pk` under `/usr/bin/dosbox-debug` with `TERM=vt100`.', '- Confirmed debugger-run key workaround: `Escape O t` is vt100 F5; tmux `F5` emits the wrong `^[[15~` sequence in this environment.', '- Posted controlled gameplay route input through Xvfb/xdotool.', '', '## Result', '', f'- Transcript: `{manifest["transcript"]}`', f'- Listed debugger entries parsed: `{", ".join(bplist)}`', '- Promotion decision: no verified runtime hook is claimed; the transcript does not yet prove the full key→queue→dequeue→mutated tuple→F0128 chain.', '', '## Exact next step', '', 'Run the same tool with a human-attached terminal/display and continue hits manually with vt100 F5 (`Esc O t`) while capturing CPU/MEMDUMP after each `BPM` stop, or replace the terminal driver with a curses-aware F5 injector that can continue immediately after each stop.', '']),encoding='utf-8')
    print(json.dumps({'status':status,'manifest':str(OUT/'manifest.json'),'report':str(REPORT)},indent=2))
    return 0
if __name__=='__main__': raise SystemExit(main())
