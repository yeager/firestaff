#!/usr/bin/env python3
"""Source-lock the DM1/V1 inventory toggle route to ReDMCSB."""
from __future__ import annotations
import json, os, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", "/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"))
EVIDENCE_JSON = ROOT / "parity-evidence/verification/v1_inventory_toggle_redmcsb_gate.json"

RANGES = [
    ("COMMAND.C",396,405), ("COMMAND.C",412,415), ("COMMAND.C",602,609),
    ("COMMAND.C",2180,2184), ("COMMAND.C",2296,2300),
    ("PANEL.C",2244,2305), ("PANEL.C",2314,2352), ("PANEL.C",2358,2429),
    ("m11_game_view.c",5188,5191), ("m11_game_view.c",19989,19995),
]

def read(path: Path) -> str:
    if not path.is_file(): raise AssertionError(f"missing source file: {path}")
    return path.read_text(encoding="latin-1" if path.suffix.upper()==".C" else "utf-8", errors="replace")

def line_no(text: str, off: int) -> int:
    return text.count("\n", 0, off) + 1

def excerpt(rel: str, start: int, end: int, needles: list[str]) -> None:
    path = REDMCSB_ROOT / rel if rel.endswith(".C") else ROOT / rel
    lines = read(path).splitlines()
    body = "\n".join(lines[start-1:end])
    miss = [n for n in needles if n not in body]
    if miss: raise AssertionError(f"{rel}:{start}-{end} missing {miss}")
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
    m=re.search(r"(?m)^(?:int|void|static\s+int|static\s+void)\s+"+re.escape(name)+r"\s*\(", text)
    if not m: raise AssertionError(f"missing function {name}")
    b=text.find("{", m.end()); depth=0
    for i in range(b, len(text)):
        if text[i]=='{': depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0: return m.start(), text[m.start():i+1]
    raise AssertionError(f"unterminated function {name}")

def verify_redmcsb() -> list[str]:
    cp = REDMCSB_ROOT / "COMMAND.C"; ct=read(cp)
    pp = REDMCSB_ROOT / "PANEL.C"; pt=read(pp)
    notes=[]
    s,w = window(ct, "MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement[9] = {\n        { C001_COMMAND_TURN_LEFT,               CM1_SCREEN_RELATIVE")
    for n,p in require_order(w,"movement right-click toggle",[("table","G0448_as_Graphic561_SecondaryMouseInput_Movement"),("C083 command","C083_COMMAND_TOGGLE_INVENTORY_LEADER"),("screen zone","C002_ZONE_SCREEN"),("right button","MASK0x0001_MOUSE_RIGHT_BUTTON")]): notes.append(f"ReDMCSB movement {n}: {cp}:{line_no(ct,s+p)}")
    s,w = window(ct, "        { C011_COMMAND_CLOSE_INVENTORY,                                 CM1_SCREEN_RELATIVE")
    for n,p in require_order(w,"inventory close routes",[("right-click close","C011_COMMAND_CLOSE_INVENTORY"),("screen zone","C002_ZONE_SCREEN"),("close icon","C566_ZONE_CLOSE_INVENTORY_ICON")]): notes.append(f"ReDMCSB inventory table {n}: {cp}:{line_no(ct,s+p)}")
    s,w = window(ct, "#ifdef MEDIA707_I34E_I34M", "#endif")
    for n,p in require_order(w,"I34E keyboard toggles",[("F1","C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0, 0x0002"),("F2","C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1, 0x0003"),("F3","C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2, 0x0004"),("F4","C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3, 0x0005")]): notes.append(f"ReDMCSB keyboard {n}: {cp}:{line_no(ct,s+p)}")
    s,w = window(ct, "if ((L1160_i_Command >= C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0)", "if (L1160_i_Command == C100_COMMAND_CLICK_IN_SPELL_AREA)")
    for n,p in require_order(w,"command queue toggle dispatch",[("range","L1160_i_Command <= C011_COMMAND_CLOSE_INVENTORY"),("candidate gate","!G0299_ui_CandidateChampionOrdinal"),("toggle cpse","F0355_INVENTORY_Toggle_CPSE(AL1159_i_ChampionIndex)"),("leader command","L1160_i_Command == C083_COMMAND_TOGGLE_INVENTORY_LEADER"),("leader exists","G0411_i_LeaderIndex != CM1_CHAMPION_NONE"),("leader toggle","F0355_INVENTORY_Toggle_CPSE(G0411_i_LeaderIndex)")]): notes.append(f"ReDMCSB queue {n}: {cp}:{line_no(ct,s+p)}")
    s,w = window(pt, "void F0355_INVENTORY_Toggle_CPSE")
    for n,p in require_order(w,"PANEL.C F0355 toggle",[("function","F0355_INVENTORY_Toggle_CPSE"),("dead champion return","!M516_CHAMPIONS[P0719_i_ChampionIndex].CurrentHealth"),("eye-mouth guard","G0333_B_PressingMouth || G0331_B_PressingEye"),("panel content","G0424_i_PanelContent = C00_PANEL_INVENTORY"),("previous ordinal","AL1102_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal"),("same closes","P0719_i_ChampionIndex = C04_CHAMPION_CLOSE_INVENTORY"),("clear ordinal","G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)"),("close chest","F0334_INVENTORY_CloseChest"),("movement arrows","F0395_MENUS_DrawMovementArrows"),("restore input","G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement"),("redraw dungeon","F0098_DUNGEONVIEW_DrawFloorAndCeiling"),("set ordinal","G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(P0719_i_ChampionIndex)"),("hatch arrows","F0136_VIDEO_HatchScreenBox(C009_ZONE_MOVEMENT_ARROWS"),("inventory graphic","F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY"),("draw slots","F0291_CHAMPION_DrawSlot(P0719_i_ChampionIndex, AL1102_ui_SlotIndex)"),("dirty flags","MASK0x4000_VIEWPORT | MASK0x1000_STATUS_BOX | MASK0x0800_PANEL | MASK0x0400_ICON")]): notes.append(f"ReDMCSB F0355 {n}: {pp}:{line_no(pt,s+p)}")
    excerpt("COMMAND.C",396,405,["C083_COMMAND_TOGGLE_INVENTORY_LEADER","C002_ZONE_SCREEN","MASK0x0001_MOUSE_RIGHT_BUTTON"])
    excerpt("COMMAND.C",412,415,["C011_COMMAND_CLOSE_INVENTORY","C566_ZONE_CLOSE_INVENTORY_ICON"])
    excerpt("COMMAND.C",602,609,["C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0","C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3"])
    excerpt("COMMAND.C",2180,2184,["C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0","C011_COMMAND_CLOSE_INVENTORY","F0355_INVENTORY_Toggle_CPSE"])
    excerpt("COMMAND.C",2296,2300,["C083_COMMAND_TOGGLE_INVENTORY_LEADER","G0411_i_LeaderIndex","F0355_INVENTORY_Toggle_CPSE"])
    excerpt("PANEL.C",2244,2305,["F0355_INVENTORY_Toggle_CPSE","G0424_i_PanelContent = C00_PANEL_INVENTORY","P0719_i_ChampionIndex = C04_CHAMPION_CLOSE_INVENTORY"])
    excerpt("PANEL.C",2314,2352,["G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)","G0448_as_Graphic561_SecondaryMouseInput_Movement","F0098_DUNGEONVIEW_DrawFloorAndCeiling"])
    excerpt("PANEL.C",2358,2429,["G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(P0719_i_ChampionIndex)","C017_GRAPHIC_INVENTORY","F0291_CHAMPION_DrawSlot"])
    return notes

def verify_firestaff() -> list[str]:
    path=ROOT/"m11_game_view.c"; text=read(path); notes=[]
    s,w = window(text, "if (input == M12_MENU_INPUT_INVENTORY_TOGGLE)", "/* While an overlay is open")
    if "showDebugHUD" in w: raise AssertionError("inventory toggle route should not branch on showDebugHUD")
    for n,p in require_order(w,"Firestaff input route",[("input","M12_MENU_INPUT_INVENTORY_TOGGLE"),("clear map","state->mapOverlayActive = 0"),("toggle","M11_GameView_ToggleInventoryPanel(state)"),("redraw","return M11_GAME_INPUT_REDRAW")]): notes.append(f"Firestaff input {n}: {path}:{line_no(text,s+p)}")
    s,w = func(text,"M11_GameView_ToggleInventoryPanel")
    if "mapOverlayActive" in w or "showDebugHUD" in w: raise AssertionError("toggle API must not collide with map/HUD state")
    for n,p in require_order(w,"Firestaff toggle state",[("null guard","if (!state) return 0"),("flip active","state->inventoryPanelActive = !state->inventoryPanelActive"),("open branch","if (state->inventoryPanelActive)"),("reset slot","state->inventorySelectedSlot = 0"),("return active","return state->inventoryPanelActive")]): notes.append(f"Firestaff toggle {n}: {path}:{line_no(text,s+p)}")
    hdr=read(ROOT/"m11_game_view.h")
    if "int M11_GameView_ToggleInventoryPanel(M11_GameViewState* state);" not in hdr: raise AssertionError("missing public toggle declaration")
    excerpt("m11_game_view.c",5188,5191,["M12_MENU_INPUT_INVENTORY_TOGGLE","mapOverlayActive = 0","M11_GameView_ToggleInventoryPanel"])
    excerpt("m11_game_view.c",19989,19995,["M11_GameView_ToggleInventoryPanel","inventoryPanelActive","inventorySelectedSlot = 0"])
    return notes

def verify_json():
    data=json.loads(EVIDENCE_JSON.read_text())
    if data.get("gate") != "v1_inventory_toggle_redmcsb": raise AssertionError("evidence JSON gate id mismatch")
    got={(x.get("file"),x.get("start"),x.get("end")) for x in data.get("sourceRanges",[])}
    miss=set(RANGES)-got
    if miss: raise AssertionError(f"evidence JSON missing ranges: {sorted(miss)}")
    nc="\n".join(data.get("nonClaims",[]))
    if "HUD" not in nc or "deprecated remote source" not in nc: raise AssertionError("evidence JSON must state HUD/deprecated-remote-source non-claims")
    print(f"evidence={EVIDENCE_JSON.relative_to(ROOT)} status=ok")

def main():
    print("probe=v1_inventory_toggle_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    lines=verify_redmcsb()+verify_firestaff(); verify_json()
    print("v1InventoryToggleRedmcsbGateOk=1")
    for line in lines: print("- "+line)
if __name__ == "__main__":
    try: main()
    except Exception as e:
        print(f"FAIL: {e}", file=sys.stderr); sys.exit(1)
