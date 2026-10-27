# Nexus menu-candidate VDP1 source join — 2026-10-27

This is a bounded source-join report for a same-frame Ymir debug capture. The
raw capture and commercial retail resources remain outside Git under
`/Volumes/Extern-disk/firestaff-data/`; this directory stores only evidence
metadata and reproduction instructions.

## Capture identity

The capture came from TRV2 profile
`/dev/shm/ymir-nexus-dma-coldboot-20261027/`, using the authentic Japanese
Nexus retail corpus. The captured frame is 8050. Its VDP1 receipt records 178
commands for that frame and zero dropped rows.

The retained title-menu reference is frame 497 of
`nexus_v1_ymir_title_menu_vdp_bundle_20261025`. Comparing command records
excluding live local-coordinate fields gives 175 identical records out of
178. The three differences are at command indices 171, 172, and 177; the first
two differ in texture source/size words, and the last differs in command words.
The comparison is recorded in the external capture directory's
`command-list-comparison.txt`.

This similarity supports treating frame 8050 as a menu candidate. The copied
capture did not include its input schedule or an independently paired
screenshot, so the report does not identify its exact menu selection or claim
that it is the same UI state as the reference.

## Authenticated sources and capture hashes

The Japanese retail source files were staged from TRV2's
`/home/trv2/.firestaff/data/nexus/` to the external data directory. All 160
source files were checked for matching names, sizes, and SHA-256 digests after
copying. The complete hash lists remain outside Git in
`/Volumes/Extern-disk/firestaff-data/`.

| Input | SHA-256 |
| --- | --- |
| `MENU.BPK` | `740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636` |
| `LEV00.DGN` | `24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a` |
| Captured `vdp1-vram.bin` | `49bf5ba69310125baad9b2f002fc1de1dafd9c9ec2bab5308a899a9737901eeb` |
| Captured `vdp1-command-trace.csv` | `f7afffcaa726059f71c543867f6e693d15f8bdc141500640db6beb57813875a9` |

## Results

The existing Firestaff `analyze_nexus_menu_bpk_vdp1_snapshot_join.py`
decompressed all 162 real PRS3 surfaces in the Japanese `MENU.BPK`; 86 whole
surface byte sequences occur in this frame's VDP1 VRAM. None of the 137
textured command source starts falls within those matched surface ranges.
This proves byte residency only, not that those surfaces are drawn in this
frame.

The existing `analyze_nexus_vdp1_source_join.py` matched 121 of 137 textured
commands informatively against the authenticated retail corpus, including
`LEV00.DGN` Structure2 spans. The remaining 16 command spans have byte matches
that are ambiguous across multiple retail structures or files. No menu text,
asset role, loader/DMA route, palette semantics, or display ownership is
inferred.

Both full analyzer outputs are retained beside the raw external capture as
`menu-bpk-vdp1-join.txt` and
`nexus-menu-dma-source-join-20261027.txt`.

## Reproduction

From the Firestaff repository root, with the external data/capture paths above:

```sh
python3 scripts/analyze_nexus_menu_bpk_vdp1_snapshot_join.py \
  /Volumes/Extern-disk/firestaff-data/.firestaff/data/nexus/MENU.BPK \
  /Volumes/Extern-disk/firestaff-data/nexus-menu-dma-capture-20261027/vdp1-vram.bin \
  --vdp1-command-trace \
  /Volumes/Extern-disk/firestaff-data/nexus-menu-dma-capture-20261027/vdp1-command-trace.csv \
  --vdp1-command-trace-frame 8050

python3 scripts/analyze_nexus_vdp1_source_join.py \
  /Volumes/Extern-disk/firestaff-data/nexus-menu-dma-capture-20261027 \
  --data-dir /Volumes/Extern-disk/firestaff-data/.firestaff/data/nexus \
  --frame 8050 --ymir-bundle
```
