#!/usr/bin/env python3
from __future__ import annotations
import argparse, importlib.util, json, re, shutil, subprocess, tempfile, threading, time, os
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass379_dm1_v1_true_stop_codepath_probe"
OUT = ROOT / "parity-evidence/verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS330_PATH = ROOT / "tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py"
F0128 = "23AD:40FE"
F0097 = "2809:1EFF"
F0097_ENTRY = "2809:1E31"
PROBE = "07FB:01EB"
LOAD_PREFIX = "wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200"
LOAD_SUFFIX = "one wait:3500"
MOVE_ROUTE = "kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900"
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)

def load_pass330():
    spec=importlib.util.spec_from_file_location("pass330", PASS330_PATH)
    mod=importlib.util.module_from_spec(spec); assert spec and spec.loader; spec.loader.exec_module(mod); return mod

def clean(text: str) -> str:
    return load_pass330().clean(text)

def drain(child, seconds: float) -> str:
    return load_pass330().drain(child, seconds)

def dbg(child, cmd: str, log: list[dict[str, Any]], transcript: list[str], wait=.75) -> str:
    child.sendline(cmd); time.sleep(.25); out=drain(child, wait); transcript.append(out); log.append({"t":time.time(),"cmd":cmd,"excerpt":clean(out)[-1200:]}); return out

def code_lines(text: str) -> list[str]:
    return [m.group(0)[:180] for m in CODE_LINE_RE.finditer(clean(text))]

def last_code_addr(text: str) -> str|None:
    matches=[m.group('addr').upper() for m in CODE_LINE_RE.finditer(clean(text))]
    return matches[-1] if matches else None

def window_from_line(text: str, addr: str, radius=8) -> list[str]:
    lines=clean(text).splitlines(); out=[]
    for i,l in enumerate(lines):
        if addr in l.upper():
            out.extend(lines[max(0,i-radius):i+radius+1])
    return out[-80:]

def monitor_probe(child, seconds:int, transcript:list[str], cmdlog:list[dict[str,Any]], stops:list[dict[str,Any]], stop_event:threading.Event):
    deadline=time.time()+seconds; buf=''; saw_running=False; probe_hits=0
    while time.time()<deadline:
        chunk=drain(child,.25)
        if chunk:
            transcript.append(chunk); buf += chunk; c=clean(buf)
            if '(Running)' in c: saw_running=True
            if '(Running)' in c and '->' in c.split('(Running)',1)[-1]:
                post=c.split('(Running)',1)[-1]; addr=last_code_addr(post)
                row={"t":time.time(),"addr":addr,"postRunningCodeLines":code_lines(post)[-14:],"probeWindow":window_from_line(post, addr or '')}
                stops.append(row)
                if addr == PROBE:
                    probe_hits += 1
                    details=[]
                    for cmd in ["CPU", "U 07FB:01D1", "D DS:3C92", "D DS:3C94", "D DS:3CE0", "BPLIST"]:
                        details.append({"cmd":cmd,"out":clean(dbg(child, cmd, cmdlog, transcript))[-1800:]})
                    row["details"] = details
                    if probe_hits >= 4:
                        stop_event.set(); break
                    child.send('\x1bOt'); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue after 07FB probe hit","addr":addr}); buf=''; continue
                if addr in {F0128,F0097,F0097_ENTRY}:
                    stop_event.set(); break
                child.send('\x1bOt'); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue non-target stop","addr":addr}); buf=''
        time.sleep(.05)
    return saw_running

def run_probe(seconds:int) -> dict[str,Any]:
    mod=load_pass330(); start=time.time(); transcript=[]; cmdlog=[]; routelog=[]; stops=[]
    OUT.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='firestaff-pass379-') as td:
        stage=Path(td)/'dos'; shutil.copytree(mod.ORIG, stage); conf=Path(td)/'dosbox.conf'; mod.write_conf(conf, stage)
        display=f':{200+(os.getpid()%200)}'
        xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.8)
        if xvfb.poll() is not None:
            return {"ran": True, "stage": "xvfb", "blocker": f"Xvfb failed for {display}"}
        child=pexpect.spawn('dosbox-debug',['-conf',str(conf),'-exit'], env={**os.environ,'DISPLAY':display,'TERM':'vt100'}, encoding='utf-8', timeout=2, echo=False)
        child.delaybeforesend=.05
        try:
            time.sleep(3); transcript.append(drain(child,1)); win=mod.find_win(display)
            if not win or not str(win).isdigit(): return {"ran":True,"stage":"dosbox window","blocker":f"dosbox window not found: {win}"}
            child.send('\x1bOt'); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run unarmed to load/menu prefix"})
            mod.drive(display, win, LOAD_PREFIX, routelog)
            ok=mod.wait_prompt_by_pause(child, display, win, cmdlog, transcript, 'pass379 load/menu prefix pause')
            if not ok: return {"ran":True,"stage":"load-ready marker","blocker":"no prompt after load/menu prefix"}
            for cmd in ['BPDEL *', f'BP {F0128}', f'BP {F0097}', f'BP {F0097_ENTRY}', f'BP {PROBE}', 'BPLIST']:
                dbg(child,cmd,cmdlog,transcript)
            arm_time=time.time(); child.send('\x1bOt'); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run after pass379 multi-arm"})
            stop_event=threading.Event(); t=threading.Thread(target=mod.drive,args=(display,win,LOAD_SUFFIX+' '+MOVE_ROUTE,routelog,stop_event),daemon=True); t.start()
            saw_running=monitor_probe(child,seconds,transcript,cmdlog,stops,stop_event); t.join(timeout=1)
            final_pause_addr = None
            retained = None
            if not stop_event.is_set():
                ok=mod.wait_prompt_by_pause(child, display, win, cmdlog, transcript, 'pass379 final retention pause')
                if ok:
                    final_pause_addr = last_code_addr(''.join(transcript))
                    dbg(child,'BPLIST',cmdlog,transcript)
                    bplist = clean(cmdlog[-1].get('excerpt','')).upper()
                    retained = all(x in bplist for x in [F0128, F0097, F0097_ENTRY, PROBE])
            route_after_arm=any(r.get('t',0)>arm_time for r in routelog)
            addrs=[s.get('addr') for s in stops]
            direct={"f0128_23AD_40FE":F0128 in addrs,"f0097_2809_1EFF":F0097 in addrs,"f0097_entry_2809_1E31":F0097_ENTRY in addrs,"probe_07FB_01EB":PROBE in addrs}
            if direct['f0128_23AD_40FE'] and (direct['f0097_2809_1EFF'] or direct['f0097_entry_2809_1E31']):
                status="PASS379_STRICT_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN"; blocker=None; stage=None
            elif direct['f0128_23AD_40FE']:
                status="PASS379_STRICT_F0128_ONLY_F0097_BLOCKED"; blocker="F0128 strict stop recaptured but F0097/VIDRV did not stop in bounded continuation"; stage="f0097 continuation"
            elif direct['probe_07FB_01EB']:
                status="BLOCKED_PASS379_07FB_TRUE_STOP_PROVES_CONTROL_F0128_MAP_OR_PATH_UNRESOLVED"; blocker="Strict 07FB:01EB stop(s) prove debugger true-stop control after route; F0128/F0097 candidates still never stop, narrowing blocker to FIRES CS:IP map/source-path mismatch rather than input loop or debugger prompt control"; stage="candidate address/source path"
            elif retained is True:
                status="BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED"; blocker=f"F0128/F0097/07FB breakpoints retained and route input delivered, but no strict candidate stop occurred; final forced pause landed at {final_pause_addr}, showing the pass377 07FB:01EB pause site is not stable and narrowing the blocker to FIRES CS:IP mapping/source-path rather than prompt or input delivery"; stage="candidate address/source path"
            else:
                status="BLOCKED_PASS379_NO_DIRECT_PROBE_STOP"; blocker="No strict stops at F0128/F0097 or 07FB:01EB in bounded route"; stage="probe stop"
            return {"ran":True,"durationSeconds":round(time.time()-start,3),"status":status,"stage":stage,"blocker":blocker,"sawRunning":saw_running,"routeInputAfterArming":route_after_arm,"breakpointRetainedPostRoute":retained,"finalPauseCodeAddr":final_pause_addr,"routeLog":routelog,"commandLog":cmdlog,"stops":stops,"directHits":direct}
        finally:
            try:
                transcript.append(drain(child,.5)); child.terminate(force=True)
            except Exception: pass
            xvfb.terminate();
            try: xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired: xvfb.kill()
            (OUT/'pass379_codepath_probe.clean.txt').write_text(clean(''.join(transcript))[-350000:]+'\n', encoding='utf-8')

def source_summary(mod) -> list[dict[str,Any]]:
    rows=mod.source_audit()
    rows.append({"id":"post_route_probe_site","file":"pass377 transcript","ok":True,"anchors":{"07FB:01EB":"post-route pause call 0A98:05EB"},"missing":[]})
    return rows

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=75); args=ap.parse_args(); args.seconds=max(10,min(75,args.seconds))
    mod=load_pass330(); rt=run_probe(args.seconds)
    cmdlog=rt.pop('commandLog',[]); routelog=rt.pop('routeLog',[])
    (OUT/'pass379_route_keylog.json').write_text(json.dumps(routelog,indent=2,sort_keys=True)+'\n')
    (OUT/'pass379_command_log.json').write_text(json.dumps(cmdlog,indent=2,sort_keys=True)+'\n')
    manifest={"schema":PASS+'.v1',"timestampUtc":datetime.now(timezone.utc).isoformat(),"status":rt.get('status'),"repo":str(ROOT),"branch":mod.run(['git','branch','--show-current']).stdout.strip(),"head":mod.run(['git','rev-parse','HEAD']).stdout.strip(),"sourceAudit":source_summary(mod),"priorManifests":{"pass330":"parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json","pass360":"parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json","pass377":"parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/manifest.json"},"addresses":{"F0128_DUNGEONVIEW_Draw_CPSF":F0128,"F0097_VIDRV_09_BlitViewPort_indirect_call":F0097,"F0097_DUNGEONVIEW_DrawViewport_entry":F0097_ENTRY,"postRoutePauseProbe":PROBE},"runtimeProbe":{**rt,"boundedSeconds":args.seconds,"transcript":str((OUT/'pass379_codepath_probe.clean.txt').relative_to(ROOT)),"routeKeylog":str((OUT/'pass379_route_keylog.json').relative_to(ROOT)),"commandLog":str((OUT/'pass379_command_log.json').relative_to(ROOT))},"blocker":rt.get('blocker'),"decision":"07FB:01EB is used as a direct control-path probe because pass377 paused there after route while F0128 was retained. A strict post-Running stop there narrows the active blocker away from input delivery/debugger prompt control and toward candidate F0128/F0097 CS:IP mapping or a missing source-path transition.","notPromotedBy":["BPLIST","BP command echo","tmux/capture-pane","source-only address binding","07FB stop alone"]}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    REPORT.write_text('\n'.join(["# Pass379 — DM1 V1 true-stop codepath probe","",f"Status: `{manifest['status']}`","","## Runtime result","",f"- Direct hits: `{manifest['runtimeProbe'].get('directHits')}`",f"- Saw running: `{manifest['runtimeProbe'].get('sawRunning')}`; route after arming: `{manifest['runtimeProbe'].get('routeInputAfterArming')}`",f"- Stage/blocker: `{manifest.get('blocker')}`",f"- Transcript: `{manifest['runtimeProbe'].get('transcript')}`","","## Narrowing","",manifest['decision'],"",f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`"])+"\n")
    print(json.dumps({"status":manifest['status'],"blocker":manifest.get('blocker'),"directHits":manifest['runtimeProbe'].get('directHits'),"manifest":str((OUT/'manifest.json').relative_to(ROOT))},indent=2,sort_keys=True))
    return 0
if __name__=='__main__': raise SystemExit(main())
