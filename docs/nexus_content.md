# Nexus V1 — Content Differences Between Versions

## Sources
- `docs/NEXUS_FILE_CLASSIFICATION.md`
- `docs/nexus_overview.md`
- `docs/nexus_features.md`
- `docs/nexus_dungeon.md`
- `docs/nexus_creatures.md`
- `docs/nexus_champions.md`
- `docs/nexus_music.md`
- `docs/nexus_save.md`
- Extracted Saturn ISO file listing

---

## 1. Content Overview: What Changed vs. DM1

Nexus is a **3D remake of DM1**, not a sequel. The core dungeon, creatures, and game mechanics are derived from DM1. Here is what changed:

| Aspect | DM1 (1987) | Nexus V1 (1998) | Delta |
|--------|-----------|-----------------|-------|
| Rendering | 2D sprite blitting | 3D polygon rasterizer | All new |
| Dungeon levels | 10 (D0–D7 +入口) | 16 (LEV00–LEV15) | +6 levels |
| Grid per level | Variable 16–30 | Fixed 32×32 | Larger, uniform |
| Dungeon data size | ~33 KB total | ~4.3 MB total | ~130× larger |
| 3D geometry | None | Baked into DGN files | All new |
| Creature models | 2D sprite sheets | 3D .MNS polygon meshes | All new |
| Audio | Global SND.GAM (one bank) | Per-level SNDLEV*.SAL (32 files) | All new |
| FMV cutscenes | None | 3 AVI files (101 MB total) | All new |
| Champion names | Western (Thor, Sara…) | Japanese (Syra, Leyla, Nabi…) | Localized |
| Language | English | Japanese only | Localized |
| Text encoding | ASCII | Shift-JIS (katakana) | All new |
| Viewport distance | 2 squares | 4 squares | Doubled |
| Level scripts | Hardwired game loop | SLEV*.BIN per level (2–12 KB each) | All new |
| Minimap data | ASCII chart | SMAP00–15.BIN (17–30 KB each) | All new |
| Font | ASCII bitmap | 256-char font incl. Japanese | All new |
| Save format | Binary slot files | Saturn SRAM (8 KB) | All new |
| Platform | Amiga/ST/DOS | Sega Saturn | All new |

---

## 2. What Nexus Adds (New Content)

### 2.1 3D Graphics Pipeline
- First-person 3D polygon view using Saturn VDP1 (or software fallback on PC)
- Per-level 3D wall/floor/ceiling geometry embedded in DGN files
- Edge-function triangle rasterizer
- Z-buffer for correct wall occlusion
- DMDF 3D model format for creatures (.MNS files = Monster?)

### 2.2 Extended Dungeon — 16 Levels
- DM1 had 10 levels; Nexus has 16 (LEV00–LEV15)
- Grid is fixed 32×32 vs. DM1's variable grid sizes
- Extra levels are in addition to the original DM1 layout, not replacing it
- Source: `docs/nexus_dungeon.md` confirms 16 level files (147–321 KB each)

### 2.3 FMV Cutscenes (3 AVI Files)
- `DMV0.AVI` (34 MB), `DMV1.AVI` (28 MB), `DMV2.AVI` (39 MB) = 101 MB total
- Intro, ending, possibly mid-game cutscenes
- These are the only video content in the series
- Saturn AVI format — playable on Saturn hardware

### 2.4 Per-Level Audio Banks
- 32 per-level sound files: `SNDLEV00-15.SAL` + `SNDLEV00-15.MAP`
- Each SAL file: 290–460 KB of sound effects per level
- vs. DM1's single global SND.GAM bank
- 8 Red Book CD-DA audio tracks (tracks 2–9 of disc)

### 2.5 Per-Level Scripts (SLEV*.BIN)
- 2–12 KB of script data per level (`SLEV00.BIN` through `SLEV15.BIN`)
- Declarative scripting vs. DM1's hardwired game loop
- Script VM processes sensor events, creature spawns, teleport triggers
- DM1 had no scripting — sensors were compile-time behavior in the game loop

### 2.6 Minimap Data (SMAP*.BIN)
- Each level has a 17–30 KB minimap/automap binary
- DM1 had only an ASCII chart manually tracked by the player
- Nexus provides built-in per-level map data

---

## 3. What Nexus Preserves from DM1 (Untouched)

### 3.1 Core Mechanics
- Same 4 champion classes: Fighter, Wizard, Priest, Ninja
- Same 24-champion roster (Japanese names in Nexus)
- Same spell system: 16 spells from DM1
- Same food/water (1500 each), same consumption model
- Same champion advancement (experience → stat increases)
- Same inventory system (4 hand slots, pack, belt)
- Same combat formula (attack roll, defense, damage)
- Same dungeon square types (wall, floor, door, torch, teleporter, sensor, pit, altar, etc.)

### 3.2 Dungeon Layout
- Nexus dungeon is based on DM1's dungeon layout
- The 16 levels extend the original 10-level DM1 dungeon
- Creature placements derived from DM1 equivalents

### 3.3 Creatures
- Same creature roster as DM1 (~15 core types)
- Same creature stats (HP, ATK, DEF, SPD, XP)
- Creature behavior is simplified: DM1 full AI (wariness, pit avoidance, teleporter awareness) vs. Nexus simplified 3-state AI

---

## 4. Content Unique to Nexus (Not in Any Other DM Game)

- **3D polygon dungeon rendering** (unique in series)
- **FMV cutscenes** (unique in series)
- **Per-level sound banks** (unique vs. DM1's single global bank)
- **Level scripting VM** (SDDRVS.TSK) — declarative scripting no other DM game has
- **16-level fixed grid** dungeon (DM1: variable grid, 10 levels)
- **Minimap binary data** (DM1 had ASCII chart only)
- **Japanese-language UI** with katakana champion names
- **Anti-Magic and Anti-Fire defaults at 5** (DM1 started these at 0)
- **4-square visibility** (DM1: 2 squares)
- **24 katakana-name champions** (all Japanese names vs. Western names in all other games)

---

## 5. Files Unique to Nexus (Not in DM1/CSB/DM2)

```
LEV00-15.DGN     — 16 level data files (DM1 had DUNGEON.DAT, 33 KB)
SLEV00-15.BIN    — per-level scripts (new concept)
SMAP00-15.BIN    — per-level minimap data (new concept)
SNDLEV00-15.SAL  — per-level sound banks (new concept)
SNDLEV00-15.MAP  — per-level sound mapping tables (new concept)
DMV0-2.AVI       — FMV cutscenes (new concept)
.MNS files       — 3D creature model files (30 files)
DM.BIN           — main engine binary (542 KB)
0DMSTRT.BIN      — startup data
FONT256.S2D      — Japanese font (new concept)
ITEM.IBS         — item icon bitmap set
MENU.BPK         — menu graphics
TM.BIN           — texture map / tilemap
SDDRVS.TSK       — sound driver task / script VM
```
