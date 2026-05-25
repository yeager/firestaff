# DM1 V1 Champion Death/Permanence — Source Lock

Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/REVIVE.C
F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel (line 704)
F0283_CHAMPION_ViAltarRebirth (line 902)

## Death Triggers
A champion dies when CurrentHealth reaches 0 or below.
Health reaches 0 through combat damage, poison, or hazards.

## Resurrect vs Reincarnate
Two paths when a champion dies on a mirror square:

RESURRECT (C160_COMMAND_CLICK_IN_PANEL_RESURRECT):
- Champion is restored at the mirror
- Possessions on the mirror square are unequipped and dropped
- No stat loss
- Available when champion is dead on mirror

REINCARNATE (C161_COMMAND_CLICK_IN_PANEL_REINCARNATE):
- Champion is revived with a new name (renamed)
- All skills cleared to 0 (reset)
- Stats reduced: each statistic current=max decreased by 1/8th
- Health/Stamina/Mana halved
- Then 12 random statistics get +1 to current and max each
- More severe penalty than resurrect
- Available when champion is dead on mirror

Source: REVIVE.C:808-840

## Resurrect/Reincarnate Flow
1. Remove all objects from champion slots
2. Unlink objects from dungeon list at mirror square
3. Find and disable the portrait sensor on the square
4. If REINCARNATE: rename champion, clear skills, apply stat penalties
5. Print RESURRECTED or REINCARNATED message
6. Champion becomes active again

BUG0_87: The engine disables the first sensor found, not the portrait sensor specifically.
If another sensor is placed before the portrait sensor, wrong sensor gets disabled.
Champion mirror may remain available for re-use.

Source: REVIVE.C:796-800

## Vi Altar Rebirth
F0283_CHAMPION_ViAltarRebirth: alternative revival at VIP altar.
- Champion moved to first empty cell if needed
- Health reduced to half of max
- Maximum health reduced by ~1/6 (maxHealth - maxHealth/64 - 1), min 25
- All slots cleared (later versions only)
- Used for special dungeon events

Source: REVIVE.C:902-950

## No Permanent Death
DM1 V1 does not have permanent death in the standard game.
Any dead champion on a mirror square can be resurrected or reincarnated.
If the party leaves the mirror square without reviving, the champion remains dead.
When the party returns, the options are still available.

## Skill Reset on Reincarnate
F0008_MAIN_ClearBytes clears the entire Skills array.
Base class skills (Fighter/Ninja/Priest/Wizard) are recalculated from hidden skills.
Since hidden skills are all reset to 0, base class skills also become 0.

Source: REVIVE.C:808-810
