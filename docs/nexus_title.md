# Nexus V1 Title Screen — Source-Locked Audit

## Sources
- src/frontend/title_frontend_v1.c (DM1 reference)
- docs/nexus_menus.md (menu system status)
- docs/nexus_overview.md (graphics architecture)
- src/nexus/nexus_v1_viewport.c, nexus_v1_rasterizer.c

---

## 1. Nexus Title Screen — Original Saturn

The supplied retail corpus contains a real `TITLE.CG` atlas. Firestaff
validates and decodes its 32-byte prefix plus packed 4bpp payload into a
source-owned surface. The Saturn screen placement, palette consumer, and any
animated executable-side title composition are not yet authenticated, so the
surface remains a data receipt rather than a claim of final title parity.

The same corpus contains `LOGOBG.DG2`, a separate retail PP layer. Firestaff
now decodes its 320×224 indexed pixel plane and 256-word BGR555 palette into
an optional source-owned surface. It remains no-draw until a Saturn VDP2
capture proves layer selection, palette bank, timing and placement.

---

## 2. DM1 Title Screen (Reference)

DM1 title (source-locked from ReDMCSB):
- File: TITLE.C, function F0437_STARTEND_DrawTitle
- Static 2D bitmap logo: "DUNGEON MASTER" in stylized text
- Pre-rendered graphic, not real-time 3D
- No animation (static image until player presses key)

DM1 title is a flat 2D sprite-based render — the complete opposite of
Nexus's real-time 3D approach.

---

## 3. Firestaff Title Screen Implementation

The source-owned `TITLE.CG` decode exists in `nexus_v1_ui_surfaces.c` and is
loaded by the Nexus engine. A complete Saturn title consumer is not yet
implemented: no VDP1/VDP2 placement, palette upload order, or executable-side
animation route is admitted without capture evidence.

The bounded `MAPD/TIBG` receipt also requires the complete section: five
64×28 maps followed by sixteen big-endian palette words (`0x8c74` bytes from
the MAPD start). A shorter block is rejected before palette reads; this is
format validation only and does not authorize Saturn presentation.

### Retail MAPD preservation observation

Decoding the five authenticated 512×224 indexed maps from the external
English retail corpus produces five distinct source images whose visible
letterforms are `N`, `E`, `X`, `U`, and `S`, respectively. This identifies the
content of the five MAPD planes; it does not establish their Saturn display
order, frame timing, VDP2 layer assignment, or executable title-state owner.

The decoded pixel-plane SHA-256 receipts are:

| MAPD plane | Observed letterform | Pixel-plane SHA-256 |
|---:|:---:|:---|
| 0 | N | `f816105d6362d0b569e2787f4895710cb6272841fbe00d9e1ec494b5ababefa5` |
| 1 | E | `cee98c6b82e36b37a6bc285d62896468194de01faeff048c0ab8865f84962ad6` |
| 2 | X | `d1795a76f459f789943c6985500c968ed9b961cc781c7dd56b88abdbf6bd28cd` |
| 3 | U | `b5cd2e767d4ee592f908268a00400251ceb4f69c4393a7df775c1e0fcd5e1498` |
| 4 | S | `6dbb2f3bea87a8b79219f0a73c0b2f49d12a572829c8913b501fbce58f74647` |

These are preservation receipts derived from the real `TITLE.BIN` and
`TITLE.CG`; no generated image is used. The Saturn capture corpus currently
contains no exact MAPD plane span in VDP2 VRAM, so the production title route
remains capture-gated.

### Japanese retail title VDP2 receipt

A post-intro, same-session Japanese retail capture identifies the actual
title sequence and binds two source spans without unpacking the CUE corpus:

- the full `TITLE.CG` character-generator payload is in VDP2 VRAM at
  `0x24020` in Saturn word-swapped order;
- the 16-word `TITLE.BIN` MAPD palette is in CRAM at `0x400`, also
  word-swapped;
- VDP2 reports `TVMD=0x8000`, `BGON=0x0003`, and `CHCTLA=0x0013`.

None of the five raw MAPD planes occurs as an exact VDP2 VRAM span in that
frame. The receipt therefore proves source upload and palette residence, not
the tilemap transform, layer placement, timing, or final title composition.

---

## 4. Title Screen vs Intro Movie

The supplied corpus includes `TITLE.CG` and separate DMV video assets, but the
runtime relationship between them and the executable-side title state has not
been proven from a Saturn capture. Firestaff therefore does not assume that
the title is a 3D animation or that a particular DMV file precedes it.

---

## 5. Title Screen Architecture

### Original Saturn
- `TITLE.CG` is a source asset in the retail corpus
- `TITLE.CG` VRAM and the MAPD palette CRAM upload are capture-proven; map
  transform, placement and ordering remain capture-gated
- FONT256/SLEV text ownership remains separate and unproven

### Firestaff PC
- `nexus_v1_load_startup_surfaces()` loads the real `TITLE.CG` surface
- The startup handoff retains the source surface but blocks final placement
- No synthetic title art or inferred animated camera route is permitted

---

## 6. What Needs to be Built

1. Bind `TITLE.CG` to the original Saturn VDP1/VDP2 placement and palette
   sequence through an authenticated capture.
2. Recover any executable-side title animation/state transition owner.
3. Bind title text through the real FONT256/SLEV consumer, not host strings.

---

## 7. Comparison Table

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Title type | Static 2D bitmap | `TITLE.CG` source atlas; final composition unproven |
| Source | TITLE.C asset | `TITLE.CG` source atlas; VDP1/VDP2 consumer unproven |
| Animation | None (static) | Not proven; executable/capture route remains gated |
| Language | English | Per-revision text status; no global language claim |
| Impl status | Complete (ReDMCSB) | Source decode exists; Saturn presentation remains gated |
