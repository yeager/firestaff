# Pass288 — DM1 V2 entry viewport symbolic palette normalization

## Scope

This pass reduces one source-locked original-vs-Firestaff mismatch class in the DM1 V2 entry viewport renderer. It does **not** claim final pixel parity.

## Source/evidence lock

- ReDMCSB remains the locked draw-order route: floor/ceiling, then D3..D0 visible-square composition.
- Pass282 original PC34 entry viewport crop remains the reference image: parity-evidence/verification/pass282_dm1_v2_original_pixel_capture/original_entry_viewport_224x136.png.
- Pass286 comparator remains the source of truth for exact pixel status.

## Change

The Firestaff DM1 V2 flat materialization seam still uses symbolic rectangles, but its material colors now normalize to the pass282 original grayscale viewport palette before PNG export. This removes the previous “all pixels differ solely because Firestaff uses non-original RGBA material colors” blocker and leaves the comparator focused on geometry/order/asset-rendering differences.

## Result

- Previous pass286 mismatch: 30464/30464 pixels (ratio 1.0).
- New pass286 mismatch after regeneration: 16405/30464 pixels (ratio 0.5385044642857143).
- Firestaff PNG SHA256: 5105608387856212d7f600571eb69eab3f3469eaa1bb1913e937f4532ea95894.
- Diff PNG SHA256: 78e325b9b080376ac14a95ae6c363bccc99a4212aebe6f2f86f36a21174d91db.

## Remaining blocker

The Firestaff PNG is still a symbolic V2 composition/materialization seam, not an original GRAPHICS.DAT bitmap-backed renderer. Pixel parity remains blocked until original DM1 PC34 viewport bitmaps/geometry are rendered for the same route state.
