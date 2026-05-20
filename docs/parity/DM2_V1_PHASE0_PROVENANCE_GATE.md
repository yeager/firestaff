# DM2 V1 Phase 0 Provenance Gate

This is the first DM2/Skullkeep V1 gate. It locks the source and asset inputs that later parser/runtime work must cite before making any DM2 parity claim.

The gate is intentionally provenance-only:

- It does not implement a DM2 parser or runtime path.
- It does not claim viewport, input, save, audio, or gameplay parity.
- It keeps ReDMCSB as DM1/CSB discipline evidence only; ReDMCSB is not a DM2 source.

## Locked Anchors

Verifier: `tools/verify_dm2_v1_phase0_provenance_gate.py`

Evidence: `parity-evidence/verification/dm2_v1_phase0_provenance_gate/manifest.json`

Local N2 source caches used by the verifier:

- Original DM2 assets: `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/`
- skproject mirror: `~/.openclaw/data/firestaff-dm2-sources/skproject.git`
- Sphenx SKWin SPX cache: `~/.openclaw/data/firestaff-dm2-sources/sphenx-skwin/`

Locked DM2 original assets:

- `Dungeon-Master-II-Skullkeep_DOS_EN.zip` sha256 `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`
- `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` sha256 `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228`
- `SKULL.ASM` sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`

Both DM2 ZIP layouts share the locked core data member hashes for `DUNGEON.DAT`, `GRAPHICS.DAT`, `SONGLIST.DAT`, and `SKULL.EXE`.

Locked external source references:

- `https://github.com/gbsphenx/skproject` master HEAD `a962896e42aaf54c76157a7b062fb5b0526929e6`
- skproject tree `a095e458cfaaa0490b9c4d4d2adf88108a8ad92f`
- Sphenx SKWin page `https://dmbuilder.sphenxmusics.fr/skwin.php` sha256 `ef5ed8402262d5eb95b4fe0f6e3ef6e1074a69cdaa9c2f864d3309f545d63091`
- Sphenx package `https://dmbuilder.sphenxmusics.fr/skwin/SkWinCurrent.zip` sha256 `ed6f1f8a38c43fbff36421090c4bab4e5f939707db12f55cfcfac04688df4645`

## Gate Rule

Any DM2 V1 parser/runtime slice must cite this gate or a stricter successor, then add a smaller source-evidence manifest for the specific behavior being implemented. DM1 or CSB assumptions are not accepted as DM2 evidence without a DM2-specific SKWin/skproject/SKULL.ASM anchor.
