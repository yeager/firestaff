#!/usr/bin/env python3
from __future__ import annotations
import argparse, csv, hashlib, json, os, subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable
from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RUN_ROOT = Path('/home/trv2/.openclaw/data/firestaff-n2-runs/pass462-title-end-capture-parity')
RED = Path(os.environ.get('REDMCSB_SOURCE_ROOT', '/home/trv2/.openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source'))

@dataclass
class CaptureRow:
    index: int
    path: Path
    sha256: str
    width: int
    height: int

def body(path: Path) -> str:
    return path.read_text(errors='replace')

def require_markers(path: Path, markers: list[str], missing: list[str], prefix: str) -> None:
    text = body(path)
    for marker in markers:
        if marker not in text:
            missing.append(f'{prefix}:{marker}')

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def load_manifest(path: Path) -> list[CaptureRow]:
    rows=[]
    if not path.exists():
        return rows
    with path.open(newline='') as f:
        for row in csv.DictReader(f, delimiter='\t'):
            rows.append(CaptureRow(int(row['index']), Path(row['path']), row['sha256'], int(row['width']), int(row['height'])))
    return rows

def metrics(a: np.ndarray, b: np.ndarray) -> tuple[float,int,int]:
    d=np.abs(a.astype(np.int16)-b.astype(np.int16))
    return float(np.mean(d)), int(np.count_nonzero(np.any(d != 0, axis=2))), int(np.max(d))

def norm(arr: np.ndarray) -> np.ndarray:
    out=arr.copy()
    out[np.max(out, axis=2) < 16] = 0
    return out

def make_refs(out_dir: Path) -> Path:
    ref = out_dir / 'title_reference_frames'
    ref.mkdir(parents=True, exist_ok=True)
    env=os.environ.copy()
    env['FIRESTAFF_PASS57_DUMP_DIR']=str(ref)
    with (out_dir/'title_reference_probe.txt').open('w') as f:
        subprocess.run([str(ROOT/'run_firestaff_v1_title_render_probe.sh')], cwd=ROOT, env=env, check=True, stdout=f, stderr=subprocess.STDOUT)
    return ref

def latest_title_dir(run_root: Path) -> Path | None:
    cands=[p for p in run_root.glob('*-title') if (p/'title_capture_manifest.tsv').exists()]
    return max(cands, key=lambda p: p.stat().st_mtime) if cands else None

def main(argv: Iterable[str] | None=None) -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument('--capture-dir', type=Path)
    ap.add_argument('--run-root', type=Path, default=DEFAULT_RUN_ROOT)
    ap.add_argument('--out-dir', type=Path, default=ROOT/'parity-evidence/verification/pass462_title_end_original_capture_parity')
    args=ap.parse_args(argv)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    missing=[]
    require_markers(RED/'TITLE.C', ['F0437_STARTEND_DrawTitle', 'M526_WaitVerticalBlank', 'C001_GRAPHIC_TITLE'], missing, 'TITLE.C')
    require_markers(RED/'ENDGAME.C', ['F0444_STARTEND_Endgame', 'C006_GRAPHIC_THE_END', 'F0022_MAIN_Delay(300)', 'C005_GRAPHIC_CREDITS'], missing, 'ENDGAME.C')
    require_markers(RED/'DATA.C', ['G0012_ai_Graphic562_Box_Endgame_TheEnd'], missing, 'DATA.C')
    capture_dir=args.capture_dir or latest_title_dir(args.run_root)
    rows=load_manifest(capture_dir/'title_capture_manifest.tsv') if capture_dir else []
    ref_dir=make_refs(args.out_dir)
    refs=[]
    for p in sorted(ref_dir.glob('frame_*.ppm')):
        refs.append((p.name, norm(np.asarray(Image.open(p).convert('RGB'), dtype=np.uint8))))
    matches=[]
    for r in rows:
        if not r.path.exists():
            matches.append({'index':r.index,'status':'missing-file'})
            continue
        actual=sha(r.path)
        im=Image.open(r.path).convert('RGB')
        compare_im = im
        if im.size == (640, 400):
            compare_im = im.resize((320, 200), Image.Resampling.NEAREST)
        elif im.size == (720, 400):
            # Text-mode prompt frames from DOSBox are intentionally not cropped into
            # false references; they remain blockers/nonmatches.
            compare_im = im
        arr=norm(np.asarray(compare_im, dtype=np.uint8))
        best=None
        for name, ref in refs:
            if arr.shape != ref.shape:
                continue
            m=metrics(arr, ref)
            if best is None or m < best[:3]:
                best=(*m, name)
        matches.append({'index':r.index,'path':str(r.path),'sha256':actual,'shaOk':actual==r.sha256,'width':im.width,'height':im.height,'compareWidth':compare_im.width,'compareHeight':compare_im.height,'bestReference':best[3] if best else None,'mae':best[0] if best else None,'diffPixels':best[1] if best else None,'maxDelta':best[2] if best else None,'titleLike': bool(best and compare_im.size==(320,200) and best[0] <= 2.0 and best[2] <= 12)})
    title_like=sum(1 for m in matches if m.get('titleLike'))
    unique_title_refs=sorted({m.get('bestReference') for m in matches if m.get('titleLike') and m.get('bestReference')})
    status = 'PASS462_TITLE_STILL_CAPTURE_MATCHED_ENDGAME_ROUTE_BLOCKED' if not missing and title_like else 'BLOCKED_PASS462_TITLE_END_ORIGINAL_CAPTURE_PARITY_INCOMPLETE'
    endgame_blocker = 'BLOCKED_PASS462_ENDGAME_ORIGINAL_RUNTIME_ROUTE_MISSING'
    report={
        'pass':'pass462',
        'status':status,
        'baseHead':subprocess.check_output(['git','rev-parse','--short','HEAD'], cwd=ROOT, text=True).strip(),
        'sourceAuditOk': not missing,
        'missingSourceMarkers':missing,
        'titleCaptureDir': str(capture_dir) if capture_dir else None,
        'titleCaptureCount':len(rows),
        'titleLikeMatches':title_like,
        'uniqueTitleReferences':unique_title_refs,
        'titleComparisonArtifact': str(args.out_dir/'title_matches.json'),
        'titleReferenceDir':str(ref_dir),
        'endgameStatus': endgame_blocker,
        'endgameReason':'no deterministic original PC34 runtime route/save-state to reach F0444 endgame on N2; source schedule is locked, emulator pixel/video parity remains unclaimed',
        'notes':['TITLE comparison is still-frame identity evidence only; DOSBox hotkey screenshots are sparse/blocking and do not prove cadence.', 'Endgame capture must be rerun once a deterministic original route or saved-game fixture reaches the end sequence.']
    }
    (args.out_dir/'title_matches.json').write_text(json.dumps(matches, indent=2, sort_keys=True))
    (args.out_dir/'manifest.json').write_text(json.dumps(report, indent=2, sort_keys=True))
    md=args.out_dir/'pass462_title_end_original_capture_parity.md'
    md.write_text('\n'.join([
        '# Pass 462 — DM1 V1 title/end original capture parity', '',
        f"Status: `{status}`", '',
        f"- Source audit OK: `{not missing}`",
        f"- Title capture dir: `{report['titleCaptureDir']}`",
        f"- Title captures: `{len(rows)}`",
        f"- TITLE-like still matches: `{title_like}`",
        f"- Unique TITLE references: `{', '.join(unique_title_refs) if unique_title_refs else 'none'}`",
        f"- Endgame blocker: `{endgame_blocker}`", '',
        'Scope: source-first audit plus N2 DOSBox still capture comparison. Cadence/video parity is not claimed.', ''
    ]))
    print(status)
    print(args.out_dir)
    return 0 if not missing and title_like else 1
if __name__ == '__main__':
    raise SystemExit(main())
