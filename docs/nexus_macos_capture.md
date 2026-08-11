# Nexus Saturn-capture på macOS

## Dummy-audio och Cocoa/OpenGL

Vid headless eller tidsbegränsad Saturn-capture kan man prova SDL:s dummy-
ljudbackend i capture-processens miljö:

```sh
SDL_AUDIODRIVER=dummy \
  probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --mednafen /extern/nexus-capture/bin/mednafen \
  --bios /privat/nexus/BIOS.bin \
  --bios-sha256 <sha256> --bios-region eu \
  --disc /privat/nexus/NEXUS.cue \
  --disc-sha256 <sha256> \
  --trace /extern/nexus-capture/run/runtime-vdp12.raw \
  --validator scripts/validate_nexus_saturn_runtime_capture.py \
  --manifest /extern/nexus-capture/run/manifest.txt \
  --timeout-seconds 120
```

Launchern vidarebefordrar `SDL_AUDIODRIVER` explicit till den Mednafen-
process som startas. BIOS, disc-image och capture-byte ska ligga utanför repot
och anges med lokala sökvägar.

`SDL_AUDIODRIVER=dummy` väljer SDL:s dummy-backend om den ljudväg som används
respekterar SDL:s miljövariabel. Det väljer inte macOS Cocoa som videobackend.
På macOS kan Mednafen fortfarande använda SDL:s Cocoa-fönster och
OpenGL-videoväg. Den aktuella Mednafen-körningen måste ändå läsas i loggen:
den rapporterade `Using "SDL" audio driver with SexyAL's default device
selection`, så dummy-ljudets faktiska effekt är inte verifierad i den
instrumenterade Saturn-binära filen.

Inställningen ändrar inte Saturnens CD-DA, SCSP, SAL eller SFX-semantik och
ska inte användas som produktionsljudläge.

## Verifiering

Efter en lyckad körning ska launchern själv validera raw-layouten. En separat
kontroll kan göras med:

```sh
python3 scripts/validate_nexus_saturn_runtime_capture.py \
  /extern/nexus-capture/run/runtime-vdp12.raw --require-frames 1
```

Detta bekräftar endast att raw-filen har rätt Saturn VDP1/VDP2-layout och
minst det begärda antalet frames. En reset-frame är inte automatiskt en
startup-, meny-, HUD- eller viewport-capture. Sådana påståenden kräver fortsatt
autentiserad VDP1/VDP2-komposition och source-/asset-consumer-bindning.

Launchern skriver dessutom `capture_exit_status` samt SHA-256-fält för VDP1-
write-trace och writer-code-trace till manifestet även om VDP2-capturen avbryts.
Det gör ett negativt frame-resultat granskningsbart utan att uppgradera det till
ett raw- eller skärmbevis.

## 2026-08-10: EU cold-start före handoff

En extern capture från frame 0 med EU-BIOS och regionmatchad fransk retail-disc
validerade 60 råa VDP1/VDP2-ramar. Råfilens SHA-256 var
`39e70710bd1b7edeedfb2ec53a1edc0c27546b10f47cf06a6904591c558c66bf`, och
Start injicerades i runtime-ram 45–54. Capturen visar ändringar i VDP1-
framebuffer samt VDP2-register, VRAM och CRAM. Frame 59 identifieras som NBG1
character mode med tre aktiva lager, men `asset_consumer_identity=unbound` och
`host_composition_admission=blocked`. Detta är transportbevis och ett
reproducerbart negativt source-join-resultat; det öppnar inte startup, meny,
HUD eller viewport.

## Fristående VDP1-snapshot

När VDP1-writes når en känd källadress kan den instrumenterade binären även
skriva `FIRESTAFF_NEXUS_VDP1_SNAPSHOT_V1` till fil. Följande miljövariabler
används av den externa launchern:

```sh
FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT=/extern/nexus-capture/run/vdp1-snapshot.raw
FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT=0x10a00
# Valfria proveniensbevis från samma session:
FIRESTAFF_NEXUS_TRACE_VDP1_REGS=/extern/nexus-capture/run/vdp1-regs.trace
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP=/extern/nexus-capture/run/source.dump
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT=0x63e00
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE=0x8200
# Flera möjliga VDP1-konsumenter kan provas i samma session.
FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST=0x0601307c,0x060262c4,0x060262d4

# Riktad SH-2-läslogg för att följa transformen före VDP1-skrivningen:
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS=/extern/nexus-capture/run/vdp1-source-reads.trace
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN=0x06000000
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX=0x08000000
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN=0x06012f00
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX=0x06013100
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT=100000
```

Snapshotten valideras med:

```sh
python3 scripts/validate_nexus_vdp1_snapshot.py \
  /extern/nexus-capture/run/vdp1-snapshot.raw
```

För en snapshot som utlöses av en känd kommandolisteadress kan den bundna
VDP1-payloaden granskas utan att lita på snapshotens transport-COPR:

```sh
python3 scripts/analyze_nexus_vdp1_command_window.py \
  /extern/nexus-capture/run/runtime-vdp12.raw \
  --capture-frames 1 --command-offset 0x47c0 --command-count 8 --require-end
```

`--command-offset` är endast till för en adress som observerats i samma
runtime-session. Verktyget beskriver VDP1-kommandon och källbytes-hashar, men
godkänner inte startup, meny, HUD, viewport, CLUT eller asset-ägarskap.

`VDP1_REGS` och `VDP1_SOURCE_DUMP` vidarebefordras nu av launchern och får
manifest-hash i samma session. De är avsedda för den separata
source-to-VRAM-kontrollen; en source-dump utan motsvarande register-, frame-
och retail-bytesreceipt öppnar inte någon konsument-gate.

Den validerade körningen på extern disk gav VDP1-state `ptmr=0x02`,
`edsr=0x03`, en 1 048 577-byte VDP1-payload och writer-code vid `0x10a00` i
samma session. Detta uppnår VDP1-transportbeviset. Snapshotten tas dock vid
den första matchande källskrivningen; den är därför inte i sig bevis på en
komplett draw-lista, CLUT-bindning eller startup-/meny-/HUD-/viewport-
komposition.

En senare samma-session-snapshot efter observerad adress `0x0485c` gav fem
polygon-/texturposter följda av ett VDP1-end-record vid `0x04860`; den
transportbundna kommandosekvensen kan därför granskas, men saknar fortfarande
VDP2-frame-hook och konsumentbindning för skärmidentitet.

Den verifierade macOS-observationen är därför:

- `SDL_AUDIODRIVER=dummy` vidarebefordras reproducerbart till extern Mednafen-
  capture, men dess faktiska SexyAL-effekt måste verifieras i loggen,
- Cocoa/OpenGL-videovägen är fortfarande separat från ljudinställningen,
- aktiv VDP1-draw-lista och semantisk meny/HUD/viewport-admission är fortsatt
  capture-gated tills samma runtime-session binder dessa konsumenter.
