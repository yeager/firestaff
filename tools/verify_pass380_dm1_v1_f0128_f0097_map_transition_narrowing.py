#!/usr/bin/env python3
"""Pass380 verifier: narrow the pass379 F0128/F0097 blocker."""
from __future__ import annotations
import json, re, shutil, struct, subprocess, tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass380_dm1_v1_f0128_f0097_map_transition_narrowing"
OUTDIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUTDIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
FIRES_MAP = Path.home() / ".openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP"
FIRES = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Reference/Original/I34E/FIRES"
PASS377 = ROOT / "parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/manifest.json"
PASS379 = ROOT / "parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/manifest.json"
LOAD_SEG = 0x0733
EXPECTED = {
    "F0128_DUNGEONVIEW_Draw_CPSF": {"link": "1C7A:40FE", "runtime": "23AD:40FE"},
    "F0097_DUNGEONVIEW_DrawViewport": {"link": "20D6:1E31", "runtime": "2809:1E31"},
    "F0097_VIDRV_09_BlitViewPort_indirect_call": {"link": "20D6:1EFF", "runtime": "2809:1EFF", "bytes": "26ff5f24"},
}

def compact(s: str) -> str: return " ".join(s.split())
def run(cmd: list[str]) -> subprocess.CompletedProcess[str]: return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)

def source_check(fn: str, needles: list[str]) -> dict[str, Any]:
    p = SRC / fn
    lines = p.read_text(encoding="latin-1", errors="replace").splitlines()
    found, missing = {}, []
    for n in needles:
        cn, hit = compact(n), None
        for i, line in enumerate(lines, 1):
            if cn in compact(line): hit = i; break
        (missing.append(n) if hit is None else found.__setitem__(n, hit))
    return {"file": fn, "path": str(p), "ok": not missing, "found": found, "missing": missing}

def parse_csip(s: str) -> tuple[int, int]:
    a, b = s.split(":", 1); return int(a, 16), int(b, 16)
def plus_loader(link: str) -> str:
    seg, off = parse_csip(link); return f"{seg + LOAD_SEG:04X}:{off:04X}"
def linear(csip: str) -> int:
    seg, off = parse_csip(csip); return seg * 16 + off
def runtime_to_link_linear(csip: str) -> int: return linear(csip) - LOAD_SEG * 16

def unlzexe_bytes() -> bytes:
    with tempfile.TemporaryDirectory(prefix="pass380-unlzexe-") as td_s:
        td = Path(td_s); target = td / "FIRES.EXE"; shutil.copy2(FIRES, target)
        proc = subprocess.run([str(Path.home() / "bin/unlzexe"), str(target)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
        if proc.returncode != 0: raise RuntimeError(proc.stdout)
        # unlzexe variants differ: some write FIRES.EXENEW, while 0.8p1
        # rewrites the target in place and moves the compressed file to .olz.
        unpacked = td / "FIRES.EXENEW"
        if not unpacked.exists():
            unpacked = target
        return unpacked.read_bytes()

def mz_file_offset(blob: bytes, seg: int, off: int) -> int:
    return struct.unpack_from("<14H", blob, 0)[4] * 16 + seg * 16 + off

def map_symbols(map_text: str) -> dict[str, str]:
    out = {}
    for line in map_text.splitlines():
        m = re.search(r"\b([0-9A-F]{4}:[0-9A-F]{4})\s+(?:idle\s+)?_(F\d+_[A-Z0-9_]+)", line)
        if m and m.group(2) not in out: out[m.group(2)] = m.group(1)
    return out

def map_segments(map_text: str) -> list[dict[str, Any]]:
    segs = []
    for line in map_text.splitlines():
        m = re.match(r"\s*([0-9A-F]+)H\s+([0-9A-F]+)H\s+([0-9A-F]+)H\s+(\S+)\s+CODE", line)
        if m: segs.append({"start": int(m.group(1), 16), "end": int(m.group(2), 16), "length": int(m.group(3), 16), "name": m.group(4)})
    return segs

def nearest_symbol(symbols: dict[str, str], link_lin: int) -> dict[str, Any] | None:
    best = None
    for name, csip in symbols.items():
        addr = linear(csip)
        if addr <= link_lin and (best is None or addr > best["linear"]): best = {"name": name, "link": csip, "linear": addr, "delta": link_lin - addr}
    return best

def final_pause_decode(pass379: dict[str, Any], symbols: dict[str, str], segs: list[dict[str, Any]]) -> dict[str, Any]:
    addr = pass379.get("runtimeProbe", {}).get("finalPauseCodeAddr")
    if not addr: return {"ok": False, "runtime": None, "reason": "missing finalPauseCodeAddr"}
    link_lin = runtime_to_link_linear(addr)
    seg_hit = next((s for s in segs if s["start"] <= link_lin <= s["end"]), None)
    near = nearest_symbol(symbols, link_lin)
    return {"ok": bool(seg_hit and near), "runtime": addr, "linkLinearHex": f"0x{link_lin:05X}", "segment": seg_hit["name"] if seg_hit else None, "nearestSymbol": {k: v for k, v in (near or {}).items() if k != "linear"}, "isF0128OrF0097": bool(near and near["name"] in {"F0128_DUNGEONVIEW_DRAW_CPSF", "F0097_DUNGEONVIEW_DRAWVIEWPORT"})}

def main() -> int:
    errors = []; OUTDIR.mkdir(parents=True, exist_ok=True)
    source = [
        source_check("DUNVIEW.C", ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]),
        source_check("DRAWVIEW.C", ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"]),
        source_check("GAMELOOP.C", ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking)"]),
        source_check("COMMAND.C", ["void F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]),
        source_check("CLIKMENU.C", ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]),
    ]
    if not all(r["ok"] for r in source): errors.append("source audit failed")
    map_text = FIRES_MAP.read_text(encoding="latin-1", errors="replace"); symbols = map_symbols(map_text); segs = map_segments(map_text)
    map_checks = {"path": str(FIRES_MAP), "loaderSegmentDelta": f"{LOAD_SEG:04X}", "symbols": {}, "ok": True}
    for label, sym in {"F0128_DUNGEONVIEW_Draw_CPSF": "F0128_DUNGEONVIEW_DRAW_CPSF", "F0097_DUNGEONVIEW_DrawViewport": "F0097_DUNGEONVIEW_DRAWVIEWPORT"}.items():
        got = symbols.get(sym); exp = EXPECTED[label]; ok = got == exp["link"] and plus_loader(got or "0000:0000") == exp["runtime"]
        map_checks["symbols"][label] = {"mapSymbol": sym, "link": got, "runtimeWithLoader": plus_loader(got or "0000:0000"), "expected": exp, "ok": ok}; map_checks["ok"] = bool(map_checks["ok"] and ok)
    if not map_checks["ok"]: errors.append("map rebase failed")
    blob = unlzexe_bytes(); seg, off = parse_csip(EXPECTED["F0097_VIDRV_09_BlitViewPort_indirect_call"]["link"]); vidrv_bytes = blob[mz_file_offset(blob, seg, off):mz_file_offset(blob, seg, off)+4].hex()
    static_bytes = {"FIRES": str(FIRES), "vidrvCallBytesAt20D6_1EFF": vidrv_bytes, "expected": EXPECTED["F0097_VIDRV_09_BlitViewPort_indirect_call"]["bytes"], "ok": vidrv_bytes == EXPECTED["F0097_VIDRV_09_BlitViewPort_indirect_call"]["bytes"]}
    if not static_bytes["ok"]: errors.append("VIDRV byte binding failed")
    p377 = json.loads(PASS377.read_text()); p379 = json.loads(PASS379.read_text()); pass379_rt = p379.get("runtimeProbe", {}); dh = pass379_rt.get("directHits", {})
    artifact_checks = {"pass377": {"path": str(PASS377.relative_to(ROOT)), "status": p377.get("status"), "routeInputAfterArming": p377.get("runtimeProbe", {}).get("routeInputAfterArming"), "breakpointRetainedPostRoute": p377.get("runtimeProbe", {}).get("breakpointRetainedPostRoute")}, "pass379": {"path": str(PASS379.relative_to(ROOT)), "status": p379.get("status"), "routeInputAfterArming": pass379_rt.get("routeInputAfterArming"), "breakpointRetainedPostRoute": pass379_rt.get("breakpointRetainedPostRoute"), "sawRunning": pass379_rt.get("sawRunning"), "directHits": dh}}
    artifacts_ok = artifact_checks["pass379"]["routeInputAfterArming"] is True and artifact_checks["pass379"]["breakpointRetainedPostRoute"] is True and artifact_checks["pass379"]["sawRunning"] is True and not any(dh.values())
    if not artifacts_ok: errors.append("pass379 artifact predicates failed")
    final_pause = final_pause_decode(p379, symbols, segs)
    if not final_pause.get("ok"): errors.append("final pause decode failed")
    map_resolved = map_checks["ok"] and static_bytes["ok"]
    transition_narrowed = map_resolved and artifacts_ok and final_pause.get("ok") and not final_pause.get("isF0128OrF0097") and all(r["ok"] for r in source)
    status = "BLOCKED_PASS380_SOURCE_PATH_TRANSITION_NARROWED_MAP_BINDING_HELD" if transition_narrowed else "BLOCKED_PASS380_MAP_OR_TRANSITION_STILL_UNRESOLVED"
    if errors: status = "FAIL_PASS380_AUDIT_INCOMPLETE"
    conclusion = "FIRES CS:IP mapping held: F0128 and F0097 rebase from FIRES.MAP with loader +0733, and the VIDRV call bytes remain bound at 20D6:1EFF -> 2809:1EFF. pass379 delivered route input after arming and retained the candidate breakpoints, but stopped nowhere at F0128/F0097/07FB; its forced pause decodes into IMAGE_TEXT near F0683, not the DUNVIEW/DRAWVIEW symbols. The remaining blocker is therefore the ReDMCSB source-path transition from delivered route input into F0380/F0365/F0366, G0321_B_StopWaitingForPlayerInput, and the next outer-loop F0128 draw; do not retarget F0128/F0097 CS:IP from pass379 alone." if transition_narrowed else "Audit did not fully resolve map-vs-transition; inspect errors before promoting the narrowing."
    manifest = {"schema": PASS + ".v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"]).stdout.strip(), "head": run(["git", "rev-parse", "HEAD"]).stdout.strip(), "sourceRoot": str(SRC), "sourceAudit": source, "mapAudit": map_checks, "staticByteAudit": static_bytes, "priorArtifacts": artifact_checks, "finalPauseDecode": final_pause, "decision": {"mapBindingResolved": map_resolved, "sourcePathTransitionNarrowed": transition_narrowed, "conclusion": conclusion}, "notPromotedBy": ["BPLIST", "BP command echo", "tmux/capture-pane", "source-only address binding", "route keylog alone", "forced-pause IMAGE_TEXT sample"], "errors": errors}
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    final_segment = final_pause.get("segment"); final_symbol = (final_pause.get("nearestSymbol") or {}).get("name")
    REPORT.write_text("\n".join(["# Pass380 — DM1 V1 F0128/F0097 map vs transition narrowing", "", f"Status: `{status}`", "", "## Decision", "", conclusion, "", "## Evidence", "", f"- Source root: `{SRC}`", "- FIRES map rebase: F0128 `1C7A:40FE` -> `23AD:40FE`; F0097 `20D6:1E31` -> `2809:1E31`.", "- VIDRV call byte binding: `20D6:1EFF` -> `2809:1EFF` remains `26 ff 5f 24`.", f"- pass379 final forced pause decodes to `{final_segment}` near `{final_symbol}`; this is not F0128/F0097.", "", f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`"])+"\n", encoding="utf-8")
    print(json.dumps({"status": status, "mapBindingResolved": map_resolved, "sourcePathTransitionNarrowed": transition_narrowed, "manifest": str(MANIFEST.relative_to(ROOT)), "errors": errors}, indent=2, sort_keys=True))
    return 0 if not errors and status.startswith("BLOCKED_PASS380_") else 1
if __name__ == "__main__": raise SystemExit(main())
