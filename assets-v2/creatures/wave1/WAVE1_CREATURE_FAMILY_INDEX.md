# Firestaff V2 Wave 1 creature family index

This index tracks the full currently discoverable V2 creature-family source set covered by the bounded Wave 1 prep/upscale/export workflow.

## Coverage

- families in scope: **10**
- generated asset outputs: **30** front-view stills (`near`, `mid`, `far`)
- resolution outputs: **60** image files across 4K masters and 1080p derivatives
- timing rule: smoother V2 animation is allowed later, but perceived gameplay timing and travel/attack speed must stay aligned to the original

## Family inventory

| Family | Source | Coverage | Notes |
|---|---|---|---|
| Demon | `assets/cards/creature-refs/demon.png` | front near/mid/far → 4K + 1080p | reference sprite source |
| Giant Scorpion | `assets/cards/creatures/giant-scorpion.png` | front near/mid/far → 4K + 1080p | card art source |
| Giggler | `assets/cards/creature-refs/giggler.png` | front near/mid/far → 4K + 1080p | reference sprite source |
| Mummy | `assets/cards/creatures/mummy.png` | front near/mid/far → 4K + 1080p | card art source |
| Red Dragon | `assets/cards/creatures/red-dragon.png` | front near/mid/far → 4K + 1080p | card art source |
| Screamer | `assets/cards/creature-refs/screamer.png` | front near/mid/far → 4K + 1080p | reference sprite source |
| Skeleton | `assets/cards/creatures/skeleton.png` | front near/mid/far → 4K + 1080p | card art source |
| Stone Golem | `assets/cards/creatures/stone-golem.png` | front near/mid/far → 4K + 1080p | card art source |
| Swamp Slime | `assets/cards/creature-refs/swamp_slime.png` | front near/mid/far → 4K + 1080p | reference sprite source |
| Vexirk | `assets/cards/creature-refs/vexirk.png` | front near/mid/far → 4K + 1080p | reference sprite source |

## Regeneration

```bash
python3 tools/generate_v2_creature_wave1_pack.py
```

The pack generator reads `creature_family_inventory.json`, regenerates every family directory/manifest pair, then refreshes the aggregate manifest in `assets-v2/manifests/`.
