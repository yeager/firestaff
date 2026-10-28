# English Configuration scene texture-source join (2026-10-28)

## Capture

Ymir 0.4.2-dev `--debug`, authenticated English retail Saturn media, USA
1.01a IPL, isolated profile. This is the authentic English Configuration page
capture retained outside Git at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-english-configuration-page-evidence-20261028/initial/`.
The bundle reports capture frame 119, 189 VDP1 command records, and zero trace
rows dropped for that capture frame. The screenshot SHA-256 is
`748d38319dfb5acb6defb00307e391cc043e30b6984da70dea7f1f0505ee216a`; the
captured VDP1 VRAM SHA-256 is
`12230c8142fcc33c9429bf9cb203137eafcd6fdc3c5c0aa4ca3c32913e55b77a`.

## Observed source matches

`scripts/analyze_nexus_vdp1_source_join.py` examined 148 textured draw spans in
frame 119 against the hash-admitted retail files and decoded DGN Structure2
image spans. It reports:

- 132 command source spans have exact byte matches to one or more retail DGN
  Structure2 image spans.
- 137 spans contain informative source bytes; 11 spans are zero/uninformative.
- 113 command spans also occur byte-exactly in `LEV00.DGN` as a whole file.
- The other five informative spans do not have an exact Structure2 match in
  this scan.

The authentic `LEV00.DGN` SHA-256 is
`24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a`. The
exact span report is retained in the external evidence bundle as
`initial/dumps/vdp1-source-join.txt`.

Reproduce the scan with:

```sh
python3 scripts/analyze_nexus_vdp1_source_join.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-english-configuration-page-evidence-20261028/initial/dumps \
  --data-dir /Users/bosse/.firestaff/data/nexus --frame 119 --ymir-bundle
```

## Evidence boundary

These byte joins support authentic DGN source candidates for many scene
textures. Repeated source bytes across level files make some joins ambiguous;
the scan does not bind the active level instance, map cell/face ownership,
CLUT, palette, geometry placement, camera, complete composition, or Firestaff
runtime output. It does not authorize using a guessed DGN scene as a substitute
background. The full Configuration background remains unimplemented pending
those joins.

## Image, CLUT and face-owner joins

The existing command-sequence joiner was extended to read this Ymir bundle
directly (`--ymir-bundle`). It now handles indexed color-mode-1 image/CLUT
pairs and direct-color mode-5 image spans, preserving the byte order observed
in the Ymir capture. On this authentic frame it reports:

- 132 textured draws in the validated 189-record command chain.
- 128 exact DGN material matches: 120 image/CLUT pairs and 8 direct-color
  image spans.
- 112 of those matched draws join to one or more canonical Structure3 face
  owners.
- Four draws have no exact DGN material match; 16 matched draws have no
  canonical Structure3 face-owner row.

The detailed command report is retained externally as
`initial/dumps/vdp1-dgn-material-sequence-join.txt`. Reproduce with:

```sh
python3 scripts/analyze_nexus_vdp1_dgn_command_sequence_join.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-english-configuration-page-evidence-20261028/initial/dumps \
  --data-dir /Users/bosse/.firestaff/data/nexus --frame 119 --ymir-bundle
```

Repeated DGN image and material bytes can have several level/face owners. These
matches therefore do not identify which instance was selected for a draw. The
face-owner join also does not recover the selection call, camera transform,
culling, display origin, menu/HUD ownership, or full composition. Renderer
admission remains blocked.
