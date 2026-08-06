# Nexus V1 Intro — Source-Locked Audit

> Den här äldre introöversikten får inte användas som bevis för en färdig
> Firestaff-titelskärm. Aktuell startupstatus finns i
> [`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).

## Sources
- docs/nexus_startup.md (boot sequence)
- docs/NEXUS_FILE_CLASSIFICATION.md (disc layout)
- docs/nexus_overview.md (Nexus vs DM1)
- src/nexus/nexus_v1_engine.c

---

## 1. Intro Sequence — Original Saturn

The original Saturn version has NO traditional intro movie in the Firestaff
implementation sense. The disc contains FMV files (DMV0-2.AVI) but the startup
sequence is:

1. Saturn BIOS boots from CD
2. SYSTEM.CNF points to NEXUS.BIN entry point
3. `nexus_v1_init()` kör source discovery och behåller verifierade receipts
4. Saturnens title-route är capture-observerad men Firestaffs presentation är
   fortfarande stängd; ingen host-cropping eller syntetisk titel används
5. FMV cutscenes play DURING gameplay at specific triggers (not at startup)

---

## 2. Disc FMV Files

The disc image contains three AVI files on Track 1 (ISO):

| File | Purpose | Format |
|------|---------|--------|
| DMV0.AVI | Intro/first cutscene? | AVI (CD-ROM video) |
| DMV1.AVI | Mid-game cutscene? | AVI |
| DMV2.AVI | Ending cutscene? | AVI |

These are Sega Format AVI files (not standard AVI), likely using
Sega/MPEG1 or similar codec adapted for Saturn CD-ROM.

---

## 3. FMV Playback Status

**NOT YET IMPLEMENTED in Firestaff Nexus.**

- No AVI decoder in the codebase
- No nexus_v1_fmv_play() or equivalent function
- The DMV files are referenced in NEXUS_FILE_CLASSIFICATION.md but
  no decoder/player exists in src/nexus/
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

Retailen har startup/title-resurser (`TITLE.CG`, `TITLE.BIN`, `LOGOBG.DG2` och
andra ytor), men deras Saturn VDP2-lager, timing och placering är inte bundna
till Firestaff. En host-rasterizer får därför inte beskrivas som den
ursprungliga animerade logon.

---

## 6. Not Yet Implemented

- AVI demuxer for DMV0-2.AVI files
- Sega Format AVI codec decode
- FMV playback integration with game state machine
- VDP1/VDP2 video output for FMV
