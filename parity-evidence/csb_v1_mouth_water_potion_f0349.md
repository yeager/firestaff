# CSB V1 F0349 mouth transactions

ReDMCSB `PANEL.C` `F0349_INVENTORY_ProcessCommand70_ClickOnMouth` is the
authority for this bounded native CSB transaction. Potion type 15 adds 1600
water, capped at 2048, and transforms the same C08 Thing record into potion
type 20 (the empty flask). The flask remains in the leader hand and the
champion statistics panel is marked for redraw.

Firestaff now performs that operation against the selected CSB runtime's real
dungeon record and `CSB_V1_PartyState`. M11 only presents the committed result;
it cannot reinterpret the Thing through the DM1 world mirror. Validation,
vital update, record transformation, redraw flag, and leader-hand retention
form one native transaction. A dead champion, malformed record, disallowed
mouth slot, or any potion type whose complete effect is not yet implemented
fails before mutation.

The deterministic C09 food branch is source-owned as well. Icons 168 through
175 map to ReDMCSB `DUNGEON.C` `G0242` exactly: apple 500, corn 600, bread
650, cheese 820, screamer slice 550, worm round 350, drumstick 990, and dragon
steak 1400. Food is capped at 2048. `F0298` semantics detach the consumed
record with `Next = THING_NONE`, clear the CSB leader hand, remove its weight
from the active leader, and mark load/statistics for redraw. The existing CSB
audio owner receives source sound C08 SWALLOW as an immediate request.

The C09 waterskin branch follows ReDMCSB's dynamic icon and charge rules.
Subtype 1 has icon 8 while empty and icon 9 while charged; an empty skin is
rejected without mutation. A charged skin adds 800 water capped at 2048,
decrements the two-bit `ChargeCount` in place, retains the same Thing in the
leader hand, applies its charge-dependent weight delta, marks load and
statistics for redraw, and requests C08 SWALLOW. Validation completes before
the vital/record/hand transaction begins.

All mouth-admitted C08 potion branches now use the complete F0349 arithmetic.
Power produces the source counter and adjusted-power values; ROS/KU/DANE/NETA
use F0348's 120/150 attenuation and 170 cap, MON restores bounded stamina, EE
uses the original over-maximum mana easing, and VI heals health and wounds
through the retained CSB random seed. Antivenin deletes this champion's C75
events and clears `PoisonEventCount` exactly as F0323 (it does not invent a
`PoisonDose` reset). YA adds its exact defense and C72 expiry; `B.Defense` is
preserved in the event union so stacked shields expire independently. Every
potion becomes C20 while retaining Power, changes held load from 3 to 1, stays
in the leader hand, marks source redraw attributes, and requests C08 SWALLOW.

The focused regression also presents an unsupported Ven potion and proves
that failure preserves the leader hand, champion water, and record bytes.
Unsupported or malformed records, missing RNG ownership, and a full YA event
heap fail before any mutation.

Verification:

- `m11_csb_leader_hand_no_dm1_fallback`
- `csb_v1_m11_launcher_handoff_boundary`
