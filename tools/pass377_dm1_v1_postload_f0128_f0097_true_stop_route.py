#!/usr/bin/env python3
from __future__ import annotations
import argparse, importlib.util, json, subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass377_dm1_v1_postload_f0128_f0097_true_stop_route"
OUT = ROOT / "parity-evidence/verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source"
PASS330_PATH = ROOT / "tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py"

SOURCE_WINDOWS = [
    {"id":"clikmenu_turn_step_route", "file":"CLIKMENU.C", "lines":"142-174,180-237", "needles":["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE", "F0284_CHAMPION_SetPartyDirection", "void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0465_ai_Graphic561_MovementArrowToStepForwardCount", "G0466_ai_Graphic561_MovementArrowToStepRightCount"]},
    {"id":"dunview_f0128_entry_and_handoff", "file":"DUNVIEW.C", "lines":"8318-8338,8598-8612", "needles":["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
    {"id":"drawview_f0097_request", "file":"DRAWVIEW.C", "lines":"709-724,849-858", "needles":["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE", "M526_WaitVerticalBlank", "VIDRV_09_BlitViewPort"]},
    {"id":"vidrv_slot9_binding", "file":"VIDEODRV.C", "lines":"18-34,940-951,3566-3582", "needles":["extern void F8161_VIDRV_09_BlitViewPort", "F8161_VIDRV_09_BlitViewPort", "/*  9 */", "F8151_VIDRV_02_Blit"]},
]

def norm(s: str) -> str:
    return " ".join(s.split())

def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks=[]
    for part in spec.split(','):
        a,b = [int(x) for x in part.split('-',1)] if '-' in part else (int(part), int(part))
        chunks.append("\n".join(lines[a-1:b]))
    return "\n".join(chunks)

def audit_sources() -> list[dict[str, Any]]:
    rows=[]
    for spec in SOURCE_WINDOWS:
        path=SRC/spec["file"]
        text=source_window(path, spec["lines"]) if path.exists() else ""
        anchors={}
        all_lines=path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        for needle in spec["needles"]:
            for i,line in enumerate(all_lines,1):
                if norm(needle) in norm(line):
                    anchors[needle]=i
                    break
        missing=[n for n in spec["needles"] if norm(n) not in norm(text)]
        row = {k:v for k,v in spec.items() if k!="needles"}
        row.update({"path":str(path), "ok":path.exists() and not missing, "anchors":anchors, "missing":missing})
        rows.append(row)
    return rows

def load_json(rel: str) -> dict[str, Any]:
    p=ROOT/rel
    return json.loads(p.read_text()) if p.exists() else {"_missing": True}

def load_pass330():
    spec=importlib.util.spec_from_file_location("pass330", PASS330_PATH)
    mod=importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    spec.loader.exec_module(mod)
    return mod

def jsonable(x):
    if isinstance(x, (str, int, float, bool)) or x is None:
        return x
    if isinstance(x, dict):
        return {str(k): jsonable(v) for k, v in x.items()}
    if isinstance(x, (list, tuple)):
        return [jsonable(v) for v in x]
    return repr(x)

def write_report(m: dict[str, Any]) -> None:
    lines=["# Pass377 — DM1 V1 post-load F0128→F0097 true-stop route", "", f"Status: `{m['status']}`", "", "## Source audit", ""]
    for row in m["sourceAudit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` ok=`{row['ok']}` anchors=`{row['anchors']}`")
    promo=m.get("strictStopPromotion", {})
    lines += ["", "## Runtime attempt", "", f"- Strategy: `{m['runtimeProbe'].get('strategy')}`", f"- Method: {m['runtimeProbe'].get('method')}", f"- Direct hits: `{m['runtimeProbe'].get('directHits')}`", f"- Running observed: `{promo.get('sawRunning')}`; route after arming: `{promo.get('routeInputAfterArming')}`; retained post-route: `{promo.get('breakpointRetainedPostRoute')}`", f"- Post-route pause code addr: `{promo.get('postRoutePauseCodeAddr')}`", f"- Stage/blocker: `{m.get('blocker')}`", f"- Transcript: `{m['runtimeProbe'].get('transcript')}`", "", "## Decision", "", f"Strict true-stop promoted: `{promo.get('promoted')}`. This is a controlled post-load arming attempt from the pass360/pass376 contract. It does not promote viewport parity unless strict post-Running F0128 and later F0097/VIDRV stops both appear in order.", "", f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`"]
    REPORT.write_text("\n".join(lines)+"\n", encoding="utf-8")

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=75)
    args=ap.parse_args()
    args.seconds=max(10,min(75,args.seconds))
    OUT.mkdir(parents=True, exist_ok=True)
    source=audit_sources()
    mod=load_pass330()
    runtime=mod.run_one("post_load_arm_before_route", args.seconds)
    transcript_src=ROOT/"parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/post_load_arm_before_route.clean.txt"
    transcript_dst=OUT/"post_load_arm_before_route.clean.txt"
    OUT.mkdir(parents=True, exist_ok=True)
    if transcript_src.exists():
        transcript_dst.write_text(transcript_src.read_text(encoding="utf-8", errors="replace"), encoding="utf-8")
    route_dst=OUT/"post_load_arm_before_route_route_keylog.json"
    route_dst.write_text(json.dumps(runtime.get("routeLog", []), indent=2, sort_keys=True)+"\n", encoding="utf-8")
    direct=runtime.get("directHits", {})
    if direct.get("f0097_2809_1EFF_after_f0128"):
        status="PASS377_POSTLOAD_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN"
    elif direct.get("f0128_23AD_40FE"):
        status="PASS377_POSTLOAD_F0128_TRUE_STOP_ONLY_F0097_BLOCKED"
    else:
        status="BLOCKED_PASS377_POSTLOAD_F0128_TRUE_STOP_NOT_RECAPTURED"
    priors={"pass360":load_json("parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json").get("status"), "pass376":load_json("parity-evidence/verification/pass376_dm1_v1_original_artifact_command_manifest/manifest.json").get("status")}
    runtime_slim=jsonable({k:v for k,v in runtime.items() if k not in {"routeLog","commandLog"}})
    runtime_slim.update({"boundedSeconds":args.seconds, "routeLogCount":len(runtime.get("routeLog", [])), "commandLogCount":len(runtime.get("commandLog", [])), "transcript":str(transcript_dst.relative_to(ROOT)), "routeKeylog":str(route_dst.relative_to(ROOT))})
    promotion={
        "promoted": status == "PASS377_POSTLOAD_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN",
        "strictPostRunningF0128Stop": bool(direct.get("f0128_23AD_40FE")),
        "strictPostF0128VidrvStop": bool(direct.get("f0097_2809_1EFF_after_f0128")),
        "sawRunning": bool(runtime.get("sawRunning")),
        "routeInputAfterArming": bool(runtime.get("routeInputAfterArming")),
        "breakpointRetainedPostRoute": runtime.get("breakpointRetainedPostRoute"),
        "postRoutePauseCodeAddr": runtime.get("postRoutePauseCodeAddr"),
        "exactBlocker": None if status == "PASS377_POSTLOAD_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN" else (runtime.get("blocker") or runtime.get("stage")),
        "stage": runtime.get("stage"),
    }
    manifest={"schema":PASS+".v1", "timestampUtc":datetime.now(timezone.utc).isoformat(), "status":status, "repo":str(ROOT), "branch":mod.run(["git","branch","--show-current"]).stdout.strip(), "head":mod.run(["git","rev-parse","HEAD"]).stdout.strip(), "sourceRoot":str(SRC), "sourceAudit":source, "priorContract":priors, "addresses":{"F0128_DUNGEONVIEW_Draw_CPSF":"23AD:40FE", "F0097_VIDRV_09_BlitViewPort_indirect_call":"2809:1EFF", "F0097_DUNGEONVIEW_DrawViewport_entry":"2809:1E31"}, "runtimeProbe":runtime_slim, "strictStopPromotion":promotion, "blocker":None if status.startswith("PASS377_POSTLOAD_F0128_F0097") else promotion["exactBlocker"], "promotionRule":"Promote only if a strict post-Running stop at 23AD:40FE is followed in the same bounded owned-PTY run by 2809:1EFF/VIDRV; setup echo/BPLIST text is excluded.", "notPromotedBy":["BPLIST", "BP command echo", "tmux/capture-pane", "source-only address binding"]}
    (OUT/"manifest.json").write_text(json.dumps(jsonable(manifest), indent=2, sort_keys=True)+"\n", encoding="utf-8")
    manifest=jsonable(manifest)
    write_report(manifest)
    print(json.dumps({"status":status,"blocker":manifest["blocker"],"directHits":direct,"manifest":str((OUT/"manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
