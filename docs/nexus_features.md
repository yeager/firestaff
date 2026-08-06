# Dungeon Master Nexus — verifierade features och öppna gap

Det här dokumentet skiljer mellan riktiga bytes-/formatbevis och features som
fortfarande saknar Saturn-runtime-bindning.

## Verifierade dataformat

- `LEV00.DGN`–`LEV15.DGN`: riktiga DGN-källor; Structure1B-census och
  Structure2-textur-/palettebytes är verifierade på den lokala korpusen.
- `.MNS`: 30 riktiga DMDF-kontainrar och deras TEXT-/modelldeskriptorer är
  inventerade; Structure3-faceägare och VDP1-kommandoordning är inte bevisade.
- `MENU.BPK`: 162 riktiga PRS3-ytor dekoderas med DMWeb-reglerna; CLUT,
  destination och menysemantik saknas.
- `STABG.BIN`, `SMAP00`–`SMAP15`, `ITEM.IBS`, `FACE.BIN` och `STONE.BIN` har
  separata source-owned avkodnings- och palette receipts.
- `SLEV*.BIN`, `SNDLEV*.SAL`/`.MAP` och `SDDRVS.TSK` har bounded byte-/entry-
  receipts; scriptdispatch och ljudhändelser är inte aktiverade.

## Medvetet ej påstådda features

Firestaff påstår inte att en egen software-rasterizer är Saturns VDP1-output,
att viewportens perspektiv, fyra-rutors avstånd, creature-rendering eller
VDP2-composition har parity, eller att CD-audio/SAL kan spelas korrekt. De
nuvarande host-raster- och gameplaymodulerna är no-draw/fail-closed där
source-owned transform, pixel/palette och runtime-consumer saknas.

Inte heller är FMV-dekodning, SRAM-format, controllersemantik, shop-actions,
drops, spells, combat eller textmapping verifierade bara för att motsvarande
filer finns.

## Prioriterade nästa bevis

1. Autentiserad startup/menu-capture: VDP2-lager, CLUT, timing och verklig
   menysekvens.
2. Autentiserad VDP1-capture: DGN Structure3-face, mesh, texture upload och
   command coordinates.
3. Saturn input/HUD-capture: `STABG.BIN` och runtime-state över viewport.
4. SLEV/SAL-capture: event-/actiondispatch och MAP-selectorägare.

Se [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md) och
[NEXUS_RUNTIME_CAPTURE.md](NEXUS_RUNTIME_CAPTURE.md) för den aktuella
admissionsgränsen. DMWeb och Greatstone används som formatreferenser; de
ersätter inte en exekverad Saturn-capture.
