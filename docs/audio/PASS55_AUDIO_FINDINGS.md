# Pass 55 — V1 audio / direct M11 marker-call audit

Pass 55 is the bounded follow-up to Passes 53–54 for V1 blocker #15.  Scope is
only source-backed cleanup/audit of non-`EMIT_SOUND_REQUEST` direct
`M11_Audio_EmitMarker(...)` call sites in M11.

No original cadence, looping, prioritization, or overlap claim is made.

---

## 1. What landed

| File | Role |
|------|------|
| `m11_game_view.c` | Converts only source-backed action cues from direct procedural marker calls to mapped DM PC v3.4 sound-event-index emissions |
| `scripts/audit_m11_direct_audio_markers_pass55.py` | Focused source-shape audit that locks converted calls and enumerates remaining direct-marker TODO buckets |
| `parity-evidence/pass55_m11_direct_audio_marker_audit.txt` | Pass 55 audit PASS log |
| `PASS55_AUDIO_FINDINGS.md` | This evidence note |

Converted action cues:

- `WAR CRY` → sound event `17` (`M619_SOUND_WAR_CRY`, SND3 item 707)
- `BLOW HORN` → sound event `18` (`M620_SOUND_BLOW_HORN`, SND3 item 704)
- `SHOOT` → sound event `13` (`M563_SOUND_COMBAT_ATTACK...`, SND3 item 684)
- `THROW` → sound event `13` (`M563_SOUND_COMBAT_ATTACK...`, SND3 item 684)

Source-silent action cues:

- `CALM`, `BRANDISH`, and `CONFUSE` stay silent for PC34: ReDMCSB `MENU.C:1347-1362` only calls `F0064_SOUND_RequestPlay_CPSD(...)` for `WAR CRY` and `BLOW HORN` before routing all five actions through `F0401_MENUS_IsGroupFrightenedByAction(...)`.

These are anchored to the Pass 52 `sound_event_snd3_map_v1.[ch]` table and its
ReDMCSB `DEFS.H` / `DATA.C` source anchors.  The conversion uses the Pass 53
`M11_Audio_EmitSoundIndex(...)` path, so original SND3 assets are queued when
present and procedural marker fallback remains when assets are absent/disabled.

---

## 2. Remaining direct marker calls

This audio source-lock follow-up removes the `FIREBALL` / `DISPELL` / `LIGHTNING` action-time marker fallback after ReDMCSB source audit. Pass 55 now leaves direct marker calls only where the current V1 slice lacks a source-backed sound-index assertion or original runtime capture:

| Bucket | Count | Why still direct |
|--------|------:|------------------|
| Generic non-`EMIT_SOUND_REQUEST` tick emissions | 1 | Existing catch-all procedural cue for movement/door/damage/spell emissions that are not yet original sound-request payloads |
| `INVOKE` action cue | 1 | Random subtype path exists, but exact original action-time sound request timing/index is not captured here |

`FIREBALL` / `DISPELL` / `LIGHTNING` now stay action-time silent because ReDMCSB PC34 `MENU.C:1280-1305` routes them through `F0327_CHAMPION_IsProjectileSpellCast` without `F0064_SOUND_RequestPlay_CPSD`, and `CHAMPION.C:2073-2106` creates the projectile without requesting sound. Later projectile impact sound remains a separate `GROUP.C` concern.

This pass does **not** claim the remaining TODO buckets are original-faithful.  It only prevents accidental overclaiming by making the remaining procedural paths explicit.  It also prevents the now source-silent `CALM` / `BRANDISH` / `CONFUSE` and `FIREBALL` / `DISPELL` / `LIGHTNING` action blocks from regressing to procedural marker fallbacks.

---

## 3. Probe/audit result

```text
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 17 M619_SOUND_WAR_CRY
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 18 M620_SOUND_BLOW_HORN
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 13 M563_SOUND_COMBAT_ATTACK
PASS P55_DIRECT_AUDIO_AUDIT_02 war cry action emits event 17
PASS P55_DIRECT_AUDIO_AUDIT_02 blow horn action emits event 18
PASS P55_DIRECT_AUDIO_AUDIT_03 shoot and throw emit source-backed event 13
PASS P55_DIRECT_AUDIO_AUDIT_04A calm/brandish/confuse stay source-silent
PASS P55_DIRECT_AUDIO_AUDIT_04B fireball/dispell/lightning action cast stays source-silent
PASS P55_DIRECT_AUDIO_AUDIT_04 remaining direct marker calls are documented TODO buckets {'generic_non_sound_request_emission': 1, 'invoke_action_fallback': 1}
PASS P55_DIRECT_AUDIO_AUDIT_05 converted action near T%u: %s SHOOTS
PASS P55_DIRECT_AUDIO_AUDIT_05 converted action near T%u: %s THROWS
PASS P55_DIRECT_AUDIO_AUDIT_SUMMARY 0 failures
```

Rerun regressions:

- Pass 53 runtime SND3 probe: 5/5 PASS
- Pass 54 runtime SONG probe: 6/6 PASS
- Existing M11 audio probe: 8/8 PASS

---

## 4. What Pass 55 does NOT claim

- No original runtime audio capture.
- No SFX/title-music cadence, looping, prioritization, or overlap parity claim.
- No claim that the remaining direct marker TODO buckets are original-faithful.
- No audio for `CALM` / `BRANDISH` / `CONFUSE`; this pass source-locks their absence.
- No distribution of original audio assets.

Pass 55 narrows the trigger-point gap from "remaining direct marker calls
unknown" to "four source-backed action cue emissions converted; FIREBALL/DISPELL/LIGHTNING action-time marker fallback removed as source-silent; two remaining
direct-marker buckets explicitly documented; and CALM/BRANDISH/CONFUSE kept
source-silent under audit."
