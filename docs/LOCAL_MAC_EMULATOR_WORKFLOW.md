# Lokal Mac-workflow för Firestaff, Mednafen och Tsugaru

Det här är arbetsflödet för den externa utvecklingsdisken. Det är en lokal
referens, inte ett krav på att någon emulator eller spelmedia ska checkas in.

## Firestaff och SDL3

På den här Macen finns SDL3 via Homebrew. Bygg från den externa worktree:n:

```sh
brew install sdl3
cmake -S /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp \
  -B /Volumes/Extern-disk/firestaff-theron-active2-build \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /Volumes/Extern-disk/firestaff-theron-active2-build --parallel
```

För headless verifiering används SDL:s dummy-videodriver:

```sh
SDL_VIDEODRIVER=dummy \
  /Volumes/Extern-disk/firestaff-theron-active2-build/firestaff_m11_phase_a_probe
```

Theron i Firestaff använder `WASD` för rörelse. Musknapp 1 är Button I och
musknapp 2 är Button II. På touch är kort tryck Button I och långt tryck
(500 ms) Button II. Touch ska gå genom samma Firestaff-inputroute som musen;
den får inte skapa en separat spelmekanik.

## Mednafen på macOS

Mednafen är ett terminalprogram. Den lokala installationen ligger normalt i
`/opt/homebrew/bin/mednafen`. Starta en verifierad PC Engine-CD-bild genom att
ange CUE-filen som argument:

```sh
/opt/homebrew/bin/mednafen \
  "/Volumes/Extern-disk/theron-mednafen-us/Dungeon Master - Theron's Quest (USA).cue"
```

System Card läggs i Mednafen-profilen utanför repot, till exempel
`~/.mednafen/firmware/syscard3.pce`. Den får aldrig kopieras till Firestaffs
worktree, läggas i `.firestaff/data` i repot eller pushas till GitHub.

Mednafen har ingen egen launcher-GUI på macOS. Under körning öppnas Player 1:s
inputmapping med `Alt+Shift+1` och Player 2:s med `Alt+Shift+2`. Följ alla
prompter i ordning, även turbo- och extra-layoutfälten. Firestaffs egna
WASD/mus/touch-bindningar är däremot en del av SDL-inputvägen och ska inte
förväxlas med Mednafen-mappningen.

För en lokal Theron-capture används den riktiga CUE/BIN-källan och System Card
från deras externa platser. Trace-, screenshot- och debugutdata ska också
ligga på extern disk eller i `/tmp`, aldrig i Git-index.

## Tsugaru på macOS

Tsugaru är FM TOWNS/Marty-emulatorn. Den officiella macOS-vägen är GUI-appens
`Tsugaru_GUI.app`; CUI-binären används av GUI:t och ska ligga kvar i samma
distribution enligt Tsugarus dokumentation.

Lokal installation och körning:

```sh
open "/path/to/Tsugaru_GUI.app"
```

Om den byggs lokalt från den officiella källan:

```sh
git clone https://github.com/captainys/TOWNSEMU.git /Volumes/Extern-disk/TOWNSEMU
cd /Volumes/Extern-disk/TOWNSEMU/gui/src
git clone https://github.com/captainys/public.git
cd ..
cmake -S src -B build
cmake --build build --config Release
cp build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI \
   build/main_gui/Tsugaru_GUI.app/Contents/MacOS/
open build/main_gui/Tsugaru_GUI.app
```

Tsugaru kan läsa ISO, CUE och MDS, men källans dokumentation rekommenderar
MDS/MDF för CD-bilder med ljud och varnar för tvetydig PREGAP-tolkning i CUE.
För Firestaffs källtrohet ska därför format, tracklayout och hash dokumenteras
innan data används. Tsugarus ROM/firmware ska ligga i dess egna lokala profil,
aldrig i Firestaffs repository.

## Repository-spärr

Kontrollera före commit och push:

```sh
git status --short
bash scripts/verify_no_original_media_tracked.sh
```

Originala BIN/CUE/ISO/BIOS/System Card-filer, dumpade ROM:ar och stora
emulatordatafiler får inte finnas i Git-index. Endast källkod, metadata,
hashar, receipts, tester och verifierade skärmdumpar utan mediepayload får
publiceras.

Källor: [Tsugaru README](https://github.com/captainys/TOWNSEMU) och
[Mednafen-dokumentationen](https://mednafen.github.io/documentation/).
