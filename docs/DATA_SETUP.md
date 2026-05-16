# Firestaff — Game Data Setup

Firestaff requires original game data files that you own. The engine does not include any copyrighted game assets.

## Directory Structure

Place your game data in `~/.firestaff/data/` (or use `--data DIR`):

```
~/.firestaff/data/
  dm1/
    GRAPHICS.DAT
    DUNGEON.DAT
  dm1-multilingual/        (optional, for French/German text)
    GRAPHICS.DAT
    DUNGEON.DAT             (English)
    DUNGEONF.DAT            (French)
    DUNGEONG.DAT            (German)
  csb/
    GRAPHICS.DAT
    DUNGEON.DAT
  dm2/
    GRAPHICS.DAT
    DUNGEON.DAT
  nexus/
    (extracted Saturn ISO files — see docs/NEXUS_PLAN.md)
```

## Required Files Per Game

### Dungeon Master (dm1)
| File | Size | Description |
|---|---|---|
| `GRAPHICS.DAT` | ~355 KB | All graphics (walls, creatures, items, UI) |
| `DUNGEON.DAT` | ~33 KB | Dungeon layout, items, creatures, text |

Source: Dungeon Master PC DOS version (PC-34 or PC-34 Multilingual).

### Chaos Strikes Back (csb)
| File | Size | Description |
|---|---|---|
| `GRAPHICS.DAT` | ~400 KB | CSB graphics |
| `DUNGEON.DAT` | ~40 KB | CSB dungeon data |

Source: Chaos Strikes Back PC DOS, Amiga, or Atari ST version.

### Dungeon Master II: Skullkeep (dm2)
| File | Size | Description |
|---|---|---|
| `GRAPHICS.DAT` | varies | DM2 graphics |
| `DUNGEON.DAT` | varies | DM2 dungeon data |

Source: Dungeon Master II PC DOS version.

### DM Nexus (nexus)
Requires extracted files from the Sega Saturn disc image.
See `docs/NEXUS_PLAN.md` for extraction instructions.
Use `tools/extract_nexus_iso.py` to extract from your disc image.

## Validation

Run the built-in validator to check your data setup:

```bash
firestaff --validate
firestaff --validate --data /path/to/data
```

## Multi-Language Support

For French and German dungeon text, use the PC-34 Multilingual version:
- `DUNGEON.DAT` — English
- `DUNGEONF.DAT` — French  
- `DUNGEONG.DAT` — German

Place these in `dm1-multilingual/`. Firestaff auto-selects based on language setting.

## Where To Get Game Data

Firestaff does not distribute game data. You need legitimately owned copies.
The original games are available from various legal sources for retro games.
