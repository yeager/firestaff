#!/usr/bin/env python3
from __future__ import annotations
import json, sys
from collections import defaultdict
from pathlib import Path
from PIL import Image, ImageStat
REPO = Path(__file__).resolve().parent.parent
DEFAULT_RUN = Path.home()/".openclaw/data/firestaff-n2-runs/20260428-183628-pass133b-startup-variant-original-route"

def region_stats(img: Image.Image, box):
    crop = img.crop(box).convert("RGB")
    stat = ImageStat.Stat(crop)
    pixels = list(crop.getdata())
    nonblack = sum(1 for r,g,b in pixels if r+g+b > 12) / max(1, len(pixels))
    bright = sum(1 for r,g,b in pixels if r+g+b > 180) / max(1, len(pixels))
    return {"nonblack": round(nonblack,4), "bright": round(bright,4), "mean": [round(x,2) for x in stat.mean]}

def main():
    run = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_RUN
    data = json.loads((run/"pass133_results.json").read_text())
    rows=[]; by_hash={}
    for result in data.get("results", []):
        scenario = result.get("scenario", "")
        for row in result.get("rows", []):
            cls = row.get("class", "")
            sha = row.get("sha12", "")
            if cls != "graphics_320x200_unclassified":
                continue
            img_path = run/scenario/row.get("file", "")
            if not img_path.exists():
                continue
            img = Image.open(img_path)
            w,h = img.size
            stats = {
                "full": region_stats(img, (0,0,w,h)),
                "viewport": region_stats(img, (0,0,224,136)),
                "right_column": region_stats(img, (224,0,320,200)),
                "spell_area": region_stats(img, (224,42,320,73)),
                "lower_panel": region_stats(img, (0,136,320,200)),
            }
            entry = {"scenario": scenario, "phase": row.get("phase"), "value": row.get("value"), "label": row.get("label"), "sha12": sha, "file": str(img_path.relative_to(run)), "stats": stats}
            rows.append(entry)
            by_hash.setdefault(sha, entry)
    # rank hashes with non-empty right/spell areas because they are likely selector/control/champion intermediates.
    ranked = sorted(by_hash.values(), key=lambda e: (e["stats"]["right_column"]["nonblack"], e["stats"]["spell_area"]["nonblack"], e["stats"]["viewport"]["nonblack"]), reverse=True)
    out_json = REPO/"parity-evidence/pass135_pass133_unclassified_frame_atlas.json"
    out_md = REPO/"parity-evidence/pass135_pass133_unclassified_frame_atlas.md"
    out_json.write_text(json.dumps({"schema":"pass135_pass133_unclassified_frame_atlas.v1", "run": str(run), "unclassified_rows": len(rows), "unique_hashes": len(by_hash), "ranked_unique": ranked, "rows": rows}, indent=2)+"\n")
    md=["# Pass 135 — pass133 unclassified frame atlas", "", f"- run source: `{run}`", f"- unclassified rows: {len(rows)}", f"- unique unclassified hashes: {len(by_hash)}", "", "## Ranked unique unclassified frames", "", "Ranked by right-column/spell-area nonblack ratios to identify possible selector/champion/control screens for the next route pass.", "", "| sha12 | scenario | phase/value | right nonblack | spell nonblack | viewport nonblack | file |", "|---|---|---|---:|---:|---:|---|"]
    for e in ranked:
        phase = str(e.get("phase") or "") + ":" + str(e.get("value") or e.get("label") or "")
        md.append(f"| `{e['sha12']}` | `{e['scenario']}` | `{phase}` | {e['stats']['right_column']['nonblack']:.4f} | {e['stats']['spell_area']['nonblack']:.4f} | {e['stats']['viewport']['nonblack']:.4f} | `{e['file']}` |")
    md += ["", "## Interpretation", "", "This pass does not claim party-control readiness. It narrows pass133's classifier gaps: if top-ranked unclassified frames have high right/spell density, the next pass should add explicit classifier signatures or route probes for those exact hashes; if they remain low-density/transient, original-route work should shift toward source/asset-backed Hall-of-Champions or save/party setup rather than broader startup variants.", ""]
    out_md.write_text("\n".join(md))
if __name__ == "__main__": main()
