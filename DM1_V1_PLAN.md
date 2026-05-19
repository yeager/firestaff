# DM1 V1 Remaining Work Plan

## Status: Most core systems implemented
Creature AI, combat, spells, inventory, needs, doors, sensors, save/load all have working implementations. The work is now about ReDMCSB parity correctness and missing edge cases.

## Phase 1: Rendering Parity (visual bugs)
1. **Missing side walls** — occlusion edge cases at D*L2/D*R2
2. **Floating objects** — zone Y coords fixed, verify item cell positioning
3. **Inscriptions hard to read** — font scaling at wall depth, contrast
4. **Swoosh animation** — TITLE.DAT path resolution for boot sequence

## Phase 2: Combat & Creature Parity  
5. **Creature damage formula** — replace simplified baseDmg/3 with ReDMCSB F0230 armor/defense/random
6. **Party attack damage** — verify orchestrator EMIT_DAMAGE_DEALT matches F0231
7. **Creature projectiles** — ranged creatures (vexirk, etc.) create spell projectiles
8. **Creature group splitting** — F0199 group behavior when attacked
9. **Creature death drops** — F0326 item generation on creature death

## Phase 3: Spell & Magic Parity
10. **Projectile lifecycle** — fireball/lightning/poison movement + collision
11. **Spell area effects** — ZO VEN (open door), DES EW (destroy wall), etc.
12. **Potion creation** — ZO_KATH_RA and other flask-based spells
13. **Shield/light spells** — FUL (light), YA (shield), OH (healing)

## Phase 4: World Mechanics
14. **Door sensors complete** — pressure plates, party-on triggers
15. **Pit mechanics** — falling damage, open/close pit sensor chain
16. **Teleporter audit** — scope/rotation/audibility flags
17. **Fake wall types** — imaginary vs open fake walls
18. **Timeline events** — creature spawn, door auto-close, Vi Altar

## Phase 5: Champion Systems
19. **Inventory click-drag** — equip/swap items between slots
20. **Food/water consumption** — eating items, fountain interaction
21. **Skill advancement** — practice-based level-up per ReDMCSB formulas
22. **Champion resurrection** — Vi Altar full cycle

## Phase 6: Audio & Polish
23. **Sound effects** — door sounds, creature footsteps, combat hits
24. **Ambient sounds** — wind, dripping water per dungeon level
25. **Torch light level** — palette dimming as torch burns down

## Phase 7: Touch/Input
26. **Touch screen support** — input abstraction for touch/click
27. **Gesture mapping** — swipe for movement, tap for interact

## Phase 8: Final
28. **Full playthrough verification** — level 0-13 complete
29. **Release build** — v2.1.0 or v3.0.0
