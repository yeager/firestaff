# Pass 88 — DM1 V1 text-surface source audit

Scope: bounded evidence for the parity-matrix text/label rows that were still blanket `UNPROVEN`. This pass intentionally does **not** touch or claim viewport/wall/item parity, champion HUD top-row work, original overlay/capture work, or V2 assets.

## Commands

```sh
tools/pass88_v1_text_surface_source_audit.py > parity-evidence/pass88_v1_text_surface_source_audit.json
python3 - <<'PY'
import json
d=json.load(open('parity-evidence/pass88_v1_text_surface_source_audit.json'))
print(d['summary'])
for c in d['checks']:
    print(('PASS' if c.get('ok') else 'FAIL'), c['id'], c.get('line'))
PY
```

Result: `15/15` source/helper checks pass.

## Evidence summary

The machine-readable evidence is in `parity-evidence/pass88_v1_text_surface_source_audit.json`.

Key source facts verified:

- Message area: ReDMCSB PC source defines `M532_MESSAGE_AREA_ROW_COUNT = 4`, `C015_ZONE_MESSAGE_AREA = 15`, and prints message rows at `y = 177 + row * 7` (`DEFS.H`, `TEXT.C`). Firestaff exposes C015 and filters synthetic tick/debug text before drawing player-facing rows.
- Spell labels: ReDMCSB uses graphic `C009` for the spell-area backdrop and graphic `C011` for the two visible 14×13 spell-symbol label lines; `CASTER.C` blits C011 into C013. Firestaff has the same C011 14×13 cell seam.
- Inventory labels: ReDMCSB `PANEL.C` draws `C030_GRAPHIC_FOOD_LABEL`, `C031_GRAPHIC_WATER_LABEL`, and `C032_GRAPHIC_POISONED_LABEL` as source graphics. Firestaff maps those same graphic ids (30/31/32).
- Status name text: Firestaff routes champion name text through C163..C166 source-zone helpers; this is already a source-zone fixture, not a new HUD-top-row claim.

Reference presence also recorded:

- `~/.openclaw/data/firestaff-redmcsb-source/`
- `~/.openclaw/data/firestaff-greatstone-atlas/`
- `~/.openclaw/data/firestaff-original-games/DM/`
- local extracted DM PC 3.4 `TITLE`, `GRAPHICS.DAT`, `DUNGEON.DAT`, `SONG.DAT` with hashes in the JSON manifest.

## Matrix impact

This narrows five text rows without claiming full pixel parity:

- **Status text labels:** source-zone routed for visible champion names; full original overlay still not claimed.
- **Spell labels:** source-backed C011 graphic-cell labels are verified.
- **Inventory labels:** Food/Water/Poisoned labels are source graphic blits, not Firestaff text.
- **Message log text:** geometry/filtering is source-backed; exact message-content/cadence still requires original-behavior cases.
- **Dialog/plaque text:** dialog text zones are already source-backed; dungeon inscriptions/scroll/plaque capture parity remains open.

## Honesty boundary

No pixel parity, semantic route parity, wall-plaque/scroll text parity, or original overlay parity is claimed by this pass. It is a source/helper audit only.
