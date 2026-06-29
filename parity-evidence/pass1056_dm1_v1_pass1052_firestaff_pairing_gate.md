# Pass1056 DM1 V1 pass1052 original-to-Firestaff pairing gate

Status: `PASS`

This gate makes the pass1052/pass1054 capture state reproducible: four
clean original PC 3.4 viewport crops exist, the pass1054 Firestaff pairing
manifest exists, and the one promoted wall row remains an exact 0-pixel
match. Nonzero rows stay scout-only and are not same-state parity claims.

## Inputs

- Original crops: `verification-screens/pass1052-dm1-original-route-24h-turncycle/viewport_224x136`
- Pairing artifacts: `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/pairs`
- Pairing manifest: `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/manifest.json`

## Result

- Original crops OK: `True`
- Pair rows: `4`
- Exact match count: `1`
- Expected exact wall match present: `True`
- Pass1054 scout-only status pinned: `True`
- Non-exact rows remain nonzero scout rows: `True`
- Pair artifact hashes match manifest: `True`

| Original crop | Best Firestaff crop | MAE | Changed pixels | Status |
|---|---|---:|---:|---|
| `01_party_hud_original_viewport_224x136.png` | `hall_1_4_dirS_viewport_224x136.ppm` | 4.234178 | 1303 | Scout only |
| `02_left_1_wall_original_viewport_224x136.png` | `hall_1_4_dirE_viewport_224x136.ppm` | 0.0 | 0 | Exact pixel match |
| `03_left_2_view_original_viewport_224x136.png` | `hall_1_2_dirS_viewport_224x136.ppm` | 12.887474 | 3967 | Scout only |
| `04_left_3_view_original_viewport_224x136.png` | `hall_1_4_dirE_viewport_224x136.ppm` | 4.937533 | 2858 | Scout only |

## Non-claims

- This does not promote the three nonzero rows as same-state parity.
- This does not close creature-chain or champion-panel capture gaps.
- This does not replace a future debugger-observed original route transcript.
