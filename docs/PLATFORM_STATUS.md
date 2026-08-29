# Game and platform status

**Reviewed 2026-08-29.** This is the operational status matrix. It separates
media recognition, verified runtime slices and end-to-end playability. A row
marked as supported does not mean that every menu, save format or visual
parity gate is complete.

## Status terms

| Status | Meaning |
|---|---|
| **Playable** | Original data reaches normal gameplay through a maintained runtime gate. |
| **Verified route** | Authentic media is read and a bounded startup, New Game or gameplay route is tested. |
| **Data path** | The format or media is admitted and source-locked, but end-to-end gameplay is not proven. |
| **Preservation** | The edition is documented or retained as reference only. It cannot select a normal game route. |
| **Unsupported** | The platform is outside the game's Firestaff support matrix. |

## Dungeon Master

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC DOS 3.4 | **Playable** | The supplied English distribution ZIP is consumed in memory: direct CLI, start-menu handoff and a native movement step all bind `dungeon-master/dmaster/DATA/GRAPHICS.DAT` (MD5 `fa6b1aa29e191418713bf2cda93d962e`) through the admitted IMG3 decoder. V1 startup, menus, viewport, HUD, input, combat and saves use matching original data. | Broader original-vs-Firestaff capture and V2 finished material. |
| PC DOS French SFX/RAR2 | **Playable after manual extraction** | The supplied original compressed package is detected and reports that RAR2 is unsupported. Its manually unpacked authentic `EUDATA` reaches `dm1-runtime` through direct CLI and start menu; the actual launcher receipt admits the matching `IMG3` source decoder only after the original graphics and dungeon pair bind. The supplied `DMSAVE.DAT`/`.BAK` pair additionally completes a source-backed F0435 → F0433 → F0435 round trip against its original `DUNGEON.DAT`, preserving party, C03/C04 timeline and active-group state. | Firestaff will not decode or extract RAR2 itself. |
| Atari ST | **Verified route** | Native ST/STX media is read in RAM. The supplied English 1.0a, English 1.2, German 1.2 and French 1.3 archive routes reach the bounded runtime through direct CLI and start menu; each selected `GRAPHICS.DAT` is hash-locked. The supplied preservation collection is read as `ZIP → ZIP → STX` and admits only its `[!]` original member, including its release-specific `DUNGEON.DAT` identity; cracked siblings remain excluded. The menu receipt reports the admitted `DMCSB1` Atari decoder only after its original graphics and dungeon pair bind. | Complete gameplay parity and original-media capture. |
| Amiga 2.0 English | **Verified route** | The supplied preservation ZIP → original ZIP → ADF is read in RAM, hash-locked at `GRAPHICS.DAT` MD5 `6a2f135b53c2220f0251fa103e2a6e7e`; both the original 2.0 package and the supplied HD package (`Dungeon Master (1988)(FTL)[HD].zip` → ADF) reach the bounded native runtime through direct CLI and start-menu handoff. The menu receipt reports the admitted big-endian IMG2 decoder only after its original graphics and dungeon pair bind. | Broader gameplay, Amiga-specific presentation/input ownership, saves and capture parity. |
| Other Amiga editions | **Data path** | Version fingerprints and media/protection references are classified. | Authentic media plus per-edition native runtime proof. |
| FM Towns | **Verified route** | The supplied JA/EN original ZIP reaches `dm1-runtime` through direct CLI and start menu only after the native `TMENU.INF` selection and selected `EDM.EXP`/`JDM.EXP` receipt validate. The boot receipt publishes that selected program and its MD5 alongside the authenticated FM Towns graphics receipt. | Wider gameplay, executable/CD-audio parity and capture proof. |
| PC-9801 | **Preservation** | Japanese floppy provenance is documented. | No Firestaff runtime route. |
| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |

## Chaos Strikes Back

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Atari ST | **Verified route** | Native STX title, 50 Hz `ANIMATE.SCR`/`ANIMATE.DAT` title path, FTLCODE handoff, first runtime HUD/viewport frame and start-menu CLI route are tested against supplied campaign media. The Atari presenter emits a nonzero FNV-1a receipt for its completed source-owned 224×136 indexed viewport; it remains separate from the PC F0128 session gate. The supplied nested retail archive is read directly as ZIP → ZIP → STX in memory; its `GRAPHICS.DAT` is hash-locked. | Broader campaign and capture parity. |
| Amiga | **Verified route** | Native A31E and A31M ZIP → ADF routes read graphics, dungeon and required launcher/language programs in RAM. A31E verifies `APPB.FTL` / `BJELoad_R` and reaches `csb-entrance-0`; A31M now binds `TITL.DAT` as a sibling in the same selected virtual ADF, not a nearby host file. Its start-menu receipt reports that source-owned title boundary and hash before the language-selection route to `APPB.FTL`/`KAOS.FTL`. | Full campaign, save and visual parity. |
| FM Towns | **Verified route** | English/Japanese native packages have separate data, startup, Utility Disk and input paths. Start-menu launch reports the source-bound F31 `TITLE.ANM` palette/frame handoff and nonzero frame hash; an explicit F0435 save instead reports its verified C03 executable handoff. | Wider gameplay, Champion Editor and CD-audio parity. |
| DOS / PC | **No original release** | CSB has no DOS/PC edition. `--platform pc` is closed before media selection; CSBWin is source/disassembly evidence only. | No PC runtime route is planned from CSBWin; it must not be presented as DOS support. |
| PC-9801 | **Unsupported** | Preservation reference only. | None. |
| X68000 | **Unsupported** | Preservation reference only. | None. |

## Dungeon Master II: The Legend of Skullkeep

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| DOS | **Verified route** | Hash-verified PC data, GDAT/G1 loading, startup, runtime slices and sound tests. All eight supplied `data/sksave0..3.{dat,bak}` members resume through the native start-menu route directly from the distribution ZIP in memory; the real GDAT New Game rectangle is likewise dispatched from that retained ZIP through M11 into the native runtime. | Complete V1 renderer/mechanics parity and full SKSAVE ownership. |
| Amiga | **Verified route** | Original Amiga archive reaches title, New Game, bounded runtime and the clipped native CHARSHEET inventory frame through verified GDAT/RAW4 material. | Wider gameplay, save and full pixel/audio parity. |
| FM Towns | **Verified route** | Authentic Towns data and platform-specific startup, input, CD-audio and gameplay slices are tested. | Complete native runtime ownership and full parity. |
| Macintosh large retail | **Partial native route** | Authentic English retail ZIP is read in RAM. HFS, big-endian dungeon data, New Game, bounded wall input, and an accepted real-GDAT M11 frame with zero fallback draws pass. Its 256-row `PalIRGB`/`dtPalette16` pair remains distinct from the Amiga physical 16-colour palette route. A native QuickTime reader now admits exact original `moov`/`mdat` sample spans for all four retail films; the pixel decoder still requires optional FFmpeg, so the normal dependency-free build rejects `Title.MooV` playback and must not present it as title/movie verified. | Add bounded built-in Cinepak and QuickTime Animation pixel decoders plus `twos`/`raw ` PCM; then complete Mac GAME_LOAD/Resume, dynamic pointer/drag owner, MIDI timing and full pixel/audio parity. No authentic Mac save is present. |
| Macintosh Japanese/French | **Preservation** | Authentic media is retained or classified as preservation input. | Separate graphics/dungeon/runtime ownership and language-specific proof. |
| X68000 | **Unsupported** | Not part of the DM2 support matrix. | None planned in the current target. |

## DM Nexus

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Sega Saturn Japanese | **Source route (blocked)** | Authentic Track 1/DM.BIN, CUE-declared CDDA Track 02–09 BIN ownership, STABG source consumption, NBG1 palette-bank/origin plus raw bitmap/CRAM capture decoding, DGN/DMDF/MNS/PRS3 parsing and bounded phase-launch tests are native. A pointer-only launcher route (game card → Saturn → Original) retains the real CUE/BIN transport and reaches the native title boundary. The public title route remains fail-closed at `title-vdp-capture-required`. | A same-revision title-state capture must bind TITLE.CG, active MAPD span, palette, VDP register/layer state and title consumer/timing before any title/menu/gameplay claim. |
| Saturn demo or fan translations | **Preservation** | Classified separately from the canonical Japanese retail route. | Independent media and runtime proof. |

## Theron's Quest

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC Engine/TurboGrafx-16 Japanese | **Verified route** | Authentic Rev 1 CUE reaches native title, stage, Soul Room and Akutuba runtime (`party=1,0,0`). Its Track 02 source consumer binds all seven campaign dungeons (2,266 source objects; Drator: 8 maps/291 objects); identity and level framing are verified. | Captured transitions, bitmap/palette binding, saves and positive gameplay behavior. |
| PC Engine/TurboGrafx-16 US | **Verified startup route** | Authentic CloneCD ZIP (`.ccd` + bounded `.img` Track 02 slice) starts natively from memory through CLI and start menu; title and scripted Soul Room startup are verified. | US gameplay handoff, captures, saves and later-dungeon proof. |

## Data and preservation rules

Firestaff reads the original file or container in its supplied form. It may
use temporary in-memory decoding during development, but production does not
extract game data to a Firestaff-owned directory. A DOSBox save is never
treated as a Macintosh save, and a parser or synthetic fixture never upgrades
a row to playable. See [game-data setup](DATA_SETUP.md),
[preservation](wiki/Preservation.md) and [verified hashes](VERIFIED_HASHES.md).
