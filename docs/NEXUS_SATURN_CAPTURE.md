# DM Nexus: authentic Saturn capture

This is the workflow for creating a source-faithful VDP1/VDP2 capture without
placing BIOS, disc images, or dumped runtime bytes in the repository.

## What can be sent to Mednafen

Yes, but not the entire Firestaff instrumentation as it exists today. The
current chain is a Nexus verification tool and includes Firestaff-specific
environment variables, source tracing, and analysis formats. It is useful for
proving an asset chain, but too large and specialized for an upstream change.

The planned Mednafen PR should therefore be a small, standalone diagnostic
change:

- optional raw VDP1/VDP2 frame capture behind an explicit configuration flag,
- optional VDP2-register, VRAM, and CRAM snapshot after frame rendering,
- deterministic binary format with version signature and endian definition,
- no BIOS, disc, or Firestaff data in the Mednafen source tree,
- no game- or Nexus-specific assumptions in the emulator core.

Writer PC, source registers, CD reading, SLEV/SAL, and Firestaff asset
verification remain in the separate capture layer. They can be used in the PR
description as reproduction evidence, but must not become hard-coded Nexus
logic in Mednafen.

Current status: the generic snapshot portion is extracted as a clean external
patch and passes `git apply --check`. On a clean external checkout,
`make -C src/ss -j2 vdp1.o vdp2.o` also builds with the patch. PR material is
on the external disk at
`/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1-clean/PR_DESCRIPTION.md`.
It has not yet been submitted upstream; format discussion and a full Mednafen
build remain before submission.

## Prerequisites

- A user-owned Saturn BIOS file on external disk. Verify SHA-256 before running.
- A user-owned Nexus CUE/CCD/TOC/M3U container on external disk. Verify
  SHA-256 before running.
- A local Nexus data directory with `TM.BIN`, `FONT256.S2D`, and other verified
  source files.
- An instrumented Mednafen 1.32.1 build on external disk with the patches in
  `scripts/build_mednafen_nexus_saturn_capture.sh`.

The capture script hash-checks BIOS and disc before Mednafen starts. It writes
only the manifest, trace, and raw dump to the specified external directory.

## Build the Mednafen capture tool

```sh
scripts/build_mednafen_nexus_saturn_capture.sh \
  --build-dir /Volumes/Extern-disk/nexus-saturn-capture/mednafen-build \
  --prefix /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix
```

The patch chain instruments:

1. VDP2 write addresses, values, and SH-2 PC.
2. SH-2 registers at the writer PC.
3. Source words from relevant register pointers.
4. Frame ID from the Mednafen capture hook.
5. The SH-2 `PR` register on every VDP2-writer row, enabling the separate
   call-chain check to bind an observed return address to a retail `BSR`.
6. A raw VDP2 snapshot immediately after the actual CRAM write.
7. A frame capture after `VDP2REND_EndFrame()`, so VDP2 registers, VRAM, and
   CRAM describe the frame Mednafen actually rendered.
8. VDP1-VRAM writes with a frame boundary at the same capture hook as VDP2.

When `FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS` is requested, the launcher also
rejects a binary lacking that hook (exit 78). This prevents an older producer
from emitting a trace without the `pr=` field and being incorrectly used as
call-chain proof.

All produced trace files are diagnostic evidence. They must not be used for
semantic admission without verified asset identity, ordering, and same snapshot.

### Frame-bounded VDP1 trace

The build script's VDP1 chain uses the V2 format when the frame patch is installed:

```text
FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2
frame=299
addr=0x63e00 size=2 value=0x.... pc0=0x........ pc1=0x........
frame=300
```

A marker is written at the same vertical-blanking hook as the raw dump's frame
ID, before VDP1 writes for that frame are recorded. Writes after the `frame=300`
marker therefore belong to the VDP1 image captured as frame 300; this is a
transport boundary, not an asset owner. Select a frame with:

```sh
python3 scripts/analyze_nexus_vdp1_write_trace.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run/vdp1-writes.trace \
  --frame 300
```

V1 traces without frame markers remain supported but cannot be selected with
`--frame`. A missing or duplicate marker invalidates the analysis.

### Stable startup witness, 2026-08-11

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
register-, source- och VDP-skrivspår. Om processen timeoutar skrivs dessutom
`capture_termination=timeout` i manifestet. Om processen timeoutar eller
rådumpen inte får sitt capture-magic ska körningen kasseras som
observationsförsök, även om enskilda tracefiler hann skrivas.

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

### Startup witness from frame 0, 2026-08-11

A separate run from reset, with the same hash-verified merged disc and Japanese
BIOS, shows a real startup sequence in the transport layer:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-source-20260811b/`

At frame 100, VDP2 leaves reset state and uses four character layers. From
frame 110, `TVMD=0x8000`, `BGON=0x000f`, `CHCTLA=0x1010`, and `CHCTLB=0x1022`.
At the same time VDP1 changes from idle to five draw records and changes source
positions across subsequent frames. This is verified startup animation, but
not a semantically identified menu or title image.

The more stable 80-frame witness
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-stable-vdp1-window-se2woL/`
also has a complete mode-5 direct-colour draw at frame 0. The draw reads the
VDP1-VRAM span `0x63e00..0x6c000` (33,280 bytes). The span was compared with
all local Nexus files and the English ISO for every frame in the same capture,
both as raw bytes and with 16-bit Saturn byte order restored. No exact match
was found. The capture chain is therefore authentic, but the source-buffer/CD-
read receipt needed to determine whether the span comes from `TITLE.CG`,
`TITLE.BIN`, `MENU.BPK`, or another runtime-decompressed source is still
missing. `source_join=unbound` and `semantic_admission=blocked` therefore
remain correct.

The VDP1 write trace from the same run is invalid as complete write proof: the
validator rejects row 200242 (`addraddr=...`). That row therefore does not
count as a VDP1 write, and the run must not be elevated to semantic admission.
This is a capture/instrumentation error, not a claim about Nexus asset ownership.
The raw frame capture remains useful for the separate capture-only decoder when
the frame boundary and register ordering are validated.

### Corrected input window, 2026-08-11

A new external run used A+START (`0x30`) at SMPC input counter 3500 and
captured 100 frames from capture frame 300:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-menu-input-corrected-20260811a/`

The transport validator accepts the entire run (`frames=100`, all 100 frames
with active VDP1 observation). The same run shows an observed VDP2 state
transition: frame 0 has `BGON=0x000f` with NBG0–NBG3, frame 50 has
`BGON=0x0103` with NBG0/NBG1, and frame 99 has `BGON=0x080c` with NBG2/NBG3.
This is strong input/transport witness evidence from the same retail disc, but
without an exact VDP1/VDP2 source join, `asset_consumer_identity=unbound` and
`host_composition_admission=blocked` remain the correct outcome.

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

### SH-2-producent för VDP2-tilemap, 2026-08-11

En separat J-BIOS/engelsk-disc-körning med riktad WorkRAM-läsning fångade
28 616 läsningar i `0x06013000..0x06014fff` och 200 001 VDP2-skrivningar.
Vid PC `0x0601184c` skrivs VDP2-VRAM från en runtimepekare i `r5`, med första
observerade pekare `0x06013c58` och efterföljande pekare i samma WorkRAM-
område. Detta är den tidigare källhookens verkliga producentväg; `r4` är
VDP2-destinationen och ska inte beskrivas som assetkälla.

De verifierade 16-bytefönstren från WorkRAM matchar ingen unik fil i den
hashverifierade Nexus-korpusen. Resultatet är därför
`vdp2_destination_transport=verified`, `asset_identity=unbound` och
`semantic_admission=blocked`: producenten är identifierad, men den ännu
saknade CD-/dekomprimeringsbindningen hindrar menytext, FONT256 och
produktionskomposition.

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

## Verification

Verify in this order:

1. The validator accepts the correct number of raw frames and capture magic.
2. The writer PC matches the analyzed `TM.BIN` code region.
3. The register trace shows the correct source register and source words.
4. The write and register traces have the same length and ordering.
5. Each write value matches the corresponding big-endian source word in `TM.BIN`.
6. The post-write snapshot's CRAM value matches the same write at the correct
   Saturn address mapping.

Only after step 6 may a VDP2 consumer use the capture slice. If any step is
missing, `semantic_admission=blocked` remains; the result is provenance proof,
not authentic menu, HUD, or viewport rendering.

## Verified SH-2 transform in Firestaff

The observed inner loop is now reproduced as the standalone function
`nexus_v1_saturn_expand_tile_8x48`. It follows the external Mednafen capture:

- `0x060132e0` uses input stride `0x80` and output stride `0x1c0`.
- `0x060135f8` selects eight rows and starts its coefficients from the runtime
  literal pool at `0x0601364c`/`0x06013650`.
- `0x060136c4` selects the table pair from byte 4, reads nibble/pixel data from
  `+16 + pixel*4 + (row>>1)`, masks with `0xf000`, and feeds the MACL result
  through `>>8` and `exts.w`.

The implementation is in `src/nexus/nexus_v1_saturn_tile_transform.c` and is
tested by `nexus_v1_saturn_tile_transform`. It is deliberately not connected to
a menu, HUD, viewport, CLUT, or VDP1 command list. This is therefore a verified
transform step, not another completed Mednafen PR for Nexus.

BIOS, disc images, raw captures, and temporary Mednafen build trees must remain
on external disk and must not be committed.
