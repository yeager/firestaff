# DM2 V1 — Known Bugs & Quirks

## Overview

Dungeon Master II is a relatively stable game for its era, but several bugs, quirks, and design quirks have been documented by the community over decades of play. These range from game-freezing crashes to harmless visual glitches.

## Game-Breaking Bugs

### Saved Game Corruption from Item Duplication
- **Severity**: High
- **Description**: Duplicating items (armor, weapons) using the cloning exploit can corrupt saved games, making them unloadable. The money duplication trick is safe; duplicating other items is not.
- **Workaround**: Never duplicate items other than gold via the money box trick. If a saved game is corrupted, restore from a backup.

### Game Hang from Cloning Exploits
- **Severity**: High
- **Description**: Stealing the same item multiple times from champion mirrors, or dropping the same cloned item from two cloned champions on the same square, causes the game to hang with no way to recover except force-quitting.
- **Workaround**: Be very careful with item cloning — never drop identical cloned items on the same square. Never steal the same item repeatedly.

### Divide-by-Zero Crash
- **Severity**: High (theoretical)
- **Description**: If the player's position is set inside a wall tile (invalid dungeon data, corrupted save, or manually edited dungeon.dat), the raycasting renderer will attempt to divide by zero when computing wall height, crashing the game.
- **Workaround**: Validate dungeon.dat edits; restore from backup if position becomes invalid.

## Documented Quirks

### Dungeon.dat Edits Only Affect New Games
- **Severity**: Low (design choice, not a bug)
- **Description**: Edits to dungeon.dat only appear in new games. Once a game is saved, a `dmgame.dat` is created that locks in the dungeon state from that moment. Existing saves ignore subsequent dungeon.dat changes.
- **Forum reference**: Dungeon Master Forum, DMDC2 Help thread (2014)

### Transparency Loss on Imported Graphics
- **Severity**: Low
- **Description**: In DM2GDED, when importing a replacement 8-bit bitmap image for a sprite, transparency is sometimes lost on non-first images. The imported image appears with a grey square instead of the intended transparent background.
- **Workaround**: Re-export, check palette index, ensure the transparent color index matches the original's; or use the first image slot as the test case before applying to others.

### Palette Color Conflicts
- **Severity**: Low
- **Description**: HP/Stamina/Mana bar colors share palette indices with menu text and cursor colors. Changing bar colors also affects text/cursor colors, and vice versa. DM2GDED allows palette editing but not independent control.
- **Workaround**: Accept the tradeoff; pick colors that work for both purposes.

### Creature Permissions
- **Severity**: Low (design)
- **Description**: Creatures placed in map areas where they don't natively exist will die or disappear. Flying creatures (Attack Minions, etc.) can be placed more freely. The game enforces creature/area validity at runtime.
- **Forum reference**: DM2 editor questions thread (emetar, 2014)

### Map Expansion Shifts Depth References
- **Severity**: Medium
- **Description**: Expanding a map in the editor can shift the destination of depth-based connections (ladders, pits, holes). A ladder that previously led down to a specific map may misalign after the parent map is expanded.
- **Workaround**: After expanding a map, manually verify all ladder/teleporter destinations. Use explicit teleporter + ACTUator combinations instead of relying on depth parameters for critical connections.

### Shop Item Display Quirks
- **Severity**: Low
- **Description**: Putting items on a merchant's table outside of the normal shopping flow (e.g., putting items on the table in a combat area) can cause the merchant to become hostile and throw items at the player.
- **Workaround**: Only use shop interfaces for transactions.

### Portrait Import Requirement
- **Severity**: Low
- **Description**: Champion portraits must be 32x29 pixels with exactly 16 colors from the game's palette. Outside these constraints, the portrait will not display correctly.
- **Workaround**: Use the exact specifications; use the online IFF conversion tool referenced in the DM encyclopaedia.

## Visual/UI Quirks

### Moving Pits and Teleporters
- **Severity**: Medium (hazard)
- **Description**: Moving pits (Cloud Clan area, 4x8 room at southern side, coordinates 06,68,43) and moving teleporters (Skullkeep level 3, 5x4 room at southeastern aisle, coordinates 03,53,34) are caused by invisible creatures walking around. These invisible creatures can be avoided using Scout Map minions or a Freeze life Magical Box.
- **Note**: This is not a bug but a gameplay hazard; documented here for completeness.

### Champion Cloning Visual Oddities
- **Severity**: Low
- **Description**: Cloned champions share a visual representation with their original. Torham's portrait appears on cloned champions, making it hard to distinguish party members by portrait alone.
- **Workaround**: Rename champions or rely on position/order for identification.

### Sound IRQ Configuration
- **Severity**: Low (platform-specific)
- **Description**: On some systems/DOSBox configurations, ambient sound may be absent until the IRQ is changed from the default. User Tianhe1A reported that changing IRQ from 1 to 5 restored sound on their setup.
- **Workaround**: Adjust sound IRQ in the game's setup utility or DOSBox config if sound is missing.

## Bugs in Community Tools

### DMDC2 Dungeon Edit Visibility
- **Severity**: Medium
- **Description**: Users sometimes cannot see their edits in the game because the edited dungeon.dat is not the one being loaded. Common cause: editing the wrong copy of dungeon.dat, or the game is loading `dmgame.dat` instead (from an existing save).
- **Workaround**: Always edit the dungeon.dat directly in the game's `data/` folder, or copy edited file there. Start a new game to see edits.

## Unverified / Possible Bugs

### Macros/Text Overflow
- Some community reports suggest that editing certain text fields (shop item lists, ACTUator text) beyond expected lengths can cause display or logic issues, but these are not well-documented.

### Spell System
- The spell system is the same engine as DM/CSB; no known spell-specific bugs, but custom spell re-definitions via DM2GDED Rev.5 could theoretically cause instability if spell effect parameters are set to invalid values.

## Version Differences

- **DOS/PC version**: Most extensively documented; most community tools target this version
- **Amiga version**: Different graphics format (IFF), same dungeon structure; portrait modification possible via sample_palette.IFF
- **Macintosh version**: Different executable format (.68k Mac app), movie-based cutscenes; Easter egg for designer names works on this version
- **Sega CD version**: Different platform; limited community documentation

## Summary Table

| Bug/Quirk | Severity | Type | Workaround |
|-----------|----------|------|------------|
| Saved game corruption (item dup) | High | Exploit side-effect | Don't duplicate non-gold items |
| Game hang (clone exploit) | High | Exploit side-effect | Careful with item cloning |
| Div-by-zero (wall collision) | High | Invalid data | Validate dungeon.dat |
| Dungeon.dat edits = new game only | Low | Design choice | Start new game for edits |
| Transparency loss on import | Low | Tool bug | Check palette index |
| Palette color conflicts | Low | Design constraint | Accept tradeoff |
| Creature death out of area | Low | Design | Use appropriate creatures |
| Map expansion depth shift | Medium | Editor quirk | Verify ladder/teleporter targets |
| Money box link persistence | Low | Design | Save/reload to unlink |
| Portrait size constraint | Low | Format spec | 32x29, 16 colors, IFF |
