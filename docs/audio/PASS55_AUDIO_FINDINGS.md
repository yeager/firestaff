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

These are anchored to the Pass 52 `sound_event_snd3_map_v1.[ch]` table and its
ReDMCSB `DEFS.H` / `DATA.C` source anchors.  The conversion uses the Pass 53
`M11_Audio_EmitSoundIndex(...)` path, so original SND3 assets are queued when
present and procedural marker fallback remains when assets are absent/disabled.

---

## 2. Remaining direct marker calls

Pass 55 deliberately leaves direct marker calls where the current V1 slice lacks
a source-backed sound-index assertion or original runtime capture:

| Bucket | Count | Why still direct |
|--------|------:|------------------|
| Generic non-`EMIT_SOUND_REQUEST` tick emissions | 1 | Existing catch-all procedural cue for movement/door/damage/spell emissions that are not yet original sound-request payloads |
| `CALM` / `BRANDISH` / `CONFUSE` action fallback | 1 | V1-slice cues only; exact original sound request/index not asserted in this pass |
| `FIREBALL` / `DISPELL` / `LIGHTNING` cast-action cue | 1 | Projectile creation is source-backed, but exact cast-vs-impact sound request timing/index is not captured here |
| `INVOKE` action cue | 1 | Random subtype path exists, but exact original sound request timing/index is not captured here |

This pass does **not** claim those TODO buckets are original-faithful.  It only
prevents accidental overclaiming by making the remaining procedural paths
explicit.

---

## 3. Probe/audit result

```text
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 17 M619_SOUND_WAR_CRY
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 18 M620_SOUND_BLOW_HORN
PASS P55_DIRECT_AUDIO_AUDIT_01 map contains sound event 13 M563_SOUND_COMBAT_ATTACK
PASS P55_DIRECT_AUDIO_AUDIT_02 war cry action emits event 17
PASS P55_DIRECT_AUDIO_AUDIT_02 blow horn action emits event 18
PASS P55_DIRECT_AUDIO_AUDIT_03 shoot and throw emit source-backed event 13
PASS P55_DIRECT_AUDIO_AUDIT_04 remaining direct marker calls are documented TODO buckets {'generic_non_sound_request_emission': 1, 'calm_brandish_confuse_fallback': 1, 'spell_projectile_action_fallback': 1, 'invoke_action_fallback': 1}
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
- No distribution of original audio assets.

Pass 55 narrows the trigger-point gap from "remaining direct marker calls
unknown" to "four source-backed action cue emissions converted; four remaining
direct-marker buckets explicitly documented and guarded by audit."
