# CSB V1 Phase 0 Provenance Gate

This is a CSB V1 provenance-only gate. It locks the local N2 CSB source and asset anchors that later parser/runtime slices must cite before making any CSB parity claim.

The gate is deliberately narrow:

- It does not enable CSB launch, rendering, gameplay, save compatibility, champion import, or Make New Adventure behavior.
- It does not treat DM1 V1 assets or ReDMCSB-backed DM1 behavior as CSB proof.
- ReDMCSB remains the primary source for shared DM1 code discipline only. CSBWin and the CSB lineage source are secondary CSB evidence and are labeled that way in the verifier output.

## Locked Anchors

Verifier: `tools/verify_csb_v1_phase0_provenance_gate.py`

Evidence: `parity-evidence/verification/csb_v1_phase0_provenance_gate.json`

Local N2 caches used by the verifier:

- Original CSB assets: `~/.openclaw/data/firestaff-original-games/DM/_canonical/csb/`
- Secondary CSB lineage source: `~/.openclaw/data/firestaff-csb-source/CSB/`
- Secondary CSBWin source: `~/.openclaw/data/firestaff-csbwin-source/CSBWin/`
- ReDMCSB shared DM1/CSB discipline reference: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

Locked original archive anchors:

- `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` sha256 `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5`
- `Game,Chaos_Strikes_Back,Amiga,Software.7z` sha256 `77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58`

Selected Atari ST hard-disk payloads are locked for the current CSB V1 lane:

- Core pair: `GRAPHICS.DAT` and `DUNGEON.DAT`
- Runtime support: `HCSB.DAT`, local `HCSB.HTC`, `MINI.DAT`, and `SWITCH.DAT`
- Title/animation and utility payloads: `ANIMATE.DAT`, `CHAOS.FTL`, `HINT.FTL`, `CMAIN`, `GAME.PRG`, and `UTILITY.PRG`

Amiga hard-disk payload anchors are locked as variant evidence only:

- Core pair: `Graphics.DAT` and `Dungeon.DAT`
- Dungeon variants: `DungeonF.DAT` and `DungeonG.DAT`
- Title/end and runtime payloads: `TITL.DAT`, `ENDA.DAT`, `ANIM.FTL`, `GRF1.FTL`, `KAOS.FTL`, and `SWSH.FTL`

The verifier also records that no standalone `song`/`music`/`mus` file is present in the selected Atari or Amiga hard-disk anchors. CSB audio parity must therefore be source-locked later through the graphics/CSBgraphics sound-record path, not inferred from a filename-only music asset.

## Gate Rule

Any CSB V1 parser, launch, rendering, audio, utility/import, or gameplay slice must cite this gate or a stricter successor, then add behavior-specific evidence for the exact CSB variant being changed. A DM1 V1 source lock is not enough unless the slice proves the CSB variant uses the same rule.

## Verification

```sh
python3 -m py_compile tools/verify_csb_v1_phase0_provenance_gate.py
python3 tools/verify_csb_v1_phase0_provenance_gate.py
python3 -m json.tool parity-evidence/verification/csb_v1_phase0_provenance_gate.json >/dev/null
