# Nexus V1 Cheats and Debug Modes Audit — Source-Locked

## Summary
No cheat codes, debug modes, or developer backdoors have been documented
for Dungeon Master Nexus. The Firestaff codebase contains zero cheat
references, no debug menus, and no god-mode flags.

## 1. Known Cheat/Debug Status

What Firestaff found:
- Zero cheat code references in src/nexus/ source files
- Zero debug mode flags in nexus_v1_engine.c or nexus_v1_game.c
- Zero Konami-code patterns or equivalent in any source or docs
- No god mode, all items, warp level, or free gold patterns found
- No debug menu, devconsole, or developer UI in the codebase

Original Saturn game: No publicly documented cheat codes for Nexus were
found in any known Saturn game cheat database or fan sites. Nexus was
Japanese-only (1998, FTL Games/Athena) with no Western release and
virtually no hacking/modding community activity.

## 2. Debug Artifacts in Source

The only "debug" references in nexus source are routine development notes:

- nexus_sensors.md: "Debug scripts can be hot-swapped without recompiling EXE"
  (refers to SDDRVS.TSK script VM development workflow, not a game feature)
- nexus_performance.md: "Back-face culling -- skip triangles facing away from camera"
  (standard rasterizer optimization, not debug infrastructure)
- nexus_performance.md: "Debug scripts can be hot-swapped" (dev workflow note)

No production debug/build cheats exist in the source.

## 3. SDDRVS.TSK Debug Scripts

SDDRVS.TSK is a Saturn task/script VM that processes per-level event scripts
(SLEV00-15.BIN). Scripts define teleporter logic, door animations, trap
triggers, and cutscene sequencing.

nexus_sensors.md notes: "Debug scripts can be hot-swapped without recompiling EXE"
-- developers could swap SLEV*.BIN files without rebuilding the game.
This hot-swap capability could theoretically inject custom script logic,
but only if SDDRVS.TSK were fully understood and implemented.

Status: SDDRVS.TSK parser is NOT implemented in Firestaff.

## 4. In-Game Teleport Mechanics (Not Cheats)

The game has in-universe teleport mechanics (types 9/10) that warp the party
to another dungeon square with HP damage on arrival. These are normal game
mechanics, not cheats. No debug teleport commands exist in source.

## 5. DM1 Cheats Carried to Nexus?

DM1 has known cheat codes documented in firestaff materials. Since Nexus
runs DM1 game logic but with different rendering engine and data structures:
- Maybe: DM1 cheat input sequences could theoretically work
- Unlikely: Champion/party data structures differ significantly
- Not tested: No DM1 cheats verified against Nexus
- No evidence: Zero DM1 cheat references in any Nexus docs or source

## 6. Comparison Table

| Feature         | DM1        | Nexus                |
|-----------------|------------|----------------------|
| Known cheat codes| Several    | None documented      |
| Debug menu      | Unknown    | No source evidence   |
| God mode        | No         | No                   |
| Level skip      | No         | No                   |
| All items       | No         | No                   |
| Debug scripts   | N/A        | SDDRVS.TSK (unimpl.) |
| Hot-swap scripts| N/A        | Dev-only, not feature|

## 7. Conclusion

No cheat codes or debug modes are known to exist for Nexus. This is
consistent with a niche Japanese Saturn title from 1998 -- such games
almost never had their cheat codes widely documented or preserved.

Unlike DM1 (which appeared on many platforms with an active cracking/scene
community), Nexus flew entirely under the radar. If the Saturn disc image
were obtained and an SH2 disassembly produced, debug features might be
found in the binary (debug print strings, breakpoint handlers, developer
menu branches), but no such analysis has been performed.
