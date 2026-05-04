# DM1 PC 3.4 `GRAPHICS.DAT` trusted-reference shortlist

Date: 2026-04-23

This is the compact follow-on list for future original-vs-4K comparison work.

## Status keys

- `locked` - supported by stronger naming/runtime evidence
- `provisional` - visually plausible and useful, but not yet fully locked
- `do not use` - currently unsafe as a trusted original anchor

## High-priority UI and world references

| Index / family | Intended role | Status | Notes |
|---|---|---|---|
| `0009` | spell strip family | provisional | Best current narrow-strip candidate. |
| `0010` | action strip family | provisional | Stronger than most nearby UI candidates because of shape and visible `PASS` text. |
| `0033..0035` | slot-border / HUD-cell family | provisional | Good secondary comparison family after panel bindings are stabilized. |
| `0078` | floor-like world strip | provisional | Strong visual fit. |
| `0007` / `0008` | small status-family elements | provisional, verify first | Do not claim champion-info left/right frames yet. |
| `0020` | empty information/content panel | provisional, verify first | Promising but not locked. |
| `0079` | ceiling-like or far-surface strip | provisional, verify first | Companion-world strip, but weaker than `0078`. |
| `0000` | formerly assumed viewport frame | do not use | Current Firestaff binding is contradicted by the exported bitmap. |

## Trusted control family

| Index / family | Intended role | Status | Notes |
|---|---|---|---|
| `0303..0320` | ornate lock family | locked | Backed by repo note citing DMExtract PC map and Greatstone PC page. |

## Minimal replacement-PDF build rules

1. Never use `0000` as a trusted viewport/frame anchor.
2. Prefer `0009`, `0010`, `0033..0035`, and `0078` first.
3. Treat `0007`, `0008`, `0020`, and `0079` as captioned provisional references until runtime binding is proven.
4. Mark each comparison entry with `locked` or `provisional` directly in the page caption.
5. Use screenshot crops for whole-screen geometry when single-graphic composition assembly is uncertain.
6. Keep the ornate lock family as a sanity-check control set because its identity is already locked.
