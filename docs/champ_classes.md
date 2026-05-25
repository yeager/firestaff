# DM1 V1 Champion Classes — Source Lock

Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/DEFS.H:757-768
Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/CHAMPION.C:199-270

## Base Classes (4)

| Index  | Class  | Hidden Skills            | Primary Use       |
|--------|--------|-------------------------|-------------------|
| C00    | Fighter | Swing, Thrust, Club, Parry | Melee combat    |
| C01    | Ninja   | Steal, Fight, Throw, Shoot | Stealth/combat  |
| C02    | Priest  | Identify, Heal, Influence, Defend | Healing/magic |
| C03    | Wizard  | Fire, Air, Earth, Water   | Elemental magic   |

Source: DEFS.H:757-760

## Class Skill Breakdown

Each base class (Fighter/Ninja/Priest/Wizard) has 4 hidden skills that map to it:
- Fighter: C04 (Swing), C05 (Thrust), C06 (Club), C07 (Parry)
- Ninja: C08 (Steal), C09 (Fight), C10 (Throw), C11 (Shoot)
- Priest: C12 (Identify), C13 (Heal), C14 (Influence), C15 (Defend)
- Wizard: C16 (Fire), C17 (Air), C18 (Earth), C19 (Water)

Source: CHAMPION.C:758, 266-267

Class experience = sum of its 4 hidden skill experiences >> 1 (halved/average)

## Skill List (20 total)

C00-C03: Base classes (derived from hidden skills, not directly set)
C04: SWING — Fighter melee weapon swing
C05: THRUST — Fighter melee thrust
C06: CLUB — Fighter blunt weapon
C07: PARRY — Fighter defense
C08: STEAL — Ninja pickpocket (only usable in specific dungeons)
C09: FIGHT — Ninja bare-handed combat
C10: THROW — Ninja throwing weapons
C11: SHOOT — Ninja missile weapons
C12: IDENTIFY — Priest item identification
C13: HEAL — Priest healing magic
C14: INFLUENCE — Priest charm/command
C15: DEFEND — Priest protective magic
C16: FIRE — Wizard fire magic
C17: AIR — Wizard air magic
C18: EARTH — Wizard earth magic
C19: WATER — Wizard water magic

Source: DEFS.H:757-768

## Class Mechanics

There is NO class-restriction on weapon/armor usage in DM1 V1.
Any champion can use any equipment regardless of class.
Class determines which hidden skills contribute to the base class experience.

Class is implicit from the champion definition text - there is no class selection screen.
The champion definition file (not class selection) determines starting skills.

## Stat Bonuses by Class (on level-up)

Fighter: Strength +2 (major), Dexterity +1 (minor) — alternates
Ninja: Strength +1 (minor), Dexterity +2 (major) — alternates
Priest: Wisdom +2 (major) or +1 (minor), Antifire +0..2
Wizard: Wisdom +1 (minor) or +2 (major), Antimagic +0..3

Source: CHAMPION.C:908-970
