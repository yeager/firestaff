# Trusted-scope original vs V2 comparison PDF

This file documents the replacement for the invalid comparison PDF.

## Why this exists

`docs/reports/original-vs-v2-4k-asset-comparison.pdf` is no longer trustworthy as an authoritative mapping/reference artifact because `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md` confirmed that some of its original-side semantic bindings were wrong or too strong.

This trusted-scope replacement is intentionally smaller. It includes only pages where:

1. the original `GRAPHICS.DAT` mapping is currently trusted by the audit, and
2. a clear Firestaff V2 counterpart asset already exists in the repo.

## Source-of-truth rule

- Greatstone/SCK is the primary asset-reference source.
- ReDMCSB is the primary code/usage reference.
- Firestaff local exports are raw material only.

## Included mappings in this PDF

- `0009` — spell area background
- `0010` — action area background
- `0033` — slot box normal

## Trusted mappings intentionally excluded from the PDF

These mappings remain trusted by the audit, but are excluded here because this pass did not find a clear like-for-like Firestaff V2 comparison asset already present in the repo:

- `0020` — panel empty
- `0035` — slot box acting hand
- `0303` — stone lock, left side
- `0304` — stone lock, front

## Explicitly not reused from the invalid PDF

These prior original-side claims must not be carried forward into trusted comparison work without re-verification:

- `0000` as a viewport-frame base
- `0007` as a left status-box frame
- `0008` as a right status-box frame
- `0034` as a generic highlight overlay
- `0078` / `0079` as trusted floor/ceiling anchors

## Rebuild

Run:

```bash
python3 tools/build_trusted_original_vs_v2_asset_pdf.py
```

Output:

- `docs/reports/original-vs-v2-4k-asset-comparison-trusted-scope.pdf`
