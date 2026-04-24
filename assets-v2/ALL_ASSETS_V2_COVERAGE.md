# Firestaff V2 All-Assets Coverage

## Purpose

This document maps the current V2 asset direction across every major asset class that is discoverable or already implied by the repo.
It is a coverage map, not a claim of finished remastering.

## Coverage summary

| Asset class | Discoverable in repo | Current V2 state | Pipeline readiness | Main hooks |
|---|---|---|---|---|
| UI / HUD shells | Yes | Active Wave 1 manifests and rebuilt/provisional assets exist | Ready now | `assets-v2/ui/wave1/`, `assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json` |
| Creatures | Yes | Active Wave 1 manifests, specs, and family directories exist | Ready now | `assets-v2/creatures/wave1/`, `assets-v2/manifests/firestaff-v2-wave1-creatures.manifest.json` |
| Environment / structural | Yes, as classified/planned families | No dedicated V2 asset pack yet | Brief-ready | `assets-v2/environment/wave1/README.md` |
| Items / action icons | Yes, as classified/planned families | No dedicated V2 asset pack yet | Brief-ready | `assets-v2/items/wave1/README.md` |
| Effects / projectiles / magic | Yes, as classified/planned families | No dedicated V2 asset pack yet | Brief-ready | `assets-v2/effects/wave1/README.md` |
| Portraits | Yes, as classified/planned family | No dedicated V2 asset pack yet | Brief-ready | `assets-v2/portraits/wave1/README.md` |
| Title / intro / end / branding | Partly | Branding evidence exists; no dedicated V2 title/end pack yet | Brief-ready | `assets-v2/branding/wave1/README.md`, `assets/branding/firestaff-logo.png` |
| Typography / text-bearing UI | Yes, as classified/planned family | No dedicated V2 typography pack yet | Brief-ready | `assets-v2/typography/wave1/README.md` |

## Notes by class

### UI / HUD shells
- Already covered by the current Wave 1 UI pack.
- `viewport-frame` remains explicitly provisional/blocked until the `0000` mapping is re-locked.
- Action area, spell area, status boxes, and party HUD cells already have concrete 4K/1080p hooks.

### Creatures
- Already covered by the current Wave 1 creature inventory and per-family manifests.
- The style direction in `V2_ALL_ASSETS_STYLE_GUIDE.md` now makes the creature rules consistent with the broader all-assets brief.

### Environment / structural
- Discoverable through `V2_ASSET_CLASSIFICATION_TABLE.md` and related V2 planning docs.
- Honest current state: no checked-in V2 environment pack or manifest yet.
- Next hook is a bounded door/stairs/wall-module brief rather than pretending a full repaint set already exists.

### Items / action icons
- Discoverable through the V2 classification table and graphics-plan documents.
- Honest current state: no dedicated V2 item/icon asset directories or manifests exist yet.
- Next hook is a bounded icon/action-hand starter set.

### Effects / projectiles / magic
- Discoverable through the V2 classification table.
- Honest current state: no dedicated V2 effect pack exists yet.
- Next hook is a small projectile/impact layer brief.

### Portraits
- Discoverable through the V2 classification table and Wave 1 exclusions.
- Honest current state: no portrait pack is checked in.
- Next hook is a single portrait presentation contract for one champion payload.

### Title / intro / end / branding
- Branding evidence exists in `assets/branding/`.
- Honest current state: no dedicated V2 title/intro/end pack is checked in.
- Next hook is a title/end branding brief rather than broad claims.

### Typography / text-bearing UI
- Discoverable through V2 docs that already exclude old bitmap text from direct upscale.
- Honest current state: no V2 typography system is checked in.
- Next hook is a restrained HUD label and action text contract.
