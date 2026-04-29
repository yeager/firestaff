# Pass 163 — champion recruit source path lock

This pass narrows Lane A after pass162: corrected Resurrect/Reincarnate box clicks still collapse to `48ed3743ab6a` because source party creation starts earlier, at the wall champion portrait sensor.

## Source path

1. `MOVESENS.C` handles `C127_SENSOR_WALL_CHAMPION_PORTRAIT` and calls `F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)`.
2. `REVIVE.C:F0280` sets `G0299_ui_CandidateChampionOrdinal` and increments `G0305_ui_PartyChampionCount`.
3. Only after that does `COMMAND.C` route `M568_PANEL_RESURRECT_REINCARNATE` clicks to `REVIVE.C:F0282`.
4. `F0282` assumes candidate state already exists at `G0305_ui_PartyChampionCount - 1` and finalizes/cancels it.

## Checks

- SRC_RECRUIT_001 PASS — `MOVESENS.C` lines 1501, 1502: The source candidate-add transition is triggered by wall sensor C127, not by the panel button.
- SRC_RECRUIT_002 PASS — `REVIVE.C` lines 272, 276: F0280 marks a candidate and increments party count before the resurrect/reincarnate decision panel is finalized.
- SRC_RECRUIT_003 PASS — `COMMAND.C` lines 1985, 1990: Panel clicks dispatch to F0282 only when the panel content is already Resurrect/Reincarnate.
- SRC_RECRUIT_004 PASS — `REVIVE.C` lines 704, 744, 747: F0282 assumes F0280 already inserted a candidate at party_count-1, then clears candidate state after Resurrect/Reincarnate/Cancel.
- SRC_RECRUIT_005 PASS — `COMMAND.C` lines 231, 232, 231, 232: The old y=165 route was outside the original C160/C161 boxes; pass162 corrected this but still hit static-no-party, proving the missing step is earlier C127/F0280 candidate creation.

## Pass162 linkage

- pass162 completed: 3
- pass162 buckets: {'blocked/static-no-party': 3}
- Interpretation: next route work must find a reproducible original input path that visibly triggers the C127/F0280 candidate transition before clicking C160/C161. More Resurrect/Reincarnate panel coordinate permutations are now low value unless preceded by proof of candidate state.
