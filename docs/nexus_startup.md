# Dungeon Master Nexus — uppstartsstatus

Detta är en evidensrapport för uppstarten, inte en specifikation av en färdig
Nexus-startskärm.

## Verifierad initieringsordning

`nexus_v1_init()` väljer först ett hashverifierat extracted-korpus när `DM.BIN`
och `LEV00.DGN` finns som riktiga lösa filer. En giltig samlokaliserad retail-
ISO kan därefter läsas som kompletterande källa för exakta ISO-medlemmar. Om
endast ISO-data finns används ISO-läsaren direkt.

Efter källval försöker motorn läsa riktiga `RLOWFIX.BIN`, startup-ytor,
`FACE.BIN`, `FONT256.S2D`, ljudmetadata och övriga källfiler. Saknade eller
obundna konsumenter får inte ersättas med syntetiska ytor.

## Verifierade startup-resurser

| Resurs | Status |
|---|---|
| `TITLE.CG` + `TITLE.BIN` | MAPD/TIBG-avkodning och fem retailbilder verifierade för den kanoniska profilen och den dokumenterade engelska revisionen; VDP2-placering saknas |
| `LOGOBG.DG2` | PP-pixlar och 256-entry BGR555-palette verifierade; lager/timing saknas |
| `WARNING.BIN`, `GAMEOVER.BIN`, `STABG.BIN` | riktiga ytor avkodas och får bytesproveniens; presentation saknas |
| `FACE.BIN` | 20 verkliga PRS3-porträtt, 56×56, med källpaletter; VDP1-destination saknas |
| `FONT256.S2D` | 242 CG-tiles; Saturns page/attribute/glyph-mapping saknas |
| `MENU.BPK` | 162 PRS3-ytor avkodas; menyordning, CLUT och VDP1/VDP2 saknas |

## Runtimegräns

`firestaff_nexus` får inte presentera en hostbyggd titel, championmeny,
portraitplacering eller textfooter som om den vore Saturn-output. M11-handoff
och viewport förblir fail-closed tills en autentiserad Saturn-capture binder
resurs, palett, destination och timing.

## Källor

- DMWeb Nexus file formats och DMN Data File Decoder.
- `src/nexus/nexus_v1_engine.c` och `src/nexus/nexus_v1_title.c`.
- `docs/NEXUS_RUNTIME_CAPTURE.md`.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
