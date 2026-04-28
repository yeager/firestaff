# M10 Phase 18: Champion Lifecycle Probe

## Scope (v1)

- F0830-F0834 hunger/thirst + stamina regen
- F0835-F0840 status-effect expiry
- F0841-F0843 move-timer enforcement (F0310 port)
- F0844-F0847 health/mana/stat regen + temp XP decay
- F0848-F0853 XP award + level-up
- F0854-F0856 emission helpers
- F0857-F0859 serialisation & init from PartyState

## NEEDS DISASSEMBLY REVIEW

- `F0830`: timeCriteria bit-manipulation verbatim port.
- `F0832`: stamina gain-cycle expansion loop (maxStamina halving vs currentStamina).
- `F0835`: C80..C83 magic-map per-champion counters (CSB) are v1 stub.
- `F0853`: Fontanel has no kill-XP function; v1 returns 0 to avoid double-counting with Phase 13 unclaimed-kill markers.

## DUNGEON.DAT integration

- sign-extended food=230, water=210 from Phase 10 uint8 seeds
