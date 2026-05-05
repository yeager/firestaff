# Firestaff V2 Wave 1 Starter Spell Effects

Source audit anchors:
- ReDMCSB `PROJEXPL.C` `F0213_EXPLOSION_Create` lines 149-166 maps explosion thing constants to explosion type, sound, and fireball/lightning impact handling.
- ReDMCSB `PROJEXPL.C` `F0218_PROJECTILE_HasImpactOccured` lines 560-585 creates poison/fireball/lightning explosion families on impact.
- Firestaff V1 `dm1_v1_projectile_explosion_render_pc34_compat.c` keeps the original projectile/explosion aspect families available for parity.

Contract:
- Master PNGs are authored at exactly 2048x2048, with exact 1024x1024 derived exports.
- V2 overlays are cosmetic bindings only; gameplay damage, timelines, projectile ownership, and spell success remain V1-owned.
- First families: fireball burst, poison/smoke cloud, lightning bolt, and slime/flash fallback.
