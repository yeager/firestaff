# Nexus placeholder- och provenance-audit

Datum: 2026-08-08

Detta är en källtroget inventering av vad som är retail-data, vad som är
isolerad testdata och vad som fortfarande saknar autentisk Saturn-
konsumentcapture. En parser som kan läsa bytes är inte i sig ett bevis för
VDP1-/VDP2-placering, CLUT, HUD, viewport, ljud eller gameplay-semantik.

## Verifierad retail-data

- `DM.BIN`, `TM.BIN`, `FACE.BIN`, `FONT256.S2D`, `ITEM.IBS`, `MENU.BPK`,
  `LEV00.DGN`–`LEV15.DGN`, `SLEV00.BIN`–`SLEV15.BIN`,
  `SNDLEV##.SAL/.MAP` och `SDDRVS.TSK` läses från den operatorägda
  Nexus-katalogen och hashkontrolleras innan de används i realdata-prober.
- `ITEM.IBS` verifieras med 243 deklarationer, 223 regularbilder och 109
  golvbilder (`nexus_v1_item_ibs`).
- `DM.BIN` HUD-layouten verifieras som 80 poster och hitrect-tabellen som 40
  poster. Detta är källaägd geometri, inte bevis för Saturns slutliga
  komposition.
- Alla 16 DGN-nivåer har en source-bound Structure3-face-kampanj med 18 478
  no-draw-targets. Kampanjens ledger kräver fortfarande original-Saturn-
  capture och tillåter ingen decoder eller renderer.

## Isolerade syntetiska/testvägar

- BPX0/BPX3-kontrakten i `nexus_v1_bpx_bpk.c` används bara i explicit
  probe-/testmål; CMake exkluderar filen från Nexus-produktionskällorna.
- DGN-materialproben använder en lokal syntetisk pose. Den ligger inte längre
  i `include/nexus_v1_game.h`; koordinaterna finns endast i
  `tests/test_nexus_v1_dgn_material_raster.c`.
- Fixture-dekodrar för S2D-text, ljus, PRS3-kontrakt och legacy-mekanik är
  fortsatt uttryckliga testbanor. De får inte leverera M11-pixlar eller
  Nexus-runtime-state.

## Spärrade no-op-/fallbackgränser

- `nexus_v1_drops.c` fabricerar ingen DM1-formad loot- eller guldtabell.
- `nexus_v1_item_use.c` ändrar inte inventory eller champion-state; ITEM.IBS
  bevisar deklarationer/ikoner/material men inte Saturns action-dispatch.
- `nexus_v1_title_sequence.c` innehåller host-planeringstider, men M11:s
  title-/warning-yta kräver autentisk capture. Timingmetadata får inte
  presenteras som retail-animation.
- `nexus_v1_sound.c` dekodar inte SAL till host-PCM. SLEV-dispatch,
  MAP-event, SAL-format, SDDRVS-handoff och playback kräver en gemensam
  Saturn/SCSP/68K-exekveringscapture.
- DGN Structure3, ITEM/VDP1-texturer, CLUT, HUD/viewport-komposition och
  startup/menu-presentering förblir no-draw eller capture-gated.

## Runtime-captureläge

Den externa katalogen innehåller 37 validerade `runtime-vdp12.raw`-filer.
Inventeringen räknar 11 reset/no-layer-, 14 RBG0-, 100 NBG1- och 14 övriga
aktiva VDP2-frames. Alla har `asset_consumer_identity=unbound` och
`startup_menu_hud_viewport_identity=unbound`.

De autentiska aktiva VDP1-witnesses bevisar Saturn-hårdvarustate och
command-to-VRAM-korridorer, men deras source-span har ännu ingen exakt
bindning till retail-MNS, DGN, ITEM, MENU, TITLE eller CLUT. Reset-capture
bevisar inte startup eller meny.

`writer-code.trace` och `vdp1-writes.trace` är dessutom separata, externa
diagnostikartefakter. De får inte kopplas till en `runtime-vdp12.raw` från en
annan körning bara för att PC, VRAM-adress eller byteprefix råkar sammanfalla.
Det aktuella writer-kvittot (`PC=0x06013098`, `VRAM=0x47c00`) har därför
fortsatt `runtime_code_source_identity=unbound`; samma-körningsidentitet,
relokerad/dekomprimerad kodägare och retail-asset måste först visas i ett
gemensamt capturepaket.

## Verifieringskommandon

```sh
FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus \
  ctest --test-dir build --output-on-failure -R \
  'nexus_v1_(startup_menu_source|title_mapd_real|dgn_geometry_readiness|dgn_face_mesh_corpus|startup_media_gate|slev_task_corpus_receipt|item_ibs|sal_map_corpus)$'

python3 tools/verify_nexus_production_source_boundary.py
python3 scripts/analyze_nexus_capture_inventory.py \
  /Volumes/Extern-disk/nexus-saturn-capture
```

Så länge en required consumer-capture saknas är den källtroget korrekta
åtgärden att behålla grinden stängd, inte att ersätta den med syntetiska
pixlar, guessed timing, DM1-loot eller host-PCM.
