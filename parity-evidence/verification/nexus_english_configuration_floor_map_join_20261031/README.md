# Nexus English Configuration floor-map join (2026-10-31)

This receipt extends the authentic frame-119 Ymir material join with a
source-side search of the retail `LEV00.DGN` Structure1B map. It identifies a
unique floor-material pattern corresponding to 16 early VDP1 quads. It does
not establish the active map-cell selection routine or camera projection.

## Authenticated inputs

- English retail Saturn capture: the frame-119 Configuration screenshot and
  Ymir VDP1 bundle retained in
  `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-english-configuration-page-evidence-20261028/initial/`.
- Screenshot SHA-256:
  `748d38319dfb5acb6defb00307e391cc043e30b6984da70dea7f1f0505ee216a`.
- VDP1 VRAM SHA-256:
  `12230c8142fcc33c9429bf9cb203137eafcd6fdc3c5c0aa4ca3c32913e55b77a`.
- VDP1 command trace SHA-256:
  `624b664cb7b64f7df5d719edbff929256462875226966d3992656946bdf6384f`.
- VDP1 capture receipt SHA-256:
  `807274ace341cdf12280c201399c989f082c5645051443d4aa4e77349f4d1e20`.
- Retail `LEV00.DGN` SHA-256:
  `24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a`.

The Ymir receipt records a complete 189-command frame-119 trace with no dropped
rows. The analyzer verifies all three capture-file hashes, the Ymir trace
receipt, and the retail DGN hash before reporting a join.

## Observed floor-material rows

The 16 unowned indexed quads have exact Structure2 image and CLUT matches to
`LEV00.DGN`, with no matching raw Structure3 material selectors. Their
command-space Y spans and material IDs are:

| Y span | Draw count | Structure2 material IDs |
| --- | ---: | --- |
| 33–40 | 3 | 2, 2, 2 |
| 40–52 | 3 | 2, 1, 2 |
| 52–73 | 5 | 2, 3, 1, 3, 2 |
| 73–122 | 5 | 1, 1, 1, 5, 4 |

Structure1B stores the same four rows at one and only one location in the
64×64 map:

| Map row | X span | Floor-material IDs |
| ---: | ---: | --- |
| 26 | 31–33 | 2, 2, 2 |
| 27 | 31–33 | 2, 1, 2 |
| 28 | 30–34 | 2, 3, 1, 3, 2 |
| 29 | 30–34 | 1, 1, 1, 5, 4 |

The row sequence and its unique map occurrence are source-backed evidence that
these draws use a real `LEV00.DGN` floor-material pattern. Their increasing
command-space Y spans are consistent with a receding floor fan. The capture
does not trace the retail producer's selected cells or projection math, so it
does not prove that this unique candidate patch was the active source at this
frame.

## Reproduction

Set `NEXUS_DATA` to the directory containing the authenticated Nexus files and
`EVIDENCE_DUMPS` to the `initial/dumps` folder above:

```sh
python3 scripts/analyze_nexus_vdp1_structure1b_floor_map_join.py \
  "$EVIDENCE_DUMPS" --data-dir "$NEXUS_DATA"
```

This is a capture analyzer only. Active cell selection, camera projection,
culling, display origin, VDP2 composition, complete pixel ownership, and
Firestaff runtime parity remain unproven; semantic admission remains blocked.
