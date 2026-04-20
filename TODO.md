# Firestaff — TODO / Feature Roadmap

**🔥 Projekt-namn ändrat 2026-04-20**: ReDMCSB → **Firestaff** (efter MacGuffin-artefakten i DM1).
Rename av filer/directories sker post-Phase 20 i ett rent cut för att undvika kollision med pågående implementation.

Levande lista. Uppdatera när saker är klara eller nya idéer kommer.

## Pågående / precis klart

- [x] M10 Phase 1-12 — dungeon data layer (klart)
- [x] M10 Phase 13 — combat system (klart, 45 inv)
- [x] M10 Phase 14 — magic system (klart, 45 inv)
- [x] M10 Phase 15 — save/load (klart, 45 inv, CRC32-verifierad)
- [x] M10 Phase 16 — monster AI / CREATURE_TICK (klart, 40 inv, 3 creature-typer fullt impl)
- [ ] M10 Phase 17 — projectile & explosion flight (impl pågår)
- [ ] M10 Phase 18 — champion lifecycle (hunger/status/rest/XP) — plan klar, 48 inv
- [ ] M10 Phase 19 — runtime dynamics (generators/light-decay/fluxcage) — plan klar, 46 inv
- [ ] M10 Phase 20 — tick orchestrator + deterministic harness (plan pågår) — M10-finalen

### Phase 17-20 körordning
- Impl körs sekventiellt p.g.a. verify-script-race (alla appendar till samma fil)
- Planering på Opus 4.6 per Dannes direktiv
- Implementation på Opus 4.7

## Startmeny-spec (från MEMORY.md + 2026-04-20)

### Grundläggande
- [ ] `asset_validator_compat` modul (MD5-baserad)
- [ ] Listning av ALLA spel i DM-serien (DM1, CSB, DM2)
- [ ] Gråmarkera spel som saknar original DAT-filer
- [ ] Popup för sökväg till originalfiler (bara första gången; aldrig mer när satt)
- [ ] Grön ✓ om MD5 matchar, röd ✗ om saknas/wrong-hash
- [ ] Sök via MD5 inte filnamn (tolerant mot översättningar/rename)

### Visuell identitet i spelväljare (2026-04-20)
- [ ] **Förpacknings-/box-art** som primär spelidentifierare i menyn
- [ ] En bild per spel (DM1, CSB, DM2) — iconic 1987/1989/1995 box covers
- [ ] Hi-res scans hittas via:
  - MobyGames (oftast 800x1200+)
  - dmweb.free.fr galleries
  - LaunchBox community resources
- [ ] Gråmarkering: desaturate + grå overlay för saknade spel (se Grundläggande)
- [ ] Hover/selected state: subtle highlight eller glow
- [ ] Multi-version: om spelare har både Atari ST och PC 3.4, visa båda som varianter med liten version-badge
- [ ] Storlek i UI: ~200x300px display, 2x/3x assets för retina/4K
- [ ] Licens: fair use preservation (ingen re-sale), men acceptera DMCA om någon rättighetshavare beklagar sig
- [ ] Fallback: om box-art saknas → generisk swords-and-dungeon-icon med speletiteln

### UI-komponenter (tidigare spec)
- [ ] Språkväljare (en/fr/sv/de minimum)
- [ ] Grafiknivåer: Original / Uppskalad / AI-HD
- [ ] Upplösningar + aspect ratio
- [ ] Fönsterlägen: fullscreen / resizable / fixed / borderless
  - Integer scale / fri skalning / aspect lock
- [ ] Cheats-sektion
- [ ] Credits: FTL Games / Software Heaven, Inc. / Christophe Fontanel
- [ ] "Var hittar jag originalfilerna?" istället för Köp-knapp (inga GOG/Steam-länkar)

### Buggfix-togglar per spel (2026-04-20)

**Full spec**: `tmp/firestaff/BUGFIX_TOGGLE_SPEC.md`

**Källkanon**: http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm

Fontanel har redan dokumenterat **93 buggar + 108 changes** över 9 game-versioner (DM Atari ST 1.0a EN → CSB Atari ST 2.1 EN). Vi måste inte återuppfinna hjulet.

#### Fontanel-struktur
- **Bug-ID**: `BUGX_YY` (X = version som introducerade, YY = löpnummer)
- **Change-ID**: `CHANGEX_YY_KATEGORI`
- **Versioner 0-8**:
  - 0: DM Atari ST 1.0a EN
  - 1: DM Atari ST 1.0b EN
  - 2: DM Atari ST 1.1 EN
  - 3: DM Atari ST 1.2 EN
  - 4: DM Atari ST 1.2 GE
  - 5: DM Atari ST 1.3a FR
  - 6: DM Atari ST 1.3b FR
  - 7: CSB Atari ST 2.0 EN
  - 8: CSB Atari ST 2.1 EN
- **Change-kategorier**: FIX, OPTIMIZATION, LOCALIZATION, IMPROVEMENT
- Varje post har **Affected versions** — exakt vilka versioner som har buggen/changen

#### UI-val (smartare än bara på/av)

**Version presets** (en-klick):
- [ ] "DM 1.0a EN (purist)" — alla 89 1.0a-buggar på, inga changes
- [ ] "DM 1.3b FR (senaste DM)" — alla fixes till 1.3b applicerade, bara kvarvarande buggar kvar
- [ ] "CSB 2.1 EN (senaste CSB)" — allt till 2.1 applicerat
- [ ] "PC 3.4 (vår baseline)" — vad Fontanels WIP-kod faktiskt implementerar
- [ ] "Modern" — alla FIX + utvalda IMPROVEMENT-changes

**Custom mix** (advanced):
- [ ] Individuella togglar per bug/change
- [ ] Gruppering per kategori: Bugs (93) / FIX (~60) / OPTIMIZATION (~20) / LOCALIZATION (~15) / IMPROVEMENT (~30)
- [ ] Varje post visar: ID, symptom, cause, affected versions, resolution
- [ ] Sökfilter: "visa bara buggar som påverkar vald version"

**Gate-logik per spel** (inga meningslösa togglar):
- [ ] DM1-buggar (BUG0-BUG2 + BUG5) syns bara för DM1
- [ ] CSB-specifika (BUG7) syns bara för CSB
- [ ] DM2 får egen lista när vi kommit dit

#### Kritiska exempel-buggar (från dokumentationen)
- **BUG0_02**: 850-1000 timmar → event-time overflow på 24 bits → game hangs. Fixed: none. Phase 12 sak.
- **BUG0_03**: Graphical glitch i mörker vid hög CPU-last (fixad i CSB 2.0 via CHANGE7_01_FIX). Rendering-sak.
- **BUG0_08**: Objects disappear när dungeon thing-pool tar slut. Phase 9-sak. Fixed: none.
- **BUG0_09**: Discard-thing kan trigga sensor → oönskad effekt. Fixed i CSB 2.0 via CHANGE7_17_FIX.
- **BUG0_38** (nämnt tidigare): Cursed items / Luck-collapse.

#### Arkitektur
- [ ] Ladda bug-databasen från YAML/JSON (~200 poster)
  - Schema: `{id, symptoms, cause, affected_versions[], affected_files[], resolution, category, default_state}`
- [ ] Scrapea dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm för initial data
- [ ] Kodsidan: `bug_flags_compat` modul, bit-mask (128+ flaggor, inte bara 64)
- [ ] Per-flag activation check: `if (BUG_ACTIVE(BUG0_02)) {original_behavior} else {fixed_behavior}`
- [ ] Save-filer markerar sin profil — load warnar om profil-mismatch
- [ ] Replay-determinism kräver identiska flaggor (bake into TickStreamRecord world-hash)
- [ ] Changes också togglable (inte bara buggar): "vill jag ha CHANGE3_14_IMPROVEMENT eller ej?"

#### Community-värde
- [ ] Publicera vår profilerade bug-databas som JSON/YAML på GitHub
- [ ] Låter andra DM-portar (scummvm, dmjava) återanvända den
- [ ] Credit Fontanel i datafilen som "Based on ReDMCSB documentation by Christophe Fontanel"

## Feature-förslag (2026-04-20 brainstorm)

### Top-3 priority (hög värde / låg kostnad)

- [ ] **Replay-system via tick-stream** — deterministiskt datalager ger nästan gratis replay. 5 KB-filer, delbara, speedrun-gold. Bygger på Phase 12 (timeline) + Phase 15 (save).
- [ ] **.po-baserat översättningssystem** — extrahera DUNGEON.DAT-texter → .po → Weblate/Transifex → live-reload utan recompile. Perfekt synergi med dagsjobbet.
- [ ] **DM Lore Museum / preservation-sektion** — FTL-historia, Doug Bell/Andy Jaros, scannade manualer, intervjuer, credits. Fungerar också som copyright-alibi ("preservation engine, inte piratering").

### Gameplay-features

- [ ] **Ironman-läge + live speedrun-timer** — toggle. Save-pillars only + real-time/in-game-tick + automatiska splits per level.
- [ ] **Cartographer-läge (3 nivåer)**: Original (paper map) / Magic-map-only / Always-on
- [ ] **In-game item lookup** — Shift+click för stats (originalet kräver memorering)
- [ ] **Crash recovery / autosave** — tyst autosave var 5:e minut i `~/.firestaff/autosave-N.dat`

### Accessibility (baseline 2026)

- [ ] Färgblindfilter (DM har röda/gröna potions)
- [ ] Stor text-läge
- [ ] Dyslexi-font (OpenDyslexic)
- [ ] Fullständig tangentombindning
- [ ] One-handed mode
- [ ] Skärmläsar-integration för menyer

### Community / modding

- [ ] **Portrait/avatar-packs** — champion-porträtt hårdkodade, låt folk byta (eget/AI-genererat). Enkel mod-hook.
- [ ] **Sound-pack-stöd** — DM hade iconic SFX. Community HD/remastered-packs.
- [ ] **Wall/texture-packs** — AI-uppskalade + handgjorda. Utöver inbyggda grafiknivåer.
- [ ] **Custom dungeons** — stöd DM2 Custom Dungeon Editor-format. MD5-baserad detection av original vs custom.
- [ ] **Scripting API (Lua/WASM)** — expose datalagret efter V1. Spell-mods, custom AI, new monsters utan att forka C-kod.

### Avancerat / wildcard

- [ ] **Deterministic speedrun-verification** — upload tick-stream, server kör samma datalager, verifierar hash. Leaderboard utan videos/cheater-rädsla.
- [ ] **Co-op "It Takes Four"** — 4 spelare, 1 champion var, Tailscale/LAN. Experimentellt.
- [ ] **Cross-port challenge runs** — DM1 → CSB → DM2 som EN kontinuerlig run med character carry-over. "DM Triathlon."
- [ ] **"Director mode" debug-verktyg** — se monster-AI tankar live, varför den valde den champion. Underhållning + debug.
- [ ] **Cloud save sync** — frivilligt. Via Signal-protokoll eller egen.
- [ ] **"Beginner hints" tutorial-overlay** — opt-in tips första gången (DM är brutal).

## Deferrade technical items

- [ ] Fontanel-save-filer obfuskering (`Noise[10]`/`Keys[16]`/`Checksums[16]`) — inte implementerad i Phase 15. Behövs ENDAST om interop med original DMSAVE1.DAT ska stödjas.
- [ ] `GLOBAL_DATA.GameID` / `MusicOn` — bärs i `header.reserved[36]` som platshållare, fyll på i audio-fas.
- [ ] `DungeonMutation.fieldMask` semantik — opakt nu, replay-engine tolkar senare.
- [ ] Phase 13 DISASSEMBLY: `F0308_CHAMPION_IsLucky` + cursed-items BUG0_38 — hidden state, Luck-statistiken fick collapse till 0 i v1.
- [ ] Phase 13 DISASSEMBLY: poison-cloud `F0307` vs Vitality — caller applicerar, Phase 16 eller senare.
- [ ] Phase 13 DISASSEMBLY: cell/direction packing på kill — tangerar ACTIVE_GROUP, väntar.
- [ ] Phase 14 DISASSEMBLY: `PowerOrdinalToLightAmount[6]` — extrahera från `GRAPHICS.DAT` entry 562.
- [ ] Phase 14 DISASSEMBLY: `spellPower * 40` STATUS_TIMEOUT-tick-scalar — MEDIA720 init-korrelation.
- [ ] Implementera resterande 24 creature types (Phase 16 v1 har bara Stone Golem, Mummy, Skeleton)

## Plattformsstöd (2026-04-20 krav)

**Mål:** Körbar på **macOS, Linux, Windows** från dag 1 av V1-release.

### Nuvarande arkitektur-styrka
- Alla 20 M10-phases är **ren C, noll OS-specifika anrop**
- Pure data-lager portar sig automatiskt (bit-identisk determinism oavsett plattform)
- Borland LCG RNG är pure integer math — ingen plattformsdrift
- `_pc34_compat` LSB-first serialisering är endian-explicit (måste verifieras på ARM och x86_64)

### Tech stack (V1 rendering/audio/input)
- **SDL3** (eller SDL2 om SDL3 fortfarande är bleeding-edge vid release) — cross-platform de facto standard
- Rendering: SDL3 renderer API (eller OpenGL via SDL3 för grafiknivåer)
- Audio: SDL_mixer för DM:s iconic SFX + BGM
- Input: SDL tangentbord + mus
- Build: **CMake** (plattformsneutralt, integrerar med alla IDE:er)
- Packaging:
  - macOS: `.app` bundle + signed DMG
  - Linux: AppImage + Flatpak + .deb + .rpm
  - Windows: installer via NSIS eller WiX + portabel ZIP

### Filsystems-abstraktion
- [ ] Wrapper-modul `fs_portable_compat` — hanterar path-separators, user-data-dir, config-dir
- [ ] User-data-dir:
  - macOS: `~/Library/Application Support/Firestaff/`
  - Linux: `~/.local/share/firestaff/` (XDG)
  - Windows: `%APPDATA%\Firestaff\`
- [ ] Config-filer: YAML/TOML (platform-neutral format)
- [ ] Save-filer: binary med CRC32 integrity (Phase 15 format)
- [ ] Asset-sök: MD5-validering fungerar lika på alla plattformar

### CI/CD
- [ ] GitHub Actions: bygg på alla tre plattformar vid varje PR
- [ ] Automated tests: kör verify-script på alla tre plattformar
- [ ] Automated release: tagga versioner, bygg paket, ladda upp till GitHub Releases
- [ ] Cross-platform determinism-test: run-twice-hash måste vara IDENTISK på mac/linux/win (kritiskt för replay-features)

### Minimum supported versions (förslag)
- macOS: 11 Big Sur+ (2020, kan tänkas skala ner till 10.15 Catalina)
- Linux: Ubuntu 22.04+, Debian 12+, Fedora 38+ (glibc >= 2.35)
- Windows: 10 64-bit+ (Windows 11 är primary test target)
- Arkitekturer: x86_64 + ARM64 (Apple Silicon, Windows on ARM, Raspberry Pi m.fl.)

### Inte i scope för V1
- iOS / Android (mobil är V3+)
- Web/WASM (kan portas senare via Emscripten)
- Retro-plattformar (Atari, Amiga — Fontanel har redan dem i ReDMCSB)
- Console-plattformar (Switch/Steam Deck via Linux är dock automatiskt)

## Milestones

- **V1 (current):** DM1 PC 3.4 EN kärna, playable loop med save/load, **mac/linux/windows från dag 1**
- **V2:** i18n/l10n (sv/fr/de), startmeny-arkitektur, asset_validator, grafiknivåer, bugfix-togglar
- **V3:** CSB + DM2 integration, cross-port runs, full creature roster, mobil?

## Post-M10 rename (ReDMCSB → Firestaff)

Utförs efter Phase 20 är grön, innan M11 startar:
- [ ] `git mv tmp/firestaff/ firestaff/` (eller flytta till riktig projektrot)
- [ ] Prefix rename: `redmcsb_*` → `firestaff_*` i probe-filer och scripts
- [ ] Rename `run_firestaff_m10_verify.sh` → `run_firestaff_m10_verify.sh`
- [ ] Uppdatera alla PHASEnn_PLAN.md referenser
- [ ] Kör verify-script — måste fortfarande exit 0 med alla 20 phases PASS
- [ ] Suffix `_pc34_compat` STANNAR (beskriver Fontanel-korrekt layout)
- [ ] Funktion-numrering F0xxx STANNAR (Fontanel-arv)
- [ ] Nytt repo-namn på GitHub om/när vi publicerar: `firestaff`
- [ ] Tagline: "An open Dungeon Master engine" (diskoverability utan varumärkesrisk)
