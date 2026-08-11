# DM Nexus: autentisk Saturn-capture

Detta är arbetsflödet för att skapa en källtrogen VDP1/VDP2-capture utan att
lägga BIOS, disc-image eller dumpade runtime-bytes i repot.

## Vad som kan skickas till Mednafen

Ja, men inte hela Firestaff-instrumenteringen som den ser ut i dag. Den
nuvarande kedjan är ett verifieringsverktyg för Nexus och innehåller
Firestaff-specifika miljövariabler, källspårning och analysformat. Det är bra
för att bevisa en assetkedja, men för stort och för specialiserat som en
upstream-ändring.

Den planerade Mednafen-PR:n ska därför vara en liten, fristående
diagnostikförändring:

- valfri rå VDP1/VDP2-frame-capture bakom en tydlig konfigurationsflagga,
- valfri VDP2-register-, VRAM- och CRAM-snapshot efter frame-renderingen,
- deterministiskt binärformat med versionssignatur och endian-definition,
- ingen BIOS-, disc- eller Firestaff-data i Mednafen-källträdet,
- inga spel- eller Nexus-specifika antaganden i emulatorkärnan.

Writer-PC, source-register, CD-läsning, SLEV/SAL och Firestaffs
assetverifiering hör kvar i det separata capturelagret. De kan användas i
PR-beskrivningen som reproduktionsbevis, men ska inte bli hårdkodad
Nexus-logik i Mednafen.

Nuvarande status är alltså: den generiska snapshotdelen är extraherad som en
ren extern patch och passerar `git apply --check`. På en ren extern checkout
bygger dessutom `make -C src/ss -j2 vdp1.o vdp2.o` med patchen. Ett PR-underlag
finns på extern disk i
`/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1-clean/PR_DESCRIPTION.md`.
Den är fortfarande inte inskickad upstream; formatdiskussion och full
Mednafen-byggning återstår före submission.

## Förutsättningar

- En användarägd Saturn BIOS-fil på extern disk. Kontrollera SHA-256 före körning.
- En användarägd Nexus CUE/CCD/TOC/M3U-container på extern disk. Kontrollera
  SHA-256 före körning.
- En lokal Nexus-datakatalog med `TM.BIN`, `FONT256.S2D` och övriga verifierade
  källfiler.
- En instrumenterad Mednafen 1.32.1 byggd på extern disk med patcharna i
  `scripts/build_mednafen_nexus_saturn_capture.sh`.

Capture-scriptet hashkontrollerar BIOS och disc innan Mednafen startas. Det
skriver bara manifest, trace och rådump till den angivna externa katalogen.

## Bygg Mednafen-captureverktyget

```sh
scripts/build_mednafen_nexus_saturn_capture.sh \
  --build-dir /Volumes/Extern-disk/nexus-saturn-capture/mednafen-build \
  --prefix /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix
```

Patchkedjan instrumenterar:

1. VDP2-write-adresser, värden och SH-2-PC.
2. SH-2-register vid writer-PC:n.
3. källord från relevanta registerpekare.
4. frame-id från Mednafen-capture-hooken.
5. en rå VDP2-snapshot direkt efter den faktiska CRAM-skrivningen.
6. en frame-capture efter `VDP2REND_EndFrame()`, så att VDP2-register, VRAM
   och CRAM beskriver den frame som Mednafen faktiskt renderade.
7. VDP1-VRAM-skrivningar med en framegräns vid samma capture-hook som VDP2.

Alla producerade tracefiler är diagnostiska bevis. De får inte användas för
semantic admission utan att asset-identitet, ordningsföljd och samma snapshot
är verifierade.

### VDP1-trace med framegräns

Byggscriptets VDP1-kedja använder V2-formatet när framepatchen är installerad:

```text
FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2
frame=299
addr=0x63e00 size=2 value=0x.... pc0=0x........ pc1=0x........
frame=300
```

En markör skrivs vid samma vertikal-blanking-hook som rådumpens frame-id,
innan VDP1-skrivningarna för den framen registreras. Skrivningar efter
`frame=300`-markören tillhör därför den VDP1-bild som fångas som frame 300;
detta är en transportgräns, inte en assetägare.
Välj en frame med:

```sh
python3 scripts/analyze_nexus_vdp1_write_trace.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run/vdp1-writes.trace \
  --frame 300
```

V1-traces utan frame-markörer stöds fortfarande, men kan inte väljas med
`--frame`. Saknad eller duplicerad markör gör analysen ogiltig.

### Stabil startup-witness, 2026-08-11

Den externa J-BIOS/English-capture-körningen
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-stable-vdp1-window-se2woL/`
validerar 80 frames efter en 1 200-frame boot-window. Frame 0 har en komplett
VDP1-kedja med fyra poster: system clip `(319,223)`, local coordinate `(0,0)`;
en mode-5 direct-colour draw och END. VDP1-framebufferten ändras över tiden,
och en extern framebuffer-dekodning visar den Saturn-renderade Victor-start-
animationen. Rådumpens SHA-256 är
`49b0e2cfa3d0394fda966ca40f0adc3bc36475f298b4fa743188d3bec1c999f1`.

Detta bevisar en autentisk VDP1 startup-frame och timing, men inte vilken
retailfil som äger mode-5-källan, inte PRS3/FONT256/MENU-konsumtion och inte
M12-produktionsrendering. `semantic_admission` förblir därför `blocked`.

VDP1-V2-state har två operatörsvarianter i omlopp. Den aktuella patchen
skriver även `sysclipx` och `sysclipy`; äldre externa Mednafen-buildar skriver
samma V2-state utan de två suffixfälten. Firestaffs transportvalidator
accepterar båda varianterna, men ingen av dem ger i sig assetägarskap eller
produktionsrendering.

## Så görs själva dumpningen

Körningen sker i denna ordning. Sökvägarna pekar medvetet på extern disk.

1. Packa upp BIOS lokalt på den externa disken och beräkna SHA-256. BIOS-filen
   kopieras inte till repot.
2. Kontrollera SHA-256 för Nexus CUE och tillhörande binärfiler. Starta inte
   en capture om identiteten saknas.
3. Bygg den patchade Mednafen-kopian i en separat katalog på extern disk.
4. Skapa en ny körkatalog och sätt miljövariablerna för rådump, registerspår,
   VDP2-write-spår och post-write-snapshot.
   För VDP1-källproveniens kan samma körning dessutom rikta registerprovet
   mot flera SH-2-PC:n med
   `FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST=0x0601307c,0x060262c4`.
   Det är viktigt att PC-listan, source-dumpen och rådumpen produceras av
   samma process; separata körningar får inte fogas ihop som ett bevis.
   När VDP1-skrivaren anropar en transform kan den aktuella SH-2-koden och
   registerläget fångas med
   `FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE_AT=0x06012f4a`.
5. Starta Saturn-profilen genom
   `firestaff_nexus_v1_saturn_raw_capture_launcher.sh`. Launchern validerar
   BIOS och disc, startar Mednafen, väntar tills körningen är klar och skriver
   manifestet.
6. Kör analysverktygen mot exakt samma körkatalog. Rådumpen, spåren,
   snapshoten och manifestet ska ha samma sessionsnamn.
7. Behandla resultatet som `blocked` tills både skrivordning och källbytes
   identitet är verifierade. En tekniskt giltig VDP2-snapshot är inte i sig
   bevis för att bytesen är menytext, FONT256 eller HUD.

Varje receipt ska därför innehålla BIOS- och disc-hash, sessionsnamn,
framefönster, rådumpens layout samt SHA-256 för varje faktiskt producerat
register-, source- och VDP-skrivspår. Om processen timeoutar eller rådumpen
inte får sitt capture-magic ska körningen kasseras som observationsförsök,
även om enskilda tracefiler hann skrivas.

### Runtime-transformen före VDP1

SH-2-kodkvittot från samma externa Mednafen-gren visar att VDP1-kedjan inte är
en direkt PRS3-kopia. Rutinen vid `0x060132e0` anropas med 18 iterationer,
läser med `0x80` bytes stride och skriver med `0x1c0` bytes stride. Den anropar
pixelrutinen vid `0x060135f8`, som gör åtta pass. Den innersta rutinen vid
`0x060136c4` hämtar packade bytes från tile-input, applicerar `0xf000`-masken,
justerar nibblepositionen och skriver 16-bitars output med två runtimevärden
som koefficienter. Detta är ett transform-/tile-expansionssteg efter asset-
dekodningen. Det får inte ersättas med en host-side PRS3-blit utan samma
input-, koefficient- och CLUT-bevis.

Koefficientkvittot från en autentisk körning visade initialt `r10=0x04bc` och
`r9=0x0a70`. Därefter uppdaterades literalpoolen med signerade 16-bitarsvärden
från SH-2-kedjan. Spåret är därför en del av samma-sessionens proveniens, men
är inte i sig ett bevis på vilken meny-, HUD- eller viewport-asset som valdes.

### Verifierad extern körning 2026-08-11

En autentisk engelskspråkig data-track-körning gjordes på extern disk med den
hashverifierade japanska Saturn-BIOS:en
(`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`).
Original-CUE:n refererade till ljudspår som saknades lokalt. Originalet
ändrades inte: ISO:n kopierades byte för byte till extern disk och en separat
data-track-only-CUE skapades där.

ISO-hash:
`16786e6165d8cbf7f6394dd9bc7171fbb561c1ba40b77ad7cba3c275fde2804e`.
Härledd CUE-hash:
`f3575af985cadbecc74edda0c51451ffeea775054ec5fcdd7c4f960dcdc0cc17`.
Körkatalog:
`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-english-source-20260811c/`.

Körningen producerade 600 frames. Validatorn fann förändringar i VDP1
VRAM/framebuffers samt VDP2 register/VRAM/CRAM. En kompletterande SH-2
source-write-körning gav 500 000 begränsade rader, men ingen komplett
contiguous ISO-chunk som binder VDP1-konsumenten till `MENU.BPK`, `DGN` eller
`DM.BIN`.

Resultatet är därför fortfarande uttryckligen
`semantic_admission=blocked`: capturekedjan är verifierad som observation,
men den bevisar ännu inte menytext, HUD, viewport, PRS3-palettägare eller
DGN-faceägare. Den härledda CUE:n får inte presenteras som en komplett
retail-disc med ljudspår.

### Startup-witness från frame 0, 2026-08-11

En separat körning från reset, med samma hashverifierade merged-disc och
japanska BIOS, visar en verklig uppstartssekvens i transportlagret:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-source-20260811b/`

Vid frame 100 lämnar VDP2 resetläget och använder fyra character layers.
Från frame 110 är `TVMD=0x8000`, `BGON=0x000f`, `CHCTLA=0x1010` och
`CHCTLB=0x1022`. Samtidigt går VDP1 från idle till fem draw records och
ändrar källpositioner över efterföljande frames. Detta är verifierad
startup-animation, men inte en semantiskt identifierad meny eller titelbild.

Den stabilare 80-frame-witnessen
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-stable-vdp1-window-se2woL/`
har dessutom en komplett mode-5 direct-colour draw vid frame 0. Drawen läser
VDP1-VRAM-spannet `0x63e00..0x6c000` (33 280 bytes). Spannet jämfördes mot
alla lokala Nexus-filer och den engelska ISO:n för varje frame i samma
capture, både som råa bytes och med 16-bitars Saturn-byteordning återställd.
Ingen exakt träff hittades. Capturekedjan är alltså autentisk, men vi saknar
fortfarande source-buffer-/CD-läsningsreceipten som krävs för att säga om
spannet kommer från `TITLE.CG`, `TITLE.BIN`, `MENU.BPK` eller en annan
runtime-dekomprimerad källa. `source_join=unbound` och
`semantic_admission=blocked` är därför fortsatt korrekt.

VDP1-write-spåret från samma körning är ogiltigt som komplett skrivbevis:
validatorn avvisar rad 200242 (`addraddr=...`). Den raden räknas därför inte
som en VDP1-write och körningen får inte höjas till semantic admission.
Detta är ett capture-/instrumenteringsfel, inte ett påstående om Nexus'
assetägare. Den råa frame-capturen är fortfarande användbar för den
separata, capture-only-dekodern när framegräns och registerordning valideras.

### Korrigerat inputfönster, 2026-08-11

En ny extern körning använde A+START (`0x30`) vid SMPC-input-räknare 3500 och
fångade 100 frames från capture-frame 300:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-menu-input-corrected-20260811a/`

Transportvalidatorn godkänner hela körningen (`frames=100`, alla 100 frames
med aktiv VDP1-observation). Samma körning visar en observerad VDP2-state-
övergång: frame 0 har `BGON=0x000f` med NBG0–NBG3, frame 50 har `BGON=0x0103`
med NBG0/NBG1, och frame 99 har `BGON=0x080c` med NBG2/NBG3. Det är ett starkt
input-/transportwitness från samma retail-disc, men utan exakt VDP1/VDP2-
source join är `asset_consumer_identity=unbound` och
`host_composition_admission=blocked` fortfarande det korrekta resultatet.

Frame 50 har därefter jämförts mot hela den hashverifierade Nexus-korpusen.
VDP1 mode-5-källan (`source_offset=0x10a00`, 2048 bytes) har ingen exakt
MENU.BPK-, MNS-, DGN- eller retail-filträff. VDP2 character-lane har 0/4
exakta FONT256 Page/Character Generator/Palette-spaner och 0/1 exakt
palette-CRAM-match; en attributspan matchar, men det räcker inte för att
identifiera textkonsumenten. Resultatet är därför fortsatt
`source_join=unbound` och `text_consumer_identity=unbound`.

Minimal extern körning:

```sh
run=/Volumes/Extern-disk/nexus-saturn-capture/run-menu-$(date +%Y%m%d-%H%M%S)
mkdir -p "$run"
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="$run/vdp2-writes.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="$run/post.snapshot"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=64

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 301 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/nexus-saturn-capture/mednafen-home \
  --bios /Volumes/Extern-disk/nexus-saturn-capture/bios-j/Sega\ Saturn\ BIOS\ \(J\)\ \(1.01\).bin \
  --bios-sha256 <verifierad_sha256> --bios-region jp \
  --disc "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.cue" \
  --disc-sha256 <verifierad_sha256> \
  --trace "$run/runtime-vdp12.raw" \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest "$run/manifest.txt" \
  --trace-session nexus-vdp2-dump
```

Det viktiga är inte en viss frame-adress, utan att hela beviskedjan kommer
från samma körning. Mednafen-delen fångar emulatorns observerade tillstånd;
Firestaff-delen avgör därefter om tillståndet kan bindas till en känd källa.

## Starta en riktad capture

Exempelvärdena nedan är platshållare; använd egna, hashverifierade sökvägar:

```sh
export FIRESTAFF_NEXUS_TRACE_VDP2_REGS=/Volumes/Extern-disk/run/vdp2-writer-regs.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES=/Volumes/Extern-disk/run/vdp2-writes.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT=/Volumes/Extern-disk/run/post.snapshot
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=64

# För den byggda Mednafen-diagnostiken som dumpar källbytesfält från
# FirestaffTraceVdp2Registers måste register-hooken dessutom få ett eget
# PC- och adressintervall. VDP2-VRAM ligger i intervallet 0x00000–0x3ffff.
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC=0x06011860
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX=0x40000
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT=20000

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 301 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/.../prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/.../mednafen-home \
  --bios /Volumes/Extern-disk/.../Sega-Saturn-BIOS.bin \
  --bios-sha256 <sha256> --bios-region jp \
  --disc /Volumes/Extern-disk/.../Dungeon-Master-Nexus.cue \
  --disc-sha256 <sha256> \
  --trace /Volumes/Extern-disk/run/runtime-vdp12.raw \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest /Volumes/Extern-disk/run/manifest.txt \
  --trace-session nexus-vdp2-source-join
```

När `FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT` är tomt sparas alla CRAM-
writes från den valda PC:n upp till `..._LIMIT`. Varje post-write-record består
av en textrad med `frame`, `area` och `addr`, följd av:

```text
RawRegs (0x200 bytes)
VRAM    (0x80000 bytes)
CRAM    (0x1000 bytes)
```

Snapshoten tas efter CRAM-tabellens skrivning, inte vid senare frame capture.
Det är viktigt eftersom en vanlig frame-snapshot annars kan visa att samma
CRAM-adress senare har skrivits över.

Frame-hooken ligger däremot efter `VDP2REND_EndFrame()`. Det är avsiktligt:
VDP2-registret `BGON`, tilemap/bitmap-läget i `CHCTLA`, name table, character
generator och CRAM ska läsas efter renderkonsumenten har gjort sin frame-
uppdatering. Byggscriptet applicerar detta som den separata patchen
`scripts/mednafen_1.32.1_nexus_capture_post_render.patch`.

En komplett dumpning på extern disk ser därför ut så här:

```sh
run=/Volumes/Extern-disk/nexus-saturn-capture/run-menu-$(date +%Y%m%d-%H%M%S)
mkdir -p "$run"
export FIRESTAFF_NEXUS_TRACE_VDP2_REGS="$run/vdp2-writer-regs.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC=0x06011860
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX=0x40000
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS=/Volumes/Extern-disk/run/vdp2-source-reads.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX=0x80000
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN=0x06002fc4
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX=0x06002fc6
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES=/Volumes/Extern-disk/run/scu-dma-writes.trace
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN=0x05e00000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX=0x05f00000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="$run/vdp2-writes.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="$run/post.snapshot"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=80
export FIRESTAFF_NEXUS_TRACE_CD_READS="$run/cd-reads.trace"
export FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA=1
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES="$run/sh2-source-writes.trace"
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN=0x06000000
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX=0x08000000

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 600 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/nexus-saturn-capture/mednafen-home \
  --bios /Volumes/Extern-disk/nexus-saturn-capture/bios-j/Sega\ Saturn\ BIOS\ \(J\)\ \(1.01\).bin \
  --bios-sha256 <verifierad_sha256> --bios-region jp \
  --disc "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.cue" \
  --disc-sha256 <verifierad_sha256> \
  --trace "$run/runtime-vdp12.raw" \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest "$run/manifest.txt" \
  --trace-session nexus-vdp2-post-render
```

### SCU-DMA-resultat från autentisk startup/menu-körning

Den 11 augusti 2026 kördes samma hashverifierade BIOS- och merged-disc-
session med SCU-DMA-hooken först filtrerad mot `0x05e00000..0x05efffff` och
sedan utan adressfilter. Den filtrerade körningen gav noll träffar. Den
ofilterade 600-frame-körningen gav 984 130 DMA-skrivningar i den fångade
begränsningen; de observerade destinationerna låg i `0x05c0xxxx..0x05c7xxxx`,
alltså VDP1/registerkedjan, inte i VDP2:s `0x05e...`-fönster.

Detta är ett verifierat negativt resultat: SCU-DMA-hooken fungerar och fångar
faktiska Saturn-skrivningar, men den binder ännu inte menytext, FONT256 eller
VDP2-tilemap till en DMA-källa. VDP2-källan måste därför fortsatt sökas i den
CPU-/SH-2-skrivkedja eller annan bussväg som faktiskt används av Nexus. Ingen
semantic admission eller produktionsrendering får öppnas på grundval av denna
DMA-capture ensam.

Körningarna finns endast på extern disk:

`run-codex-scu-dma-source12-20260811/` (filtrerad, tom trace) och
`run-codex-scu-dma-all-source14-20260811/` (ofilterad trace).

Firestaffs transporttest `test_nexus_v1_saturn_runtime_capture` accepterar
även det generiska `MDFN_SS_SATURN_RUNTIME_CAPTURE_V1`-formatet och verifierar
big-endian VDP1/VDP2-ord samt att semantic admission förblir spärrad.

Verifiera sedan samma session, inte en annan frame eller en annan disc:

```sh
python3 scripts/analyze_nexus_vdp2_composition.py \
  "$run/runtime-vdp12.raw" --frame 300 --capture-frames 600 --require-layer NBG1
python3 scripts/analyze_nexus_vdp2_char_source_join.py \
  "$run/runtime-vdp12.raw" --data-dir /Users/bosse/.firestaff/data/nexus \
  --frame 300 --capture-frames 600 \
  --vdp2-write-trace "$run/vdp2-writes.trace"
python3 scripts/analyze_nexus_vdp2_post_write_snapshot.py \
  "$run/post.snapshot" --data-dir /Users/bosse/.firestaff/data/nexus \
  --asset TM.BIN --source-file-offset 0x1a0c0 \
  --destination-start 0x100400 --minimum-writes 64
```

För att följa skivdata till SH-2-buffer används dessutom `cd-reads.trace` och
`sh2-source-writes.trace`. `FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA=1` filtrerar
bort BIOS/boot-sektorns upprepade LBA 0-läsningar. Source-tracen loggar både
byte-, ord- och långordsaccesser; tidigare versioner loggade bara långord och
missade därför den bytevisa kopieringen till runtimebufferten.

De här spåren bevisar transportkedjan först när en icke-noll LBA, den aktiva
CS2-läsningen och motsvarande destination kan sammanfogas i samma körning.
Enbart en `0x05890008`-läsning eller en matchande runtime-adress är inte
tillräckligt för källbindning.

Ett positivt transporttest för den verifierade engelska ISO:n kan exempelvis
kräva en sammanhängande `DM.BIN`-kopia:

```sh
python3 scripts/analyze_nexus_sh2_source_trace.py \
  "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.iso" \
  "$run/sh2-source-writes.trace" \
  --require-member DM.BIN \
  --require-destination-range 0x06090000:0x060a0000 \
  --require-pc 0x000002b4
```

Detta är ett transportbevis, inte ett bevis på att `DM.BIN` är FONT256,
menytext eller VDP2-konsument. Den senare klassningen måste fortfarande göras
mot VDP2:s aktiva source-register, tilemap och CRAM i samma capture.

En lyckad transportkontroll räcker inte för semantic admission. Om
`FONT256`-spannen, textkod→glyph-mappningen eller den faktiska menyägaren inte
är bundna ska verktygen uttryckligen lämna `source_join=unbound` eller
`semantic_admission=blocked`. Det hindrar en autentisk hårdvarudump från att
bli en obestyrkt host-rendering.

## Verifiering

Verifiera i denna ordning:

1. validatorn godkänner rätt antal råframes och rätt capture-magic.
2. writer-PC:n matchar den analyserade `TM.BIN`-kodregionen.
3. registerspåret visar rätt source-register och source words.
4. write-spåret och registerspåret har samma längd och ordning.
5. varje write-värde matchar motsvarande big-endian source word i `TM.BIN`.
6. post-write-snapshotens CRAM-värde matchar samma write vid rätt Saturn-
   adressmappning.

Först efter punkt 6 får en VDP2-konsument använda capture-slicen. Saknas någon
punkt förblir `semantic_admission=blocked`; då är resultatet ett provenance-
bevis, inte en autentisk meny-, HUD- eller viewport-rendering.

## Verifierad SH-2-transform i Firestaff

Den observerade innerloopen är nu reproducerad som den fristående funktionen
`nexus_v1_saturn_expand_tile_8x48`. Den följer den externa Mednafen-capturen:

- `0x060132e0` använder indata-steg `0x80` och utdata-steg `0x1c0`.
- `0x060135f8` väljer åtta rader och startar koefficienterna från runtime-
  literalpoolen vid `0x0601364c`/`0x06013650`.
- `0x060136c4` väljer tabellparet från byte 4, läser nibble/pixel-data från
  `+16 + pixel*4 + (row>>1)`, maskerar med `0xf000` och matar MACL-resultatet
  genom `>>8` och `exts.w`.

Implementeringen finns i `src/nexus/nexus_v1_saturn_tile_transform.c` och
testas av `nexus_v1_saturn_tile_transform`. Den är medvetet inte kopplad till
en meny, HUD, viewport, CLUT eller VDP1-command-lista. Därför är detta ett
verifierat transformsteg och inte ännu ett färdigt Mednafen-PR för Nexus.

BIOS, disc-images, råcaptures och temporära Mednafen-buildträd ska ligga på
extern disk och får inte commitas.
