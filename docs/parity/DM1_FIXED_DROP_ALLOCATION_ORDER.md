# Fixed-possession allocation and RNG ordering

Status: M10 and M11 now reserve each item before cell RNG and publish it before the
next entry. The streamed helper's bounded worm-pool regression covers 32 seeds,
four capacities and centered/off-center cells (256 cases), including exact RNG
state. The creature behavior executable passes 2,489 checks, and eight F0191
explosion integration cases pass. The M11 runtime gate passes 138 checks,
including zero/one free junk slot with centered/off-center worm drops and exact
RNG state. M10 zero/one-slot integration verifies bounded allocation and raw/
decoded floor-chain ownership, not the complete death-path RNG sequence.
CSB conversion, full M10 death RNG, allocation reclamation and original captures
remain open.
This is not an original-emulator capture.

## Original contract

ReDMCSB `GROUP.C`, F0186, lines 610-645 performs each table entry in order:

1. Draw the optional-drop decision when the table entry has the random flag.
2. Classify the item and update the weapon-sound selection.
3. Allocate through F0166; continue immediately if allocation fails.
4. Initialize type and cursed state, then draw the destination cell.
5. Move the item through F0267 before processing the next entry.

For PC 3.4's MEDIA016 branch, a centered source draws one random cell;
an off-center source first draws whether to randomize, then conditionally
draws the cell. Neither cell draw occurs after failed allocation.

## Native mismatch

`F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat` in
`src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` resolves every optional
decision and cell before its caller allocates any record. Consequently an
allocation failure still consumes cell RNG and can change subsequent optional
decisions. Successful per-item move side effects are also deferred until after
all drop decisions; their RNG implications require separate verification.

Affected materializers include:

- `orch_drop_creature_fixed_possessions_compat` (M10 death aftermath, now
  converted to the streamed materializer).
- `m11_materialize_creature_fixed_possession_drops` (M11 runtime, now converted
  to the streamed materializer).
- The CSB native runtime materializer calling F0824; it additionally initializes
  a local seed instead of using a demonstrated original shared RNG owner.

The F0186 melee receipt also uses the precomputed resolver. Its payload is not
proof of allocation-failure parity. Existing successful-pool tests do not cover
this contract.

## Required correction and evidence

Use a per-entry resolve/allocate/cell/move protocol, preserving optional-decision
order and the weapon-selection update before allocation. Do not merely count
free records up front: F0166 allocation/reclamation and F0267 side effects are
part of the original sequence.

Cover mandatory and optional worm drops with zero, one and sufficient available
records, centered and off-center cells, exact final RNG state, raw/decoded
ownership and subsequent optional-drop outcomes. Extend to mixed item classes
and allocation reclamation. Compare original-media/emulator behavior separately;
bounded source-shaped fixtures alone cannot establish platform-wide parity.
