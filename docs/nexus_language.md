# Dungeon Master Nexus — språk- och textstatus

Det här dokumentet är en källtroget korrigerad statusrapport. Den tidigare
versionen påstod japansk-only, Shift-JIS-namn och färdig text-rendering utan
att det var bevisat för den lokala korpusen.

## Verifierat i den lokala korpusen

- Den europeiska engelska ISO:n innehåller `DMN_ABS.TXT`, `DMN_BIB.TXT` och
  `DMN_CPY.TXT`; deras bytesidentitet verifieras av
  `scripts/verify_nexus_v1_asset_manifest.py`.
- `RLOWFIX.BIN` innehåller en verifierad `PLRD`-resource med 20 poster à 64
  byte. Varje post behåller sex `TABL`-index och de råa glyph-koderna från
  `TABL`.
- `FONT256.S2D` ger 242 verifierade 8×8-byte CG-tiles från riktiga SCR-
  regioner.

## Inte verifierat

`TABL`-koderna är inte ännu bundna till ett Saturn-teckensnitt, en sida,
attributtabell eller en VDP2-textkonsument. Firestaff får därför inte konvertera
dem till gissade ASCII-, JIS- eller Unicode-namn. `name_ascii` och äldre
språk-/rosterfält får endast förekomma i isolerade fixture-tester.

Det går inte heller att dra slutsatsen att all speltext är japansk eller
engelsk från en enskild ISO-metadatafil. Språkstatus måste bindas per källa,
revision och textkonsument.

## Källor och implementation

- DMWebs filformats- och datafildekoder används för `PLRD`/`TABL`, `FONT256`
  och SCR-regionerna.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md) är
  den samlade statusen.
- `src/nexus/nexus_v1_rlowfix_text.c` behåller råa TEXT/TABL-bytes.
- `src/nexus/nexus_v1_saturn_font.c` behåller verkliga CG-tiles men markerar
  glyph-mappningen som ej redo.
- `src/nexus/nexus_v1_screen_text.c` avvisar riktig SCR-text tills mappingen
  är källbunden. Den generiska indexed-textfunktionen är endast test-/fixture-
  material och är inte en Nexus-textkonsument.

Ingen hosttext får ersätta en saknad eller obevisad Saturn-textyta.
