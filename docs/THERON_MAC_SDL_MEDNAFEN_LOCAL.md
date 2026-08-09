# Lokal macOS-runbook: Theron, SDL2 och Mednafen

Det här dokumentet beskriver den lokala Firestaff-miljön på Bosse:s Mac. Det
är en arbetsanteckning för återanvändning, inte en del av speldata och inte ett
påstående om att en capture är semantiskt godkänd.

## Fasta sökvägar på extern-disken

```text
Firestaff-repo:
  /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp

Firestaff-build:
  /Volumes/Extern-disk/firestaff-theron-active2-build

Riktig SDL2-prefix (Cocoa, inte sdl2-compat):
  /Volumes/Extern-disk/theron-sdl2-real-20260809b/install

Ren Mednafen-källtree:
  /Volumes/Extern-disk/theron-mednafen-clean-source-20260809/mednafen

Instrumenterad Mednafen-build:
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809

Instrumenterad binär:
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
```

Riktiga spelmedia och System Card ligger under:

```text
/Users/bosse/.firestaff/data/theron/TQUS02.bin
/Users/bosse/.firestaff/data/theron/TQUS19.iso
/Users/bosse/.mednafen/firmware/syscard3.pce
```

Använd alltid hashverifierade filer från denna lokala datakatalog. Lägg inte
speldata i repot och skapa inte ersättningsmedia.

## Bygg riktig SDL2 en gång

Homebrew-installationen `sdl2` är på den här maskinen `sdl2-compat`. Den får
inte användas som bevis för autentisk Cocoa/Quartz-input. Bygg i stället SDL2
från en officiell SDL2-källarchive till extern-disken:

```bash
SDL_ROOT=/Volumes/Extern-disk/theron-sdl2-real-20260809b
SDL_ARCHIVE=/Volumes/Extern-disk/SDL2-2.30.9.tar.gz

mkdir -p "$SDL_ROOT/source" "$SDL_ROOT/install"
tar -xzf "$SDL_ARCHIVE" -C "$SDL_ROOT/source" --strip-components=1
cd "$SDL_ROOT/source"
./configure --prefix="$SDL_ROOT/install" \
  --disable-video-wayland --disable-video-x11 \
  --disable-video-kmsdrm --disable-video-vulkan \
  --disable-audio-alsa --disable-audio-pulseaudio \
  --disable-audio-jack --disable-audio-pipewire
make -j"$(sysctl -n hw.ncpu)"
make install
```

Verifiera länken innan capture:

```bash
otool -L \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
scripts/verify_theron_mednafen_sdl2_runtime.sh \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
```

Verifieringen ska visa en direktlänk till SDL2-prefixens
`libSDL2-2.0.0.dylib` och får inte visa `sdl2-compat`.

## Bygg instrumenterad Mednafen

Källträdet måste vara rent, eftersom Firestaffs scripts applicerar flera
instrumenteringspatchar. Återanvänd samma rena källtree och välj alltid en
ny, explicit build-root när patchserien ändras:

```bash
cd /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp

FIRESTAFF_MEDNAFEN_SDL2_PREFIX=/Volumes/Extern-disk/theron-sdl2-real-20260809b/install \
FIRESTAFF_MEDNAFEN_BUILD_ROOT=/Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809 \
scripts/build_mednafen_theron_irq2_trace.sh \
  /Volumes/Extern-disk/theron-mednafen-clean-source-20260809/mednafen
```

Byggscriptet kopierar källan, applicerar Firestaffs capturepatchar, bygger
och kör SDL2-länkkontrollen. Mednafen-builden är ett lokalt arbetsartefakt;
den ska inte checkas in.

## Capturelägen

### Headless smoke-test

Dummy-video är användbart för att kontrollera att patcharna och receipt-format
fungerar, men är inte en Quartz/app-capture:

```bash
THERON_STATE="/Users/bosse/.mednafen/mcs/Dungeon Master - Theron's Quest (USA).bee0988239a817f20a64cd38fc8caeac.mc0"

THERON_MEDNAFEN_HOME=/Users/bosse/.mednafen \
MEDNAFEN_BIN=/Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen \
THERON_US_CUE=/tmp/theron-capture-input/TQUS-minimal.cue \
THERON_SYSTEM_CARD=/Users/bosse/.mednafen/firmware/syscard3.pce \
THERON_LIVE_TRACE_OUTPUT=/Volumes/Extern-disk/theron-auth-capture.trace \
THERON_CAPTURE_AUTOLOAD_STATE="$THERON_STATE" \
THERON_CAPTURE_SECONDS=20 \
THERON_CAPTURE_STARTUP_GRACE=5 \
THERON_CAPTURE_SDL_VIDEODRIVER=dummy \
THERON_CAPTURE_SOUND=0 \
scripts/capture_theron_mednafen_live_trace.sh
```

Ett headless-resultat får inte ensam öppna RNG, spawn, AI, T700, T900 eller
grafiksemantik. Sidecars måste ha rätt media-, System Card- och
disassemblyproveniens och klarera respektive verifierare.

### Äkta Cocoa/Quartz-input

Lämna `THERON_CAPTURE_SDL_VIDEODRIVER` tomt eller använd `cocoa`. Mednafen
måste vara förgrundsapp och Terminal/den körande hjälpprocessen måste ha
Accessibility/Input Monitoring-rättigheter i macOS. Capture-scriptet
använder den incheckade Swift/Quartz-hjälparen och kräver PID-bunden fokus.

Exempel på PCE-input via hosten:

```bash
THERON_CAPTURE_SDL_VIDEODRIVER=cocoa \
THERON_CAPTURE_HOST_KEY_SEQUENCE='run@8,i@480,i@900' \
THERON_CAPTURE_HOST_KEY_HOLD=1 \
THERON_CAPTURE_INPUT_ROUTE=pid \
scripts/capture_theron_mednafen_live_trace.sh
```

För ett reproducerbart test utan Quartz kan motsvarande PCE-knappar skickas
via den instrumenterade scripted-input-routen, men det är då en emulatorkörning
och inte bevis för fysisk macOS-input.

## Snabb kontroll före återanvändning

```bash
git -C /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp status --short --branch
otool -L /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen \
  | grep 'libSDL2-2.0.0.dylib'
scripts/verify_theron_mednafen_sdl2_runtime.sh \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
```

Om verifieringen visar `sdl2-compat`, bygg inte om spelsemantiken och kalla
inte resultatet autentiskt. Peka i stället om `FIRESTAFF_MEDNAFEN_SDL2_PREFIX`
till den riktiga SDL2-prefixen ovan och bygg om Mednafen.

## Tsugaru på Mac: FM Towns

Tsugaru gäller Firestaffs FM Towns-spår, inte Theron's Quest på PC Engine.
Theron Track 02 och dess HuC6280/System Card-capture körs med Mednafen ovan.
Tsugaru används för verkliga FM Towns-CD-bilder, originalets TownsOS/TBIOS
och den separata FM Towns-körningen för DM1, CSB och DM2.

Tsugaru använder en katalog med FM Towns-ROM-filer som första argument. Det är
inte samma sak som Firestaffs enskilda `FMT_F20.ROM` för TBIOS-shimmen. Håll
emulator, BIOS och spelmedia på extern-disken och lägg aldrig in dem i Git:

```bash
TOWNS_ROOT=/Volumes/Extern-disk/TOWNSEMU
TSUGARU_GUI="$TOWNS_ROOT/gui/build/main_gui/Tsugaru_GUI.app"
TSUGARU_CUI="$TOWNS_ROOT/gui/build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI"
TOWNS_ROM_DIR=/Volumes/Extern-disk/FirestaffUserData/firmware/fm-towns-rom
TOWNS_DISC=/Volumes/Extern-disk/FirestaffUserData/data/dm1/fmtowns_extract/Dungeon-Master_FM-Towns_JA-EN/track01.iso
```

I den lokala datakatalogen finns även riktiga FM Towns-medier för CSB och DM2;
byt bara `TOWNS_DISC` till respektive `track01.iso` eller `.cue`. En ISO är
lämplig för data-only-test. För en skiva med CD-audio ska originalets kompletta
`.cue`/`.bin` eller `.mds`/`.mdf` användas. Tsugaru stöder ISO, CUE och MDS,
men upstream rekommenderar MDS när ljudspår finns eftersom CUE kan vara
tvetydig kring PREGAP/INDEX 00.

### Bygg Tsugaru från extern-disken

Detta är den verifierade macOS-layouten från Tsugarus upstream-repo. `public`
ska ligga under `gui/src`; bygg inte direkt från repositoryroten:

```bash
git clone https://github.com/captainys/TOWNSEMU.git "$TOWNS_ROOT"
git -C "$TOWNS_ROOT/gui/src" clone https://github.com/captainys/public.git public
cmake -S "$TOWNS_ROOT/gui/src" -B "$TOWNS_ROOT/gui/build"
cmake --build "$TOWNS_ROOT/gui/build" --config Release --parallel

# Tsugarus macOS-GUI behöver CUI-programmet i samma appmiljö.
cp "$TOWNS_ROOT/gui/build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI" \
   "$TOWNS_ROOT/gui/build/main_gui/Tsugaru_GUI.app/Contents/MacOS/"
```

Om källtree eller build redan finns, kör inte `git clone` igen. Kontrollera i
stället att följande filer finns:

```bash
test -x "$TSUGARU_CUI"
test -d "$TSUGARU_GUI"
test -d "$TOWNS_ROM_DIR"
```

### Starta GUI

På macOS startas appbunten `Tsugaru_GUI`. Första gången väljs den riktiga
Towns-ROM-katalogen i Tsugarus inställningar och därefter öppnas discimagen
via File/Open. Starta från Terminal om appbunten inte syns i Finder:

```bash
open "$TSUGARU_GUI"
```

Om macOS stoppar en lokalt byggd app, öppna den en gång med Ctrl-klick → Open
och godkänn den lokala utvecklingsbuilden. Använd inte en nedladdad BIOS- eller
spelersättning för att komma runt problemet.

### Starta CUI reproducerbart

Detta är den användbara kommandoraden för Firestaffs lokala FM Towns-media:

```bash
"$TSUGARU_CUI" \
  "$TOWNS_ROM_DIR" \
  -CD "$TOWNS_DISC" \
  -GAMEPORT0 KEY \
  -SCALE 160
```

Visa alla lokala flaggor med:

```bash
"$TSUGARU_CUI" -HELP
```

`-TOWNSTYPE MARTY` används endast tillsammans med en riktig Marty-ROM och ska
inte sättas för en full FM Towns-ROM. `-CMOS /sökväg/CMOS.BIN` kan läggas till
om en separat CMOS-profil ska sparas. Stäng emulatorn med dess normala Quit-
kommando så att CMOS kan skrivas; tvångsstängning kan lämna den osparad.

### Kontroller

Med `-GAMEPORT0 KEY` använder Tsugaru tangentbordsemulering för gamepad 0:

```text
Riktningar: piltangenter
Action A:   A
Action B:   S
Action C:   Z
Action D:   X
```

Detta är Tsugarus FM Towns-kontroller och ska inte blandas ihop med Firestaffs
Theron-bindning `WASD` + musknapp 1/2. För en fysisk handkontroll används
`PHYS0`–`PHYS3`; om riktningarna rapporteras som analog spak används
`ANA0`–`ANA3` i stället för `KEY`.

### Vad Tsugaru bevisar — och inte bevisar

Tsugaru är rätt verktyg för FM Towns-originalets TownsOS/TBIOS, BIOS-ROM,
keyboard-I/O och native `TMENU.EXP`/`EDM.EXP`-körning. Det bevisar inte
automatiskt Firestaffs egna FM Towns-renderare eller en Theron-semantik.
Firestaffs separata C-brygga är fortfarande fail-closed tills en riktig
Tsugaru-wrapper binder TBIOS-, timing- och I/O-anrop. För källhänvisning och
denna gräns, se `docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md`.

Källan för kommandorad, ROM-katalog, CD-flaggan, kontrollmappningen och CUE-
begränsningarna är Tsugarus officiella README:
<https://github.com/captainys/TOWNSEMU#starting-the-command-line-program>.

### Firestaffs Tsugaru-gräns

Den källbundna FM Towns-dokumentationen finns i
`docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md`. Den beskriver två separata vägar:

1. Tsugaru som subprocess, där de verkliga `TMENU.EXP`/`EDM.EXP`-programmen
   körs genom Tsugarus TownsOS/TBIOS.
2. En minimal, fail-closed TBIOS-shim i Firestaff för verifierade BIOS-glyph-
   och TBIOS-anrop.

Den valfria C-bryggan är deklarerad i
`include/fmtowns_tsugaru_bridge.h` och implementerad i
`src/shared/fmtowns_tsugaru_bridge.c`. Den är inte samma sak som att Tsugaru
redan är länkad i produktionen: utan en separat Tsugaru-wrapper är bryggan
`UNBOUND`, och Firestaff får inte hitta på BIOS-, disk- eller bildresultat.

För Shift-JIS-glyphs räcker den riktiga BIOS-ROM-bundna lokala shimen enligt
den befintliga dokumentationen; en full Tsugaru-wrapper behövs först för
TBIOS-anrop, I/O och faktisk FM Towns-programkörning. Detta påverkar inte
Mednafen-capturekedjan för Theron.

## Senast verifierade Mednafen-körning

2026-08-09 kördes den instrumenterade binären med den riktiga SDL2-prefixen,
hela USA-CUE:n, verifierat US Track 02 (`TQUS02.bin`) och System Card 3.0.
Den bounded scripted replay-körningen gav följande source-bound transport-
receipt:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
raw_sector_spans=161
scsi_read_commands=51
scsi_read_sector_bindings=161
authenticated_cd_ram_receipts=2
main_ram_consumer_reads=4096
vce_palette_snapshot_bytes=1024
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
transition=observed
```

Detta bevisar CD-sektor/FIFO → RAM-transport och en observerad main-RAM-
konsument, men inte vilken nivå-, objekt-, tile- eller creatureägare som läser
bytesen. Körningen får därför inte öppna RNG, spawn, AI, attack, skada, loot,
generatorer, T700 eller T900. Scripted replay är dessutom en
emulatorintern inputväg, inte fysisk macOS-input.

Tracebasen ligger utanför repot på extern-disken:

```text
/Volumes/Extern-disk/theron-auth-capture-full-scripted-20260809.trace
```

Den äldre körningen utan autentiserade CD→RAM-receipts ska inte blandas med
denna körning; capture-sessioner hålls separata.
