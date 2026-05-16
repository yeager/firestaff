#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, subprocess, sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
RED=Path.home()/".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DUNVIEW=RED/"DUNVIEW.C"; DEFS=RED/"DEFS.H"; COORD=RED/"COORD.C"
VIEW=ROOT/"m11_game_view.c"; PROBE=ROOT/"probes/m11/firestaff_m11_game_view_probe.c"; CMAKE=ROOT/"CMakeLists.txt"
EVIDENCE=ROOT/"parity-evidence/pass582_dm1_v1_explosion_viewport_clip_source_lock.md"
MANIFEST=ROOT/"parity-evidence/verification/pass582_dm1_v1_explosion_viewport_clip_source_lock/manifest.json"
STATUS="PASS582_DM1_V1_EXPLOSION_VIEWPORT_CLIP_SOURCE_LOCKED"

def read(p, enc="utf-8"): return Path(p).read_text(encoding=enc)
def compact(s): return " ".join(s.split())
def block(p,a,b,enc="latin-1"):
    lines=read(p,enc).splitlines()
    if len(lines)<b: raise AssertionError(f"{p} shorter than {b}")
    return "\n".join(lines[a-1:b])
def req(s, needles, label, ordered=False):
    c=compact(s); last=-1
    for n in needles:
        pos=c.find(compact(n))
        if pos<0: raise AssertionError(f"{label}: missing {n!r}")
        if ordered and pos<=last: raise AssertionError(f"{label}: out of order {n!r}")
        last=pos
def win(p, lines, claim, needles, ordered=False):
    a,b=map(int,lines.split("-")); req(block(p,a,b),needles,f"{p.name}:{lines}",ordered)
    return {"file":p.name,"lines":lines,"claim":claim,"needles":needles,"ordered":ordered}

def check():
    red=[
      win(DUNVIEW,"6028-6071","D0C explosions use C004 explosion-pattern path into G0296_puc_Bitmap_Viewport.",[
        "if (AP0145_i_ViewSquareExplosionIndex == C12_VIEW_SQUARE_D0C_EXPLOSION)",
        "AL0128_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(AL0127_i_ExplosionAspectIndex + M636_GRAPHIC_FIRST_EXPLOSION_PATTERN)",
        "F0133_VIDEO_BlitBoxFilledWithMaskedBitmap(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport",
        "F0638_GetZone(C004_ZONE_EXPLOSION_PATTERN_D0C, L2481_ai_XYZ)",
        "G2073_C224_ViewportPixelWidth",
        "F0493_CACHE_AddDerivedBitmap(C000_DERIVED_BITMAP_VIEWPORT)"],True),
      win(DUNVIEW,"6079-6137","Non-D0C explosions select coordinates and scale before fetching the scaled explosion bitmap.",[
        "if (L0196_B_RebirthExplosion)","G0227_aai_Graphic558_RebirthStep2ExplosionCoordinates",
        "if (L0191_ps_Explosion->Centered)","G0225_aai_Graphic558_CenteredExplosionCoordinates",
        "G0226_aaai_Graphic558_ExplosionCoordinates","L0194_i_ExplosionScale",
        "F0114_DUNGEONVIEW_GetExplosionBitmap(AL0127_i_ExplosionAspectIndex, L0194_i_ExplosionScale"],False),
      win(DUNVIEW,"6147-6174","Explosion bitmap boxes are clipped to the 224x136 viewport and empty clips are skipped.",[
        "M771_BOX_BOTTOM(L0145_auc_Box) = F0024_MAIN_GetMinimumValue(135",
        "AL0127_i_Y = F0025_MAIN_GetMaximumValue(0","if (AL0127_i_Y >= 136)",
        "M769_BOX_RIGHT(L0145_auc_Box) = AL0127_i_X","M768_BOX_LEFT(L0145_auc_Box) = F0026_MAIN_GetBoundedValue(0",
        "if (M769_BOX_RIGHT(L0145_auc_Box) <= M768_BOX_LEFT(L0145_auc_Box))",
        "L0136_i_ByteWidth = M077_NORMALIZED_BYTE_WIDTH(L0136_i_ByteWidth)"],True),
      win(DUNVIEW,"6192-6194","PC34/I34E final explosion draw uses computed zone index and viewport destination.",[
        "F0791_DUNGEONVIEW_DrawBitmapXX(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2474_i_ZoneIndex, L0143_B_FlipHorizontal, C10_COLOR_FLESH"]),
      win(DEFS,"3747-3753","C004 is the source zone id for the D0C explosion pattern.",["#define C004_ZONE_EXPLOSION_PATTERN_D0C","#define C007_ZONE_VIEWPORT"],True),
      win(COORD,"2389-2410","MEDIA720 F0635 clips zones and rejects empty clips before callers blit.",[
        "L2302_i_ = M704_ZONE_LEFT(L2307_ai_XYZ) - M704_ZONE_LEFT(P2130_pi_XYZ)",
        "M708_ZONE_WIDTH(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue",
        "M709_ZONE_HEIGHT(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue",
        "if ((M708_ZONE_WIDTH(P2130_pi_XYZ) <= 0) || (M709_ZONE_HEIGHT(P2130_pi_XYZ) <= 0))","return NULL"],True)]
    specs=[
      ("m11_game_view.c D0C explosion pattern helper",read(VIEW),["int M11_GameView_GetV1ExplosionPatternD0CZoneId(void)","return 4;","int M11_GameView_GetV1ExplosionPatternD0CZone(int* outX","if (outX) *outX = 0;","if (outY) *outY = 0;","if (outW) *outW = 32;","if (outH) *outH = 29;"]),
      ("m11_game_view.c deferred explosion pass",read(VIEW),["static void m11_draw_dm1_deferred_explosion_pass","DUNVIEW.C:5915 exits the packed-cell","m11_draw_dm1_deferred_center_explosion","m11_draw_dm1_deferred_side_explosion"]),
      ("m11 game-view probe covers C004 helper",read(PROBE),["M11_GameView_GetV1ExplosionPatternD0CZoneId() == 4","M11_GameView_GetV1ExplosionPatternD0CZone(&expX, &expY, &expW, &expH)","expX == 0 && expY == 0 && expW == 32 && expH == 29","\"explosion pattern and viewport-centered text zones expose layout-696 C004/C006 geometry\""]),
      ("CMake registration",read(CMAKE),["NAME pass582_dm1_v1_explosion_viewport_clip_source_lock","verify_pass582_dm1_v1_explosion_viewport_clip_source_lock.py"])]
    fire=[]
    for label,text,needles in specs:
        req(text,needles,label,False); fire.append({"label":label,"needles":needles,"ordered":False})
    return {"schema":"pass582_dm1_v1_explosion_viewport_clip_source_lock.v1","status":"passed","statusToken":STATUS,"redmcsbRoot":str(RED),"claim":"D0C explosions use source zone C004; non-D0C explosion bitmaps are viewport-clipped before blit.","redmcsbChecks":red,"firestaffChecks":fire,"nonClaims":["Does not duplicate pass579/pass557 D4-before-nearer-wall ordering.","Does not claim original DOS pixel parity.","Does not use Greatstone as primary evidence.","No renderer behavior change in this pass."]}

def write_outputs(man):
    MANIFEST.parent.mkdir(parents=True,exist_ok=True); EVIDENCE.parent.mkdir(parents=True,exist_ok=True)
    cmd=[sys.executable,str(Path(__file__).resolve()),"--check-only"]; r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    man=dict(man); man["verificationRuns"]=[{"command":cmd,"returncode":r.returncode,"passed":r.returncode==0,"outputTail":"\n".join(r.stdout.splitlines()[-12:])}]
    MANIFEST.write_text(json.dumps(man,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    lines=["# Pass582 DM1 V1 Explosion Viewport Clip Source Lock","","Status: "+man["statusToken"],"","## Claim","",man["claim"],"","This pass is narrower than D4 row-order work and does not claim pixel parity.","","## Primary ReDMCSB Evidence"]
    for c in man["redmcsbChecks"]:
        lines.append(f"- {c['file']}:{c['lines']} - {c['claim']}"); lines += ["  - "+n for n in c["needles"]]
    lines += ["","## Firestaff Evidence"]
    for c in man["firestaffChecks"]:
        lines.append("- "+c["label"]); lines += ["  - "+n for n in c["needles"]]
    lines += ["","## Verification",""]
    for run in man["verificationRuns"]:
        lines.append(f"- {' '.join(run['command'])} -> {run['returncode']}")
        if run["outputTail"]: lines += ["~~~",run["outputTail"],"~~~"]
    lines += ["","## Non-Claims",""] + ["- "+x for x in man["nonClaims"]] + [""]
    EVIDENCE.write_text("\n".join(lines),encoding="utf-8")

def main():
    ap=argparse.ArgumentParser(); ap.add_argument("--check-only",action="store_true"); args=ap.parse_args()
    man=check()
    if not args.check_only: write_outputs(man)
    print("status="+STATUS)
    print("anchors=DUNVIEW.C:6028-6071,DUNVIEW.C:6079-6137,DUNVIEW.C:6147-6174,DUNVIEW.C:6192-6194,DEFS.H:3747-3753,COORD.C:2389-2410")
    return 0

if __name__=="__main__":
    try: raise SystemExit(main())
    except (AssertionError,OSError,subprocess.SubprocessError) as e:
        print(f"status=FAIL_PASS582_DM1_V1_EXPLOSION_VIEWPORT_CLIP_SOURCE_LOCK reason={e}",file=sys.stderr); raise SystemExit(1)
