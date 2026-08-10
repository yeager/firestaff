# DM Nexus: autentisk Saturn-capture

Detta är arbetsflödet för att skapa en källtrogen VDP1/VDP2-capture utan att
lägga BIOS, disc-image eller dumpade runtime-bytes i repot.

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

Alla producerade tracefiler är diagnostiska bevis. De får inte användas för
semantic admission utan att asset-identitet, ordningsföljd och samma snapshot
är verifierade.

## Starta en riktad capture

Exempelvärdena nedan är platshållare; använd egna, hashverifierade sökvägar:

```sh
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS=/Volumes/Extern-disk/run/vdp2-writer-regs.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES=/Volumes/Extern-disk/run/vdp2-writes.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT=/Volumes/Extern-disk/run/post.snapshot
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=64

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
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS="$run/vdp2-writer-regs.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="$run/vdp2-writes.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="$run/post.snapshot"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=80

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

BIOS, disc-images, råcaptures och temporära Mednafen-buildträd ska ligga på
extern disk och får inte commitas.
