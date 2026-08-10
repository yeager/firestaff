# Theron JP-roster till startup-party (2026-08-10)

## Resultat

Den hashverifierade JP Track 02-rosterklustern kan nu användas vid startupens
forcefield-handoff. `theron_v1_party_refresh_jp_source_records()` läser de
åtta riktiga posterna via `theron_v1_track02_jp_roster_read()` och uppdaterar
endast den redan valda party-state:n när varje vald champion matchar en
source-record.

Följande fält binds från bytesen i `TQJP02.bin`:

- namn
- HP, stamina och mana
- de sju attributbytesen
- de 16 skillbytesen och högsta skill per klass
- primärklass härledd från dessa source skills

JP-recorden ligger vid den verifierade råa rosterklustern `0xB3D98` och får
endast användas med JP BIN-MD5 `b7afb338ad31be1025b53f9aff12d73a`. Den första
Theron-posten verifieras lokalt som HP `175`, stamina `1500` och mana `35`.

## Gräns

Detta är en source-bound rosterbindning, inte ett bevis på originalets
portrait-, palette-, text- eller T900-consumer. `portrait_index` lämnas därför
`THERON_PORTRAIT_UNAVAILABLE`. Utrustning, inventory, use/equip/stack, RNG,
AI, T700 och ljud öppnas inte av denna receipt.

## Verifiering

```text
./build/test_theron_v1_track02_champion_roster                 PASS
./build/test_theron_v1_startup_media_palette_bind              PASS
FIRESTAFF_THERON_DATA=~/.firestaff/data/theron \
  ./build/test_theron_v1_track02_dungeon_loader                 PASS
./build/test_theron_v1_combat_mechanics                         PASS (121/121)
cmake --build build --target firestaff --parallel 1             PASS
```

Inga BIN-, BIOS- eller capturefiler läggs i repositoryt.
