# Dungeon Master Nexus — championstatus

## Källbunden roster

Den riktiga europeiska `RLOWFIX.BIN`-filen innehåller en `RES*`-directory och
en `PLRD`-resource med 20 poster à 64 byte. Firestaff läser de numeriska
fält, klass-/nivåfält, portraitordinals och equipmentord som faktiskt ligger i
PLRD. `FACE.BIN` binds separat till 20 riktiga portraitrecords.

Den tidigare åttamannatabellen och den 24-poster stora hårdkodade rosterlistan
är inte Nexus-källa. De får endast användas av uttryckliga äldre fixture-tester.
24 är lagringskapacitet, inte verifierat antal retail-champions.

## Namn och text

PLRD pekar på `TABL`-poster. Firestaff behåller både index och rå 16-bitars
glyph-koder, men ingen Saturn TEXT/FONT256-konsument har ännu bevisat hur de
blir synlig text. Därför publicerar production inte namn som ASCII, Shift-JIS,
katakana eller svenska översättningar.

## Vad som inte får ärvas från DM1/DM2

DM1-/DM2-källor får inte fylla i Nexus-statistik, class semantics, combat,
spellkostnader, XP, drops, item-use, food/water, alignment eller resurrection.
Sådana routes är blockerade eller endast fixture-isolerade tills en Nexus-
disassembly/capture binder dem. En PLRD-byte får inte ensam ges en DM1-
betydelse.

## Källor

- DMWebs Nexus file formats och `DMNDataFileDecoder.vbs`-strukturer.
- `src/nexus/nexus_v1_champions.c` och
  `src/nexus/nexus_v1_rlowfix_text.c`.
- `tests/test_nexus_v1_champion_plrd.c` mot den riktiga RLOWFIX-korpusen.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
