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

Den verifierade macOS-observationen är därför:

- `SDL_AUDIODRIVER=dummy` vidarebefordras reproducerbart till extern Mednafen-
  capture, men dess faktiska SexyAL-effekt måste verifieras i loggen,
- Cocoa/OpenGL-videovägen är fortfarande separat från ljudinställningen,
- aktiv VDP1-draw-lista och semantisk meny/HUD/viewport-admission är fortsatt
  capture-gated tills samma runtime-session binder dessa konsumenter.
