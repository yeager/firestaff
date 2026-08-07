# Firestaff v3.0.79

Firestaff v3.0.79 packages source-owned runtime work from `main`.

## Highlights since v3.0.78

- **DM1/CSB**: HUD panels, champion labels, and C015 OPENROOM messages now
  require the original PC34/CSBWin surfaces, fonts, and dungeon text records.
- **DM2**: G1/GDAT creature and item material handoffs now retain byte-level
  provenance before they can reach the viewport.
- **Theron's Quest**: Track 02 loader records and their envelopes remain
  authenticated through runtime admission.
- **Nexus**: DGN Structure1 and FACE.BIN PRS3 capture routes now retain
  exact retail provenance and remain no-draw until decoder semantics are proven.

## Verification

- Focused source-gate and runtime checks passed locally.
- GitHub Actions builds and packages platform artifacts from the `v3.0.79` tag.
