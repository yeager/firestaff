#!/usr/bin/env python3
"""Pass373 verifier: launcher route reaches live viewport redraw and source-locked wall/occlusion renderer path."""
from __future__ import annotations
import json, os, pathlib, re, shutil, subprocess, sys
from datetime import datetime, timezone
from typing import Any
ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = pathlib.Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(pathlib.Path.home()/".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
DM1_DATA = pathlib.Path(os.environ.get("FIRESTAFF_DM1_CANONICAL_DATA", "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"))
BUILD_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS373_BUILD_DIR", str(pathlib.Path.home()/".openclaw/data/firestaff-builds/pass373-verify")))
HOME_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS373_HOME_DIR", str(pathlib.Path.home()/".openclaw/data/firestaff-homes/pass373-verify")))
SCRIPT = "enter,down,down,down,down,down,down,enter,right,up"
EXPECTED_STATUS = "PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH_PROVED"
DUNVIEW_LOCKS = [
    {"file":"DUNVIEW.C","lines":"2962-3003","function":"F0098_DUNGEONVIEW_DrawFloorAndCeiling","claim":"viewport floor/ceiling base is drawn into G0296/G0087 before wall passes","markers":["void F0098_DUNGEONVIEW_DrawFloorAndCeiling(","F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);","F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea);","G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;"]},
    {"file":"DUNVIEW.C","lines":"3048-3082","function":"F0100/F0101/F0102 blitters","claim":"wall, opaque wall, and door bitmaps target the viewport bitmap","markers":["void F0100_DUNGEONVIEW_DrawWallSetBitmap(","void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(","void F0102_DUNGEONVIEW_DrawDoorBitmap(","G0296_puc_Bitmap_Viewport"]},
    {"file":"DUNVIEW.C","lines":"4547-4910","function":"F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","claim":"ordered cell contents use source cell-order nibbles and normalized view cells","markers":["STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(","P0146_ui_OrderedViewCellOrdinals >>= 4;","L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);","P0141_T_Thing = L0146_T_FirstThingToDraw;"]},
    {"file":"DUNVIEW.C","lines":"6400-6835","function":"F0116/F0117/F0118 D3 side/center squares","claim":"D3 wall/doorpass branches draw wall panels and return to enforce occlusion","markers":["F0100_DUNGEONVIEW_DrawWallSetBitmap(G0698_puc_Bitmap_WallSet_Wall_D3LCR","F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT","C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT","return;"]},
    {"file":"DUNVIEW.C","lines":"7244-7937","function":"F0121/F0124 D2C/D1C center squares","claim":"center wall/door branches draw opaque/door layers before ordered contents","markers":["STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(","STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(","F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G3013_i_WallSet_Wall_D1C","F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"]},
    {"file":"DUNVIEW.C","lines":"8318-8618","function":"F0128_DUNGEONVIEW_Draw_CPSF","claim":"main viewport pass samples view squares, draws far-to-near D3/D2/D1/D0, then requests viewport blit","markers":["void F0128_DUNGEONVIEW_Draw_CPSF(","F0098_DUNGEONVIEW_DrawFloorAndCeiling(","F0116_DUNGEONVIEW_DrawSquareD3L(","F0121_DUNGEONVIEW_DrawSquareD2C(","F0124_DUNGEONVIEW_DrawSquareD1C(","F0127_DUNGEONVIEW_DrawSquareD0C(","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]},
]
OTHER_SOURCE_LOCKS = [
    {"file":"DRAWVIEW.C","lines":"709-722","function":"F0097_DUNGEONVIEW_DrawViewport","claim":"source draw pass flips the viewport redraw request and waits for vblank","markers":["void F0097_DUNGEONVIEW_DrawViewport(","G0324_B_DrawViewportRequested = C1_TRUE;","M526_WaitVerticalBlank();"]},
]
PRODUCT_MARKERS = [
    {"file":"main_loop_m11.c","claim":"script token route feeds the active game view and redraw result calls M11_GameView_Draw","markers":["m11_next_script_input(&scriptCursor)","M11_GameView_HandleInput(&gameView, input)","if (result == M11_GAME_INPUT_REDRAW)","M11_GameView_Draw(&gameView"]},
    {"file":"main_loop_m11.c","claim":"runtime probe records the same live movement pipeline result used by the redraw route","markers":["FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON","lastDm1V1MovementPipelineResult.core.queue.dequeued","lastDm1V1MovementPipelineResult.viewportDirty"]},
    {"file":"m11_game_view.c","claim":"input dispatch enters DM1 V1 compat command pipeline and returns redraw for dirty viewport","markers":["m11_dm1_v1_pipeline_command_for_input","DM1_V1_MovementPipeline_EnqueueCommandPc34Compat","DM1_V1_MovementPipeline_ProcessOneTickPc34Compat","lastDm1V1MovementPipelineResult.viewportDirty","return M11_GAME_INPUT_REDRAW"]},
    {"file":"m11_game_view.c","claim":"normal V1 viewport renderer draws source-backed wall/occlusion layers before debug-only procedural fallback","markers":["static void m11_draw_viewport(","m11_draw_dm1_side_walls(state","m11_draw_dm1_front_walls(state","m11_draw_dm1_center_doors(state","m11_dm1_nearest_blocking_center_depth_index(cells)","m11_draw_dm1_side_destroyed_door_masks(state","m11_draw_dm1_side_contents(state","if (state->showDebugHUD)"]},
]
PRIOR_GATES = [
    [sys.executable, "tools/verify_pass361_dm1_v1_viewport_occlusion_redraw_order_gate.py"],
    [sys.executable, "tools/verify_pass362_dm1_v1_viewport_walls_source_lock_landable.py"],
    [sys.executable, "scripts/verify_dm1_v1_viewport_wall_draw_order_source_lock.py"],
    [sys.executable, "tools/verify_v1_viewport_occlusion_gate.py"],
    [sys.executable, "tools/verify_v1_viewport_side_wall_occlusion_gate.py"],
]
def run(cmd:list[str], *, env:dict[str,str]|None=None, timeout:int=180)->dict[str,Any]:
    p=subprocess.run(cmd,cwd=ROOT,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout)
    return {"cmd":[str(c) for c in cmd],"returncode":p.returncode,"outputTail":p.stdout[-5000:]}
def compact(s:str)->str: return " ".join(s.split())
def block(path:pathlib.Path, spec:str)->str:
    lines=path.read_text(encoding="latin-1",errors="replace").splitlines(); out=[]
    for part in spec.split(","):
        a,b=([int(x) for x in part.split("-")] if "-" in part else (int(part),int(part)))
        out.append("\n".join(lines[a-1:b]))
    return "\n".join(out)
def function_body(path:pathlib.Path, name:str)->str:
    text=path.read_text(errors="replace"); m=re.search(r"static\s+void\s+"+re.escape(name)+r"\s*\(", text)
    if not m: return ""
    brace=text.find("{", m.start()); depth=0
    for pos in range(brace, len(text)):
        if text[pos]=="{": depth+=1
        elif text[pos]=="}":
            depth-=1
            if depth==0: return text[m.start():pos+1]
    return ""
def verify_markers(items:list[dict[str,Any]], root:pathlib.Path)->list[dict[str,Any]]:
    rows=[]
    for item in items:
        p=root/item["file"]
        text=block(p,item["lines"]) if "lines" in item and p.exists() else (p.read_text(errors="replace") if p.exists() else "")
        missing=[m for m in item["markers"] if compact(m) not in compact(text)]
        rows.append({**item,"path":str(p),"ok":p.exists() and not missing,"missingMarkers":missing})
    return rows
def assert_order(text:str, needles:list[str])->bool:
    pos=-1
    for n in needles:
        p=text.find(n,pos+1)
        if p<0: return False
        pos=p
    return True
def main()->int:
    OUT_DIR.mkdir(parents=True,exist_ok=True); checks=[]
    source=verify_markers(DUNVIEW_LOCKS+OTHER_SOURCE_LOCKS, REDMCSB)
    product=verify_markers(PRODUCT_MARKERS, ROOT)
    checks += [{"kind":"redmcsb_source_lock","file":r["file"],"lines":r.get("lines"),"function":r.get("function"),"ok":r["ok"],"missingMarkers":r["missingMarkers"]} for r in source]
    checks += [{"kind":"firestaff_product_lock","file":r["file"],"claim":r["claim"],"ok":r["ok"],"missingMarkers":r["missingMarkers"]} for r in product]
    body=function_body(ROOT/"m11_game_view.c", "m11_draw_viewport")
    order=["m11_draw_viewport_background(state", "m11_draw_dm1_floor_pits(state", "m11_draw_dm1_side_walls(state", "m11_draw_dm1_front_walls(state", "m11_draw_dm1_center_doors(state", "m11_dm1_nearest_blocking_center_depth_index(cells)", "m11_draw_dm1_side_contents(state", "if (state->showDebugHUD)"]
    checks.append({"kind":"firestaff_viewport_order_lock","file":"m11_game_view.c","function":"m11_draw_viewport","ok":bool(body) and assert_order(body,order),"orderedMarkers":order})
    for cmd in PRIOR_GATES:
        r=run(cmd, timeout=240); checks.append({"kind":"prior_wall_occlusion_gate","cmd":cmd,"ok":r["returncode"]==0,"result":r})
    if BUILD_DIR.exists(): shutil.rmtree(BUILD_DIR)
    r=run(["cmake","-S",str(ROOT),"-B",str(BUILD_DIR),"-G","Ninja"], timeout=180); checks.append({"kind":"cmake_configure","ok":r["returncode"]==0,"result":r})
    r=run(["cmake","--build",str(BUILD_DIR),"--target","firestaff","--parallel","1"], timeout=900); checks.append({"kind":"cmake_build_firestaff","ok":r["returncode"]==0,"result":r})
    if HOME_DIR.exists(): shutil.rmtree(HOME_DIR)
    HOME_DIR.mkdir(parents=True,exist_ok=True)
    probe_json=OUT_DIR/"launcher_route_viewport_redraw_probe.json"
    env=os.environ.copy(); env.update({"HOME":str(HOME_DIR),"SDL_VIDEODRIVER":"dummy","FIRESTAFF_AUTOTEST":"1","FIRESTAFF_FAIL_IF_NO_LAUNCH":"1","FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON":str(probe_json)})
    cmd=["timeout","45s",str(BUILD_DIR/"firestaff"),"--duration","9000","--width","1920","--height","1080","--data-dir",str(DM1_DATA),"--script",SCRIPT]
    r=run(cmd, env=env, timeout=60); checks.append({"kind":"launcher_route_runtime_probe","script":SCRIPT,"ok":r["returncode"]==0 and probe_json.exists(),"result":r})
    runtime={}; live_ok=False
    if probe_json.exists():
        runtime=json.loads(probe_json.read_text()); party=runtime.get("party",{}); pipe=runtime.get("pipeline",{})
        live_ok=(runtime.get("launchedEver")==1 and runtime.get("active")==1 and party.get("mapIndex")==0 and party.get("mapX")==0 and party.get("mapY")==3 and party.get("direction")==3 and pipe.get("dequeued")==1 and pipe.get("command")==3 and pipe.get("stepApplied")==1 and pipe.get("movementBlocked")==0 and pipe.get("anyMovementOccurred")==1 and pipe.get("viewportDirty")==1)
    checks.append({"kind":"live_runtime_redraw_state","ok":live_ok,"observed":runtime,"expected":"launcher route reaches active DM1 V1 state; forward command applies one movement step and sets viewportDirty=1"})
    ok=all(c.get("ok") for c in checks); status=EXPECTED_STATUS if ok else "BLOCKED_PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH"
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest={"schema":f"{PASS}.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"repo":str(ROOT),"branch":run(["git","branch","--show-current"])["outputTail"].strip(),"head":run(["git","rev-parse","HEAD"])["outputTail"].strip(),"sourceRoot":str(REDMCSB),"dm1Data":str(DM1_DATA),"script":SCRIPT,"probeJson":str(probe_json.relative_to(ROOT)),"sourceLocks":source,"productLocks":product,"checks":checks,"scope":"route-token launcher input -> M11_GameView_HandleInput -> DM1 V1 movement pipeline viewportDirty -> M11_GameView_Draw -> source-locked wall/occlusion renderer path","notClaimed":["original DOS keyboard-buffer/NumLock parity","DOSBox/FIRES CS:IP debugger hit","pixel-perfect viewport parity"]}
    MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
    lines=["# Pass373 — DM1 V1 launcher viewport redraw wall/occlusion path","",f"Status: `{status}`","","## Verdict",""]
    if ok:
        lines.append("The full Firestaff launcher route-token path reaches a live DM1 V1 movement step, marks the viewport dirty, and source-locks the consequent redraw path into the normal V1 wall/door/occlusion renderer stack.")
    else:
        lines.append("The path is still blocked; inspect the manifest checks for the first failing source/runtime gate.")
    lines += ["", "## Runtime proof", ""]
    lines.append("- Script: " + SCRIPT)
    lines.append("- Probe JSON: " + manifest["probeJson"])
    lines.append("- Party: " + json.dumps(runtime.get("party", {}), sort_keys=True))
    lines.append("- Pipeline: " + json.dumps(runtime.get("pipeline", {}), sort_keys=True))
    lines += ["", "## ReDMCSB source audit anchors", ""]
    for item in DUNVIEW_LOCKS+OTHER_SOURCE_LOCKS:
        lines.append("- {file}:{lines} — {function}: {claim}".format(**item))
    lines += ["", "## Firestaff path anchors", ""]
    for item in PRODUCT_MARKERS:
        lines.append("- {file} — {claim}".format(**item))
    lines += ["", "## Gates", ""]
    for c in checks:
        if c["kind"] in ("prior_wall_occlusion_gate","cmake_configure","cmake_build_firestaff","launcher_route_runtime_probe","live_runtime_redraw_state","firestaff_viewport_order_lock"):
            lines.append("- {kind} ok={ok}".format(kind=c["kind"], ok=c.get("ok")))
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    lines += ["","## Scope guard","","- This is not original DOS keyboard-buffer/NumLock parity.","- This is not a DOSBox/FIRES debugger-hit proof.","- This is not pixel-perfect viewport parity."]
    REPORT.write_text("\n".join(lines)+"\n")
    print(json.dumps({"status":status,"manifest":str(MANIFEST.relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT))},indent=2))
    return 0 if ok else 1
if __name__=="__main__": raise SystemExit(main())
