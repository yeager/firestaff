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

## 3. SDDRVS.TSK and SLEV status

The external corpus contains `SDDRVS.TSK` and `SLEV00.BIN`–`SLEV15.BIN`, but
their event ownership and ABI are not proven. Firestaff's disassembly audit
only establishes that the SLEV files have an SH-2-shaped entry/header and
bounded literal references. It does not establish teleporter, door, trap or
cutscene rules, and it does not prove that `SDDRVS.TSK` is their dispatcher.

Status: Firestaff retains the authentic bytes and profile receipts, but does
not parse or execute them. No hot-swap facility, cheat route, or script
semantics should be inferred from the files.

## 4. Teleport claims

Teleport-like square records are present in the bounded DGN study, but their
retail event consumer and damage/timing semantics are not source-bound. They
must not be presented as a verified playable mechanic or as a debug command.

## 5. DM1 Cheats Carried to Nexus?

DM1 has known cheat codes documented in Firestaff materials. Nexus has a
different executable and data model, so DM1 behaviour cannot be inherited:
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
community), Nexus has little public hacking documentation. The authentic
Saturn disc and SH-2 disassembly have been inspected for the current bounded
routes, but no retail cheat/debug route has been source-bound. Further debug
claims require a new disassembly or runtime witness.
