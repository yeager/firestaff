# Dungeon Master Nexus — översikt med evidensgränser

Dungeon Master Nexus är ett Sega Saturn-spel med riktiga DGN-nivåer,
DMDF/MNS-modeller, PRS3/UI-media, Saturn-fontdata, per-level SLEV/SAL/MAP-
filer och CD-medier. Firestaffs Nexus-arbete är en källbunden bring-up; det är
inte bevisat att en host-rasterizer eller DM1-härledd gameplaymodell har parity
med originalets SH-2/VDP1/VDP2-runtime.

## Verifierad lokal korpus

Den användartillhandahållna europeiska engelska ISO:n och de lösa filerna i
`.firestaff/data/nexus` är hashverifierade. Korpusen omfattar 137 manifest-
medlemmar: 131 lösa filer och sex autentiserade ISO-medlemmar. Den blandade
runtime-läsaren använder lösa verifierade filer först och ISO:n endast som
kompletterande källa för saknade exakta medlemmar.

## Källstatus

| Område | Verifierat | Kvarstår |
|---|---|---|
| Uppstart | TITLE/MAPD, LOGOBG, WARNING/GAMEOVER/STABG, FACE och FONT-bytes | Saturn VDP2-lager, timing, textmapping och destination |
| Meny | MENU.BPK/BPPK, 162 PRS3-ytor och pixelavkodning | CLUT, VDP1-upload, placering och menysekvens |
| HUD | STABG-tiles/palette och DM.BIN-hitrects | input-/VDP-konsument och runtime-state-bindning |
| Viewport | DGN Structure1B/2 och MNS/DMDF-census | Structure3-faceägare, mesh-transform, VDP1 command/texture use |
| Runtime | SLEV/SAL/MAP bounded receipts | event-/actiondispatch och ljudselector |

## Källor

- DMWeb Nexus file-format pages och DMN Data File Decoder.
- Greatstone för DM-formatjämförelser och byteformatreferenser.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
- [NEXUS_RUNTIME_CAPTURE.md](NEXUS_RUNTIME_CAPTURE.md).

Alla obevisade routes är fail-closed. Ingen syntetisk titel, roster, HUD,
viewporttextur, ljudsample eller gameplaysemantik får maskera ett saknat
Saturn-bevis.
