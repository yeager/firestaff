# Theron's Quest verklig mediarevision (2026-08-08)

Den autentiserade Track 02-intaget använder de riktiga filerna i
`~/.firestaff/data/theron/` när de finns:

- US: `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`
- JP: `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a`

Revisionen bekräftar följande:

- US- och JP-rosterbytes kan läsas från originalets codonström.
- Regionala 4bpp-palettkandidater avkodas som riktiga HuC6260-ord. US- och
  JP-offseten hålls separata.
- Track 02:s startup-bitmapkandidater kan dekomprimeras till riktiga
  indexerade bytes, men deras VDC-destination, palettägare och semantiska
  startup-rutter är inte bundna av en körningscapture.
- Den statiska VCE-konsumentspannen i `$96a5` är verifierad, men dess dynamiska
  källa (`$27c4/$27c5`) är ännu inte sammanfogad med palettfönstret eller en
  VDC-skärm.
- Produktionsgrinden lämnar därför `startup_presentation_allowed` och
  palette promotion avstängda. Det är avsiktligt: riktiga bytes får inte
  förvandlas till uppfunnen spelbetydelse.
- Inga autentiserade `vram_dungeon.bin`/`vce_dungeon.bin`-snapshots finns i
  den delade data- eller workspace-roten. Den första Theron-bilden kan därför
  inte publiceras i README ännu.

Verifiering:

```text
theron_v1_vram_trace_loader                 PASS
theron_v1_track02_palette_window            PASS
theron_v1_startup_media_palette_bind       PASS
```

Nästa källbundna steg är en körning med originalets System Card, instrumenterad
Mednafen och samma Track 02-media. Capture-receiptet måste binda FIFO/RAM,
VCE/VDC-destination och skärmroute innan produktionen får visa bilden.
