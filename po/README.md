# po/ — Firestaff Translation Catalogs

Per-domain i18n layout for Firestaff. See `I18N_PO_LAYOUT_PLAN.md` at repo root for the full specification.

## Domains

| Domain         | Scope                                          | Status       |
|----------------|-------------------------------------------------|-------------|
| `startup-menu` | Top-level launcher, mode selection, game cards  | Seed (Slice 1) |
| `dm1`          | Dungeon Master 1 in-game HUD/runtime            | Seed (Slice 1) |
| `csb`          | Chaos Strikes Back runtime                       | Seed (Slice 1) |
| `dm2`          | Dungeon Master 2 runtime                         | Seed (Slice 1) |

## File Naming

- **Templates:** `<domain>.pot` — canonical source-string templates
- **Catalogs:** `<domain>.<lang>.po` — per-language translations

## Rules

- English catalogs (`.en.po`) are loaded at runtime, not just used as source.
- `.pot` files are the template source for translators and merge tools.
- Fallback chain: active domain in active language → same domain in English → key itself.
- No cross-domain fallback (e.g., `csb` never silently reads from `dm1`).

## Validation

Run `po/validate_po_layout.sh` to verify structural consistency.
