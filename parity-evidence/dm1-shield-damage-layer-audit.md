# DM1 shield damage-layer audit

## Source requirements

ReDMCSB `CHAMPION.C` F0313:1354 adds action defense, individual
`ShieldDefense` (including YA), party `ShieldDefense`, and held armour
shield defense to wound defense. F0321:1842-1851 gathers this wound
defense for non-normal attacks. F0321:1879-1883 separately subtracts
party spell/fire shield from the appropriate statistic-adjusted attack.
These are distinct contributors, not interchangeable representations.

Important exception: F0321's magic branch jumps to T0321024 after spell
shield subtraction, skipping the body-defense scale; the fire branch does
not. A body-layer change must not invent YA mitigation for magic damage.

## Current integration gap

Update: the M10/M11 builders now set an explicit runtime body-shield
layer. F0733 and F0733b use it independently of the elemental subtraction
field. The legacy 76-byte serializer rejects split-layer snapshots before
writing; deserialization clears the new runtime fields. Legacy records
retain their old interpretation. The following describes the diagnosed
pre-fix representation; full end-to-end damage evidence is still pending.

Both `m11_build_projectile_defender_champion_snapshot` in
`src/engine/m11_game_view.c` and its M10 counterpart in
`src/memory/memory_tick_orchestrator_pc34_compat.c` put the sum of action,
individual and party shield in `CombatantChampionSnapshot_Compat`'s
`partyShieldDefense` for physical attacks. For magic/fire they instead
put only the matching party spell/fire shield in that field.

The wound-defense baseline builders contain equipment contributions but
do not separately preserve the individual/action/party shield sum.
`memory_combat_pc34_compat.c` uses `partyShieldDefense` both in wound
defense accumulation and in fire/magic attack subtraction. Consequently,
the current snapshot cannot independently express both source layers.
This is a static data-flow finding, not yet an original-emulator damage
comparison or a completed fix.

## Required verification and repair

- Use an original allocated YA potion and original equipment/creature
  metadata; controlled attack inputs and fixed RNG state may live in RAM.
- Compare otherwise identical attacks before YA, during YA and after C72
  expiry, keeping party spell/fire shield independently nonzero.
- Cover physical, sharp, fire and magic through both M10 and live M11
  snapshot builders, with allowed-wound masks exercising F0313.
- Derive expected damage from the source's separate operations, not the
  same snapshot helper being tested. Preserve integer rounding/order.
- Review snapshot serialization and existing fixture contracts before
  adding or separating fields. Do not silently redefine serialized bytes.
- Keep normal/psychic attack exceptions and platform statistic-adjustment
  variants explicit; do not infer complete combat parity from YA expiry.

YA consumption and live C72 expiry have original-media regression evidence;
that evidence does not close this damage-layer gap.

The new `dm1_shield_layers` production test proves deterministic and RNG
wound-defense independence across six slots, legacy round-trip shield
semantics and non-mutating rejection of unrepresentable snapshots. The
integrated rebuild and twenty original-media consumable/death cases pass.
Those cases do not replace the required actual attack comparisons above.

The damage-level primitive regression uses independently calculated values:
raw 128, anti-stat 42, elemental shield 10 and body shield 40 yield fire
damage `((128 - 10) * (130 - 40/2)) >> 6 = 202`; without body shield the
result is 239. Magic is 118 and normal damage is 128. Both deterministic
and RNG-bearing fire paths pass; two existing projectile receipt/handoff
tests also pass. This is still not the original-media live-builder attack
comparison required above.

The existing M11 action/stamina runtime suite now repeats its lightning
impact with either party shield 200 or recipient shield 200. Both produce
86 HP and consume the projectile through AdvanceProjectilesOnce. This
proves a live snapshot/impact connection in a controlled existing fixture,
not original-media fire damage or emulator parity.
