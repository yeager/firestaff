# CSB V1 Phase 0 provenance gate - 2026-05-20

Scope: evidence-only source/provenance gate for future CSB V1 work on N2. This does not enable launch, runtime, rendering, gameplay, save compatibility, or pixel parity.

Status: PASS

## Source roles

- Primary: ReDMCSB `Toolchains/Common/Source` for CSB save headers, dungeon IDs, dungeon validation, New Adventure readiness, and Atari CSB loader/save routing.
- Secondary: CSB lineage `CSB/src` and CSBWin for workflow/payload/load boundaries only.
- Original data: N2 `firestaff-original-games/DM` canonical CSB anchors plus curated Atari extraction support payloads.

## Locked source trees

- `secondary_csb_lineage`: `dda570585abb4c8113a3298d21c0b599e6cac4f9` ok=True (<csb-secondary>/CSB/src)
- `secondary_csbwin_lineage`: `2f63d10d9b8c155e0be17888271d394255ce1bac` ok=True (<csbwin-secondary>/CSBWin)

## Locked files

- `redmcsb_defs_h` `primary_source` bytes=479889 sha256=`33b3160a5dd7d9c62b9ad0f3d26ac09ddacd6328c9717656fe2bbe8786625728` ok=True path=`<redmcsb-primary>/Toolchains/Common/Source/DEFS.H`
- `redmcsb_new_adventure_gate` `primary_source` bytes=2688 sha256=`a54c7be10052f89f923597a22a19ca6441ca005ddfef4adb651b56d85c471690` ok=True path=`<redmcsb-primary>/Toolchains/Common/Source/CEDTINCH.C`
- `redmcsb_dungeon_validation` `primary_source` bytes=2962 sha256=`67169cab3c30b2acd18481a867c5692c333747966602bbc4027a355bdcf5e219` ok=True path=`<redmcsb-primary>/Toolchains/Common/Source/CEDTINCU.C`
- `redmcsb_atari_csb_loader` `primary_source` bytes=22631 sha256=`74bee0f78640d6f9a2dec64d66285e4e96942d16606ac3e1b70ff18b8ecf9723` ok=True path=`<redmcsb-primary>/Toolchains/Common/Source/HINTLOAD.C`
- `redmcsb_save_routing` `primary_source` bytes=20504 sha256=`f6acc8c39b5525227b0ece435e87eb787ec054228164e9c6af51953b376d1b07` ok=True path=`<redmcsb-primary>/Toolchains/Common/Source/CEDTINC8.C`
- `csb_lineage_readme` `secondary_source` bytes=972 sha256=`ac62c130353221fc2599bf6e045d09910ab498d4055c8e35893a22153421788a` ok=True path=`<csb-secondary>/CSB/src/README`
- `csb_lineage_chaos_cpp` `secondary_source` bytes=180270 sha256=`e79895b6ed1ad5620de790ea80f2a0ad197fa32881a7da41d30db3740b6d9891` ok=True path=`<csb-secondary>/CSB/src/Chaos.cpp`
- `csb_lineage_graphics_cpp` `secondary_source` bytes=82105 sha256=`938e6b058fcd9449ece41789df880e1466e1cf3a2083a44a68b1d5b4a8f96430` ok=True path=`<csb-secondary>/CSB/src/Graphics.cpp`
- `csbwin_game_readme` `secondary_source` bytes=714 sha256=`0e3943251e85c248de5c560dc96d2e6a7f6fc5c1a175331210c76da386d76966` ok=True path=`<csbwin-secondary>/CSBWin/Game/readme.txt`
- `csbwin_csbwin_cpp` `secondary_source` bytes=48281 sha256=`89418e01b0a8eef330451320d19078a3510cbc699f635c8af22820365e4ceb23` ok=True path=`<csbwin-secondary>/CSBWin/CSBwin.cpp`
- `csbwin_savegame_cpp` `secondary_source` bytes=87215 sha256=`f0009ba8235509e25d9dbea439daf0b68ac29cfa0dcd6241bba876b1f69cd0f2` ok=True path=`<csbwin-secondary>/CSBWin/SaveGame.cpp`
- `canonical_csb_readme` `original_provenance` bytes=2420 sha256=`5a810e1f971223668a772d99f5e925859580a8e5a4c05997971fbc48b3ca3134` ok=True path=`<original-games>/DM/_canonical/csb/README.md`
- `canonical_atari_graphics` `original_asset` bytes=319080 sha256=`33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` ok=True path=`<original-games>/DM/_canonical/csb/atari-GRAPHICS.DAT`
- `canonical_atari_dungeon` `original_asset` bytes=2098 sha256=`3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` ok=True path=`<original-games>/DM/_canonical/csb/atari-DUNGEON.DAT`
- `canonical_amiga_graphics` `original_asset_split_reference` bytes=435076 sha256=`3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942` ok=True path=`<original-games>/DM/_canonical/csb/amiga-Graphics.DAT`
- `canonical_amiga_dungeon` `original_asset_split_reference` bytes=2098 sha256=`3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` ok=True path=`<original-games>/DM/_canonical/csb/amiga-Dungeon.DAT`
- `selected_atari_hcsb_dat` `original_runtime_support` bytes=30793 sha256=`5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc` ok=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/HCSB.DAT`
- `selected_atari_hcsb_htc` `original_runtime_support` bytes=66172 sha256=`1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38` ok=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/HCSB.HTC`
- `selected_atari_mini_dat` `original_runtime_support` bytes=42815 sha256=`61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9` ok=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/MINI.DAT`

## Canonical symlinks

- `canonical_atari_graphics` ok=True link=`<original-games>/DM/_canonical/csb/atari-GRAPHICS.DAT` target=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/GRAPHICS.DAT`
- `canonical_atari_dungeon` ok=True link=`<original-games>/DM/_canonical/csb/atari-DUNGEON.DAT` target=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/DUNGEON.DAT`
- `canonical_amiga_graphics` ok=True link=`<original-games>/DM/_canonical/csb/amiga-Graphics.DAT` target=`<original-games>/DM/_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT`
- `canonical_amiga_dungeon` ok=True link=`<original-games>/DM/_canonical/csb/amiga-Dungeon.DAT` target=`<original-games>/DM/_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Dungeon.DAT`

## Source anchors

- `redmcsb_primary_csb_save_and_dungeon_ids` <redmcsb-primary>/Toolchains/Common/Source/DEFS.H:468-523 ok=True
- `redmcsb_primary_new_adventure_gate` <redmcsb-primary>/Toolchains/Common/Source/CEDTINCH.C:5-63 ok=True
- `redmcsb_primary_dungeon_validation` <redmcsb-primary>/Toolchains/Common/Source/CEDTINCU.C:5-77 ok=True
- `redmcsb_primary_atari_csb_loader` <redmcsb-primary>/Toolchains/Common/Source/HINTLOAD.C:11-18 ok=True
- `redmcsb_primary_save_routing` <redmcsb-primary>/Toolchains/Common/Source/CEDTINC8.C:101-118 ok=True
- `csb_secondary_required_payloads` <csb-secondary>/CSB/src/README:14-21 ok=True
- `csb_secondary_graphics_boundary` <csb-secondary>/CSB/src/Graphics.cpp:1814-1915 ok=True
- `csb_secondary_new_adventure_slots` <csb-secondary>/CSB/src/Chaos.cpp:507-623 ok=True
- `csbwin_secondary_workflow` <csbwin-secondary>/CSBWin/Game/readme.txt:1-30 ok=True

## Blockers recorded, not guessed

- `selected_atari_config_txt` absent=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/CONFIG.TXT` - CSB lineage names config.txt, but the selected original-data anchor does not provide it.
- `selected_atari_csbgame_dat` absent=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/CSBGAME.DAT` - No curated CSB saved-game/New Adventure fixture exists in the selected original-data anchor.
- `selected_atari_csbgame_bak` absent=True path=`<original-games>/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/CSBGAME.BAK` - No curated CSB saved-game backup fixture exists in the selected original-data anchor.

## Non-claims

- No CSB launch/runtime/render/gameplay/save-compatibility/pixel-parity support is enabled or claimed.
- ReDMCSB is the primary source wherever it has CSB-specific coverage; CSB and CSBWin are secondary references only.
- Atari ST and Amiga graphics are both hash-locked because they differ; they must not be substituted for each other.
- The selected Atari ST asset/support set is necessary provenance for future work, not launch clearance.

## Gate

```sh
python3 -m py_compile tools/verify_csb_v1_phase0_provenance_gate.py
python3 tools/verify_csb_v1_phase0_provenance_gate.py
python3 -m json.tool parity-evidence/verification/csb_v1_phase0_provenance_gate.json >/dev/null
```

JSON output: `parity-evidence/verification/csb_v1_phase0_provenance_gate.json`.
