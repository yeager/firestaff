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
- VDP1/VDP2 placement and palette upload order remain capture-gated
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
