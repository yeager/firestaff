# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Complete the native PC-DOS WIELD command owner before promoting the retail
  `SKSAVE1` creature-drop regression. Capture an original input-to-CD/RAM
  trace with its valid encounter, weapon choice, command arguments and RNG
  timing, then bind the remaining WIELD fallback/luck path to that trace. Do
  not replace it with a generated weapon, creature, save or combat result.
  The supplied DOS archive now proves the preceding in-memory resume route
  (`archive::data/sksave1.dat`) through a native turn. A direct WIELD probe
  reaches the authenticated calculation but correctly misses the first
  available encounter (`item=0x140c`, `creature=0x1116`, `power=26`,
  `dexterity=21`, `defense=170`, `armor=175`, `map difficulty=8`,
  `party power=11`). This receipt is not a valid source trace for selecting a
  different target, command, or RNG sequence; keep the creature-drop gate
  closed until an original interaction trace identifies one.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
