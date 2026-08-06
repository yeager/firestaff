# Nexus – audit av äldre påståenden

Detta är en dokumentationsinventering, inte en ny runtime-specifikation. Den
aktuella statusen finns i
[`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).
Äldre dokument får inte användas som bevis för att en Nexus-route är
implementerad eller källbunden.

## Påståenden som är spärrade

| Dokumentfamilj | Äldre påstående | Korrekt status från retailkorpusen |
|---|---|---|
| `nexus_content.md`, `nexus_overview.md` | DM1-härledd roster, stats och combat är verifierad Nexus-runtime | PLRD har 20 riktiga records; namn-, stats- och actionkonsumenter är inte bundna |
| `nexus_champions.md`, `nexus_creatures.md` | Hårdkodad roster, creature-stats och portraitindex är spelkällan | Hårdkodade tabeller är fixture-/diagnostic-only; PLRD/FACE är source receipts |
| `nexus_data.md`, `nexus_graphics.md` | DGN/MNS-geometri och texturer kan ritas av host-rasterizern | Structure1B/2/3 och MNS är formatbevis; transform, palette, VDP1 och command-order saknas |
| `nexus_intro.md`, `nexus_title.md` | Host-rasterizern visar den ursprungliga titeln | TITLE/LOGOBG/WARNING är avkodade ytor; VDP2-lager, timing och placering saknas |
| `nexus_hud.md`, `nexus_inventory.md` | Generiska HUD-/inventorywidgets är Nexus presentation | STABG/ITEM/FACE-data är receipts; Saturns HUD- och actionkonsument är inte fångad |
| `nexus_text.md`, `nexus_language.md` | FONT256/SJIS-konvertering räcker för synlig text | RLOWFIX TEXT4/TABL och FONT012 är bundna som bytes; glyph consumer/page/attribute/VDP2 saknas |
| `nexus_audio_format.md`, `nexus_music.md`, `nexus_sound.md`, `nexus_sfx.md` | CD-DA/SAL/MAP kan spelas genom hostantaganden eller att nivåpar automatiskt väljer spår | CDDA-spår 2–9 och SAL/MAP är metadata; nivå→CDDA, SLEV/SDDRVS event→selector och playback är capture-gated |
| `nexus_input.md`, `nexus_sensors.md`, `nexus_squares.md` | DM1-lik input-, sensor- och square-semantik gäller Nexus | Endast bounded source records är upptagna; Saturn dispatch och state writes är obevisade |
| `nexus_phase2_data_formats_H2321.md`, `nexus_test_coverage.md` | Historiska test-/stubmatriser beskriver dagens produktion | Äldre plan-/coverage-dokument överstyr inte aktuell CMake-exkludering och no-draw-gates |

## Produktionsregeln

Ingen text, palette, portrait, HUD-widget, item, mesh, viewport-pixel eller
ljudhändelse får lämna diagnostic-/receipt-lagret bara för att ett äldre
dokument kallar den “implemented”. Promotion kräver source-owned byteidentitet
samt autentiserad Saturn/Mednafen-capture av konsument, destination och timing.
Saknad capture betyder blank/no-draw eller no-op, inte syntetisk ersättning.

DMWeb och Greatstone används för byte- och formatregler. De bevisar inte i sig
VDP1/VDP2-komposition eller runtime-semantik.

Se även `TODO.md`: `NEXUS-SATURN-PRESENTATION-HANDOFF`,
`NEXUS-MENU-SEQUENCE-CAPTURE` och `NEXUS-HUD-SATURN-DISPATCH-CAPTURE`.
