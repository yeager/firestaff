# V2 shared logical catalog source gate (2026-04-30)

This gate keeps the V2 shared logical-ID catalog tied to source-backed manifest assets without touching V1/M11 viewport or HUD renderer code.

## ReDMCSB source audit used for this gate

The immediate source dependency is the Wave 1 UI logical binding for the action/spell panels and the manifest source evidence behind those bindings:

- `DEFS.H` `graphic constants`, lines 2163-2174: defines the shared graphic index namespace and `C010_GRAPHIC_MENU_ACTION_AREA` as graphic 10.
- `DATA.C` `G0000/G0001/G0002 Graphic562 boxes`, lines 119-121: defines spell, action, and movement panel screen boxes at x=224..319 with y=42..74, 77..121, and 124..168.
- `MENU.C` `G0499/G0500/G0501 action menu boxes and spell controls`, lines 494-500: defines action-menu destination variants and the spell-area controls box.
- `ACTIDRAW.C` `F0387_MENUS_DrawActionArea`, lines 317-356: clears the action area, selects the active one/two/three-action box, then blits `C010_GRAPHIC_MENU_ACTION_AREA` into that box.
- `DEFS.H` `version 1.x/2.x spell-area graphics`, lines 2214-2218: defines `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` and `C011_GRAPHIC_MENU_SPELL_AREA_LINES`, including the note that line 1 is not drawn and lines 2/3 supply symbols over graphic 9.
- `MENUDRAW.C` `F0396_MENUS_LoadSpellAreaLinesBitmap`, lines 30-40: loads and expands `C011_GRAPHIC_MENU_SPELL_AREA_LINES` into spell-area line buffers.
- `MENUDRAW.C` `F0397_MENUS_DrawAvailableSymbols` / `F0398_MENUS_DrawChampionSymbols`, lines 64-80 and 100-121: draws available rune symbols and champion selected symbols into the spell-area coordinates/zones.

The V2 manifest follow-up now records `sourceReference.sourceEvidence` for `fs.v2.ui.spell-area.base` and `fs.v2.ui.spell-area.rune-bed`, so the logical catalog can require that a shared ID is not only bound to an existing manifest asset, but bound to one with explicit ReDMCSB file/function/line evidence.

## Acceptance gate

Run:

```sh
python3 scripts/validate_v2_logical_catalog.py \
  --require-existing-manifest-binding fs.v2.shared.ui.action-panel.base \
  --require-existing-manifest-binding fs.v2.shared.ui.spell-panel.base \
  --require-bound-manifest-source-evidence fs.v2.shared.ui.action-panel.base \
  --require-bound-manifest-source-evidence fs.v2.shared.ui.spell-panel.base
```

The gate validates catalog shape, unique category/logical IDs, shared namespace/category alignment, status/binding enums, and verifies all `binding.existingManifestId` values against the V2 manifest asset IDs. The two required UI logical IDs above must remain bound to concrete manifest entries with `sourceReference.sourceEvidence`.

Current result on N2: `validated V2 logical catalog: 8 categories / 45 logical IDs / 4 manifest bindings / 3 source-evidence bindings`.
