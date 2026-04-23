# Startup text-vs-graphics audit

Date: 2026-04-23
Scope: bounded SM4 audit for the V1/original-faithful Firestaff startup/menu path after the SM1-SM3 sparse rebuild.
Truth basis: `STARTUP_VISUAL_COMPARE_FIRESTAFF_VS_REDMCSB.md` plus the local ReDMCSB truth frames/sheets named there.

## Classification

| Element | Classification | Why |
| --- | --- | --- |
| Large `FIRESTAFF` banner | REMOVE | Not present in the chosen ReDMCSB startup/menu truth set. Custom branding drift. |
| `FRONTEND PREVIEW` eyebrow | REMOVE | No support in ReDMCSB truth. Custom launcher language. |
| Bright startup status chips such as `READY`, `SCHEMA`, `OPEN` | REMOVE | No support in the chosen truth frames; they made the screen read like a custom launcher. |
| Full left/right launcher panels | REMOVE | Structural mismatch versus the sparse centered ReDMCSB composition. |
| Bottom help/footer text bar | REMOVE from V1 startup hold | Not supported by the truth frames; too launcher-like for the original-faithful path. |
| Centered title/menu placeholder text used in the rebuilt sparse V1 path | KEEP_AS_TEXT (temporary) | Firestaff still lacks verified original startup/title/menu graphics for these exact composed states. Small centered text is a bounded placeholder while keeping scale/density closer to ReDMCSB. |
| Tiny language marker in the sparse V1 path | KEEP_AS_TEXT (temporary, diagnostic) | Kept only as a minimal runtime differentiator for existing startup-menu render invariants. Not claimed as original parity. |
| Settings / options / message overlay copy in V1 path | KEEP_AS_TEXT | These are functional runtime overlays, not parity claims for the original startup hold frame. Kept small and centered to avoid reintroducing launcher-shell drift. |
| Original title/menu graphics from ReDMCSB candidate sheets | REPLACE_WITH_GRAPHIC | The truth set indicates the final faithful solution should come from tiny original graphic fragments rather than text-heavy placeholders. |

## What landed now

The V1 path now prefers subtraction and sparse centered composition over custom launcher chrome.
This audit does **not** claim Firestaff already has final original startup/title/menu graphics.
It records that the remaining centered text is a bounded placeholder until verified original graphic composition is ready.

## Remaining gap

The key unresolved SM4 gap is straightforward:
- replace the current small centered placeholder text cluster with verified original-style startup/title/menu graphics derived from the local ReDMCSB truth set.
