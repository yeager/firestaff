#!/usr/bin/env python3
"""Source-lock the DM1/V1 entrance input-wait handoff to ReDMCSB."""
from __future__ import annotations
import json, os, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", "/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"))
EVIDENCE_JSON = ROOT / "parity-evidence/verification/v1_entrance_input_wait_redmcsb_gate.json"
RANGES = [("ENTRANCE.C",850,883),("ENTRANCE.C",906,943),("COMMAND.C",551,577),("entrance_frontend_pc34_compat.c",39,58),("entrance_frontend_pc34_compat.c",61,99),("entrance_frontend_pc34_compat.c",102,103),("entrance_keyboard_routes_pc34_compat.c",2,3)]

def read(path: Path) -> str:
    if not path.is_file(): raise AssertionError(f"missing source file: {path}")
    enc = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=enc, errors="replace")

def line_no(text: str, off: int) -> int:
    return text.count("\n", 0, off) + 1

def excerpt(rel: str, start: int, end: int, needles: list[str]) -> None:
    path = REDMCSB_ROOT / rel if rel in {"ENTRANCE.C", "COMMAND.C"} else ROOT / rel
    body = "\n".join(read(path).splitlines()[start-1:end])
    missing = [n for n in needles if n not in body]
    if missing: raise AssertionError(f"{rel}:{start}-{end} missing {missing}")
    print(f"sourceRange={path}:{start}-{end} status=ok")

def require_order(text: str, label: str, markers: list[tuple[str,str]]) -> list[tuple[str,int]]:
    out=[]; cursor=-1
    for name, needle in markers:
        pos=text.find(needle)
        if pos < 0: raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        if pos <= cursor: raise AssertionError(f"{label}: {name} is out of order")
        out.append((name,pos)); cursor=pos
    return out

def window(text: str, start: str, end: str|None=None) -> tuple[int,str]:
    s=text.find(start)
    if s < 0: raise AssertionError(f"missing start marker {start!r}")
    e=text.find(end, s+len(start)) if end else min(len(text), s+12000)
    if e < 0: raise AssertionError(f"missing end marker {end!r}")
    return s, text[s:e]

def func(text: str, name: str) -> tuple[int,str]:
    m=re.search(r"(?m)^(?:static\s+const\s+char\*|const\s+char\*|unsigned\s+int|int)\s+"+re.escape(name)+r"\s*\(", text)
    if not m: raise AssertionError(f"missing function {name}")
    b=text.find("{", m.end()); depth=0
    for i in range(b, len(text)):
        if text[i]=='{': depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0: return m.start(), text[m.start():i+1]
    raise AssertionError(f"unterminated function {name}")

def verify_redmcsb() -> list[str]:
    ep=REDMCSB_ROOT/"ENTRANCE.C"; et=read(ep)
    cp=REDMCSB_ROOT/"COMMAND.C"; ct=read(cp)
    notes=[]
    s,w=window(et,"        do {\n                F0439_STARTEND_DrawEntrance();","#ifdef MEDIA748_A36M_A31E")
    for name,pos in require_order(w,"ReDMCSB entrance wait",[("draw entrance","F0439_STARTEND_DrawEntrance()"),("show pointer","M523_MOUSE_ShowPointer()"),("discard input","F0357_COMMAND_DiscardAllInput()"),("waiting mode","G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE"),("vblank wait","M526_WaitVerticalBlank()"),("pc enter maps load","G0298_B_NewGame = C001_MODE_LOAD_DUNGEON"),("process queue","F0380_COMMAND_ProcessQueue_CPSC()"),("wait loop condition","G0298_B_NewGame == C099_MODE_WAITING_ON_ENTRANCE"),("credits redraw loop","G0298_B_NewGame == M567_COMMAND_ENTRANCE_DRAW_CREDITS")]): notes.append(f"ReDMCSB entrance wait {name}: {ep}:{line_no(et,s+pos)}")
    s,w=window(et,"#ifdef MEDIA165_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_X30J_P20JA\n        F0060_SOUND_Play_CPSX(G0566_puc_Graphic534_Sound01Switch","        if (G0298_B_NewGame) {")
    for name,pos in require_order(w,"ReDMCSB entrance post-command",[("switch sound","SOUND"),("source delay","F0022_MAIN_Delay(20)"),("hide pointer","M522_MOUSE_HidePointer()")]): notes.append(f"ReDMCSB entrance post-command {name}: {ep}:{line_no(et,s+pos)}")
    s,w=window(ct,"KEYBOARD_INPUT G2195_as_PrimaryKeyboardInput_Entrance","#ifdef MEDIA496_F20E")
    for name,pos in require_order(w,"ReDMCSB entrance keyboard",[("table","G2195_as_PrimaryKeyboardInput_Entrance"),("enter dungeon","C200_COMMAND_ENTRANCE_ENTER_DUNGEON"),("quit sibling","C216_COMMAND_QUIT"),("I34E enter scancode","0x001C")]): notes.append(f"ReDMCSB entrance keyboard {name}: {cp}:{line_no(ct,s+pos)}")
    excerpt("ENTRANCE.C",850,883,["F0439_STARTEND_DrawEntrance","F0357_COMMAND_DiscardAllInput","C099_MODE_WAITING_ON_ENTRANCE","M526_WaitVerticalBlank","F0380_COMMAND_ProcessQueue_CPSC"])
    excerpt("ENTRANCE.C",906,943,["F0022_MAIN_Delay(20)","M522_MOUSE_HidePointer","F0438_STARTEND_OpenEntranceDoors"])
    excerpt("COMMAND.C",551,577,["G2195_as_PrimaryKeyboardInput_Entrance","C200_COMMAND_ENTRANCE_ENTER_DUNGEON","C216_COMMAND_QUIT","0x001C"])
    return notes

def verify_firestaff() -> list[str]:
    fp=ROOT/"entrance_frontend_pc34_compat.c"; ft=read(fp)
    kp=ROOT/"entrance_keyboard_routes_pc34_compat.c"; kt=read(kp)
    notes=[]
    s,w=func(ft,"entrance_event_line")
    for name,pos in require_order(w,"Firestaff entrance event evidence",[("wait event","ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT"),("wait citation","ENTRANCE.C:850-883 draw entrance, discard input, wait on VBlank loop until command"),("switch citation","ENTRANCE.C:906-934 plays switch sound after Enter/command"),("delay citation","ENTRANCE.C:935 waits F0022_MAIN_Delay(20), then hides pointer")]): notes.append(f"Firestaff entrance evidence {name}: {fp}:{line_no(ft,s+pos)}")
    s,w=func(ft,"ENTRANCE_Compat_GetSourceAnimationStep")
    for name,pos in require_order(w,"Firestaff entrance source sequence",[("step 4","sourceStepOrdinal == 4u"),("wait kind","ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT"),("wait vblank","step.vblankLoopCount = 1u"),("step 5","sourceStepOrdinal == 5u"),("switch kind","ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND"),("step 6","sourceStepOrdinal == 6u"),("delay ticks","step.delayTicks = 20u"),("doors","sourceStepOrdinal >= 7u && sourceStepOrdinal <= 37u")]): notes.append(f"Firestaff entrance sequence {name}: {fp}:{line_no(ft,s+pos)}")
    s,w=func(ft,"ENTRANCE_Compat_GetSourceAnimationEvidence")
    for name,pos in require_order(w,"Firestaff entrance summary",[("wait summary","wait on VBlank/input loop"),("delay summary","F0022_MAIN_Delay(20)"),("door summary","F0438 opens doors")]): notes.append(f"Firestaff entrance summary {name}: {fp}:{line_no(ft,s+pos)}")
    for needle in ["COMMAND.C:551-577","C200_COMMAND_ENTRANCE_ENTER_DUNGEON","C216_COMMAND_QUIT"]:
        if needle not in kt: raise AssertionError(f"missing keyboard evidence marker {needle!r}")
    notes.append(f"Firestaff entrance keyboard evidence: {kp}:2")
    excerpt("entrance_frontend_pc34_compat.c",39,58,["ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT","ENTRANCE.C:850-883","ENTRANCE.C:935"])
    excerpt("entrance_frontend_pc34_compat.c",61,99,["sourceStepOrdinal == 4u","step.vblankLoopCount = 1u","step.delayTicks = 20u"])
    excerpt("entrance_frontend_pc34_compat.c",102,103,["wait on VBlank/input loop","F0022_MAIN_Delay(20)","F0438 opens doors"])
    excerpt("entrance_keyboard_routes_pc34_compat.c",2,3,["COMMAND.C:551-577","C200_COMMAND_ENTRANCE_ENTER_DUNGEON","C216_COMMAND_QUIT"])
    return notes

def verify_json() -> None:
    data=json.loads(EVIDENCE_JSON.read_text())
    if data.get("gate") != "v1_entrance_input_wait_redmcsb": raise AssertionError("evidence JSON gate id mismatch")
    got={(x.get("file"),x.get("start"),x.get("end")) for x in data.get("sourceRanges",[])}
    miss=set(RANGES)-got
    if miss: raise AssertionError(f"evidence JSON missing ranges: {sorted(miss)}")
    nc="\n".join(data.get("nonClaims",[]))
    if "runtime pixel parity" not in nc or "DANNESBURK" not in nc: raise AssertionError("evidence JSON must state runtime pixel parity and DANNESBURK non-claims")
    print(f"evidence={EVIDENCE_JSON.relative_to(ROOT)} status=ok")

def main() -> None:
    print("probe=v1_entrance_input_wait_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    notes=verify_redmcsb()+verify_firestaff(); verify_json()
    print("v1EntranceInputWaitRedmcsbGateOk=1")
    for n in notes: print("- "+n)
if __name__ == "__main__":
    try: main()
    except Exception as e:
        print(f"FAIL: {e}", file=sys.stderr); sys.exit(1)
