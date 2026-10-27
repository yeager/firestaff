# Nexus V1 Intro — Source-Locked Audit

> This older intro overview must not be used as evidence of a completed
> Firestaff title screen. The current startup status is in
> [`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).

## Sources
- docs/nexus/nexus_startup.md (boot sequence)
- docs/nexus/NEXUS_FILE_CLASSIFICATION.md (disc layout)
- docs/nexus/nexus_overview.md (Nexus vs DM1)
- src/nexus/nexus_v1_engine.c

---

## 1. Startup Sequence — Original Saturn

The complete retail startup sequence and the runtime roles of the AVI members
are not yet verified. The authenticated Track 1 corpus establishes the
following bounded facts:

1. Saturn firmware starts the authentic retail disc.
2. The verified Track 1 filesystem contains `DM.BIN` and `LEV00.DGN` through
   `LEV15.DGN`; an entry point named `NEXUS.BIN` or a `SYSTEM.CNF` handoff has
   not been verified and must not be assumed.
3. Firestaff's `nexus_v1_init()` runs source discovery and retains verified
   receipts.
4. Saturn's title route is capture-observed, but Firestaff's presentation
   remains closed; no host cropping or synthetic title is used
5. The runtime roles and playback triggers of `DMV0.AVI` through `DMV2.AVI`
   have not been established from source or an authenticated execution trace.

---

## 2. Disc FMV Files

The disc image contains three AVI files on Track 1 (ISO):

| File | Purpose | Format |
|------|---------|--------|
| DMV0.AVI | Intro/first cutscene? | AVI (CD-ROM video) |
| DMV1.AVI | Mid-game cutscene? | AVI |
| DMV2.AVI | Ending cutscene? | AVI |

Their byte format and playback path have not been established. Do not infer a
codec or presentation role from the `.AVI` extension.

---

## 3. FMV Playback Status

**NOT YET IMPLEMENTED in Firestaff Nexus.**

- No AVI decoder in the codebase
- No nexus_v1_fmv_play() or equivalent function
- The DMV files are inventoried in `NEXUS_FILE_CLASSIFICATION.md`, but no
  authenticated runtime consumer or decoder/player is established in
  `src/nexus/`.
- FMV playback requires: AVI demuxer + codec decode + VDP1/VDP2 blit

---

## 4. Intro vs DM1 and DM2

| Game | Intro |
|------|-------|
| DM1 (1987) | None — title screen directly |
| CSB (1991) | None — title screen directly |
| DM2 (1993) | Optional AVI intro (SKULL.KID intro movie) |
| Nexus (1998) | No intro movie; DMV files are in-game cutscenes (not startup intro) |

Nexus does NOT have a boot-time intro. The game goes directly from
engine init to title screen. The DMV AVI files are triggered during
dungeon progression as story cutscenes.

---

## 5. Title Screen vs Intro

The retail release has startup/title resources (`TITLE.CG`, `TITLE.BIN`,
`LOGOBG.DG2`, and other surfaces), but their Saturn VDP2 layers, timing, and
placement are not bound to Firestaff. A host rasterizer must therefore not be
described as the original animated logo.

---

## 6. Not Yet Implemented

- AVI demuxer for DMV0-2.AVI files
- Sega Format AVI codec decode
- FMV playback integration with game state machine
- VDP1/VDP2 video output for FMV
