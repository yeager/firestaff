# Nexus V1 — Bonus Content and Special Editions

## Sources
- `docs/nexus_overview.md`
- `docs/NEXUS_FILE_CLASSIFICATION.md`
- `docs/NEXUS_PLAN.md`
- `docs/nexus_save.md`
- `docs/nexus_features.md`
- Saturn CD image: `Dungeon Master Nexus_SEGA-Saturn_JA.zip`

---

## 1. No Special Editions — One Release

Dungeon Master Nexus had **one official release** on one platform:

| Release | Platform | Date | Notes |
|---------|----------|------|-------|
| T-9111G V1.003 | Sega Saturn | 1998-02-03 | Only release — no variants |

No special editions, no collector's editions, no GOTY edition, no pre-order bonuses, no demo discs, no shareware release.

---

## 2. What the Disc Includes

The Saturn CD contains the complete game in one release. The disc is structured as:

### Audio Content (CD-DA Tracks)
```
Track 2:  Level 0 music (Red Book CD-DA)
Track 3:  Level 1 music
Track 4:  Level 2 music
Track 5:  Level 3 music
Track 6:  Level 4 music
Track 7:  Level 5 music
Track 8:  Level 6 music
Track 9:  Level 7 music
(Note: Levels 8–15 may use replayed or looped tracks from earlier levels)
```
This is per-level music — 8 distinct CD audio tracks, one per disc track.

### Video Content
- **3 AVI cutscenes** (101 MB total): DMV0.AVI (intro, 34 MB), DMV1.AVI (28 MB), DMV2.AVI (ending, 39 MB)
- Saturn FMV format — these play on the Saturn hardware's video decoder

### Text Files on Disc
- `DMN_ABS.TXT` — in-game text/inscriptions (Japanese Shift-JIS)
- `DMN_BIB.TXT` — bibliography/attribution
- `DMN_CPY.TXT` — copyright notice (1998 Victor Interactive Software / FTL Games / Software Heaven)

---

## 3. No Demo, No Shareware, No Trial

- Not a shareware title — sold as full retail only
- No demo disc released publicly
- No beta or promotional disc with exclusive content
- The disc is self-contained: game + all 8 audio tracks + 3 FMV videos

---

## 4. Firestaff as "Bonus" Content

Firestaff's reimplementation (the `firestaff_nexus` library) is itself a form of bonus content for the series:

### Bonus: Cross-Platform Access
- The original game only runs on real Saturn hardware (or emulators)
- Firestaff Nexus builds on Windows/macOS/Linux — effectively the only way to play Nexus today without Saturn hardware

### Bonus: Source Code
- The original has no source — Firestaff is a clean-room reimplementation
- Source code in `src/nexus/nexus_v1_*.c` provides full transparency into how the game works

### Bonus: No DRM
- Original disc required a real Saturn and the game disc
- Firestaff runs from the extracted data files with no copy protection

### Bonus: New Renderer
- Original VDP1 software rasterizer (Saturn) vs. Firestaff's CPU/software rasterizer (PC)
- Different aesthetic but same game logic — a technical bonus for preservation

---

## 5. Secret/Undocumented Features

### Per-Level Scripting (SDDRVS.TSK)
- The `SDDRVS.TSK` file (26 KB) is described as a "sound driver task" but may also contain a script VM
- If it implements a declarative scripting system, that counts as undocumented bonus functionality
- Level scripts in `SLEV00-15.BIN` (2–12 KB each) control events, spawns, triggers
- This is a hidden layer of game logic not present in any other DM title

### DMDF 3D Model Format
- 30 `.MNS` creature model files — each is a 3D polygon model with animations
- The DMDF format is unique to Nexus (not used in DM1, CSB, or DM2)
- Animation frames are embedded as 3D vertex arrays per model
- This is effectively bonus content: 3D creature models no other DM game has

### FMV in a Dungeon Crawler
- The 3 AVI cutscenes (intro/ending) are bonus narrative content
- No other game in the DM series has FMV — this is unique to Nexus

### Japanese-Only Champion Names
- 24 champions with Japanese katakana names — for non-Japanese players, this is bonus/exotic content
- Champion portraits in `FACE.BIN` (44 KB) are unique art

---

## 6. Comparison: Nexus vs. DM1/CSB/DM2 Bonus Content

| Bonus Type | DM1 | CSB | DM2 | Nexus |
|-----------|-----|-----|-----|-------|
| Demo disc | No | No | No | **No** |
| Shareware release | No | No | No | **No** |
| Special edition | No | No | No | **No** |
| Pre-order bonus | No | No | No | **No** |
| FMV cutscenes | No | No | No | **Yes (3 AVI)** |
| Per-level audio | No | No | No | **Yes (8 CD tracks)** |
| Level scripting | No | No | No | **Yes (SLEV*.BIN)** |
| 3D models | No | No | No | **Yes (30 .MNS)** |
| Multilingual | EN/JP (separate DAT) | EN | EN only | **JP only** |
| Cross-platform (modern) | ReDMCSB + DOSBox | Partial | SKULL.ASM + DOSBox | **Firestaff (native)** |

---

## 7. Bonus Content Conclusion

**The original Nexus release had no special editions or bonus discs.** The game itself IS the content — 137 files on a Saturn CD, including the only FMV in the series, unique per-level audio, and a scripting VM. The closest thing to "bonus content" is Firestaff's reimplementation, which makes Nexus playable on modern hardware with full source code transparency.
