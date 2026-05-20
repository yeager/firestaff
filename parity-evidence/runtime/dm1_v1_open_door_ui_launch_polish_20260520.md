# DM1 V1 Open Door UI Launch Polish Source Lock - 2026-05-20

Scope: spell/UI launch path only. This does not implement direct key-on-door behavior.

## ReDMCSB evidence

- `COMMAND.C:473-483` defines `G0454_as_Graphic561_MouseInput_SpellArea`: C109 caster tab, C101..C106 rune symbols, C108 cast, and C107 recant.
- `COMMAND.C:2302-2307` dispatches C100 click-in-spell-area to `F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE` only when no candidate champion is pending and a magic caster exists.
- `CLIKMENU.C:484-497` routes C108 cast through `F0408_MENUS_GetClickOnSpellCastResult`.
- `MENU.C:66` defines `Zo` / Open Door as spell row 14 with attributes `0x3C42`: projectile kind, projectile type 4, Air skill.
- `MENU.C:1867-1870` doubles the Air skill for projectile type 4, then calls `F0327_CHAMPION_IsProjectileSpellCast` with `C0xFF80 + spellType` and bounded kinetic energy.
- `CHAMPION.C:2097-2102` derives projectile step energy from `MaximumMana`, applies the low-kinetic adjustment, and calls `F0326_CHAMPION_ShootProjectile(..., attack=90, stepEnergy)`.
- `CHAMPION.C:2064-2065` launches via `F0212_PROJECTILE_Create` using champion/party direction and the source launch cell formula.
- `PROJEXPL.C:485-489` handles `C0xFF84_THING_EXPLOSION_OPEN_DOOR`: a non-destroyed button door schedules C10/C02 toggle at `GameTime+1`.
- `PROJEXPL.C:491-508` is the separate normal door pass-through/destruction branch; this slice does not alter it.

## Firestaff binding

The M11 UI cast path now treats the Open Door spell-effect emission as the source launch point: after C108/F0408-style cast success, it builds the Open Door spell effect from table row 14, derives F0327-style step energy, creates a magical Open Door projectile with attack 90, and schedules its first projectile move. Existing projectile-impact code remains the owner of delayed door toggle behavior.
