# Firestaff, TODO / Feature Roadmap

**🔥 Project renamed on 2026-04-20**: ReDMCSB → **Firestaff** (after the MacGuffin artifact in DM1).
The file and directory rename was done after Phase 20 in a clean cut to avoid collisions with in-flight implementation work.

Living list. Update when things are completed or when new ideas appear.

## In progress / just completed

- [x] M10 Phase 1-12, dungeon data layer (complete)
- [x] M10 Phase 13, combat system (complete, 45 invariants)
- [x] M10 Phase 14, magic system (complete, 45 invariants)
- [x] M10 Phase 15, save/load (complete, 45 invariants, CRC32 verified)
- [x] M10 Phase 16, monster AI / CREATURE_TICK (complete, 40 invariants, 3 creature types fully implemented)
- [ ] M10 Phase 17, projectile and explosion flight (implementation in progress)
- [ ] M10 Phase 18, champion lifecycle (hunger, status, rest, XP), plan complete, 48 invariants
- [ ] M10 Phase 19, runtime dynamics (generators, light decay, fluxcage), plan complete, 46 invariants
- [ ] M10 Phase 20, tick orchestrator plus deterministic harness (plan in progress), the M10 finale

### Phase 17-20 execution order
- Implementation runs sequentially because of verify-script race risk (all phases append to the same file)
- Planning uses Opus 4.6 per Daniel's directive
- Implementation uses Opus 4.7

## Start menu spec (from MEMORY.md + 2026-04-20)

### Core
- [ ] `asset_validator_compat` module (MD5-based)
- [ ] List **all** games in the DM series (DM1, CSB, DM2)
- [ ] Grey out games whose original DAT files are missing
- [ ] Prompt for the original file path only the first time, never again once configured
- [ ] Green ✓ when MD5 matches, red ✗ when missing or wrong hash
- [ ] Identify assets via MD5, not filename (robust against translations and renames)

### Visual identity in the game picker (2026-04-20)
- [ ] Packaging / box art as the primary visual identifier in the menu
- [ ] One image per game (DM1, CSB, DM2), iconic 1987/1989/1995 box covers
- [ ] High-resolution scans can be sourced from:
  - MobyGames (often 800x1200+)
  - dmweb.free.fr galleries
  - LaunchBox community resources
- [ ] Greyed-out state: desaturate plus grey overlay for unavailable games
- [ ] Hover / selected state: subtle highlight or glow
- [ ] Multi-version handling: if the player owns both Atari ST and PC 3.4, show both as variants with a small version badge
- [ ] UI display size: around 200x300 px, with 2x/3x assets for Retina / 4K
- [ ] Licensing stance: fair-use preservation, no resale, but comply with any future DMCA complaint if needed
- [ ] Fallback: if box art is missing, show a generic swords-and-dungeon icon with the game title

### UI components
- [ ] Language picker (minimum: en/fr/sv/de)
- [ ] Graphics levels: Original / Upscaled / AI-HD
- [ ] Resolutions and aspect ratio settings
- [ ] Window modes: fullscreen / resizable / fixed / borderless
  - Integer scale / free scaling / aspect lock
- [ ] Cheats section
- [ ] Credits: FTL Games / Software Heaven, Inc. / Christophe Fontanel
- [ ] "Where can I find the original files?" instead of a Buy button (no GOG / Steam links)

### Bug-fix toggles per game (2026-04-20)

**Full spec**: `tmp/firestaff/BUGFIX_TOGGLE_SPEC.md`

**Canonical source**: http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm

Fontanel has already documented **93 bugs + 108 changes** across 9 game versions (DM Atari ST 1.0a EN → CSB Atari ST 2.1 EN). There is no reason to reinvent that wheel.

#### Fontanel structure
- **Bug ID**: `BUGX_YY` (X = introducing version, YY = running number)
- **Change ID**: `CHANGEX_YY_CATEGORY`
- **Versions 0-8**:
  - 0: DM Atari ST 1.0a EN
  - 1: DM Atari ST 1.0b EN
  - 2: DM Atari ST 1.1 EN
  - 3: DM Atari ST 1.2 EN
  - 4: DM Atari ST 1.2 GE
  - 5: DM Atari ST 1.3a FR
  - 6: DM Atari ST 1.3b FR
  - 7: CSB Atari ST 2.0 EN
  - 8: CSB Atari ST 2.1 EN
- **Change categories**: FIX, OPTIMIZATION, LOCALIZATION, IMPROVEMENT
- Every entry has **Affected versions**, exactly listing which versions contain the bug or change

#### UI model, smarter than a simple on/off switch

**Version presets** (one-click):
- [ ] "DM 1.0a EN (purist)", all 1.0a bugs enabled, no changes
- [ ] "DM 1.3b FR (latest DM)", all fixes through 1.3b applied, only remaining bugs left
- [ ] "CSB 2.1 EN (latest CSB)", everything through 2.1 applied
- [ ] "PC 3.4 (our baseline)", whatever Fontanel's WIP code actually implements
- [ ] "Modern", all FIX changes plus a curated set of IMPROVEMENT changes

**Custom mix** (advanced):
- [ ] Individual toggles per bug / change
- [ ] Grouping by category: Bugs (93) / FIX (~60) / OPTIMIZATION (~20) / LOCALIZATION (~15) / IMPROVEMENT (~30)
- [ ] Each entry shows ID, symptom, cause, affected versions, resolution
- [ ] Search filter: "show only bugs affecting the selected version"

**Per-game gate logic** (no meaningless toggles):
- [ ] DM1 bugs (BUG0-BUG2 + BUG5) shown only for DM1
- [ ] CSB-specific bugs (BUG7) shown only for CSB
- [ ] DM2 gets its own list once we reach that milestone

#### Important example bugs from the documentation
- **BUG0_02**: 850-1000 hours, event-time overflow on 24 bits, game hangs. Fixed: none. Phase 12 topic.
- **BUG0_03**: graphical glitch in darkness under high CPU load (fixed in CSB 2.0 via CHANGE7_01_FIX). Rendering topic.
- **BUG0_08**: objects disappear when the dungeon thing pool is exhausted. Phase 9 topic. Fixed: none.
- **BUG0_09**: discard-thing can trigger a sensor, causing unwanted side effects. Fixed in CSB 2.0 via CHANGE7_17_FIX.
- **BUG0_38**: cursed items / luck collapse.

#### Architecture
- [ ] Load the bug database from YAML / JSON (~200 entries)
  - Schema: `{id, symptoms, cause, affected_versions[], affected_files[], resolution, category, default_state}`
- [ ] Scrape dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm for initial data
- [ ] Code side: `bug_flags_compat` module, bit-mask (128+ flags, not just 64)
- [ ] Per-flag activation check: `if (BUG_ACTIVE(BUG0_02)) { original_behavior } else { fixed_behavior }`
- [ ] Save files store their profile, loading warns on profile mismatch
- [ ] Replay determinism requires identical flags, bake them into the TickStreamRecord world hash
- [ ] Changes must also be toggleable, not just bugs

#### Community value
- [ ] Publish the profiled bug database as JSON / YAML on GitHub
- [ ] Let other DM ports (ScummVM, dmjava) reuse it
- [ ] Credit Fontanel in the data file as "Based on ReDMCSB documentation by Christophe Fontanel"

## Feature ideas (2026-04-20 brainstorm)

### Top-3 priority (high value / low cost)
- [ ] **Replay system via tick-stream**, deterministic data layer makes replay almost free. 5 KB files, shareable, speedrun gold. Builds on Phase 12 (timeline) + Phase 15 (save).
- [ ] **.po-based translation system**, extract DUNGEON.DAT texts to .po, feed Weblate / Transifex, live reload without recompilation. Perfect synergy with localization work.
- [ ] **DM Lore Museum / preservation section**, FTL history, Doug Bell / Andy Jaros, scanned manuals, interviews, credits. Also works as preservation framing instead of piracy framing.

### Gameplay features
- [ ] **Ironman mode + live speedrun timer**, save pillars only plus real-time / in-game ticks plus automatic splits per level
- [ ] **Cartographer mode (3 levels)**: Original (paper map) / Magic-map-only / Always-on
- [ ] **In-game item lookup**, Shift+click for stats
- [ ] **Crash recovery / autosave**, silent autosave every 5 minutes in `~/.firestaff/autosave-N.dat`

### Accessibility
- [ ] Color-blind filters (DM has red / green potions)
- [ ] Large-text mode
- [ ] Dyslexia font (OpenDyslexic)
- [ ] Full key rebinding
- [ ] One-handed mode
- [ ] Screen-reader integration for menus

### Community / modding
- [ ] **Portrait / avatar packs**, champion portraits are hard-coded, let people swap them out
- [ ] **Sound-pack support**, community HD / remastered SFX packs
- [ ] **Wall / texture packs**, AI-upscaled plus handcrafted, beyond the built-in graphics levels
- [ ] **Custom dungeons**, support DM2 Custom Dungeon Editor format, with MD5-based detection of original vs custom
- [ ] **Scripting API (Lua / WASM)**, expose the data layer after V1 so people can add spell mods, custom AI, and new monsters without forking the C code

### Advanced / wildcard
- [ ] **Deterministic speedrun verification**, upload a tick stream, server reruns the data layer and verifies the hash, leaderboard without video dependence or cheat drama
- [ ] **Co-op, It Takes Four**, 4 players, 1 champion each, Tailscale / LAN, experimental
- [ ] **Cross-port challenge runs**, DM1 → CSB → DM2 as one continuous run with character carry-over, the "DM Triathlon"
- [ ] **Director mode debug tools**, show monster AI reasoning live, both useful and entertaining
- [ ] **Cloud save sync**, optional, via Signal protocol or a custom service
- [ ] **Beginner hints tutorial overlay**, opt-in tips for first-time players

## Deferred technical items

- [ ] Fontanel save-file obfuscation (`Noise[10]` / `Keys[16]` / `Checksums[16]`) not implemented in Phase 15. Needed only if interoperability with original `DMSAVE1.DAT` should be supported.
- [ ] `GLOBAL_DATA.GameID` / `MusicOn` carried in `header.reserved[36]` as placeholders, fill them in during the audio phase.
- [ ] `DungeonMutation.fieldMask` semantics still opaque, replay engine can interpret later.
- [ ] Phase 13 DISASSEMBLY: `F0308_CHAMPION_IsLucky` plus cursed-items BUG0_38, hidden state, Luck collapsed to 0 in v1.
- [ ] Phase 13 DISASSEMBLY: poison-cloud `F0307` vs Vitality, caller applies it, likely Phase 16 or later.
- [ ] Phase 13 DISASSEMBLY: cell / direction packing on kill, touches ACTIVE_GROUP, deferred.
- [ ] Phase 14 DISASSEMBLY: `PowerOrdinalToLightAmount[6]`, extract from `GRAPHICS.DAT` entry 562.
- [ ] Phase 14 DISASSEMBLY: `spellPower * 40` STATUS_TIMEOUT tick scalar, correlated with MEDIA720 init.
- [ ] Implement the remaining 24 creature types (Phase 16 v1 only has Stone Golem, Mummy, Skeleton)

## Platform support (2026-04-20 requirement)

**Goal:** playable on **macOS, Linux, and Windows** from day 1 of the V1 release.

### Current architectural strengths
- All 20 M10 phases are **pure C with zero OS-specific calls**
- The pure data layer ports naturally, with bit-identical determinism regardless of platform
- The Borland LCG RNG is pure integer math, no platform drift
- `_pc34_compat` LSB-first serialization is endianness-explicit and must be verified on ARM and x86_64

### Tech stack (V1 rendering / audio / input)
- **SDL3** (or SDL2 if SDL3 is still too bleeding-edge at release time), the de facto cross-platform standard
- Rendering: SDL3 renderer API, or OpenGL via SDL3 for graphics levels
- Audio: SDL_mixer for DM's iconic SFX + BGM
- Input: SDL keyboard + mouse
- Build: **CMake** (platform-neutral, IDE-friendly)
- Packaging:
  - macOS: `.app` bundle + signed DMG
  - Linux: AppImage + Flatpak + `.deb` + `.rpm`
  - Windows: installer via NSIS or WiX + portable ZIP

### Filesystem abstraction
- [ ] Wrapper module `fs_portable_compat`, handles path separators, user-data dir, config dir
- [ ] User-data dir:
  - macOS: `~/Library/Application Support/Firestaff/`
  - Linux: `~/.local/share/firestaff/` (XDG)
  - Windows: `%APPDATA%\Firestaff\`
- [ ] Config files: YAML / TOML (platform-neutral format)
- [ ] Save files: binary with CRC32 integrity (Phase 15 format)
- [ ] Asset search: MD5 validation works identically on all platforms

### CI / CD
- [ ] GitHub Actions, build on all three platforms for every PR
- [ ] Automated tests, run verify scripts on all three platforms
- [ ] Automated release, tag versions, build packages, upload to GitHub Releases
- [ ] Cross-platform determinism test, run-twice hash must be identical on macOS / Linux / Windows

### Minimum supported versions (proposal)
- macOS: 11 Big Sur+
- Linux: Ubuntu 22.04+, Debian 12+, Fedora 38+ (glibc >= 2.35)
- Windows: 10 64-bit+
- Architectures: x86_64 + ARM64 (Apple Silicon, Windows on ARM, Raspberry Pi, etc.)

### Out of scope for V1
- iOS / Android
- Web / WASM
- Retro platforms (Atari, Amiga, Fontanel already has them in ReDMCSB)
- Console platforms (though Steam Deck via Linux comes mostly for free)

## Milestones

- **V1 (current):** DM1 PC 3.4 EN core, playable loop with save/load, **macOS / Linux / Windows from day 1**
- **V2:** i18n / l10n (sv/fr/de), start-menu architecture, asset validator, graphics levels, bug-fix toggles
- **V3:** CSB + DM2 integration, cross-port runs, full creature roster, mobile maybe

## Post-M10 rename (ReDMCSB → Firestaff)

Executed after Phase 20 turned green, before M11 started:
- [x] rename work tree to Firestaff
- [x] rename script and probe prefixes to `firestaff_*`
- [x] keep `_pc34_compat` suffixes
- [x] keep F0xxx function numbering
- [x] publish under the GitHub repo name `firestaff`
- [x] keep the tagline direction: "An open Dungeon Master engine"
- [x] re-run verification after rename with identical semantic output
