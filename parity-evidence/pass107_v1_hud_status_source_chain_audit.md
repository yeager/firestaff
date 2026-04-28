# Pass 107 — V1 HUD/status source-chain audit

Scope: DM1 PC 3.4 V1 champion top row / HUD status panel evidence only.

This pass adds a bounded source-chain audit between `zones_h_reconstruction.json` and the existing pass83 champion HUD overlay. It avoids another raw log dump: the JSON records the exact layout-696 records checked and the pass83 overlay zones they support.

## Result

- audit JSON: `parity-evidence/overlays/pass107_v1_hud_status_source_chain_audit.json`
- source: `zones_h_reconstruction.json`
- pass83 stats: `parity-evidence/overlays/pass83/pass83_champion_hud_zone_overlay_stats.json`
- pass: `True`
- problems: `0`

## Locked source chains

- `C150` status-box template is `67x29`; `C151..C154` place slots at x `0/69/138/207`.
- `C155..C166` name clear/text chain is present under each status slot; text is clipped with the source `+1` x offset.
- `C183..C206` bar-graph/value chain is present for all four slots and three stats (`C195..C206`).
- `C207..C218` ready/action hand source chain is present for all four slots.
- Pass83 remains schema `pass83_champion_hud_zone_overlay_probe.v3` and passing.

## Remaining HUD-specific honesty boundary

Independent Python re-resolution of type=7 proportional bar-value records is still not implemented here; bar pixel-fill parity remains covered by M11 C probes and pass83 crops, not by original runtime screenshots.

## Gate

```sh
python3 tools/pass107_v1_hud_status_source_chain_audit.py
python3 -m json.tool parity-evidence/overlays/pass107_v1_hud_status_source_chain_audit.json >/dev/null
git diff --check
```
