# Pass607 - DM1 V1 level-change autosave absence source lock

- Status: PASS607_DM1_V1_LEVEL_CHANGE_AUTOSAVE_ABSENCE_SOURCE_LOCKED
- Manifest: parity-evidence/verification/pass607_dm1_v1_level_change_autosave_absence_source_lock/manifest.json
- Scope: ordinary stairs, pits, teleporters, command C140, and save write path

## ReDMCSB Source Audit

- CLIKMENU.C:124-140, focused 135-139: F0364 takes stairs through F0267/F0154/F0155 and current-map setup; no direct save/write path.
- CLIKMENU.C:142-174, focused 167-169: turning on stairs delegates to F0364 and returns.
- CLIKMENU.C:180-349, focused 264-328: movement stairs branches delegate through F0364/F0267; ordinary accepted movement calls F0267; no direct save/write path.
- MOVESENS.C:316-998, focused teleporter 475-506, pit 538-606, result 738-821: teleporter/pit chains mutate map/position/direction/result/sensors only.
- MOVESENS.C:316-998, focused 666-730: save-symbol references in F0267 are checksum/copy-protection expressions only, not direct calls.
- MOVESENS.C:1553-1796, focused 1695-1704: party-on-stairs floor sensor filtering has no save/write path.
- DUNGEON.C:1508-1558, focused 1540-1551: level-delta lookup returns target map/coordinates only.
- DUNGEON.C:1560-1582: stairs exit direction probes square geometry only.
- DUNGEON.C:2724-2764, focused 2755-2757: current/party map setters update map metadata only.
- COMMAND.C:2045-2480, focused 2228-2370: command C140 is the only direct caller of F0433.
- LOADSAVE.C:550-1783, focused 1477-1714: F0433 owns saved-game file creation/write and G2018 last-save-time mutation.

## Direct Save Call Search

- Exact `F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF();` locations across ReDMCSB Common/Source `*.C`: [{'file': 'COMMAND.C', 'line': 2368}]

## Canonical Asset Hashes

- DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- TITLE: adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## Conclusion

ReDMCSB does not show an ordinary DM1 V1 autosave on stairs, pits, teleporters, or chained level-change movement. The source-lock result is absence, not an implementation task: save writes are routed through command C140 only.
