# Firestaff DONE — Theron's Quest

## 2026-10-06 — bind all direct targets of JP stage-two `$3114`

Bound the four remaining authentic JP direct-call windows from `$31b3`:
`$5c77` (17 bytes), `$5ca7` (103 bytes, including the alternate entry at
`$5cae`), `$5d0e` (19 bytes), and `$5d32` (15 bytes). Added exact-byte receipts
and mutation rejection at the first and last byte of each span. All four spans
and the authentic US same-offset windows differ; this does not claim regional
behavioral equivalence or semantics for nested calls. Updated the source-locked
HuC6280 listing with all four windows and their SHA-256 values from authentic
TQJP02.bin/TQUS02.bin media. Three independent `unidasm -arch h6280` runs
confirmed the instruction and branch boundaries. The updated
`test_theron_v1_stage2_disassembly_chain` compiled on `trv2` and passed three
loops against authentic US and JP Track 02 data. JP helper semantics and
gameplay behavior remain unproven.

## 2026-10-06 — bind immediate JP callees of `$31b3` targets

Extended the authenticated JP receipt to cover first-RTS byte spans for
`$5c88` (26 bytes), `$5d21` (17), `$53e8` (86), `$54b3` (73), and `$550c` (14),
the immediate callees reached by the four `$31b3` targets. Added exact-byte
receipts and first/last-byte mutation rejection. All five authentic US
same-offset windows differ. The source lock records the HuC6280 disassembly,
exact hashes, and each span's bounds. Also bound the nested `$4f7a` (15 bytes),
`$54a7` (12), `$54fc` (16), and `$53d8` (16) windows. `$4f7a` matches authentic
US bytes; the other eleven newly bound windows differ at the same US offsets.
Followed the conditional `$53e8` branch beyond its first RTS and bound the
contiguous `$543e` (63-byte), `$547d` (27-byte), and `$5498` (15-byte) JP
continuations; all three differ from authentic US at the same offsets.
Three independent authentic-media
extraction/disassembly loops confirmed the new windows. The updated
`test_theron_v1_stage2_disassembly_chain` compiled on `trv2` and passed three
loops against authentic US/JP Track 02 data, including the endpoint mutation
checks. These byte receipts are not semantic or gameplay proof.

## 2026-10-06 — source-bind and capture the stage-two helper call

Corrected the bounded `pce_fast` MPR1 probe to inspect the authentic
`JSR $3a2e` at `$4ed4` (the earlier `$4ec9` location is the function entry;
`$4ed2` lies inside `JSR $4f31`). Added raw-byte assertions for `$4ed4..$4ed6`
to `test_theron_v1_stage2_disassembly_chain`; on `trv2`, this test passed three
loops with authentic US Track 02 MD5 `f23601102138f87c33025877767ebf76` and
JP Track 02 MD5 `b7afb338ad31be1025b53f9aff12d73a`. The complete Mednafen
1.32.1 patch chain also applied to a clean source extraction in three loops;
the PCE Fast HuC6280 object compiled and the emulator relinked.

Authentic JP and US cold-start traces each recorded 32 signature-matched
`$4ed4 → $3a2e` call/target pairs with `MPR1=$f8`, mapping to physical
`$1f1a2e`. The call-site bytes were `20 2e 3a`; each target's first 64 bytes
in the trace exactly matched its same-session 8-KiB PCE Fast BaseRAM snapshot
at offset `$1a2e`. The US and JP target windows are byte-identical
(SHA-256 `4fa8ce8e5012aa6eab9fa2e8b07c60ee60ce21aa2933fcddec3ef61943ace3f2`);
the entire 114-byte `$3a2e..$3a9f` decode window is also byte-identical
(SHA-256 `849e8e9780242358f48aaa51ce3695fc18a2687988e68e61430b5da727872fa0`),
as is the 12-byte `$3965` helper. `test_stage2_runtime_helper_media_source`
now locks the helper and window directly against authentic Track 02 raw BIN
offsets for both regions; it passed three CTest loops on `trv2`. A bounded
byte/dataflow disassembly of the runtime/source window and its `$3965`
pointer-advance helper is recorded in
`docs/source-lock/theron-disassembly/theron-stage2-bytecode-dispatch-table-20261005.md`.
The matching raw bytes and runtime snapshots establish US/JP parity only for
the tested cold-start route, not all platforms or paths. The loader's copy
provenance is not inferred.
Record meanings and the caller's game-level interpretation remain open. Both
replays applied all three scripted controller events, but strict
input-consumption verification blocked because the final event had no later
controller-port read exposing its mask. Raw media, states, and traces remain
outside Git; see `TODO-theron.md` for remaining semantic and input-evidence gaps.

## 2026-10-06 — source-lock the selector root table

Extended the bounded `pce_fast` probe to record the helper's selector, key,
indexed stage-two pointer-table entry, zero-page root pointer, root count, and
result carry/fields for at most 64 lookups. The corrected PCE Fast binary
compiled on `trv2`; the complete Mednafen patch chain applied in three
isolated dry-run loops. Authentic US and JP cold-start runs each yielded 60
lookup/root/result triplets. Both exposed the same selector roots for
selectors `$00..$04`: `$4d86`, `$4dc5`, `$4e04`, `$4e43`, and `$4e82`.
`test_theron_v1_stage2_disassembly_chain` now locks those pointers against the
original Track 02 bytes and passed three CTest loops on `trv2` with authentic
US and JP media. Counts vary by key/runtime and remain intentionally
unlocked. Both captures still fail the strict final controller-event receipt;
this CPU trace does not prove the replay changed game state or assign game
semantics to the lookup results. Raw media and traces remain outside Git.

## 2026-10-06 — capture live helper registers and lookup comparisons

The first search trace exposed a probe bug: PCE Fast keeps X/Y/P in the active
interpreter locals during `RunSub`, so reading `HuCPU.X/Y/P` from the hook
captured stale register copies. Discard those earlier X/Y/P/carry fields; their
direct memory observations (selector roots, candidate bytes, and result
memory) remain usable. The probe now receives `X_local`, `Y_local`, `P_local`,
and lazy `ZNFlags` at the fetch hook. The updated patch chain passed three
isolated application loops; the corrected PCE Fast object and emulator built
on `trv2`.

An authentic US cold-start trace recorded 60 lookup results and 229 candidate
comparisons. In all 60 cases the final candidate byte matched the requested
key, the result carry was clear, and `$37d2` matched the number of candidate
comparisons (one-based). This confirms the successful lookup path for this
bounded cold-start sample only; no miss/exhaustion result or game-level field
meaning is established. All three scripted input events were recorded, but
the strict receipt still blocked because the final event mask was not read by
the controller port. Do not claim that replay changed gameplay state.

## 2026-10-05 — source-lock the stage-two MPR entry window

The authentic US and JP stage-two entry bytes at `$4000` now have a direct
raw-sector regression. The direct prologue writes MPR3, MPR4, MPR5 and MPR6,
then calls `$8000` before continuing. That static byte check alone does not
establish MPR1. The later linked `$4ed4 → $3a2e` runtime receipt supplies MPR1
and the target physical PC for its tested JP cold-start route; see the
2026-10-06 entry above. No universal mapping is inferred.

## 2026-10-05 — bound the `$8000` entry-callee dataflow

Documented the authenticated US Rev. 1 `$8000` pointer/dataflow through
`$45a6`, `$4696`, and `$48fc`, based on the existing paired raw-media receipt.
The routine saves the `$45a6` zero-page pair at `$4c/$4d`, derives pointer
fields, stages bytes read through that pointer, and conditionally calls
`$48fc`. The separately byte-bound `$4696` helper is an unsigned 8-by-8
shift-add multiply returning `$0f:$0e`; no structure-field meanings are
assigned. The receipts are US-only and do not establish JP parity or MPR1 at
`$3a2e`; da65 overlap spans remain byte-authoritative. Verification on trv2:
`test_theron_v1_stage2_disassembly_chain` passed against authentic US and JP
Track 02 files; its `$8000` and `$4696` byte receipts are explicitly US-only.

## 2026-10-05 — source-lock the `$48fc` loop in both editions

Added a raw-sector regression for the bytes from `$48ec` through the RTS at
`$4900` in authentic US and JP Track 02. The byte sequence clears
`$0404/$0405`, performs a borrow-aware decrement of `$01:$00`, loops while
either byte is nonzero, and returns at zero. The
caller-derived pair is left semantically unnamed. The focused stage-two test
passed for both editions on trv2, including the new `stage2_l48fc_countdown`
check.

## 2026-10-05 — add a bounded MPR1 probe for `$3a2e`

Extended the isolated Mednafen trace build with `stage2_mpr1_probe` rows at the
`$4ec9` call site and `$3a2e` target. Each row captures active physical PC,
MPR1, the candidate target physical address, and 64 bytes read from that
physical window. The patch applied and compiled on trv2 from a private copy of
the existing instrumented Mednafen source. An authentic US CUE/System Card
run with a scripted four-second RUN hold produced no probe rows; the input
trace confirms the hold was delivered. This tool addition does not establish
MPR1, a bank identity, or `$3a2e` behavior. The remaining requirement is a run
that actually reaches `$4ec9/$3a2e` and emits the same-call mapping receipt.

## 2026-10-05 — source-lock shared stage-two slot helpers

Extended the ID `$2c` helper family with authentic US and JP raw-sector checks
for `$4ef4`, `$4f11`, and their `$4be2` / `$4c17` caller paths. `$4ef4` stages
`$4ec2` into `$37cc`, copies `$4ec7/$4ec8` to `$37d0/$37d1`, calls `$4f31` and
`$3879`, then clears `$5b` and returns. `$4be2` calls it directly; `$4c17`
reaches it after a carry-clear result from `$4f5e`. `$4f11` selects a pointer
through `$4f31` and writes `$00,$00,$60` through it. `$3879` remains below the
loaded stage-two window, so its behavior and live call conditions remain
unverified.

## 2026-10-05 — decompile the internal ID `$2c` helper path

Extended the ID `$2c` chain from `$4483` through `$4ec9` and `$4f31` using
authentic US and JP Rev. 1 stage-two bytes. `$4ec9` decrements `$5b`, writes
`$4ec2` to `$37cc`, calls `$4f31` and the below-image `$3a2e`, then on the
no-carry path copies `$37ce/$37cf` and `$37d0/$37d1` into the `$4ec3..$4ec8`
fields. Both visible outcomes clear `$5b` and return. `$4f31` indexes the
little-endian table at `$4d7c` by twice `$4d7b` and writes the selected word to
zero-page `$00/$01`. The raw-media test locks both routines in US and JP.
At the time of this static pass, `$3a2e`'s body was unresolved; the
2026-10-06 linked runtime capture now supplies a bounded JP byte/dataflow
decode, while source-image identity, US/JP parity, carry interpretation, and
gameplay meaning remain open. This remains reverse-engineering evidence, not
a gameplay claim.

## 2026-10-05 — root dispatch ID `$11` across overlapping code/data

Followed the authentic ID `$11` dispatch call to `$5e27` (US) / `$5e57`
(JP). The alternate root decodes as `STZ $4f9c`, followed by a seven-byte
HuC6280 TII descriptor, then relative calls to `$5e4d/$5e40` (JP:
`$5e7d/$5e70`) and stores to `$4fd9/$4fda` before returning. The two callees
are locked through their RTS instructions: one writes VDC registers `$02/$03`
and performs a 64-byte TIA; the other copies `$4fdb/$4fdc` to `$4fd5/$4fd6`.
The da65 info map now preserves this alternate root and marks the overlapping
TII descriptor as bytes instead of misreading it as the linear `$5e2b` code.
An authentic US/JP raw-sector test locks the caller and both callees. These
bounded paths contain no direct `$3b33` access, but candidate execution and
indirect effects remain unproven; see `TODO-theron.md`.

## 2026-10-05 — source-lock dispatch ID `$2b` regional caller

Locked the shared `$4653` handler body for table entry `$2b` in US and JP.
It reads stream offsets `+1/+2`, calls `$4b00` and `$4f48`, copies `$4d79/$4d7a`
to `$4fdb/$4fdc`, restores the first stream operand to A, calls `$56af` (US)
or `$5729` (JP), then takes the `$40f9` three-byte cursor step. A raw-media
test locks the complete caller window and the regional target words. This
also roots the shared `$56af/$5729` helper prefix: it multiplies A by two to
index a 16-bit offset table through `$4fd9/$4fda`, adds the base, writes the
result pointer to `$0c/$0d`, and calls `$5e40/$5e70` followed by
`$573f/$57b9`. The rooted `$572c/$57a6` helper swaps the two pointer pairs;
`$573f/$57b9` tests whether `($0c)` is zero and conditionally calls
`$5761/$57db`, which adds the 16-bit value at `($0c)` to `$4fd9/$4fda`, stores
the result at `$0a/$0b`, and advances `$0c/$0d` by two. The seven selector
`$0c` candidates supply A=`$02` at this handoff, so the visible table offset
is `$04`; later helper effects and candidate execution remain unproven.

## 2026-10-05 — trace dispatch ID `$2c` to its external call boundary

Locked the authentic `$4674` target for table index `$2c` in both editions.
Its root calls `$4483`, which stores stream offset `+1` at `$4ec2`, calls
`$4ec9`, copies `$4ec3/$4ec4` into zero-page `$00/$01`, and returns. The caller
then reads offsets `+2/+3/+4` into `$02/$03/$0e`, calls `$3ab7` with A=`$0f`,
and jumps to `$4101` for the fixed five-byte cursor advance. A raw-sector test
locks the `$4483` helper and full `$4674` caller in both editions. The work at
`$4ec9` and `$3ab7` effects remain open in `TODO-theron.md`.

## 2026-10-05 — trace candidate continuations for selectors `$04..$0c`

Extended the authentic US/JP Rev. 1 Track 02 candidate walks beyond their
shared prefix, conditional on each rooted handler and the `$201c` call
returning. Selectors `$04..$0a` and `$0c` reach `$12` at target `+$37`; `$0b`
reaches it at `+$30`. The rooted suffixes expose `$36`, `$11`, `$14/$15`, and
recursive `$41` entries without assigning gameplay meanings.

Resolved the `$41` pointer roots `$7446` US / `$7448` JP: both point into the
same relative stream of eight `$14/$15` pairs, operand values descending
`$07..$00`, followed by `$09`/`RTS`. The `$7464/$7466` roots are its
`$02..$00` suffix, while `$7470/$7472` are its final `$00` pair. Expanded the
da65 byte-table overlap window to `$7445..$7478` and regenerated its tracked
US listing bytes from the authentic payload.

Selector `$0c`'s continuation reaches a rooted `$01` comparison chain over
mutable `$2781`, with seven inline values `$00..$06` and pointer targets
`$686d,$688d,$68ad,$68cd,$68ed,$690d,$692d`. Each target begins with the
matching `$1a` operand then `$13,$2d`, and has a 32-byte cursor path ending
at `$09`/`RTS` if its nested calls and three counter polls return. The rooted
`$2d` handler at `$468f` clears `$3b33`, compares it with the stream operand,
and loops until the counter exceeds that operand; an increment at `$89e7` is
guarded by bit `$20` from the byte popped at `$89e2`. The distinct `$8975`
gate also tests bit `$20`, but static proximity does not prove a shared
producer; the visible epilogue pops a byte previously loaded from `$0000`, not
processor status (`LDA $0000; PHA` at `$895d`). Separate `$3b33`
US clear/wait sites at `$503d/$5048`, `$7539/$753c`, `$7549/$754c`,
`$7733/$7736`, and helper `$88a6` show that the byte has other uses. JP's
`$753x/$773x` roots are two bytes later and its `$50xx` path differs. None is
proven to advance selector `$2d`'s poll; `$88a6` has direct callers at
`$8862,$8877,$88b1,$88d9` in both editions. Their complete rooted routines,
from `$8860,$8875,$88af,$88d7` through their respective `RTS` instructions,
are now source-locked and checked against both raw editions. Selector `$0b`
also reaches the poll.
This is static candidate control flow only: selector execution, poll
completion, the counter's runtime update path, indirect-call targets, and
gameplay meaning remain unproven.

## 2026-10-05 — bound candidate stage-two streams to authentic pointers

Corrected the rooted `$41b9` pointer read to its actual cursor offsets `+1/+2`
after confirming the dispatcher's `CLY`. Re-traced selectors `$00..$03` in
the deinterleaved authentic US and JP Rev. 1 Track 02 payloads and marked the
selector `$00` candidate through its embedded pointer in the source-lock
disassembly. US/JP pointer targets are `$73b2/$73b4` for selector `$00`,
`$78ea/$78ec` for selector `$01`'s `$12` handler, `$73b2/$73b4` for its
`$41` handler, and `$74f2/$74f4` for each of selectors `$02` and `$03`.

The recursive `$73b2/$73b4` bytes form a bounded eight-byte stream: handler
`$1d` reads six operands and advances to `$09`, whose `$4253` target is
`RTS`. The selector `$00` candidate continues through `$41` roots at `$6f23`
and `$6f26`; the latter targets `$7470` US / `$7472` JP. Both streams reach
`$09`/`RTS`. The outer `$2a` at `$6f29` maps to `$4409`; its rooted handler
reads `$1c+1` (`$00` in both regions), indexes `$4b3c` to set `$f8=$03`,
BCD-increments it to `$fc=$04`, sets `$fb=$80/$ff=$83`, calls `$e012`, and
jumps to `$40f5` (which adds two to `$1c`). Since external calls' effects on
`$1c` remain unknown, the eventual outer-stream continuation, external
effects, and command meaning remain unbound. The
US/JP `$78ea/$78ec` targets and `$74f2/$74f4` targets also decode
from their embedded pointers as matching HuC6280 routine bodies ending in
`RTS`. Since `$4319` reaches an indirect jump through runtime vector `$201c`,
the latter address matches are not proof that those routines execute.

Three consecutive sector-by-sector payload regeneration, dispatch/pointer
table, bounded-byte, and `da65` listing checks passed. Dynamic selector choice,
runtime memory mapping, and `$201c` dispatch remain unverified; this is static
address/control-flow evidence only. The earlier `$126c` claim was withdrawn.

## 2026-10-05 — source-lock the stage-two bytecode dispatch table

- ✅ Replaced the linear instruction rendering over `$410d..$41b6` with the
  authentic 85-word indirect-jump table. US and JP Rev. 1 Track 02 BINs have
  identical entries (FNV-1a `7f6a7f04`).
- ✅ The source-disassembly receipt now checks all 85 little-endian targets
  against the listing for both raw BIN editions and requires every target to
  remain within the loaded stage-two window.
- 🔒 This is static address evidence, not bytecode or gameplay semantics.
  Valid stream indices, handler operand contracts, and the authentic
  resource-to-interpreter join remain open in `TODO-theron.md`.

## 2026-10-05 — verify scripted PCE button masks at readback

- ✅ The bounded Mednafen replay verifier now checks port 0's raw PCE input
  mask and the active-low value returned by the selected direction/button
  bank. Same-frame masks are unioned; mixed-bank inputs require a matching
  return value from both banks. Positive and negative fixtures cover exact
  readback, absent bits, wrong SELECT banks, and incomplete simultaneous
  masks.
- ✅ The corrected instrumented PCE Fast build replayed four directions from
  an authentic JP Rev. 1 in-game state. Four event frames were applied and
  read back through SELECT=1 with the expected active-low direction values on
  a non-System-Card poll path. The trace and user media remain private on
  `trv2`.
- 🔒 The capture still produced no command-buffer writes, CD IRQs, or
  authenticated CD-to-RAM receipts and did not identify the active map. This
  verifies input delivery only, not party movement, door use, or Theron parity.
- 🔒 A separate eight-second no-input replay of the same authentic state
  recorded 600 `$203F` writes cycling through four values, versus 2,240 in the
  30-second directional replay. The similar rate is not a movement witness;
  the tuple remains provisional until its original consumer is bound.
- 🔒 A separate authentic JP replay returned the scheduled I/II masks through
  port 0's button bank (`SELECT=0`) but recorded no command-buffer writes,
  CD IRQ, or authenticated CD-to-RAM receipt. It does not identify the active
  map or establish a retail button, door, or T900 result.

## 2026-10-05 — preserve unresolved Track 02 tile family 7

- ✅ Production map loading now preserves authentic Track 02 tile family 7
  as a distinct unresolved square instead of projecting it to a wall. Both
  the read-only movement preview and original-command host route block entry
  without mutating world state until a Theron runtime consumer establishes
  its behavior. `THERON_SQUARE_IS_PASSABLE` likewise keeps it closed.
- ✅ On trv2, `theron_v1_track02_dungeon_loader` verified every authentic US
  and JP source tile byte against the loaded maps, counted 82 US and 78 JP
  family-7 occurrences, and checked one real floor-approachable tile per
  region through both movement routes. The loader, playability, and rendering
  CTests each passed three consecutive runs. These checks establish exact
  source preservation and Firestaff fail-closed consistency, not retail
  collision or visual semantics.

## 2026-10-05 — clear warning-only ambiguity in Theron sources

Separated the null-output guard from subsequent receipt initialization in the
trace-bundle selector, and made both arms of the runtime-object-word
conditional explicitly `uint16_t`. These preserve the existing branches and
values while removing misleading-indentation and signedness warnings from a
fresh Release build; no game behavior or source-evidence gate changed.

On `trv2`, a clean Release configuration built the Theron dungeon loader,
combat-mechanics test, and authentic-map playability probe with `-j1`. The
focused CTest selection passed three consecutive loops; the authentic US/JP
loader and playability checks retained their source-backed coverage and
fail-closed behavior. This verifies those bounded routes only, not complete
Theron parity.

## 2026-10-05 — accept authentic PCE Fast RAM-consumer traces

The bounded Mednafen main-RAM consumer parser now accepts both its original
PCE source header and the PCE Fast consumer-read source header. Its optional
instrumentation suffix accepts underscore-separated keys such as
`reader_code_bytes`, matching the fields emitted by the PCE Fast capture patch;
that field also accepts its explicit `unavailable` sentinel at a bank edge. A
parser fixture covers the PCE Fast header, both code-byte forms, and the
register snapshot without relaxing the existing address or provenance checks.

Verification on `trv2`: the parser target built in an isolated Release build
with `-j1`. Its CTest passed three consecutive runs while parsing the retained
authentic 3,354-read PCE Fast capture (MD5
`93351874cac5dea14bcf80192bbda6c1`). The receipt contained 548 reads in
`$2600-$27FF`, but semantic publication remained blocked. This capture still
does not establish active gameplay, map identity, or retail consumer semantics.

## 2026-10-05 — keep the instrumented PCE Fast startup path safe

The optional Mednafen trace-hook patch now returns from `UpdateCoreHooks()`
when the selected core has no debugger interface, as is the case for the
PCE Fast core. It also avoids calling the same PCE Fast consumer-read patch
twice in the serialized patch sequence. On `trv2`, the complete Theron
Mednafen patch set applied successfully to a fresh isolated source copy with
the patch-only gate; the focused patch dry-run, live-capture script contract
test, shell syntax checks, and `git diff --check` passed. The corrected
instrumented binary was previously rebuilt incrementally and reached SDL
video initialization without the prior startup crash.

This repairs research instrumentation only. It does not prove PCE Fast
gameplay input delivery or the unresolved join between authentic startup RAM
and the active dungeon map; see `TODO-theron.md`.

## 2026-10-04 — repeat authentic regional dungeon-source checks on trv2

Built `test_theron_v1_track02_dungeon_map`, `test_theron_v1_track02_door`,
and `test_theron_v1_track02_dungeon_loader` from the pushed source revision in
an isolated `/dev/shm` build on `trv2`. Against the installed original US and
JP Track 02 media, the US/JP map and door/teleporter CTests and the full
dungeon-loader CTest all passed three consecutive repetitions (15 test
instances total). The raw game media remained in the user's data directory
and was not copied into Git or the repository worktree.

This re-verifies bounded source-data decoding and loader stability for these
regions. It does not establish the original startup map/pose consumer, runtime
door or teleporter behavior, visuals, or complete Theron parity.

## 2026-10-04 — trace direct PCE Fast startup-state writes without synthetic data

The opt-in Mednafen 1.32.1 reference build now records changes to the six
startup RAM offsets already under investigation, including direct HuC6280
zero-page stores when MPR1 maps BaseRAM; previously those stores bypassed the
BaseRAM write callbacks. Records include old/new bytes and the instruction's
logical and physical PC. No-op stores are omitted, and a per-offset sample
budget prevents the frequently changing `$2031` field from crowding out other
offsets. A private authentic JP Rev. 1 New Game run verified the direct-store
instrumentation against an 8 KiB final RAM snapshot. The complete patch-only
gate, full serialized Mednafen build, binary module check, and focused capture
script regression passed on `trv2`.

This completes only the emulator research hook, not Theron startup parity.
`$2031`'s final writer and its active-level meaning remain unresolved; see the
matching open item in `TODO-theron.md`. The instrumentation is opt-in and does
not add an emulator or BIOS dependency to Firestaff runtime.

## 2026-10-03 — parser fallback pose cannot satisfy exact-level readiness

`theron_v1_startup_runtime_capture_all_dungeon_routes()` still records a
validated candidate route when party placement agrees with the parsed
first-floor candidate, but it increments `semantic_level_count` only when
`start_pose_provenance` is `THERON_START_POSE_PROVENANCE_RUNTIME_CAPTURE`.
The regression keeps the sparse route receipt valid while asserting one
captured route, zero exact semantic levels, and no exact-level readiness.
This closes a false readiness path; it does not establish a retail spawn or
active-map join. The `theron_v1_startup_save_resume_pc34` CTest passed four
consecutive runs on `trv2`.

## 2026-10-03 — preserve the authentic Akutuba pose/map ambiguity

The JP Rev. 1 real-media map regression now locks the source bytes at
Akutuba `(2,3)`: map 0 is open (`0x20`), while map 2 is a wall (`0x00`). This
is the coordinate tuple in the fresh authentic Akutuba `pce_fast` RAM capture,
but that capture does not identify its active map. The Drator-only
interpretation of `$2031=02` would select JP map 2 and conflict with the
observed in-dungeon view, so the tuple is not promoted as an Akutuba spawn.

The US/JP authentic full-dungeon loader regression also asserts that every
source-loaded map retains `THERON_START_POSE_PROVENANCE_FIRST_FLOOR_FALLBACK`.
Real map admission therefore cannot be mistaken for runtime start-pose
provenance. On `trv2`, a fresh isolated Release build of
`test_theron_v1_track02_dungeon_map` completed with `-j1`; the JP real-media
map test and the combined US/JP dungeon-loader test each passed three
consecutive CTest runs against installed hash-verified media. This records the
ambiguity and protects the fallback boundary; it does not resolve the active
map pointer or change production pose selection. See
`docs/source-lock/theron-pce-fast-akutuba-ram-capture-2026-10-03.md` and
`TODO-theron.md`.

## 2026-10-03 — capture a fresh authentic Ak-Tu-Ba gameplay session

Built the instrumented Mednafen 1.32.1 `pce_fast` module on `trv2` with an
opt-in, close-time raw main-RAM snapshot hook. The hook writes exactly 8 KiB
from the non-SGX PCE main RAM to a caller-selected path and does not modify
emulated memory. `FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_SNAPSHOT_SUPPORT=1`
enables that module in `build_mednafen_theron_irq2_trace.sh`; the default
instrumented build remains `pce`-only. The private emulator profile used the
authentic JP Rev. 1 CUE and System Card. Holding the mapped RUN key (Return)
for four seconds reached the real Theron title menu; Space then followed
FILE_1 and the Ak-Tu-Ba route into the first-person dungeon view.

The fresh-run RAM snapshot contains `$203F-$2041 = 01 02 03`, and its 8 KiB
size and digest are recorded in the private trv2 evidence directory, not in
Git. The JP Track 02 image passed its expected MD5 gate. This is authentic
runtime evidence for the captured bytes and route, but the same-session
level/bank-to-map-cell join is still missing, so production start-pose
selection remains provisional. The new run's native BRAM is 2 KiB and matches
the known empty menu-only image; it is not a campaign save. Separately,
Mednafen's `.mca` autosave was round-tripped twice from an existing authentic
gameplay state in an isolated profile. That proves emulator-state persistence,
not new native savegame progress.

The patch dry-run against Mednafen 1.32.1 source, raw-RAM hook checks,
unsupported-option rejection, and Bash syntax checks passed in three loops.
The full instrumented build completed on `trv2` with `-j1`, and the opt-in
capture produced exactly 8192 bytes. Fresh-start route screenshots and raw
capture remain outside Git. Native savegame support and the source-bound pose
join remain open in `TODO-theron.md`.

## 2026-10-03 — retain provenance for provisional level-start poses

`Theron_V1_Level` now labels generic-parser first-floor/default-North values
with `THERON_START_POSE_PROVENANCE_FIRST_FLOOR_FALLBACK`; explicit synthetic
room builders use a separate fixture value. The level-header API comment and
runtime helper comments no longer describe the inferred candidate as a proven
retail spawn. Corrected the stale Track 02 source-lock claim that the removed
interior-floor/East selector proved `(2,1,EAST)`. The runtime pose is unchanged
and remains provisional until an original capture binds coordinates and
direction to the authenticated map.

On trv2, the Release build completed with `-j1`. The mechanics-playability,
Track 02 level-handoff, and startup-real-asset-receipt CTests each passed three
consecutive repetitions against the installed authentic US/JP media. The
capture route can retain `$203F-$2041`, but available instrumented Mednafen
builds expose only `pce`; the visible JP gameplay state uses `pce_fast` and is
not loadable by those builds. Initial-pose runtime parity therefore remains
open in `TODO-theron.md`.

## 2026-10-03 — all captured movement commands across authentic campaign maps

Expanded `test_real_campaign_movement()` in the authentic US/JP mechanics
probe. Across all seven Track 02 dungeons, the probe selects existing
floor-to-floor and floor-to-wall neighbors from the source-header-verified
real maps and tests command types `$03`–`$06`. Each command begins from a copy
of the loaded world; successful routes must reach the same real floor with
facing preserved, while wall routes must remain blocked without changing
position or facing. The command mapping is bounded by the authenticated
`THQUEST.ASM` queue/consumer capture: `$D3B0..$D3CB` dispatch and `$CD87` move
route; see `docs/source-lock/theron-original-forward-command-capture-2026-08-21.md`
lines 18–61. This does not claim an original-runtime position join or broader
retail collision parity.

On `trv2`, a fresh isolated Release build of
`firestaff_theron_v1_mechanics_playability_probe` completed with `-j1`.
`theron_v1_mechanics_playability` passed three consecutive CTest runs against
the installed authentic US and JP Track 02 BINs. The direct probe reported
215 passed, zero failed and zero skipped. Each regional pass loaded 34 levels
and exercised all four commands on each available floor route (120 routes
across 30 levels) and each available wall route (124 blocked attempts across
31 levels). This remains host-path verification on authentic map data, not an
original-game capture.

## 2026-10-03 — report creature-spawn source checks per region

- The Track 02 spawn regression now accepts `us` or `jp` and checks only that
  edition's authentic raw BIN. It verifies the exact edition MD5 before the
  spawn-table and source-consumer assertions. A missing default BIN skips only
  that region; an unreadable explicit override and a wrong-edition BIN fail.
- CTest reports independent `theron_v1_track02_creature_spawn_us_real_media`
  and `_jp_real_media` cases with `theron;real-media;regional;no-synthetic`
  labels. On trv2, both passed three consecutive loops against the installed
  authentic US and JP Track 02 BINs. A US BIN presented as JP failed its hash
  gate, explicit missing media failed, and absent default media skipped each
  selected region independently.
- This proves regional source-byte admission and the recorded data contracts;
  it does not prove retail runtime creature spawning, category semantics, or
  combat-stat parity.

## 2026-10-03 — preserve distinct authentic Mednafen input hold durations

- The host-input capture sequence accepts a matching per-key hold list. This
  lets a four-second RUN startup press be followed by short one-second menu
  presses without holding every key equally long. Invalid lists, lists without
  a sequence, and length mismatches fail before the emulator starts.
- The host delivery uses each resolved hold duration for both X11 and Quartz
  key-down/up pairs and records the requested list in the capture receipt.
  The capture-script regression passed three consecutive loops; this verifies
  capture-tool behavior only, not game-side input consumption or gameplay.

## 2026-10-03 — authenticate the shared UI glyph bank in both regions

- The source-only 120-glyph 8×6 viewport bank now has a real-media regression
  for both original editions: US Track 02 UD `0x09A000` and JP Rev. 1 Track 02
  UD `0x099800`. Both source spans match all 720 checked-in bytes after the
  Track 02 hash and raw-sector mapping are verified.
- On trv2, the font, quest-item-name, production text-gate and seven-dungeon
  tests passed three consecutive loops against the authentic US and JP BINs.
  The unavailable JP CUE ISO projection was explicitly skipped.
- US and JP font checks are independent CTest entries; an unavailable region
  skips only its own test. Direct missing-US and missing-JP media probes each
  returned the configured skip code independently.
- Only the common Latin/UI glyph bank is verified. The viewport/font module is
  excluded from the production library; JP kana, original text consumption,
  and full rendering parity remain open.

## 2026-10-02 — keep unbound Track 02 teleporter sound mapping closed

- The authenticated packed-coordinate transition still commits its verified
  destination, but no longer requests the generic `THERON_SOUND_TELEPORT`
  sample. The `THQUEST.ASM:T600` route authenticates the coordinate handoff;
  available ADPCM evidence proves transport only, not event/sample ownership.
- The legacy object-ID compatibility route also no longer requests that
  unbound sample, so fixture behavior cannot imply original audio semantics.
- This does not claim that original Theron teleporters are silent. Their
  source-owned sound behavior remains open until the authentic event and sample
  route are bound.
- On trv2, the affected Theron targets built successfully. The authentic
  US/JP Track 02 dungeon-loader test, the production sound gate, and the
  teleporter-chain regression passed three consecutive loops. This verifies
  Firestaff's current destination handling and blocked sound boundary, not
  original-runtime audio parity.

## 2026-10-02 — exercise authentic teleporter movement-command routes

- The US/JP real-media loader regression now verifies active-to-closed
  coordinate links through an adjacent floor approach, the read-only movement
  preview, and Firestaff's original movement-command API on a cloned world.
  It checks the committed destination level and party pose without mutating
  the authentic census world.
- Three consecutive trv2 loops passed against the authentic US and JP Track 02
  files. Each edition exposed eight such routes in this case; unavailable JP
  ISO/CUE projection subchecks were skipped and are not claimed as verified.
- This is test coverage of Firestaff's own path and data, not evidence that the
  original game consumes these records identically. Scope, rotation,
  absolute-facing, sound, and original gameplay consumer evidence remain open
  in TODO.

## 2026-10-02 — census active authentic teleporter metadata

- The real-media loader regression now counts each open teleporter occurrence
  from the hash-verified US and JP Rev. 1 Track 02 files across all seven
  dungeon banks. Both editions contain 170 active occurrences and the same 25
  `(scope, rotation, absolute, sound)` combinations. The test locks every
  tuple count and reads only the local authentic files on trv2; it prints no
  game-data bytes.
- The ordinary Firestaff movement regression can exercise 41 of those routes
  per region, each with a floor approach and a floor or closed-pad terminal.
  Those routes cover 15 metadata combinations, whose exact counts are also
  locked. This narrows potential original-runtime capture cases but proves no
  teleporter-field semantics.
- The targeted loader CTest passed three consecutive loops on trv2; direct
  execution also exited 0. Its optional JP ISO-stub and CUE-projection
  subchecks were unavailable and skipped; both Track 02 BIN paths were present
  and loaded.
- This is an inventory of source records and the map OPEN gate only. It does
  not prove the party/item scope, facing, absolute-rotation, or sound behavior;
  those semantics remain open in TODO.

## 2026-10-02 — keep restored campaign and quest-item state independent

- A saved quest-item mask now restores only its provisional item bits and
  marks only its canonical current stage `IN_PROGRESS`. It cannot reconstruct
  dungeon completion or unlock later stages; the separately authenticated
  `$267C` campaign byte projects bits
  0–5 into stage state, without treating bit 6 as Demon completion.
- World deserialization now rebuilds progression from that campaign byte and
  derives the exit-complete flag from the same bounded projection. Exit
  execution also rejects a progression/world dungeon mismatch.
- Legacy snapshots without the campaign byte retain only their canonical
  current stage as `IN_PROGRESS` so the saved world can resume. This fallback
  does not complete or unlock other stages from the old host item mask.
- On the rebased `main`, trv2 built the Theron and M12 libraries plus the
  targeted test executables. Twelve progression, serialization, SRM, startup,
  chapter-marker, and real-media Track 02 loader tests passed in three
  consecutive loops; the authentic loader test ran without a media skip. This
  verifies Firestaff's bounded restore and loader paths, not full campaign
  parity. The original T900 pickup and final-stage completion consumers remain
  open in TODO.

## 2026-10-01 — expose authentic regional skill-rank source records

- US and JP Rev. 1 now expose bounds-checked raw accessors for all 15 authentic
  skill-rank records, preserving their bytes without NUL terminators. The six
  `0x60`–`0x65` prefix bytes are available as data in both editions; this does
  not infer glyph rendering or bind an original rank-display consumer.
- The regional real-media test compares every returned byte and record length
  against hash-verified Track 02, and the API regression covers missing outputs
  and unbound slot 15. Original in-game presentation remains open in TODO.

## 2026-10-01 — keep coordinate-teleporter preview consistent with authentic movement

- The read-only movement query now follows Track 02 coordinate-link records,
  validates loaded destination bounds, rejects wall/cyclic/incomplete chains,
  and treats an authenticated closed arrival pad as terminal. It does not
  mutate party or transition state. The rule is bounded to the source-locked
  destination-tile loop in `theron-runtime-spawn-capture.md:466-477`.
- The authentic US/JP corpus now checks that preview and Firestaff's mutating
  original-command API agree for every active coordinate link with an adjacent
  authentic floor:
  91 cases per region. Both regions produce 45 `TELEPORT` and 46 `BLOCKED`
  results, including 42 direct wall targets and four chains ending at a wall.
  The existing world-state hash confirms the query does not mutate the state
  it covers. The cloned mutating original-command path and preview agree even
  for special-square arrivals, but those arrivals remain deferred as game
  semantics.
- On trv2, the authentic dungeon-loader CTest passed three consecutive runs.
  Combat mechanics, teleporter chain and mechanics hardening probes passed.
  The cross-route probe exposed stale assertions for queued transitions and
  the obsolete MOVESENS citation; those assertions now match committed
  transitions and the Theron Track 02 source lock. The five-test focused
  selection then passed three repetitions. This is preview/mutator consistency,
  not an original runtime consumer capture; full Theron parity remains open.

## 2026-09-30 — census authentic active-link teleporter chains

- Extended the real-media movement loop to retain the active-chain hop count
  even when a route has no supported terminal, and to distinguish special
  square terminals from invalid/cyclic chains. The original runtime reference
  for open-pad re-entry and destination-tile handling is recorded in
  `docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md:466-477`;
  the host iteration cap remains a fail-closed guard, not original behavior.
- Hash-verified US and JP Track 02 each contain 14 active-chain roots across
  their seven banks. In each region, four chains with a floor approach end at
  walls and are rejected without party or transition mutation; one chain with
  a floor approach ends at a special square and remains deferred; nine chains
  have no adjacent floor approach. No unresolved chain with a direct floor
  approach appears in either current retail BIN.
- The isolated target built on trv2 with `-j1`; the authentic dungeon-loader
  CTest passed three repetitions with `-j2`, and direct US/JP executions
  printed the same census. This is source-data and fail-closed test coverage,
  not proof of special-square arrivals or the original in-game consumer.

## 2026-09-30 — exercise authentic coordinate-teleporter movement corpus

- The real-media dungeon-loader test now finds active coordinate-linked pads
  across all seven US and JP dungeon banks and drives every matching route
  that has an adjacent floor approach and an ordinary-floor destination through
  the original turn/forward commands. It asserts the committed destination
  level and pose and that no transition remains pending.
- The installed, hash-verified Track 02 files yielded 41 committed routes per
  region, including four cross-level routes and eight closed-pad terminal
  arrivals per region. Four additional active-to-active chains per region with
  an ordinary-floor approach terminate at a wall; the original movement
  command blocks them without changing the party pose or transition state.
  Trv2 built the target serially, and the focused real-media CTest passed three
  consecutive runs. Direct execution passed both regional route loops.
- This does not prove the remaining active-link chains, direct wall-target
  policy beyond the source-locked reject gate, special-square arrivals, pads
  without a floor approach, or the full runtime consumer. Complete Theron
  parity remains open.

## 2026-09-30 — commit the authentic coordinate-teleporter movement handoff

- Coordinate-linked Track 02 movement now commits the transition prepared by
  the source-data resolver through the existing transition executor. If the
  handoff is absent, malformed, or cannot commit, movement fails closed and
  restores the previous level, party pose, and transition fields.
- The authentic US AKUTUBA M0 regression starts at the captured `(1,0,north)`
  pose, turns left, and enters the real active `(0,0)` pad through the original
  movement command. It verifies arrival at `(2,3)` on M0, a real floor square
  with no endpoint object record. On trv2, the target built with `-j1`, the
  focused CTest passed three repetitions with `-j2`, and the direct
  real-media test confirmed the route.
- Scope is limited to this authenticated US route and the transition-state
  invariant. It does not establish JP movement, chained or wall-target
  outcomes, stairs, or complete Theron/platform parity; those remain open in
  `TODO-theron.md`.

## 2026-09-30 — guard campaign completion against fabricated quest items

- Extended the real-media M11 Continue test to assert that the authenticated
  Akutuba-complete Backup RAM byte restores campaign bit 0 and marks Akutuba's
  progression state complete while leaving `quest_items_collected` at zero and
  `quest_complete` false. This checks the existing campaign-mask projection
  against the authentic artifact; it does not claim an original quest-item
  pickup consumer.
- Built the test translation unit with `-Wall -Wextra -Werror` on trv2 and ran
  it against hash-verified US Track 02 plus the original 2 KiB Backup RAM
  capture. The Continue test passed. `TODO-theron.md` retains the outstanding
  original pickup and gameplay-consumer gaps.

## 2026-09-30 — expose authenticated JP skill-rank display text

- Added a bounds-checked JP rank-name accessor for the 15 visible text records
  shared by the hash-authenticated US and JP Track 02 binaries. Regional media
  verification binds the JP accessor's text to each retail record while
  preserving the six custom rank-prefix glyph bytes as separate source data.
- This is a data accessor only. It does not establish the rank-icon renderer,
  an in-game display consumer, or a name for progression slot 15.

## 2026-09-30 — remove unsupported semantics from two raw-data APIs

- Renamed the 64-word accessor from “experience threshold” to `raw_word` and
  the preceding 16-word accessor from “class base” to `prefix_words`. Updated
  the tests and the reverse-engineering category; no production consumer used
  either API.
- Extended each authentic US/JP region test to compare the 16 preceding words
  with the source array, in addition to verifying the byte context and all 64
  following words. On trv2, both data-free tests and all three hash-verified
  real-media tests passed (5/5 total).
- The source meanings and original consumers remain unknown; `TODO-theron.md`
  records that gap explicitly.

## 2026-09-30 — bind a regional 64-word source block to authentic media

- Added real-media CTests for US and JP Rev. 1 Track 02 at their distinct
  logical offsets (`0x1DA890` and `0x1DA0BC`). Each verifies all 64 words
  against the existing source array and checks the preceding 32 bytes; a third
  test verifies the complete 160-byte spans are identical across editions.
- On trv2, the data-free regression and all three hash-verified real-media
  tests passed. An explicit unreadable media override failed closed, while
  absent default media returned CTest's configured skip code.
- This proves only byte identity and source-array agreement. The block's
  semantics and original runtime consumer remain unverified and are recorded
  in `TODO-theron.md`.

## 2026-09-30 — authenticate JP skill-rank source records

- Added a test-only raw JP Rev. 1 rank-record reference that preserves the six
  original prefix glyph bytes and all 15 NUL-terminated payloads without
  linking this catalog into the runtime or assigning UI behavior to it.
- A direct comparison of hash-verified US and JP Track 02 BINs found the US
  records at UD `0x1C9B6B` and JP records at UD `0x89333`; all 15 payloads are
  byte-identical (134 bytes including terminators). The source-offset note
  also corrects an older claim of 16 name records: progression slot 15 has no
  verified name record.
- Separate CTests now verify each authentic edition independently. This binds
  source bytes only; original rank-icon rendering and an in-game display
  consumer remain unverified.

## 2026-09-30 — keep consumer-trace test files in task scratch

- The focused main-RAM consumer-trace CTest now requires an explicit
  task-specific `TMPDIR` instead of falling back to `/tmp`. Its three fixture
  writers also close the file after a failed write before removing it.
- The target built on trv2 with `-j1`; its parser-only CTest passed against the
  existing authentic US Drator main-RAM-consumer trace. The task `TMPDIR` was
  empty after the run. A negative run with `TMPDIR` unset failed closed with
  the expected diagnostic instead of using `/tmp`. This is test-harness
  hygiene only; it proves no new gameplay semantics or Theron parity.

## 2026-09-30 — rebuild original-runtime capture instrument on trv2

- Built the project-patched Mednafen 1.32.1 from its official source archive
  in a fresh task-specific trv2 workspace. The archive SHA-256 matches the
  official release value; the resulting binary SHA-256 is
  `199df986d425a97c1603d49142537c6e95c7fbc3626f0b88fd2b14a1276a7595`.
- The build used one job. All eight required Firestaff runtime markers are
  present, and the live-capture, controller-patch, and palette-patch checks
  pass against the clean source tree. Fixed the palette-patch dry-run test to
  use `git apply --recount`, matching the supported build script's handling
  of the patch's historical hunk counts.
- No original-runtime capture was run: the authentic System Card is absent
  from the trv2 Mednafen and Theron data directories. This verifies the
  capture tool only, not game behavior or Theron parity. The source, binary,
  and build artifacts remain outside Git in the isolated task workspace.

## 2026-09-30 — complete registered Theron test selection on trv2

- Built the missing Theron-labeled test executables and the `firestaff`
  application in the isolated trv2 build tree, using sequential `-j1` builds.
- Ran all 75 Theron-labeled CTests with `-j2`: 67 executed and passed; eight
  skipped because their optional operator-owned capture or media artifacts
  were not staged. Zero tests failed. Authentic US/JP raw BIN, CUE, JP 7z,
  US CloneCD startup routes, Track 02 loaders, and BRAM Continue tests passed.
- The skipped original-runtime capture cases remain missing evidence; this
  suite result does not establish complete gameplay or campaign parity.

## 2026-09-30 — verify retrieval text source ordinals byte-for-byte

- The production chapter-marker gate now compares all seven decoded retrieval
  records, in ordinal order, with their exact byte slices in the authenticated
  regional Track 02 source span. It also checks the selector provenance fields
  and region-specific record framing.
- The updated test target built on `trv2` and passed against the staged
  authentic US and JP Track 02 media.
- This proves ordinal-to-retail-text source binding only. It does not prove
  pickup identity, item possession, T900 state, or campaign completion.

## 2026-09-30 — separate campaign completion from quest-item collection

- Added a campaign-mask projection that changes dungeon stage state without
  writing `quest_items_collected`. The authenticated `$267C` bits 0–5 mark
  their captured dungeon completions; bit 6 remains raw and cannot complete
  Demon. Host item helpers now record provisional bits only, and stage advance
  no longer manufactures dungeon or quest completion.
- Startup exit handling requires the source campaign completion token. The
  chapter marker can still report saved quest-item bits and their authentic
  regional names, but it does not label the quest complete without the final
  completion state.
- Fresh Linux build on `trv2` succeeded. Nine focused CTests passed 9/9,
  including the real US/JP Track 02 dungeon-loader test and the production
  retrieval-name gate.
- This is a fail-closed regression fix, not proof of T900 pickup/retrieval,
  Demon completion, or complete Theron gameplay; those gaps remain in TODO.

## 2026-09-30 — classify scripted controller-read witnesses

- The scripted-input verifier now reports the authentic CPU PC of the first
  post-event controller read and counts reads at observed System Card poll
  PCs separately from other PCs. `game_or_non_system_card_poll_boundary` is
  explicitly `not_observed` when the only witness is `$E4B4/$E4B7/$E4C5/$E4C8`.
- Regression coverage passes for both a generic non-System-Card witness and a
  BIOS-only `$E4C8` witness. This is evidence-gate precision only; it does not
  claim a Theron title/menu response.

## 2026-09-30 — census stair-hosted party actuators in authentic regions

- Extended `theron_v1_track02_dungeon_loader` to enumerate floor-party actuator
  occurrences hosted on source-authenticated stair tiles. It prints each raw
  eight-byte record, linked reference, map location and raw stair tile, then
  asserts eight records for each retail region.
- The US Track 02 (`f23601102138f87c33025877767ebf76`) and JP Track 02
  (`b7afb338ad31be1025b53f9aff12d73a`) real-media run passed. All eight
  occurrences match byte-for-byte across editions. Three records are linked
  at one Drator stair square; the other five records occupy four more squares.
- This narrows capture targets and validates a source census only. It does not
  establish stair traversal, destination, party arrival pose, or the original
  actuator consumer; those remain open in TODO.

## 2026-09-30 — remove unsupported Theron skill-rank labels

- The skill-name API now shares the source-backed champion string table instead
  of carrying a divergent duplicate. It exposes the 15 null-terminated US
  Track 02 text records; ranks 8–13 are represented as `MASTER` while a
  real-media test verifies their distinct `0x60`–`0x65` prefix bytes. Removed
  the extra duplicated `ARCHMASTER` guard label at index 15 and the invented
  printable `a`–`e` prefixes.
- The new real-media CTest authenticates the US BIN by MD5 and checks all 15
  records against UD `0x1C9B6B`. The skill-name, champion-string, and real-media
  tests pass 3/3 against staged original US media.
- The 64-entry experience-threshold table does not prove a display-name record
  for progression slot 15. JP rank text/glyph data and original rank-icon
  rendering remain unverified; those paths stay fail-closed/unclaimed.

## 2026-09-29 — verify all US dungeon-story literals against retail bytes

- Added `theron_v1_track02_dungeon_lore_real_media`, which first verifies the
  authentic US Track 02 MD5 and then compares each of the seven embedded story
  records, including control bytes and padding, byte-for-byte with its
  documented user-data offset and length. A real-media run exposed one missing
  padding space in the Formic story; the literal now matches the retail bytes.
- The real-media test and the existing lore contract test pass 2/2 against the
  locally staged authentic US image. This verifies the US source strings only;
  it does not implement JP story selection, a dynamic Track 02 story consumer,
  or original text presentation.

## 2026-09-29 — source-bound retrieval-name fallback in production chapter marker

- The production chapter marker now falls back to the selected US Track 02
  retrieval record when that dungeon's item-name source is not bound. It
  accepts only the authenticated source framing (`05 03`, `THERON has
  retrieved`, the source `01` fragment separator, `the <name>.`) and rejects
  malformed or non-ASCII display bytes. JP retrieval records remain raw until
  their control bytes and Shift-JIS glyph path are proven for host rendering.
- Extended the production real-media gate to remove the already-bound US item
  name table and assert that the retrieval record alone displays “Shield
  Defiant”; the same test continues checking authentic US and all seven JP
  item-name paths. The focused production CTest passes 1/1 against locally
  staged original media.
- This verifies launcher text sourcing only. It does not prove a pickup,
  retrieval event, quest-bit mutation, or full regional UI parity.

## 2026-09-29 — authentic JP retrieval-name fallback

- The production chapter marker now extracts the artifact field between the
  source's `81 8F` separators in each authenticated JP retrieval record. It
  validates the retail `81 96 ... 81 97` record frame, strictly validates the
  extracted Shift-JIS bytes, and converts them through the existing CP932
  converter. This preserves the original retrieval-message spellings rather
  than substituting the distinct parallel JP item-name spellings.
- The real-media production gate removes each US and JP item-name bank in turn
  and verifies the corresponding retrieval fallback for all seven retail
  records; it also corrupts each record's opening frame and verifies
  fail-closed output. The direct test passed against staged US and JP media,
  and the focused retrieval/name/dungeon-loader CTest loop passed 3/3.
- Rebuilt the production marker, story, and real-data quest-name targets in an
  isolated `trv2` checkout and ran the four directly relevant CTests there;
  all 4/4 passed. Direct executions reported authentic US and JP quest-artifact
  names and the seven JP retrieval fallback records, so neither media path
  skipped.
- This proves only the launcher's bounded display-name projection, not the
  original retrieval UI, item pickup, or T900 event consumer.

## 2026-09-29 — require same-session FIFO provenance for consumer markers

- The high-level original-consumer marker verifier now requires every
  qualifying palette, non-startup-level, and object-table read to match a
  unique preceding FIFO-origin receipt in the same trace: FIFO sequence,
  generation, source LBA/offset, logical destination, physical RAM cell, and
  byte value must all agree; the reader PC must also be in main RAM. Independent
  receipt and consumer counts can no longer jointly authorize an authenticated
  marker.
- Expanded and registered `theron_v1_original_consumer_trace_markers` in CTest.
  Its synthetic trace is only verifier test input; all three matching roles
  must pass, while missing consumers, generation/sequence/destination/value
  mismatches, and a receipt ordered after its consumer are rejected. Perl
  syntax validation and the focused CTest pass.
- This improves evidence admission only. It does not create a new authentic
  capture or assert any Theron gameplay semantics.
- Rebuilt the three standalone source/probe targets on `trv2`; the production
  text, story, quest-name, and FIFO-correlation CTests passed 4/4 with `-j2`.
  The script test used an isolated `/dev/shm` checkout; the dirty shared
  checkout was not changed.

## 2026-09-29 — remove paraphrased dungeon lore copies

- Replaced the separate paraphrased seven-dungeon US lore table with a
  source-compatible accessor to `theron_v1_track02_us_dungeon_story()`. The
  accessor now returns the already source-locked Track 02 story bytes,
  including original line/paragraph/section control bytes. This prevents the
  paraphrase from standing in for retail text when an authentic story source
  already exists.
- Extended the lore test to require pointer identity with all seven shared
  source records, verify authentic narrative terms and presentation controls,
  and preserve bounds/save-label checks. The focused CTest passes 1/1.
- This does not add a dynamic US/JP story decoder or original text renderer;
  both remain open.

## 2026-09-29 — keep unbound quest-item helper out of authenticated levels

- Authenticated Track 02 levels now fail closed in the legacy world quest-item
  helper while the original T900 pickup consumer is unbound. Fixture worlds
  retain the old helper path, but it can no longer set a real level's quest
  bit, `dungeon_complete`, or exit transition without an authentic object
  transaction.
- Added a regression asserting that a source-header-verified level rejects
  the helper and keeps its quest mask, completion flag, and exit gate intact.
  This is a provenance safety boundary, not quest-item pickup parity.

## 2026-09-29 — isolated authentic JP controller-poll capture

- Ran the instrumented Mednafen build headlessly on `trv2` with the authentic
  JP Rev. 1 CUE, Track 02 MD5 `b7afb338ad31be1025b53f9aff12d73a`, and
  System Card MD5 `ff1a674273fe3540ccef576376407d1d`. The independent 360-second
  PCE replay applied `run@9600:90,i@11000:8,ii@13000:8`; all three events had
  an original CPU controller-poll witness within the 1,048,576-read bound.
- The trace contained 115 CD IRQs, 24 raw sectors, four SCSI READs and 24
  sector bindings, plus one `$E009` dispatch/return and five TII transfers.
  It contained no `$E009` data reads, byte-exact FIFO destination or
  authenticated CD-to-RAM receipt; the strict capture exited `BLOCKED` with
  `transition=missing`. This is verified negative evidence only, not gameplay
  support. Private traces remain outside Git at
  `/home/trv2/firestaff-theron-auth-capture-20260929/capture/`.

## 2026-09-29 — quest-item world helper validation

- Guarded the world-level quest-item helpers against invalid dungeon IDs,
  wrong-dungeon item bits and duplicate collection; invalid calls leave quest
  state unchanged. Added regression coverage for those cases and the valid
  single-collection path.
- The focused `theron_v1_dungeon_progression` CTest passed 1/1 (19/19 internal
  assertions). The touched implementation and test sources compiled in the
  target build. The local link used Xcode's macOS 26.5 SDK because the active
  Command Line Tools 27.0 stubs are unsupported by the installed linker.
- This is state-integrity hardening, not authentic pickup parity. The original
  T900 object/pickup consumer remains open in `TODO-theron.md`.

## 2026-09-29 — authentic US font-glyph source check

- Added a separate real-media CTest that checks the hash-verified US Track 02
  byte span at user-data offset `0x09A000` against all 120 checked-in 8×6
  glyphs. Missing media is an explicit skip; a present file with the wrong
  identity fails. The existing data-free glyph unit checks remain independent.
- On `trv2`, the focused unit and real-media tests passed 2/2. The source span
  matched all 720 bytes exactly for `TQUS02.bin` (MD5
  `f23601102138f87c33025877767ebf76`). This authenticates the fixture glyph
  bytes only; it does not add a production font consumer or prove item-name
  display, Japanese Shift-JIS rendering, or original UI parity.

## 2026-09-29 — authentic stair fail-closed census

- Expanded the real-data mechanics probe to visit every authentic stair cell
  across all seven dungeons in both US and JP Track 02 editions.
  For each approachable cell, both the move query and original forward-command
  route must remain blocked without changing dungeon/level, party pose, world
  tick, transition metadata, or queued actuator-event count. Non-approachable
  stair cells remain counted but are not mutated into reachable fixtures.
- The hash-verified authentic US and JP Track 02 probe passed: it decoded 171
  US and 170 JP stairs across the campaign; 39 US and 42 JP stairs had a
  qualifying adjacent approach, and all 81 stayed transactionally blocked.
  The census also checks active-creature occupancy at both the stair target
  and its approach, since movement resolves combat before stairs; neither
  edition had an overlapping creature on a tested route.
  The probe asserts both regional census totals, so an empty or truncated
  approach selection cannot pass. Every dungeon loaded in both editions, and
  the focused CTest passed 1/1 against installed original media. This improves
  fail-closed coverage only;
  the original stair direction/destination consumer remains unbound and stair
  traversal is not claimed as supported.

## 2026-09-29 — conservative campaign-bit projection guard

- Kept the authenticated raw `$267C` campaign mask intact while limiting its
  dungeon-progression projection to bits 0–5. The source-locked ordinal-6
  Akutuba sweep reaches a distinct final-stage branch and stalls before a
  completion write; it does not prove Demon incomplete or complete. This guard
  therefore prevents an unsupported Demon-complete projection without
  claiming the meaning of bit 6. The source lock is
  `docs/source-lock/theron-original-akutuba-completion-capture-2026-08-21.md`.
- Added a low-level projection regression with `0x40`, retaining the raw byte
  and quest-item state while refusing to mark Demon complete. This byte is
  deliberately not represented as a valid Continue state: the original BRAM
  restore rejects campaign values >= 7, as documented in
  `docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md`.
- On `trv2`, built the focused loader target and passed its CTest against the
  installed authentic US and JP Track 02 media (1/1). This verifies the
  projection guard in the real-media loader test; it does not establish Demon
  completion semantics or broader campaign parity.

## 2026-09-29 — fresh authentic regional runtime revalidation on trv2

- A clean Release build of `firestaff` and the focused Theron probes completed
  on `trv2` with `-j1`, using the original US and JP media already staged
  outside Git. Five Track 02 loader, production combat-source, bounded
  mechanics, US raw-BIN boot, and JP raw-BIN startup tests passed twice (10/10
  runs). Authentic JP CUE pair selection and raw-media intake also passed
  twice (4/4); the JP Rev. 1 CUE Akutuba runtime and US raw CUE startup passed
  twice each (4/4); the US CloneCD ZIP launcher runtime passed twice (2/2).
- The real US Akutuba-complete Backup RAM Continue-to-dungeon-2 test passed
  twice (2/2) against an isolated byte-identical copy. The source BRAM and the
  test copy both retained MD5
  `ffabc8d19b0915d4d9632a7ae2e90a97` before and after testing.
- These results revalidate existing regional data intake, startup, and the
  bounded US Continue route. They do not bind original Track 02 gameplay
  consumers, prove changed progress saves, or establish full Theron parity.

## 2026-09-29 — regional CUE pairing in mixed authentic media directories

- Bound directory CUE selection to the requested edition's already-registered
  Track 02 MD5, and require a readable, complete Track 01 AUDIO/Track 02 MODE1
  pair. Distinct matching pairs fail closed. This prevents a higher-ranked
  authentic USA CUE from displacing the requested JP CUE in a mixed regional
  data directory. Hashing explicit bounded single-image CUE slices now uses
  the same in-memory path reader as other virtual media; no game data is
  extracted or cached.
- On `trv2`, the authentic JP Rev. 1 media test retained its expected CUE
  pair and the authentic USA single-image CUE Track 02 slice hashed in bounded
  memory. The JP raw-BIN startup regression now only expects Track 01 CDDA
  readiness when both split tracks are installed; the Track 02-only variant
  remains a separate accepted data shape. These checks prove package identity
  and Track 01 availability, not original gameplay CDDA selection or full
  audio/event parity.

## 2026-09-29 — post-event controller-read evidence gate

- Scripted PCE replays now default to a 1,048,576-read trace allowance (the
  interactive route retains 65,536) and must show an original CPU read from
  controller register `$1000` after each scheduled event-frame group before
  the capture appends a controller-poll receipt. Events on the same frame
  combine before that poll. The verifier rejects missing reads, over-limit
  traces and events left at the read ceiling.
- Looped fixtures cover one and multiple event frames followed by polls, an
  event at the read cap, and an event with no later CPU poll. `bash -n`,
  `tests/test_theron_v1_mednafen_live_capture_script.sh`, and
  `git diff --check` pass. On `trv2`, a 600-second authentic US CloneCD/System
  Card cold-start with `run@9600:90` passed the verifier against the private
  input sidecar: one event frame, one nonzero apply receipt, and a subsequent
  `$1000` controller poll at read sequence 505,423 of 1,048,576 logged reads.
  This proves controller-port polling only, not a menu/game response. The full
  capture still failed its stricter VDC boundary gate: it reached 49,350 VDC
  writes, below the required 65,536, so no transition receipt was emitted.

## 2026-09-29 — Linux X11 host-input capture support

- Extended the research-only Mednafen capture runner to send host keyboard
  events through PID-verified Linux X11 windows as well as macOS Quartz. The
  X11 route requires an explicit `x11` SDL video driver, `xdotool`, the exact
  PCE mappings from the selected Mednafen profile, and an emulator-owned
  `InputGrab=1` receipt before gameplay keys are sent.
- Added a keymap regression loop for every supported SDL scancode and a
  fail-closed case for unknown mappings. `bash -n`,
  `tests/test_theron_v1_mednafen_live_capture_script.sh`, and
  `git diff --check` pass; redacted Gitleaks scans of all changed files found
  no leaks.
- The Linux grab chord now comes from the selected profile's actual
  `command.toggle_grab` SDL binding rather than assuming Ctrl+Shift+G. The
  regression loops cover standard SDL letter, digit, navigation, function,
  keypad and modifier scancodes, including both the current Ctrl+Shift+Menu
  profile and the prior Ctrl+Shift+E mapping; unsupported bindings fail closed.
  On `trv2`, an authenticated temporary Xvfb accepted the profile's
  Ctrl+Shift+Menu chord; instrumented Mednafen recorded `InputGrab=1` and the
  requested Right key-down/up from the authentic US Akutuba state.
- The complete live-capture runner still exits at its VDC-trace validation
  before emitting a receipt. A later one-second control run ended after only
  3,338 VDC writes, below its 2,097,152-write trace limit. The earlier full
  run's sidecar was overwritten before its exact footer and sequence could be
  checked, so the VDC validation failure's cause remains open. This evidence
  verifies profile-derived X11 input delivery only; it establishes no
  game-owned data handoff, item behavior, or gameplay semantics.

## 2026-09-28 — Authentic inventory transaction regression across all dungeons

- Expanded the authentic Track 02 loader regression from Akutuba alone to all
  seven US and JP dungeons. Each authentic carryable occurrence with an
  adjacent floor approach is exercised through the public M12 pickup route,
  source-backed inventory/name receipt, inventory selection and drop route;
  altered source records, altered property rows and compact-ID/source swaps
  must still reject atomically.
- On `trv2`, the focused loader target built with `-j1` and its CTest passed
  against the installed authentic regional media. The test reported 431
  TAKE/DROP cases in each region. Raw category-local type-zero records were
  tested/deferred per dungeon as `6/0, 2/0, 1/0, 3/1, 3/2, 1/2, 2/0` for
  both editions: 18 tested and five deferred of 23 per region. Deferred cases
  have no adjacent source floor in this input route; no source/map bytes were
  fabricated or changed to force access.
- The deferred records are all raw-type-zero scroll occurrences and have
  matching US/JP source refs and coordinates: D4/L1 `(2,18)` ref `5c00`,
  D5/L1 `(6,6)` ref `1c02`, D5/L2 `(1,12)` ref `5c03`, D6/L2 `(8,12)` ref
  `1c02`, and D6/L2 `(8,15)` ref `dc00`. The test prints both raw source-tile
  bytes and the loader's projected square classes around them. Those
  projections show nearby pits, walls, a secret wall, a door and teleporters;
  they do not prove the original item's reachability or its T900 pickup path.
- The authentic all-level scan found zero direct coordinate-teleporter links
  to each deferred cell in either region. At the same coordinates on other
  loaded levels, the projection is floor or wall, with no same-coordinate pit.
  This is negative evidence against only those direct route candidates; it
  does not prove that no other original transition or input route exists.
- Inspected every adjacent authentic door, teleporter and actuator record for
  the five deferred cells in both regions. The source-bound actuator decoder
  reports D4/L1 target coordinates `(3,13)` and `(4,13)`, and D5/L2 targets
  `(0,0)` and `(4,13)`; none points directly to its neighboring deferred
  scroll. This is a record-layout observation only, not proof of how the
  original actuator consumer or an indirect route behaves.
- Rebuilt the focused test target with `-j1` on `trv2`; the authentic-media
  CTest passed (1/1). It reported the same deferred occurrences and adjacent
  source records for US and JP. Optional JP BIN/CUE layouts were skipped where
  unavailable; no synthetic data was used.
- This verifies Firestaff's source-provenance and input plumbing on authentic
  data across all seven dungeons. It does not prove original T900 selection,
  object reachability, quest-item semantics, or full gameplay parity.

## 2026-09-28 — Complete registered Theron CTest selection on authentic media

- Built the missing Theron test targets and production executable on `trv2`,
  then ran all 71 Theron-labelled CTests with `-j2`: 71 passed, zero failed.
  Eight tests skipped because their optional operator-owned captures or
  media layouts are unavailable; no synthetic data substituted for them.
- The regional scan test now accepts the authentic, hash-verified JP Track 02
  file selected from its CUE sibling set while still requiring the US launch
  path to remain selected. The JP startup test prefers that exact CUE-paired
  Track 02 file when present and falls back to the standalone authentic BIN.
  Their temporary receipts now stay in CTest's build working directory.
- This verifies registered Firestaff probes and media routes, not full
  original-runtime gameplay, T900 item semantics, or complete campaign parity.

## 2026-09-28 — Authentic type-zero inventory record-integrity regression

- The Track 02 dungeon-loader test now executes source-backed TAKE and DROP
  on real Akutuba items from both authentic US and JP Track 02 media. It
  verifies 71 item transactions per region, including all six authentic
  category-local type-zero records, preserving the exact source occurrence,
  origin and property row. The compact inventory represents raw type zero as
  internal ID 126 while the inventory-source receipt retains the authentic
  raw zero. Altered source or property bytes are rejected before inventory
  mutation. Co-located earlier authentic items are temporarily marked as
  carried only to select the specific source occurrence; their flags and all
  source bytes are restored after each case.
- Verification: on trv2, the focused loader target built successfully;
  `theron_v1_track02_dungeon_loader` and
  `theron_v1_inventory_id_mapping` passed 2/2 against the authentic US and JP
  BINs (`TQUS02.bin` MD5 `f23601102138f87c33025877767ebf76`, `TQJP02.bin` MD5
  `b7afb338ad31be1025b53f9aff12d73a`). The verbose loader output reported
  `type-zero records tested/deferred: 6/0` for both editions. This verifies
  Firestaff's bounded inventory handoff and lossless source provenance only;
  it does not prove original T900 pickup/UI semantics or quest-item collection.

## 2026-09-28 — Source-backed inventory swap identity gate

- A source-level inventory swap now verifies that each non-empty compact
  champion ID maps back to the raw item type in its authenticated Track 02
  occurrence before moving the item and provenance record. The authentic
  Akutuba US/JP loader regression changes the compact ID without altering the
  original occurrence, confirms the swap is rejected, and byte-compares both
  inventory slots and both source-provenance records to prove rejection is
  atomic.
- The production `firestaff` and focused dungeon-loader targets built on
  macOS; `theron_v1_track02_dungeon_loader` and
  `theron_v1_inventory_id_mapping` passed 2/2 against authentic US and JP
  Track 02 media, with 71 TAKE/DROP cases and all six type-zero records tested
  per edition. This proves Firestaff's source-integrity boundary, not the
  original T900 inventory-swap consumer. A fresh Linux CMake build on `trv2`
  also produced `firestaff` and both focused targets; the same CTests passed
  2/2 against the authentic US/JP Akutuba data.

## 2026-09-28 — Type-zero item through M12 pickup and inventory selection

- Extended the same authentic US/JP Akutuba regression through the public M12
  pickup input, carried Track 02 name lookup, inventory-toggle selection, and
  drop input.
  Each selected raw type-zero record must produce a valid compact inventory
  slot, expose the same authentic item-name bytes while carried, and select
  that exact slot through the M12 input route, then return the same source
  occurrence to the party's floor position through M12 drop. This verifies
  Firestaff's input and source-name plumbing only; it does not infer the
  original game's T900 behavior or quest-item transaction.
- Verification on trv2: the loader target rebuilt and both the authentic
  `theron_v1_track02_dungeon_loader` and
  `theron_v1_inventory_id_mapping` tests passed (2/2) against US and JP media.
  The loader reported all six raw type-zero objects tested and none deferred
  in each regional edition.

## 2026-09-26 — SRM readiness probe target dependencies

- The Theron Track 02 media-intake code uses the shared M10 asset reader and
  M12 digest helper. Those dependencies are now published by the owning
  `firestaff_theron` target, rather than repeated selectively by each
  standalone probe.
- Formerly failing SRM-readiness, combat-mechanics, hardening, champion-roster,
  text-decode and runtime-level-receipt targets now link successfully; the
  focused link-regression tests pass 9/9. On `trv2`, the complete Theron CTest
  label set now reports 65 passed, three configured skips, and no failures.
  Authentic US/JP BINs, the combined RAR, JP/US 7z archives, BRAM captures,
  Mednafen save files and trace sidecars were staged outside the repository
  and checked against their local SHA-256 digests. Both archive boot routes,
  both BRAM checks, and authentic Main-RAM/CD-state trace receipts pass. The
  remaining skips require the missing VDC-state/SAT capture pair or an
  original command-capture corpus; no substitute capture was used. A
  byte-exact ISO reassembled from the RAR's original Track 19 and end-member
  hashes to the registered US Track 02 identity and passes direct ISO boot.
  A Theron/M12 static-link cycle is covered by the library interface
  multiplicity so standalone M11 probes link. A later local repository-wide
  all-target build was attempted, but a standalone linker failed with
  `No space left on device`; it therefore did not verify all targets. The
  remote all-target build was not rerun after that local resource failure.

## 2026-09-26 — Authentic movement checks across the seven-dungeon campaign

- Extended the real-data mechanics probe to load every dungeon and test one
  source-backed floor move and wall block wherever the authentic level layout
  contains those edges. US and JP Track 02 runs both passed all 215 checks
  without skips; each region covers 34 levels (68 authentic level loads total)
  and exercises movement and wall blocking in each of the seven dungeons.
  Some levels lack one of these sample edges; the probe neither creates tiles
  nor claims coverage for an edge absent from the map. Stair destinations,
  gameplay semantics, and other unresolved source consumers remain gated.
- Verification: `theron_v1_mechanics_playability` CTest passed against the
  operator's authentic regional BIN files; direct US and JP probe runs each
  reported `PASS: 215  FAIL: 0  SKIP: 0`.
- The same hash-verified runs now execute the resolver against all 335
  coordinate-linked teleporter records per region: 165 are disabled and 170
  enabled. The resolver commits 72 enabled routes and keeps 98 fail-closed;
  89 of those point to a wall and nine continue into another enabled pad. No
  missing level, out-of-bounds coordinate, or missing endpoint was found.
  This is a real-data boundary audit, not proof of original wall-target or
  chained-teleporter semantics, so those routes remain fail-closed.
- The expanded direct probe reports `PASS: 215  FAIL: 0  SKIP: 0` for each
  region; the registered CTest also passes.

## 2026-09-25 — Direct authenticated US and JP Track 02 boot

- The current Release executable boots directly from each supplied original
  Track 02 BIN (`TQUS02.bin` and `TQJP02.bin`) and reaches `theron-runtime`
  with the source-backed initial level loaded, both source champions present,
  all seven regional Track 02 item-name banks, and the matching Track 19 name
  bank and item mapping. Both runs accepted the scripted native movement input.
  The probes report 291 source objects and four loaded dungeon levels for
  these shared early routes. `theronTrack01CddaReady=0` is expected for these
  standalone Track 02 paths because no regional full-disc CUE/audio set is
  present in the supplied data directory; the combined RAR path has its
  separate authenticated in-memory title-audio support. This establishes
  startup and the exercised source-backed runtime routes only, not complete
  visual, gameplay, save/export, or campaign parity.

## 2026-09-25 — Full local Theron suite with operator-owned media and trace

- The complete focused Theron CTest selection passed locally: 56 passed, 5
  skipped, and no failures. In addition to the Linux run recorded below, this
  run used the locally installed authentic US and Japanese Track 02 media and
  operator-owned captures. The real Backup RAM decoder and M11 Continue tests
  discovered their authentic defaults and passed; the authentic US and JP
  CUE boot paths, seven-dungeon JP source loader, regional text/roster checks,
  and Mednafen CD-state verifier also passed. The latter consumed the existing
  authentic split-CUE capture (`theron.trace.cd`, MD5
  `c767c4af870c5fd0b527ba0cb0c8a8a0`): 51 SCSI commands, all 161 requested
  raw sectors bound, 2,048 ADPCM FIFO reads paired with RAM writes, and 87
  origin receipts. The verifier still blocks semantic publication.
- The five skips remain explicit evidence gaps: the atomic VRAM/VCE/VDC-state/
  SAT/VDC-I/O capture bundle is unavailable; the US CloneCD ZIP and its
  CloneCD-derived raw CUE are not staged; the authenticated CLI capture lacks
  its required bundle; and original-command capture inputs were not
  configured. No synthetic media was substituted. This suite result does not
  establish original-runtime transition, visual, combat, or full-campaign
  parity.

## 2026-09-25 — Missing loose BINs no longer fail unrelated startup probes

- The startup-flow probe skips its optional authentic US loose-Track-02-BIN
  integration when that exact file is absent. The combat-source test resolves
  the authenticated JP Rev. 1 Track 02 under its original CUE filename and
  runs its real regional startup/combat integration; it skips only the absent
  US loose BIN on trv2. Neither probe substitutes fixture bytes for missing
  game media. Authentic US CloneCD ZIP, US CUE and JP CUE runtime-boot tests
  also pass against the installed original media. The raw-US-BIN boot test
  remains a media skip. The JP raw-start test accepts the Rev. 1 Track 02
  archive filename and verifies its exact CUE pairing, original Track 01 audio
  availability, source-backed start and movement. Canonical loose-BIN
  integrations still run when those files are present, as verified locally.
  This repairs test behavior for supported authentic media layouts; it does
  not claim new gameplay parity.

## 2026-09-25 — JP CUE Track 02 auto-runs the seven-dungeon source regression

- The regional source-loader and startup/combat tests now discover the
  authentic Rev. 1 CUE Track 02 under its preserved archival filename when
  the canonical `TQJP02.bin` name is absent. CMake selects that real file for
  the test instead of injecting a nonexistent canonical path. The production
  roster test likewise binds the actual JP roster and reports the absent US
  source separately. The chapter-marker test translates all seven authentic
  JP quest-item names and reports the missing US bank separately. The trv2
  regressions pass against the installed Track 02 hash; the source-loader test
  verifies all seven JP source dungeons (34 maps, 2,269 source objects), while
  the runtime-entry tests exercise the authentic JP roster. No media was
  copied, generated, or substituted. This remains source-loader, roster and
  text evidence, not original transition or gameplay parity. The full trv2
  Theron suite then reported 47 passed and 14 skipped out of 61
  tests, with no failures; skips are the specific absent BIN, capture, save or
  archive inputs.

## 2026-09-25 — JP CUE ISO loads authentic source dungeons

- Byte-compared the authentic JP CUE-projected Track 02 ISO with the
  authentic JP raw BIN user-data stream after the CUE's 224-sector INDEX 01
  prefix. The startup source-dungeon handoff now restores only that absent
  coordinate prefix and passes the same original bytes through the verified
  JP BIN map/thing/property decoders. Real-media regressions verify all seven
  dungeons (34 maps, 2,269 source objects) through both the source loader and
  production handoff. ISO media does not supply the missing raw-sector spawn
  witness, which is deliberately not fabricated. This does not establish
  original dungeon-transition, visual, combat or item-action parity.

## 2026-09-24 — Native movement through the authentic JP CUE route

- Extended the full Japanese Rev. 1 CUE boot regression with six native
  movement inputs on its source-backed Akutuba map. It requires the final pose
  `(direction=2, x=3, y=0)` with the route's single source-backed champion,
  alongside its existing authentic-media and no-fallback checks. This is
  bounded host-runtime movement, not Japanese original-runtime parity or
  later-dungeon evidence.

## 2026-09-24 — Native movement from the authentic US CloneCD route

- Extended the real US CloneCD ZIP menu-to-runtime regression with six native
  movement inputs on the source-backed Akutuba map. It requires the resulting
  party pose `(direction=2, x=3, y=0)`, all three source-backed champions and
  source objects, while continuing to reject fallback graphics. This is a
  bounded native route through authentic US media, not original-runtime visual
  or gameplay parity and not later-dungeon movement evidence.

## 2026-09-24 — Native movement on the authentic Japanese Akutuba map

- Extended the real JP raw-BIN M11 regression with six native directional
  inputs and an exact final pose assertion (`direction=2, x=3, y=0`). The test
  uses the hash-authenticated Japanese Rev. 1 Track 02 and Track 19 plus its
  complete authentic CUE, and continues to reject fallback graphics. It
  proves a bounded native movement route on the authentic source map, not
  parity with a Japanese original-runtime capture or movement in later
  dungeons.

## 2026-09-24 — JP Track 19 raw BIN keeps its authenticated pregap

- `theron_v1_track19_inventory_probe` now passes against the original Japanese
  Rev. 1 Track 19 raw BIN on trv2 (`27d54f58154662885bb67d5967e5111e`). The
  item-name-bank reader now strips the CUE-authenticated 224-sector pregap,
  just like the Track 19 inventory reader, and the world admission gate accepts
  the authenticated raw-track hash as well as the normalized ISO hash after
  validating the source tables. JP Shift-JIS bytes remain opaque; this does
  not claim host glyph rendering or T900 semantic parity. The local
  `theron_v1_track19_inventory_probe` and `theron_v1_track02_dungeon_loader`
  tests also pass.

## 2026-09-24 — JP Track 02 source maps verified on trv2

- Built the current `main` worktree in an isolated Linux directory on trv2 and
  ran `theron_v1_jp_later_dungeon_runtime` against the existing authentic
  Japanese Rev. 1 Track 02 file. The test bound all seven source dungeons:
  34 maps and 2,269 source objects. This verifies map and object data, not
  JP Continue, campaign transitions, or visual parity.

## 2026-09-24 — authentic CloneCD ZIP through the mouse-click flow

- The full `theron_v1_us_clonecd_zip_runtime_boot` passes on trv2 against the
  existing authentic ZIP archive. Direct launch, the Original and Modern cards,
  mouse clicks through game/platform/presentation, and the usual M12→M11 flow
  reach a loaded runtime level with source objects and no fallback. The card
  sequence delay was adjusted to the test window so all clicks are sent in time.

## 2026-09-24 — Japanese Rev. 1 CUE startup verified on trv2

- `theron_v1_jp_cue_runtime_boot` passes against the complete authentic
  Japanese Rev. 1 CUE disc and its track files on trv2. This confirms the
  source-owned startup path, not JP Continue, campaign transitions, or visual
  parity.

## 2026-09-24 — JP raw BIN through M12/M11 on trv2

- `theron_v1_m11_launcher_handoff_boundary` passes with 62 checks and
  2 skips. The authentic Japanese Track 02 raw file is found in the user's
  data directory and passed through M12/M11 with its verified region identity.
  The US CUE and roster-text cases are skipped because the corresponding files
  are absent there; the test does not prove JP Continue or a complete campaign
  transition.

- The standalone `theron_v1_track02_dungeon_loader` run now verifies
  JP Rev. 1 independently of the US BIN: all seven dungeons' source objects
  and local item-name/type-code tables were read from the authentic raw file on
  trv2. The local combined US/JP test still passes; the missing JP ISO and
  US BIN on trv2 are reported as separate, optional skips.

## 2026-09-24 — Authentic Continue to the next unlocked chapter

- Extended the M11 Continue regression with the authentic US Track 02 and
  Akutuba-complete 2 KiB Backup RAM capture (`ffabc8d19b0915d4d9632a7ae2e90a97`).
  Its test wrapper verifies both exact media hashes before launching the
  probe, so a synthetic or substituted 2 KiB save cannot count as evidence.
  After restoring the completed Akutuba chapter, the real progression skips
  it, focuses unlocked dungeon 2, selects a source-backed Track 02 champion,
  and enters that dungeon through the Soul Room and forcefield.
  The integration now compares all seven restored attributes and all 20
  temporary/persistent skill-experience pairs directly with the decoded,
  source-verified real Backup RAM body. It requires the native runtime to load
  dungeon 2, level 0, with the selected two-member party and original objects.
  After load, a breadth-first walk over the authentic floor tiles finds a
  three-step route, and the test verifies all three native movement inputs
  move the party along it. It does not mistake the map-edge-blocked forward
  direction for a missing route.
  This verifies a real-media native route, not parity against a separate
  original-emulator transition capture. The current version passes locally;
  the earlier bounded Continue route also passed on TRV2. JP Continue, later
  stages and original transition parity remain open.

## 2026-09-24 — authentic raw BIN launch through M12 mouse cards

- Extended the raw US Track 02 real-media regression to select Theron's game,
  PC Engine platform and Original presentation using only M12 mouse clicks.
  The verified retail BIN reaches native startup with no fallback assets.
  This establishes the raw-BIN mouse route; the separate authentic CloneCD
  ZIP routes are verified on trv2.

## 2026-09-24 — Production pickup admission test

- Corrected the production runtime-input regression to reflect source-owned
  startup: pickup stays closed until an authenticated champion is admitted,
  rather than relying on the synthetic roster used by fixture builds. The
  authentic Track 02 dungeon-loader regression still exercises successful
  item pickup, slot provenance, resume and drop.

## 2026-09-24 — JP Rev. 1 ISO dungeon-data boundary

- The hash-identified 305,152-byte `TQJP02End.iso`
  (`397039af02d50d15c70b74088eb8a1cb`) is now exercised by the dungeon-loader
  regression. The supplied bytes are all zero, so this is an identity/negative-
  boundary check only; the image is not accepted as source dungeon content or
  startable campaign media. File-backed Track 02 intake retains its identity
  for diagnostics but rejects it with `source_content_empty`.
- The same test run loads all seven authentic JP raw-BIN dungeons and binds
  their local item-name/type-code tables. This is source-data coverage, not
  JP Continue, campaign, presentation, or gameplay parity.

## 2026-09-24 — US MODE1/2048 ISO forcefield source handoff

- The authentic split-image US Track 02 ISO now binds the same source-defined
  Theron roster records as raw MODE1/2352. Its raw-sector roster interval is
  translated by sector and user-data offset (not by subtracting a flat byte
  pregap), then checked against the existing record FNV and full ISO MD5.
- Startup normalizes the absent 225-sector pregap as zeroed address space
  only, preserving every ISO byte. The real US ISO CUE now passes M12→M11,
  source roster initialization, and the forcefield handoff into Akutuba: four
  authentic maps and 291 source objects load. Visual capture remains gated;
  this does not claim bitmap parity or later-dungeon/gameplay parity.
- Verification: `theron_v1_m11_launcher_handoff_boundary` with the authentic
  CUE/ISO under the isolated data root (49 passed, 0 failed, 1 media-optional
  skip).
- The real-media dungeon-loader regression also compares the normalized ISO
  against authentic raw Track 02 across all seven dungeons: 34 maps, exact
  source tile grids, 2,269 source-object records, category counts and
  property-table provenance match. It independently verifies that every ISO
  byte equals the raw BIN's user-data stream after the 225-sector pregap.
  This establishes data-layout equivalence, not campaign transitions or
  gameplay semantics for later dungeons.

## 2026-09-24 — JP/US spawn consumer source comparison

- The two independently hash-authenticated retail spans are each 269 bytes.
  Static structural comparison found 122 aligned operations, with 230 bytes
  identical and 39 operand-byte differences confined to relocated helper
  targets and regional RAM columns. JP accumulator/property fields are one
  byte earlier than the corresponding US fields. Formula constants and
  branch order agree. This is source-shape evidence only; no JP caller,
  runtime category, RNG return, spawn, AI or combat behavior is promoted.
  See `docs/source-lock/theron-jp-us-spawn-consumer-static-comparison-2026-09-24.md`.

## 2026-09-23 — Japanese spawn source records bound to retail bytes

- The JP and US real-media spawn test now checks all five regional records
  against their own MODE1/2352 user-data offsets. The JP assertions pin the
  exact eight source bytes at `$273858`, `$2738D7`, `$273902`, `$273929` and
  `$273950`; the US records remain checked at their separate offsets. The
  original Japanese and US Track 02 BINs pass with their canonical hashes.
  This verifies static source decoding only: JP runtime category publication,
  spawn execution, RNG, AI and combat remain gated on an authentic live
  consumer capture.

## 2026-09-23 — Synthetic save runtimes removed from production

- The Firestaff-only `FSTQPTY1` writer and Continue reader are no longer
  linked into `libfirestaff_theron`. They remain available only to the
  explicit SRM body-decode fixture target. The production archive gate now
  rejects both the translation unit and its export/Continue symbols, while
  authentic 2 KiB HUBM Backup RAM remains on its separate, fail-closed
  original-data route.
- The retired `.tqsv` codec is also absent from `libfirestaff_theron`, and
  production M12 no longer recognizes `.tqsv` or Firestaff-envelope `.srm`
  files as Theron browser/import/Quick Resume candidates. Fixture targets may
  still compile these codecs explicitly; the shipped application now exposes
  no synthetic Theron save or Continue symbol.
- The remaining SRM classifier keeps only the authentic 2 KiB HUBM reader in
  production. Its `FSTQPRG1`/`FSTQPTY1` decoders now return unsupported there
  and are compiled normally only by explicit fixtures. The production archive
  gate also scans linked strings and rejects either synthetic signature; both
  `libfirestaff_theron.a` and the final `firestaff` executable are clean.
- A read-only original-runtime capture now binds the complete 134-byte
  `DMS-SG.001` writer layout: `$267C`, six bytes at `$267D`, seven at `$2683`
  and six 20-byte columns through `$2701`. The production classifier exposes
  these still-neutral bytes without inventing party or inventory semantics;
  the authentic progressed BRAM fixes the body FNV-1a to `b37e696e`.

## 2026-09-03 — Native archive/CUE regression audit

- Re-ran production, startup, resume and Track 02 layout gates. The supplied
  US CloneCD ZIP reached both direct and start-menu native routes, and the
  Japanese CUE reached the Track 02 Akutuba route. Raw US CUE/BIN and paired
  raw JP/US Track 02 comparison probes remained correctly skip-safe because
  those distinct raw files are not staged.

Reviewed 2026-08-29. Completed work only.

- The later-record correlation probe is media-optional in CI: absent
  proprietary Track 02 files now produce a skip rather than a false failure.
  When supplied, the authentic Japanese Rev 1 Track 02 proves the stage-three
  self-reference and IPL-to-record topology directly from raw sectors.

- The supplied authentic Japanese Rev 1 CUE is hash-verified and reaches the
  native title → stage → Soul Room startup route (`theron-startup-2`).
- The same authentic JP Rev 1 MODE1/2352 Track 02 route reaches native
  `theron-runtime`: it loads the source Akutuba level and publishes party
  position `1,0,0` without treating Japanese raw sectors as US ISO media.
- A bounded public native Track 02 consumer now binds the same authentic JP
  source to all seven campaign dungeons, retaining 34 maps and 2,269 linked
  source-object records. This includes Drator (dungeon 2: eight maps and 291
  records). The separate world ledger retains 2,266 ground-reference
  occurrences. Transfer semantics, graphics capture and gameplay behavior
  remain independently gated.
- Track 02 intake preserves original media identity and blocks uncaptured
  graphics/palette fallback from production presentation.
- A CUE/BIN package in a ZIP archive starts natively through both direct CLI
  and the start menu. Firestaff hashes and reads the selected Track 02 member
  in memory (`archive.zip::member`) and never writes extracted game data.
- The supplied US CloneCD ZIP starts natively through direct CLI and the
  start menu. Its `.ccd` layout identifies Track 02 as an in-memory bounded
  `.img` slice (MD5 `168bd6a63784e91885df8c47be62ab5a`); both the menu's
  Original and Modern cards preserve that exact slice and reach the native
  startup handoff. The separate verified eleven-input startup route reaches
  `theron-startup-2`. CloneCD's missing 225-sector pregap has a separate,
  source-verified anchor map—no pregap or game media is synthesized or
  written to disk.

- The supplied Japanese Rev 1 Track 19 raw BIN is admitted directly from its
  CUE-defined MODE1/2352 representation. Firestaff retains its physical MD5
  (`27d54f58154662885bb67d5967e5111e`), skips the authentic 224-sector
  INDEX 00 pregap only while reading in memory, and validates the same
  3,072-sector user-data payload's item, level-label, property, and startup
  envelope receipts. No ISO projection or extracted copy is written to disk.
# Firestaff DONE - THERON

## 2026-08-21 — authentic Drator generator/RNG receipt

- A controller-driven cold start from the real US disc and authentic BRAM now
  goes through the original menu to Drator, leaves level 2 at `(2,3)`, and
  re-enters the same square from `(1,3)`.
- Two separate captures open RNG tracing on either side of the position
  transition. Sequence 1 from the first run is byte-identical at the RNG
  boundary to sequence 0 from the second run. Thus `$4667 → $CC55`, returning
  `$8F`, is the only RNG call in the event interval.
- The production receipt now requires event `0c81`, generator `0c38`, physical
  and logical entry and caller, the exact 32-byte caller image, RNG state
  `2a5429 → 988f29`, and the verified `$4639` successor. A changed caller byte
  or successor state is rejected. This admits the source plan, but still does
  not create a creature before direction, HP, and timer ownership are bound.
- The same authentic transaction builds runtime row
  `a500000120ee11030800`; the original `$CBCE` publishes it in slot 0 at
  `$60FF`. A second authentic run binds the first reads
  `$C9F3/$C9FB/$CA02 = a5/00/00` and the subsequent unlink at `$CA78`. The
  production gate now requires this entire exact lifecycle receipt. The
  gameplay meaning of the byte fields remains closed until their semantic
  consumers are bound.

## 2026-08-21 — original Akutuba campaign-bit transition

- A research-only Mednafen hook now maps the exact System Card RAM page `$6D`,
  verifies the resident `DMS-SG.001` dispatcher and changes only its bounded
  dungeon ordinal at `$DE38`. Original code issues the four-sector LBA 4201
  read and changes the real campaign byte `$267C` from `00` to `01`.
- The final bank is still Akutuba (`$20DB == 0`), so this is not claimed as a
  Drator load or generator witness. See
  `docs/source-lock/theron-original-akutuba-completion-capture-2026-08-21.md`.

## 2026-08-20

- Upgraded the world save format to version 14 so the authentically selected
  party size, active hero, leader position and direction, levitation, and door
  override persist through save/resume. Versions 1–13 remain readable under
  their legacy four-hero contract; a materialized version 13 stream is tested
  explicitly so its shorter party block does not shift the rest of the world.
  Invalid party size, active slot, or direction is rejected, and a regression
  test proves a three-hero party without host inheritance or synthetic
  padding. An empty production party may refer only to reserved Theron slot 0
  or no slot, never to an unbound companion.
- Removed the startup view's blind classification of every already-loaded
  level as `fallback-room`. A level whose authentic Track 02 header has been
  verified is now classified as `track02-semantic` and carries the semantic
  handoff receipt; fixture classification is used only when the source receipt
  is actually absent.
- Fixed the old world-hash contract violation: the `PART` section claimed to
  cover party position and direction but hashed no party fields. The hash now
  covers the same version 14 block as the save format, including selected
  roster, source-bound hero records, gold, active slot, position, direction,
  levitation, and door state. The regression test changes each field family
  separately and requires a different hash.
- Replaced the combat/startup test's fake one-byte Track 02 with the complete
  authentic `TQUS02.bin` and `TQJP02.bin` from the Theron data directory. Both
  regions now use the source-bound startup, roster, and dungeon-runtime path
  and require authentic Theron/Hakar stats. The same test feeds a truncated
  portion of each real file to the roster gate and requires byte-exact rollback
  of both party and startup flow. This also showed that production
  initialization left equipment slots at zero, which could look like host
  item 0. All unbound slots now use `-1` until an authentic T900 consumer
  actually binds equipment. The same invariant applies to the two inactive
  companion slots cleared by forcefield selection; the real-media test checks
  every equipment field in all four slots. The startup "verified Track 02"
  gate also now checks the declared MD5 identity against the entire actual
  byte buffer. Previously, a known MD5 string from the caller was sufficient.
  The US and JP tests change the final byte of each real file and require the
  party and startup flow to remain exactly unchanged; no local table hash can
  legitimize a modified complete file anymore.
- Made the entire production transition `forcefield → regional roster →
  dungeon` atomic, not just the roster step. A later dungeon-entry failure now
  restores both `Theron_StartupFlow` and the complete `Theron_V1_World`. The
  real-media test runs this after accepting the US and JP rosters by locking
  Akutuba in live progression and requiring byte-exact world restoration; no
  source records, media fields, or party stats can leak from a failed entry.
- Also made world save/resume deserialization atomic. The parser now works in
  a staging world and publishes only after validating the entire version
  1–14 stream. A regression test truncates the final byte of an otherwise
  valid version 14 world, after the early party, object, and timer sections,
  and requires the existing live world to remain byte-identical.
- Restricted party gameplay consumers to the authentically selected
  `champion_count` instead of always treating four saved slots as active.
  Leader/slot accessors, Theron's life status, total health, load recalculation,
  dungeon reset, and the remaining fixture mechanics' altar, T700, and pool
  loops now ignore inactive entries. The US/JP real-data test marks a living
  third slot outside the selected Theron/Hakar party and proves its health,
  inventory, and load are neither consumed nor mutated.
- Replaced the product's sole dungeon 7 transcription as its name source with
  all seven authentic dungeon-local Track 02 tables for both US and JP. A
  total of 924 source records are bound losslessly after exact offset, length,
  and FNV checks. Object lookup uses its verified dungeon and
  `source_item_type`. Changed bytes, wrong region, invalid indices, and
  cross-dungeon borrowing are rejected. The boot probe requires seven banks
  from the correct region in the live world.
- Removed the Demon table's incorrect global category semantics from
  production. All 66 raw type codes in each of the seven US and seven JP
  tables are read and hash-verified alongside their names. The property row is
  now bound by item index and the thing record's own source category. Akutuba
  therefore binds 74 of 77 materialized records, up from 36; the three
  exceptions are real chests without a global item index. The same invariant
  passes for all 14 dungeons.
- Bound the complete authentic Track 19 name table to the native world for
  both US and JP: all 69 records, full-file MD5, and table FNV are required.
  JP names are preserved losslessly as Shift-JIS. The boot probe reports the
  bank's region and explicitly keeps item mapping at zero until the T900
  consumer is proven. The authentic Track 19 probe and both regional raw-BIN
  startup tests pass without synthetic names.

- Fixed the VDC I/O parser against the authentic 65,536-record Mednafen
  capture. Raw bit-31-marked bus addresses are preserved and normalized
  separately, while invalid high address bits are still rejected. The 23 real
  `HuCPU.Timestamp()` resets are reported as 24 verified epochs. Both the
  isolated parser test and the full real trace pass. The parser now also keeps
  every authentic write record in a bounded replay array, and the real-data
  test verifies the first and last records and complete cleanup.
- Added an atomic Mednafen capture boundary: the 65,536th observed write is
  first applied to the VDC, then VRAM, VCE, registers, and SAT are frozen and a
  terminal sequence footer is written. CloseGame can no longer replace the
  snapshot with a later state. The patch applies in dry-run mode, and the
  capture gate requires the footer.
- Bound 2,022 authentic bytes in the graphics/VDC chain around
  `$491F..$5E81` from US Track 02, including MPR3–MPR6 save/restore, VDC
  register selection, internal jumps, and nine absolute branches. The chain
  covers the `$4943` dispatcher's subroutine targets, `$49FA` selection, both
  game-controlled `$4A09/$4A84` VDC strips, the shared `$4B24` address helper
  and `$491F` CR activation, `$4BB0` scroll writes, the real 1 KiB TIA transfer
  in the `$56DE` branch, the 512-byte VDC write body at `$50F1`, the `$5E2B`
  dispatcher for VDC registers 6/7, and the `$5CE4` producer's initialization
  and clearing of the 1 KiB buffer at `$58E0`. `$5111..$51E8` also binds the
  direct eight-record builder, its local `$517A/$519F` control bodies, and the
  15 authentic command targets in the `$51E8` table through `$533C`. The
  secondary `$533D..$555E` dispatch binds another 16 authentic table targets
  and all their contiguous handler blocks. Modified media bytes and JP
  borrowing are rejected. `$55EF..$560B` and the overlapping
  `$55F4/$55FF/$5617/$562A/$563D` entries are also verified, including that
  `$563D` is an intentional second instruction stream rather than junk
  disassembly. Its BBR4 target resolves to the overlapping `$55C8` entry, and
  the entire `$55B6..$55E0` body is bound. The chain now covers 4,899 bytes,
  14.1 percent of the original image.
- Extended the hash-gated US stage-2 disassembly chain with 567 authentic
  bytes: frame dispatcher `$4215..$424B`, coordinate updates
  `$4417..$44A2` and `$44A2..$4519`, the full `$42DB..$43A1` body with local
  entries `$4358/$4386`, and normalization routine `$4519..$4552`. The proof
  requires caller-to-callee JSR, JMP, and BSR, targets in the already-bound
  `$424B/$43A1/$43D6/$4552/$458E` bodies, exact adjacency, and rejects both
  modified media bytes and JP borrowing. The chain now covers 2,877 bytes,
  8.3 percent of the image.
- Added a complete authenticated HuC6270 image bundle with VRAM, VCE, register
  state, and SAT from the same authentic US savestate. The loader hash-gates
  all four files and renders the verified 64×64 BAT at 320×200.
- Implemented the PCE sprite compositor from the captured SAT, preserving the
  original scanline limit, draw order, priority, transparency, pattern
  addressing, sizes, and flips. The PCE 512-entry palette is losslessly
  compacted from used source entries only into M11's 256 entries; the real-data
  test measures 52 used color entries and 211 sprite pixels.
- The CLI path now also requires `--theron-vdc-state` and
  `--theron-vdc-sat` along with VRAM/VCE. The product binary, capture-script
  contract, and authentic four-file regression pass without synthetic image
  data.
- Made native TAKE→inventory→DROP lossless for the Track 02 record's two-bit
  thing position as well. Inventory provenance now stores the position,
  validates it against category/index/position in the source reference, and
  restores both the field and its metadata flags. World save format 12
  explicitly preserves the position; older formats derive it exactly from
  their already-stored source reference.
- DROP no longer creates a host copy of an authentic object. It rebinds the
  original `PICKED_UP` occurrence in the source ledger, preserves its ID and
  raw record, and moves only the runtime coordinates. The real-data test runs
  two complete TAKE→DROP cycles for every US and JP dungeon and compares ref,
  next-ref, index, category, position, and every raw byte without growing the
  object table.
- Source inventory can no longer accept a record merely because its own
  fields are self-consistent. Swap and DROP in a verified Track 02 world now
  require an exact occurrence in the loaded source ledger with matching ref,
  next-ref, index, category, position, raw size, and raw bytes. A negative
  regression test mutates only the ledger after pickup and verifies that DROP
  is rejected.
- Full source-occurrence identity now follows floor objects, TAKE, inventory,
  DROP, and save/load: the original dungeon, level, x, and y remain separate
  from the object's movable runtime position. The real corpus shows why this
  matters: the seven US dungeons contain 204 pairs with identical ref,
  next-ref, index, category, position, and raw record but different origin
  coordinates.
- World save format 13 versions both 91-byte objects and 54-byte inventory
  provenance. Version 12 and earlier retain their previous wire sizes. The
  world hash now also covers the floor object's full source identity and
  origin. US/JP round trips preserve origin across two moves; a separate
  negative test changes only `source_x` and requires DROP to be rejected.
- Native Theron can now reselect carried source items after resume. `I`
  deterministically advances to the next slot for the active champion where
  the compact item ID matches its parallel source record; slots containing
  only a generic ID are skipped. M11 retains the selected slot, and P/DROP
  then applies the unchanged strict raw-record, origin, and ledger gates. The
  US/JP real-data test serializes and deserializes the fully preloaded Track 02
  world after pickup, clears selection, reselects with `I`, and completes DROP
  from the restored world before continuing the ordinary two-cycle round trip.
- Added `--theron-native us|jp` for explicit native region selection when
  authentic `TQUS02.bin` and `TQJP02.bin` are together in the standard
  directory. The CLI resolves only the region's canonical file, then the
  existing hash-first launch intent verifies its identity. US and JP product
  probes now start from the same data root; a separate US-only check confirms
  that selecting JP is rejected instead of borrowing US media.
- Native US/JP startup from a loose, verified Track 02 now automatically
  binds authentic Track 01 CDDA when the corresponding hash-known full-disc
  archive is available in the selected Theron data root. Cold-cache tests
  require `theronTrack01CddaReady=1` for both regions.
- Authentic combined-RAR startup now binds the selected regional CUE to its
  exact same-stem Track 01 OGG without extracting archive members. The
  bounded in-memory stream admits only the authenticated US/JP OGG SHA-256 and
  decodes it through the existing Vorbis CDDA path. Real-media tests hash and
  decode both original OGG members from the user-provided RAR, require queued
  audio sectors, and verify direct US and JP archive startup with
  `theronTrack01CddaReady=1`. This establishes title Track 01 audio only; no
  gameplay track-selection command has been inferred.
- Authentic JP CUE-projected ISO support now normalizes the exact verified
  source bytes after its 224-sector INDEX 01 offset for roster and startup
  readers, and admits the three-anchor source-backed startup bitmap sampling
  route. The real JP ISO regression byte-compares against the original raw
  Track 02 and verifies the seven source dungeon banks and required startup
  bitmap routes. The legacy zero-filled JP ISO stub remains rejected.
- Native `USE_ITEM` now reaches the existing Theron command path for a door
  in the tile in front of the party. Verified locked Track 02 doors remain
  shut without the missing T900 key consumer; no compatibility keys or
  synthetic items are accepted.
- The exact combined US/JP ISO/OGG RAR is now a complete external media
  source. Firestaff materializes a region-specific CUE, builds Track 02 from
  the archive's authentic Track 19 and final segment, and verifies both the
  archive and Track 02 hashes before Mednafen can start.
- Added `theron_v1_jp_raw_bin_startup`, which isolates the authentic Japanese
  raw BIN and verifies its hash, startup profile, Soul Room selection,
  source-bound level startup, and absence of synthetic fallback data through
  the real M11 binary.
- Fixed the JP BIN handoff's raw-sector branch. The Japanese MODE1/2352 file
  now reaches `theron-runtime` with its variant-specific 32 × 27 route,
  without the US-specific spawn source or a generated fallback route.
- `--scan-data` now lists all verified Theron editions in the selected data
  root. A test against the local authentic JP and US BIN files ensures both
  appear while retaining the existing US startup file.
- Added explicit CLI options for the authenticated 64 KiB VRAM and 1 KiB VCE
  captures. They must be supplied together and only with `--game theron`. A
  real-data test runs the mounted capture pair through the production viewport
  and M11 startup path.
- Added `--theron-mednafen`, `--theron-disc`, and
  `--theron-system-card` to run the complete original disc. Check mode with
  `--theron-mednafen-check` verifies the entire media set without starting the
  GUI. The local US disc is accepted with 17 audio tracks, Track 02 hash
  `f23601102138f87c33025877767ebf76`, and System Card hash
  `ff1a674273fe3540ccef576376407d1d`; a lone Track 02 file is rejected.
- `--theron-disc` now also accepts the three known complete `.7z` disc images
  in the Theron directory. The authentic US archive (323,587,109 bytes) was
  materialized as 417,973,741 bytes of original tracks, verified as complete,
  and reopened from cache without extracting again.
- The authentic Japanese archive (320,812,522 bytes) was separately
  materialized as 419,295,603 bytes and verified as a complete 19-track disc
  with Track 02 hash `b7afb338ad31be1025b53f9aff12d73a`.
- Added `--theron-original us|jp`. The option finds the hash-verified original
  edition under Firestaff's standard data directory, and Mednafen and System
  Card 3.0 in their standard locations. Explicit paths are allowed overrides,
  but cannot be used to provide US media to the JP selection or vice versa.
- Removed the raw-BIN startup's automatic skip past the title, level
  selection, and Soul Room. The skip made two fixture-initialized champions
  appear to be a selected party. Both US and JP originals now go through the
  startup sequence and full forcefield-transition source loader; real-data
  tests require a selected two-member party and at least one materialized
  object record from Track 02.
- Connected M11's separate `pickup` command (the G/gamepad equivalent) to
  Theron's front tile and the existing provenance-checked TAKE path. On
  authentic Track 02 routes, inventory mutation still requires the exact raw
  object record, property row, and source occurrence. `drop` and `use` remain
  closed because slot selection and the T900 consumer are not yet source-bound.
- The direct regional roster readers now verify the complete selected US or
  JP Track 02 file against its published MD5 before accepting any champion
  record. The local roster structure remains an additional check, not a
  substitute for file identity. The real-data test changes only the file's
  final byte, outside the roster region, and requires both readers to reject
  the file and clear their receipts.
- Added a machine-checked gate for the original's atomic viewport command. A
  new authentic US capture binds Button I, click `$3c/$78`, command type
  `$50`, 65,536 ordered main-RAM writes, queue completion, and code/RAM images
  to the exact Track 02, System Card, Mednafen, and savestate. The receipt
  deliberately publishes no T900 semantics; a mismatched Track 02 hash is
  rejected and clears the result.
- Connected Tab/`CYCLE_CHAMPION` to the next living champion in the actually
  selected Soul Room party. Dead and unselected roster slots are skipped.
  Both US and JP real-data tests select two champions, enter the route, and
  then verify that the active slot changes from 0 to 1.
- Ran `--theron-original us` and `--theron-original jp` from the real standard
  directory with a completely empty cache for each. US materialized 20 files/
  417,973,741 bytes and JP 20 files/419,295,603 bytes; both were accepted with
  19 BIN tracks, 17 audio tracks, the correct regional Track 02 hash, and
  System Card 3.0. README and the data guide now describe this complete
  original-game path.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-14 — cross-route probe uses an explicit door transition

- ✅ The synthetic cross-route probe now opens its unlocked fixture door via
  `theron_v1_door_open()` before testing movement, consistent with the
  source boundary that prohibits implicit door/key mutation.
- ✅ The full deterministic chain then passes through pool, alarm, trigger,
  teleporter, pit, post-move drain, and TAKE.
- ✅ Verification: focused CTest 1/1 and the complete Theron regex 45/45 pass;
  six capture tests are expected skips without local sidecars.

## 2026-08-13 — source-bound pits no longer use fixture damage

- ✅ Source-bound pit query, movement and the public pit handler now remain
  fail-closed while the T700 consumer is unresolved.
- ✅ The hardening probe verifies unchanged position, HP and stamina.
- 🔒 Fixture levels retain their existing ReDMCSB-based probe behavior.

## 2026-08-13 — incomplete dungeon exits no longer report success

- ✅ Exit query and movement share the `dungeon_complete` guard and propagate
  transition failure as `THERON_MOVE_BLOCKED`.
- ✅ The hardening probe verifies that an incomplete exit leaves party position,
  transition state and stamina unchanged.
- 🔒 Quest completion and next-dungeon source consumers remain separate gates.

## 2026-08-13 — unloaded stairs no longer report success

- ✅ Stairs query and movement now require a loaded destination level and
  propagate transition failure as `THERON_MOVE_BLOCKED`.
- ✅ The hardening probe verifies that an unloaded stairs destination leaves
  party position, level, transition state and stamina unchanged.
- 🔒 This does not infer dynamic source-level loading or new stairs semantics.

## 2026-08-13 — unresolved teleporter movement is fail-closed

- ✅ Movement now checks the transactional teleporter resolver result instead
  of reporting `THERON_MOVE_TELEPORT` after a failed source-data lookup.
- ✅ The hardening probe verifies that a missing endpoint leaves party position,
  transition state, spawn coordinates and stamina unchanged.
- 🔒 This is an invariant fix, not a new teleporter semantic: only the already
  authenticated object-ID and Track 02 coordinate routes can resolve.

## 2026-08-13 — locked door sentinel no longer passes as open

- ✅ The Theron door state machine now treats `LOCKED=6` as a sentinel rather
  than as a later opening frame. Movement query, movement mutation and
  `door_open()` all share the bounded passability predicate.
- ✅ The hardening probe covers the locked-state query, movement block and
  failed open path. Source-authenticated key/object consumption remains
  capture-gated.

## 2026-08-11 — external RNG-edge capture classified without overclaim

- ✅ `theron-capture-20260811-cocoa-save.trace.*` was verified as an
  authenticated US/System Card session with 2,048 register samples, 12
  `$4644` observations, and 50 `$4667` observations.
- ✅ The test now requires parsed provenance and a closed semantic gate, but
  does not incorrectly assume that every real run must lack RNG-edge windows.
- ✅ The RNG parser also accepts the authenticated legacy sidecar layout:
  18-field rows and a window length derived from the first sequence edge.
- 🔒 No valid `$B0E5` spawn record or target publication is available yet.

## 2026-08-12 — preserve the raw A register at the RNG return boundary

- ✅ The parser preserves register A at every authenticated stack-based
  `return_boundary` and counts the observations.
- ✅ The test verifies the raw return boundary without turning A into a host
  RNG or publishing spawn/combat semantics.

## 2026-08-12 — explicitly reject C96B-only combat capture in regression

- ✅ The spawn-trace test can now accept an authenticated C96B-only/autoload
  trace via `THERON_REAL_NEGATIVE_SPAWN_REGISTER_TRACE` and requires it to
  remain rejected without `$CC4C`, a valid `$B0E5` category, and semantic
  publication.
- 🔒 The capture therefore does not enable RNG, AI, combat, generators, T700,
  or T900.

## 2026-08-12 — classify Stage 2 session without gameplay publication

- ✅ A new isolated Mednafen session with a direct SDL2 link reached Stage 2
  and produced 2,048 authenticated register samples; the parser retains this
  provenance even when `$CC4C`/`$B0E5` are entirely absent.
- 🔒 This transport/startup evidence does not establish a dungeon/object
  consumer or original mechanics.

## 2026-08-12 — correct overclaim in the parity matrix

- ✅ Combat and champion-system status now distinguishes authenticated numeric
  records/fixture compatibility from the original's still-missing
  T500/T600/T900 and portrait consumers.
- ✅ JP portrait indices remain fail-closed until real pixels and their
  HuC6280/VDC owner can be bound in the same runtime capture.
- ✅ The new external combat capture is correctly classified as negative
  C96B-only/autoload evidence. The parser's complete spawn admission remains
  closed because `$CC4C`, a valid `$B0E5` category, and loader handoff are
  missing.
- ✅ Its authentic VDC/VCE snapshot (`411960eb`/`6fb303b5`) is now bound to
  the production screen-space presenter without establishing gameplay
  semantics.

## 2026-08-11 — audio status corrected to match the evidence level

- ✅ The parity matrix now distinguishes the static System Card call-site
  catalog from a verified game-event-to-sample consumer.
- ✅ The production audio gate remains active: ADPCM transport evidence alone
  cannot create an SFX mapping.

## 2026-08-11 — inventory property payload is revalidated byte-for-byte

- ✅ TAKE/DROP and source-slot operations now compare all six carried
  property bytes with the authenticated Track 02 item-property row for the
  item ID; a mutated row is rejected.
- ✅ Real US Track 02 dungeon-loader and combat/inventory regression tests pass.
- 🔒 This strengthens provenance only; original T900 equip/use/stack rules are
  still not claimed without the source consumer.

## 2026-08-11 — spawn capture parser preserves overlay evidence

- ✅ Current external register sidecars with `return_pc`/`caller_pc` context
  are parsed while preserving the authenticated HuC6280 bank coordinate.
- ✅ `$B0E5` address hits are counted separately from valid regular-spawn
  categories; the real A=`$2C`/`$85` overlays remain negative evidence.
- 🔒 Strict spawn admission, RNG return ownership, AI, loot, generators,
  T700 and T900 remain closed because no valid category/consumer witness exists.

## 2026-08-10 — autoload replay remains pre-gameplay

- Replayed an authentic external-disk US Track 02 savestate with the
  instrumented Mednafen build. The run delivered 15 scripted PCE input events
  but no authenticated CD→RAM receipt and no gameplay-owned spawn consumer.
- The trace contained 50 `$B0E5` address hits, all with A=`$2C`/`$85` rather
  than a valid regular-spawn category `0..3`; `$4644`, `$4667`, valid spawn
  samples, RNG windows and target writes were all zero.
- No RNG, creature AI, combat, loot, generator, T700 or T900 semantics were
  promoted. The raw trace stayed on the external disk and Mednafen was closed
  after the bounded run.

## 2026-08-10 — authenticated US roster text reaches Theron slot

- Fixed the production forcefield handoff so the authenticated Track 02
  codon-text catalog binds the protagonist name as well as selected
  companions. Previously `party_init()` cleared Theron's production name and
  only companion names were re-applied.
- Added a focused forcefield regression test and verified it against the real
  US/JP Track 02 roster-media test path. No title/control codes, portraits,
  T900 equipment rules or gameplay consumers were inferred.

## 2026-08-11 — real seven-dungeon creature/object admission verified

- ✅ `test_theron_v1_track02_dungeon_loader` passes against authentic
  `TQUS02.bin` and `TQJP02.bin` for all seven dungeons.
- ✅ US/JP category-4 monster records are materialized as live creatures with
  source ref, source index, type, group members, HP, cell, direction/flags, and
  `chested` bound byte-for-byte to the record.
- ✅ Carried weapon/clothing/scroll/potion records retain their raw payload
  and authenticated property row through TAKE/DROP. This still does not prove
  the original attack, AI, RNG, T700, or T900 consumer.
- 🔒 A new external combat replay with 18 PCE events produced snapshots but no
  game-owned CD→RAM handoff or valid `$B0E5`/RNG witness; the raw trace remains
  outside GitHub.

## 2026-08-11 — production Theron viewport uses the authenticated native screen route

- ✅ `theron_vp_render_dungeon()` delegates an explicitly loaded,
  hash-verified VRAM/VCE capture to
  `theron_v1_vram_trace_render_authenticated_screen()`.
- ✅ The real-capture regression compares the production framebuffer byte for
  byte with the direct native-screen consumer before M11 presentation.
- 🔒 The route remains screen-space-only and makes no claim about
  square-to-tile mapping, perspective, HUD, objects, creatures, RNG, T700, or
  T900 semantics.

## 2026-08-10 — launch receipt no longer overclaims level/object readiness

- ✅ Corrected `theron_v1_launch_decision()` so a media-ready launch only
  advertises the authenticated bitmap/capture route.
- ✅ `level_route_ready` and `object_route_ready` now remain `0` until the
  original Track 02 level/object consumer is proven, matching
  `theron_v1_track02_provenance_runtime_consumer.c` and the current negative
  Mednafen runtime witness.
- ✅ Updated `firestaff_theron_v1_launch_decision_probe` and rebuilt the
  focused Theron targets.

## 2026-08-10 — authenticated savestate replay remains non-semantic

- ✅ A new isolated US Track 02 replay from an authentic Mednafen savestate
  verified Track 02 identity, System Card identity, and eight explicit PCE
  input events. The run reached `$B0E5` twice.
- ✅ The capture parser preserved the important distinction between raw
  address hits and valid spawn samples: `spawn_entry_b0e5_samples=0`,
  `spawn_consumer_reads=0`, `rng_consumer_samples=0`, and no target reads or
  writes.
- 🔒 No RNG, creature AI, attack, damage, loot, generator, T700, or T900
  semantics were promoted from this run. It does not meet the requirement for
  a game-owned consumer that binds a return value to an authentic source
  record.

## 2026-08-10 — palette verification now follows authenticated variant

- ✅ Fixed `test_theron_v1_startup_media_palette_bind`: the palette-window
  offset is now selected from the authenticated Track 02 MD5, not from a
  diagnostic label. Environment-driven JP runs therefore validate the real JP
  palette window instead of being misrouted to the US offset.
- ✅ Real `TQUS02.bin` and `TQJP02.bin` both pass the palette and roster checks;
  no runtime palette promotion was opened by this test-only correction.

## 2026-08-10 — real Theron reference capture published

- ✅ README now links a tracked, real original-US Mednafen dungeon capture as
  a visual reference for the bring-up.
- ✅ The README wording explicitly says this is not proof of Firestaff's full
  rendering or gameplay parity.
- ✅ The capture contains no BIOS, system-card, or game-data asset.

## 2026-08-10 — authenticated BAT→VCE palette relation receipt

- ✅ The real VDC/VCE snapshot loader now publishes an explicit
  `vce_palette_relation_verified` receipt after decoding source BAT words and
  4bpp tile bytes. It rechecks every admitted BAT palette group against the
  native little-endian BGR333 words in the authenticated VCE snapshot.
- ✅ The receipt records the observed BAT palette-group mask and is covered by
  `test_theron_v1_vram_trace_loader`.
- 🔒 This is screen-space hardware binding only. It does not authorize
  dungeon-square mapping, perspective, HUD/object ownership, RNG, AI, T700 or
  T900 semantics.

## 2026-08-10 — authenticated File-select/dungeon replay receipt

- ✅ An external capture with the complete US CUE, `Run → Button I`, and real
  movement verified 28 CD→RAM origin receipts and 32 `$E009` dispatches.
- ✅ The receipt parser kept all `$B0E5`/spawn/RNG/target events closed; no
  semantics were promoted from a menu/loader session that does not prove a
  spawn tick.

## 2026-08-10 — save-state replay rejected as non-semantic `$B0E5` overlay

- ✅ The authentic raw US CUE/savestate run verified Track 02 and reached
  `$B0E5`, but all 30 address hits carried A=`$2C`/`$85`. The existing
  source-lock parser rejects them because a regular spawn entry may be
  published only for categories 0–3.
- ✅ No synthetic RNG, creature, AI, loot, T700, or T900 rule was enabled. The
  incorrect cooked-2048-byte run was kept separate and was not used as evidence.

## 2026-08-10 — complete US CUE transport witness

- ✅ The authentic 19-track US layout was verified on the external disc with
  its CUE, CDDA tracks, and Track 02 as specified by the archive's `Decode.bat`.
- ✅ Mednafen reported Track 02 at LBA 3234; the session produced 159 raw
  sectors, 88 spawn-register samples, 17 `$4644` hits, and 64 `$4667` hits.
- 🔒 No valid `$B0E5`/RNG/spawn/object-consumer receipt was captured, so no
  synthetic RNG, AI, loot, T700, or T900 rules were published.

## 2026-08-10 — lossless Track 02 world source ledger

- ✅ The loader now binds every authentically decoded ground-reference record
  to the world ledger. Doors, teleporters, text/actuators, and carried
  item/monster records therefore retain their raw bytes, chain, map, and
  coordinates.
- ✅ World capacity was raised to 4,096, and the US campaign test verifies
  2,266 source occurrences across all seven dungeons; the JP regression also
  passes.
- 🔒 This does not establish the original RNG, AI, T700/T900, item semantics,
  or source-bound media consumers.

## 2026-08-10 — held keyboard input uses Theron cadence

- ✅ Held WASD and arrow-key input now uses Theron's own game tick instead of
  DM1's VBlank flag. A held key therefore advances or turns only at the correct
  runtime boundary and cannot run away at 60 Hz.
- ✅ The ordinary mouse cursor remains source-mapped to its current position;
  moving the mouse does not select or jump between objects. Mouse buttons 1/2,
  short touch, and long touch retain the Button I/II contract.
- ✅ `test_m11_gamepad_csb_input_bridge`, `theron_v1_boot_runtime_input`, and
  the complete main build pass.

## 2026-08-10 — remove unauthenticated creature/generator fallback

- ✅ Removed the obsolete DMWeb/DM1-indexed Theron creature and generator
  table, its standalone test and its unused translation unit. It was not
  sourced from authenticated Track 02 records and could be mistaken for live
  game semantics.
- ✅ The canonical path is now the real US/JP category-4 monster loader and
  its source-record → live-creature materialization, already covered by
  `test_theron_v1_track02_dungeon_loader`.
- 🔒 RNG, dynamic generator timing, AI, combat, loot, T700 and T900 remain
  fail-closed until an authenticated same-run runtime capture binds them.

## 2026-08-10 — cold-start transport witness

- ✅ An external cold start against the authentic US Track 02 session verified
  159 raw sectors, 32 `$E009` dispatches, two CD→RAM origin receipts, 17
  `$4644` and 64 `$4667` edges, and the VDC/VCE snapshot sizes.
- ✅ The negative control is explicit: zero `$B0E5`, RNG windows, special
  branches, spawn consumers, and target reads/writes.
- 🔒 This does not prove gameplay semantics. RNG, spawn, AI, combat, loot,
  generators, T700, and T900 remain fail-closed until a real dungeon, spawn,
  or object consumer is captured in the same session.

## 2026-08-10 — verified VDC/VCE snapshot admission

- ✅ A closed allow-list of five verified VRAM/VCE hash pairs is now shared by
  the production viewport and capture-BMP probe. Four authentic external
  US/JP snapshot pairs passed the end-to-end BAT/tile/palette and M11 tests.
- ✅ Results are in source space: 1,057, 268, 157, and 219 BAT/tile pairs
  loaded, and all four frames produced 512 palette entries and non-empty output.
- 🔒 No snapshot establishes square-to-tile mapping, perspective, HUD/object
  consumers, creatures, RNG, T700, or T900.

## 2026-08-09 — Track 02 teleporter/object-ID correction

- ✅ The authentic Track 02 teleporter record's `ldest` is now read from the
  correct bits 8–13, as specified in `DMBUILDER6/src/dms.h:98-108`.
- ✅ Authentic door and teleporter records now receive Firestaff's actual
  internal object types, allowing source-bound runtime dispatch to reach the
  correct consumer.
- ✅ A teleporter may land on a validated source-bound coordinate even when
  the destination tile has no separate object record. AKUTUBA M0 `(0,0) →
  (2,3)` is verified against the authentic US Track 02 BIN.
- ✅ BIOS, System Card, BIN/CUE/ISO, and other game media remain local on the
  external drive and are ignored by Git.

## 2026-08-09 — Firestaff Theron WASD, mouse, and touch

- ✅ Firestaff's Theron entry now uses a source-specific PC Engine mapping:
  W/S move forward/backward and A/D turn left/right. The global DM1/CSB
  strafe mapping is unchanged.
- ✅ Mouse button 1 sends Button I and mouse button 2 sends Button II. Short
  touch sends Button I and long touch sends Button II through the existing
  startup/dungeon facade; no synthetic game records or semantics are created.
- ✅ Held input is disabled during Theron startup and enabled only after the
  authentic dungeon phase loads. Mapping, SDL3, SDL2, and the complete
  Firestaff build were verified.

## 2026-08-09 — no unsupported portrait owner in the source-bound roster

- ✅ Source-bound US/JP roster initialization now marks the portrait ID as
  `THERON_PORTRAIT_UNAVAILABLE` (`0xff`) until authentic portrait bytes and
  their consumer are bound. Index `0` is no longer used as a false portrait
  reference.
- ✅ The JP roster regression continues to read all eight authentic Track 02
  records and verifies that source-initialized champion records do not publish
  a fabricated portrait ID.

## 2026-08-09 — scope the final legacy-ID branch in the teleporter chain

- ✅ The teleporter resolver's fixture/legacy-ID link now also requires the
  active `dungeon_id`; previously, only Track 02's packed-coordinate link was
  scoped. A foreign object with the same ID can therefore no longer become a
  destination when multiple authentic dungeons are resident.
- ✅ `test_theron_v1_combat_mechanics` covers the negative cross-dungeon
  destination case and passes 116/116.

## 2026-08-09 — source ledger and object pool for the full Track 02 campaign

- ✅ `theron_v1_world_load_track02_dungeon()` now replaces only the selected
  dungeon's levels, source monsters, generators, source objects, and placed
  objects. Authentic records from already-loaded dungeons therefore survive
  later level/bank loads, while reloading the same dungeon removes old records
  without duplicating them.
- ✅ Object IDs are allocated above the highest remaining ID, so dungeon-local
  cleanup cannot alias an object retained from another dungeon. Pool limits
  now fit the full verified US Track 02 campaign: 4,096 placed objects and
  256 category-4 monster records.
- ✅ The real-data regression loads AKUTUBA, DRATOR, then DRATOR again from
  `TQUS02.bin`, checking dungeon scope for monsters/generators/source objects,
  object count, and unique IDs. `test_theron_v1_track02_dungeon_loader` and
  the focused CTest suite pass 7/7.

## 2026-08-09 — object lookup scoped to authenticated dungeon

- ✅ Production mechanics lookups for objects, doors, teleporters, altars,
  pools, and triggers now match `dungeon_id`, level, and coordinates. The
  standalone pre-movement door query, teleporter destinations, alarm generator
  loop, and trigger links are also scoped to the active dungeon. The older
  `theron_v1_object_at()` remains for legacy fixture calls that explicitly
  lack dungeon scope.
- ✅ The regression test places two objects at the same level/coordinates in
  dungeons 1 and 2 and verifies that each source scope sees only its own
  object; the movement query also ignores an open door from the wrong dungeon.

## 2026-08-09 — later-level resource chain: negative verification receipt

- ✅ Documented where the authentic US level 1 probe stops at
  `DECODE_POINTER_TABLE` when the shared prologue is incorrectly tested as a
  pointer-table seed. This prevents a false full decompression from becoming
  production data.
- ✅ The source and new source-lock page bind the next requirement to
  `$23DC -> $23AD`, `$3B7E-$3B85`, the destination, and the `$2600` consumer.
  No synthetic bitmap, tile atlas, palette, map, or object semantics were
  created.

## 2026-08-09 — dungeon-aware source-creature lookup

- ✅ The production source-record-to-live-creature bridge now always matches
  `dungeon_id`, level, and coordinates. The previous lookup could mix two
  authentic records at the same coordinates in different dungeons when a
  transition or direct source call left both in the pool.
- ✅ The compatibility API remains for fixture tests, while production
  mechanics' attack, collision, and spawn paths use the new dungeon-aware
  function. The regression passes with matching coordinates in dungeons 1 and
  2; US/JP source-record loading and existing combat/item gates are unchanged.

## 2026-08-09 — Mednafen InputGrab and layout-stable Button I/II

- ✅ The capture profile on the external drive now uses `Z = Button I`,
  `X = Button II`, `Return = Run`, and `Tab = Select`; comma/period are no
  longer the macOS defaults. The capture script sends the authentic
  `Ctrl+Shift+G` chord before host input.
- ✅ The Mednafen build includes a bounded host-input receipt that permits
  continuation only when the emulator's own `InputGrab` flag writes
  `input_grab_state enabled=1`. v15 builds and links against native SDL2.
- 🔒 An authentic US Track 02 run confirmed both `InputGrab=1` and SDL key
  events, but the BIOS did not advance frames: the PCE continued to read
  `0x3f`, no raw sectors were delivered, and no game-owned consumer was
  reached. This is a verified startup/CD-handoff gap, not semantic evidence
  for RNG, creatures, AI, T700, or T900.

## 2026-08-09 — MPR-/destinationstrace i capture-builden

- ✅ The capture build now applies a post-patch `v3` hook that logs game-owned
  byte writes with logical destination, MPR-calculated physical destination,
  value, and writer PC. A clean Mednafen 1.32.1 build compiled the hook and
  includes the receipt format. `bash
  tests/test_theron_v1_mednafen_live_capture_script.sh`, `bash -n`, and
  `git diff --check` pass.
- 🔒 The receipt's `dispatch_sequence=unbound` is intentional: the writer is
  game-owned but is not yet bound to an E009/CD-sector return contract. No
  level, tile, object, RNG, AI, T700, or T900 semantics have been published.
  The local runtime verifier still stopped the capture binary because the
  machine exposes only `sdl2-compat`.

## 2026-08-09 — byte-faithful HuC6280 resource core

- ✅ `da65` verified the complete retail routine `$23AD–$252A` from the
  hash-locked US ISO; the previously truncated back-reference section in the
  source-lock listing is now complete through `$252A`.
- ✅ `theron_v1_huc6280_decode_resource()` follows the verified variable-bit
  reader, `$0100` expansion, pointer-table back-references, literal stream,
  and low/high-byte copy path. The core is fail-closed for truncation, missing
  table data, destination overflow, and address wrap.
- ✅ Authentic US/JP Track 02 BIN/ISO prologues, resource frames, hashes,
  source-lock receipt, and the Theron library pass focused C11 verification.
  No synthetic game data or semantic tile/map/object promotion was added.

## 2026-08-09 — v3 strict regular-spawn provenance gate

- ✅ A clean v3 replay on authentic US Track 02 used
  `run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30`. Capture verification
  confirmed five scripted PCE input events with Run=`0x0008` and Button
  I=`0x0001`, 5,943 input samples, 161 raw sectors, and 87 MPR-bound spawn
  register samples. Because the same run lacked `$B0E5`, a game-owned dynamic
  CD read, and a dynamic consumer return contract, no synthetic RNG, creature,
  AI, loot, T700, or T900 semantics were enabled.

- ✅ The corrected startup replay `run@8:60,i@480:30,i@900:30` is verified
  against the authentic US Track 02 chain. It produced 10,145 input samples
  with PCE wire masks Button I=`0x0001` and Run=`0x0008`, 161 raw sectors, and
  215 MPR-bound spawn register samples. It did not reach `$B0E5`, a game-owned
  dynamic CD read, or a dynamic spawn-return contract; therefore, no synthetic
  monster, RNG, AI, loot, T700, or T900 semantics were published.
- ✅ The register sidecar is now versioned as `v3` and marks the exact
  disassembly entry `LB0E5` as `spawn_entry_b0e5=1`; the physical PC must still
  match the selected HuC6280 MPR.
- ✅ The strict runtime parser requires `$B0E5` in the same run as
  `$4644`/`$4667` and both consumer windows; semantic publication also
  requires later return evidence. The execution-only parser is explicitly
  weaker and remains diagnostic.
- ✅ A new v3 capture on authentic US Track 02 reached 161 raw sectors and
  87 register samples, but no `$B0E5`; the verifier therefore rejects
  semantic publication. No synthetic monster, RNG, AI, loot, T700, or T900
  records were created.

## 2026-08-09 — macOS global-HID receipt correction

- ✅ The historical v2 spawn-register sidecar bound each physical PC to the
  MPR actually selected for the logical 8 KiB page. The old unversioned
  sidecar can no longer pass the parser.
- ✅ A new headless state autoload on the external drive with hash-verified US
  Track 02 media produced 2,048 v2 samples. Each sample contains the selected
  `mpr_pc`, and the parser accepts the authentic `$C96B–$CA69`/`$CC4C–$CD13`
  execution-window receipts; no semantic RNG/creature/AI/T700/T900 rules were
  enabled because the receipt still lacks a game-owned CD read and the
  `$4644`/`$4667` return chain.
- ✅ A separate authentic new-game replay on the same US Track 02 media
  produced 87 MPR-bound samples, 16 `$4644` pre-consumer hits, and 64 `$4667`
  helper hits. It also confirms 161 raw Track 02 sector reads and 2,048 ADPCM
  FIFO reads, but no `$C96B` hits or `spawn_consumer` RAM reads; this therefore
  still is not a publishable RNG/creature receipt.
- ✅ The Quartz helper now compiles for real: a remaining reference to the
  nonexistent variable `activationAccepted` was removed. The capture test
  type-checks the helper when `swiftc` is available, so comma/period bindings
  can no longer be omitted due to an undetected helper error.
- ✅ The Quartz helper now writes `quartz_frontmost_pid` and uses the actually
  observed frontmost process as focus evidence. The `activate()` return value
  is no longer used alone, since it can be `false` when the correct process is
  already frontmost. A new run must still bring Mednafen to the foreground
  before global HID can be accepted.

## 2026-08-09 — autentiserat execution-window-kvitto

- ✅ The register sidecar from an authentic external-drive state capture can
  be validated separately through both disassembly-locked consumer
  windows: 2,048 samples total, 2,035 in `$C96B–$CA69` and 13 in
  `$CC4C–$CD13`.
- ✅ Register PC is validated against the HuC6280's full 21-bit physical bank
  space; `$0dxxxx` code from the authentic capture is not confused with
  game-main-RAM.
- 🔒 The capture still lacks the `$4644`/`$4667` edges and return ownership.
  The strict spawn gate remains closed, as do RNG, AI, T700, T900, loot, and
  later gameplay semantics.

## 2026-08-09 — macOS Mednafen input grabbing

- ✅ The local Mednafen profile on the external drive and the user's active
  profile now use `Ctrl+Shift+G` for `command.toggle_grab` instead of the
  unusable-on-macOS `Menu` key. With input grabbing enabled, explicit SDL
  bindings for comma (`54`) and period (`55`) work as Button I/II; the
  source-bound PCE wire mask is unchanged.

## 2026-08-09 — authenticated PCE input-mask check

- ✅ `capture_theron_mednafen_live_trace.sh` now rejects a scripted Mednafen
  capture if the observed Button I/II, Select, Run, or direction mask does not
  exactly match the PCE wire layout. This prevents old binaries with incorrect
  Button I/II or Run bits from being used as runtime evidence.
- ✅ A clean rebuild of instrumented Mednafen 1.32.1 on the external drive
  produced I=`0001`, II=`0002`, Run=`0008` on authentic US Track 02. The
  capture path reached authentic sectors, then correctly stopped at the
  remaining absence of a game-owned CD read; no RNG, AI, T700, or T900 rules
  were enabled.

## Theron's Quest

### 2026-08-08 — HuC6280 runtime physical-PC provenance correction

- ✅ The Mednafen IRQ2 evidence path now reconstructs the physical HuC6280
  address from the debugger's `MPR0..MPR7` register group and the logical
  8 KiB page. The same correction is used by the RNG-consumer trace and the
  game-main-RAM admission gate.
- ✅ A fresh replay capture proves the distinction on authentic media:
  `$4644/$4667` executes at physical `0x104644/0x104667`, while copied game
  loader code executes in `0x1fxxxx`. This is provenance only; it does not
  promote RNG, AI, T700, T900, or later-level semantics.

### 2026-08-08 — PCE Button I/II keyboard binding

- ✅ Mednafen's PCE replay masks now follow the real `PCE_GamepadIDII` wire
  vector order. Button I is `0x0001`, Button II `0x0002`, and Run `0x0008`;
  the old `ConfigOrder` values no longer leak into runtime input.
- ✅ The macOS profile now uses layout-stable `Z`/`X` for Button I/II (SDL
  scancodes `29/27`); comma/period remain supported only when explicitly
  configured. A clean instrumented Mednafen
  build and patch dry-run pass; authentic US Track 02 reaches 161 raw sectors.
- 🔒 The capture still has no non-System-Card game-owned CD read, so it does
  not promote the RNG, AI, T700, T900, or later-level semantics.

### 2026-08-08 — source-bound creature spawn category provenance

- ✅ Live creatures created from authentic Track 02 monster groups now retain
  the source regular-spawn category from the retail descriptor. Scripted
  THIEF/DEMON records retain `0xff` as explicitly unbound; no AI, attack or
  RNG meaning is inferred from that value.
- ✅ The field survives the portable world save format. Save version 8 writes
  it, while version 7 loads with the field explicitly unbound for backwards
  compatibility.
- ✅ Real US/JP dungeon-loader, creature-pool and world-save regressions pass.

### 2026-08-08 — lossless T900 item-provenance

- ✅ `Theron_V1_InventorySourceRecord` now preserves the entire authentic
  Track 02 item record (record size and up to 16 raw bytes) through pickup,
  drop, and save/load. Save format version 8 retains compatibility with
  version 6's 31-byte provenance tail and version 7's creature wire format
  without inventing fields.
- ✅ `test_theron_v1_world_serialize_purchase_state` verifies the raw record's
  byte positions after a round trip. The real US/JP
  `test_theron_v1_track02_dungeon_loader` continues to pass with source-bound
  object and creature projection.
- 🔒 This preserves the source losslessly but does not enable T900's unproven
  equip/use/stack/loot rules; the runtime consumer around `$2600` remains
  capture-gated.
- ✅ The dungeon loader now also retains the complete authentic US text-codon
  stream in `Theron_DungeonLoadResult`; JP's verified zero text block remains
  zero. Unresolved HuC6280 control codes are not exposed as UI text.
- ✅ The local original RAR corpus verifies the authentic CUE, OGG track files,
  and Track 02/19 data for the CDDA handoff:
  `test_theron_v1_track01_cdda_handoff` passes with `FIRESTAFF_THERON_CUE`
  against the archive's US files. This is a source-bound CDDA/stream receipt;
  it does not prove SFX/ADPCM event ownership.

### Theron V1

- ✅ 2026-07-13 Theron Track02 completed HuC6260-word receipt: the strict
  Mednafen loader parser now retains completed VCE colour-table words after
  the authenticated dynamic CD_READ/IRQ2 gate, preserving the first
  index/value and ordered FNV receipt separately from CPU `STA` observations.
  It rejects malformed words and does not treat VCE output as Track 02 byte
  taint, palette-table location, or rendering permission. Verification:
  Ninja plus focused CTest `theron_v1_irq2_live_trace_gate`,
  `theron_v1_raw_loader_trace_ingest`, `theron_v1_raw_loader_trace_import`,
  `theron_v1_capture_preflight_chain`, and `theron_v1_capture_manifest`.

- ✅ 2026-07-13 Theron Track02 real loader-trace boundary: replaced the
  hand-authored raw I/O-row importer with a strict parser for the existing
  provenance-marked Mednafen dynamic `CD_READ`/IRQ2 receipt. It checks the
  JP/US MD5-to-record pairing, records only HuC6260 stores after that read,
  and carries the compatible real startup-bitmap receipt forward. A VCE store
  is explicitly not source-byte taint, so the parser cannot verify a palette
  descriptor relation or unlock rendering; incomplete, mismatched, or
  uninstrumented traces fail closed. Added registered CTest probes for trace
  ingestion and preflight binding. Verification: Ninja plus focused CTest
  `theron_v1_irq2_live_trace_gate`, `theron_v1_raw_loader_trace_ingest`,
  `theron_v1_raw_loader_trace_import`, `theron_v1_capture_preflight_chain`,
  and `theron_v1_capture_manifest`.

- ✅ 2026-07-05 Theron V1 probe-registration hygiene gate: added `tools/verify_theron_v1_probe_registration.py` and CTest `theron_v1_probe_registration_hygiene`. The gate requires every `probes/theron/*.c` file to be referenced from `CMakeLists.txt` and rejects the obsolete descriptor-entry API tokens that caused the stale unregistered semantic probe cleanup. Verification: CMake reconfigure succeeded; direct Python verifier passed (`17 Theron probe sources are registered`); focused CTest for startup receipt, M11 direct launch, descriptor-entry roles, and probe-registration hygiene passed 4/4.
- ✅ 2026-07-05 Theron V1 stale descriptor-entry semantic probe cleanup: removed the unregistered `firestaff_theron_v1_track02_descriptor_entry_semantic_probe.c`, which referenced obsolete descriptor-entry API names and was not wired into CMake/CTest. The live coverage remains in `firestaff_theron_v1_track02_descriptor_entry_roles_probe` plus the startup receipt descriptor-role summary. Verification: no remaining old-symbol references; targeted build passed; focused CTest for descriptor-entry roles, startup receipt, and M11 direct launch passed 3/3; direct descriptor-entry roles and startup receipt probes passed with local Track 02 data.
- ✅ 2026-07-05 Theron V1 startup receipt descriptor-role summary: `Theron_V1_StartupReceipt` now records a bounded 9-entry Track 02 descriptor-role summary from `theron_v1_track02_bind_descriptor_entry_roles()`: zero-fill count, pre/post descriptor-data counts, descriptor-table count, descriptor-window entry index, byte-before-descriptor, RTS marker, first nonzero byte after descriptor, and all-zero-after marker. The real-asset receipt probe now locks placeholder defaults plus real JP/US BIN receipts with exactly one descriptor-table role and nine total classified entries. Verification: targeted build passed; `firestaff_theron_v1_startup_real_asset_receipt_probe` passed 128/128 with local JP/US Track 02 BIN data; focused CTest for receipt + M11 direct launch passed 2/2; headless Theron launch against `~/.firestaff/data` passed. Honest scope: descriptor byte-role receipt only; no Track 02 startup bitmap/audio decode or per-dungeon semantic promotion.
- ✅ 2026-07-05 Theron V1 startup render-row test hook: `M11_GameView_GetTheronStartupRenderRows()` now exposes the exact stage-select/Soul Room text rows M11 is preparing to draw, including Continue-slot state, cursor marker, original mirror names, class labels, resurrection status, and the forcefield row. `test_theron_v1_m11_direct_launch` now gates stage-select rows, Soul Room rows for Hakar/Mara/Pental, and the Pental `RESURRECTED` state before forcefield handoff. Verification: targeted build passed; `test_theron_v1_m11_direct_launch` passed; `SDL_VIDEODRIVER=dummy ./build-codex-system-theron-start/firestaff --game theron --data-dir "$HOME/.firestaff/data" --duration 0` launched against local Track 02 data. Honest scope: render-facing text contract only; no Track 02 startup bitmap/audio decode or pixel parity claim.
- ✅ Phase 7 — Narrow semantic Track 02 descriptor-table decoder: new `theron_v1_track02_decode_descriptor_table()` reads the 9-word little-endian stride table that the bank-signal module already locates, validates the documented shape (9 entries, strictly ascending, constant stride `0x0400`, half-open range `[0x0020, 0x2420)`), and is paired with `firestaff_theron_v1_track02_descriptor_table_probe`. The probe regression-locks the synthetic positive path, alt-stride positive path, out-of-range positive path, and seven negative fixtures (truncated input, NULL input, zero expected stride, descending entries, non-strict-ascending duplicate entries, wrong stride, status-name round-trip). On real data the probe hash-gates round-trip checks against the US Track 02 ISO descriptor at `0x1584` and all three US raw BIN anchors (`0x70be06`, `0x70e2c6`, `0x710904`) plus all three JP raw BIN anchors (`0x70b4d6`, `0x70d996`, `0x70ffd4`). Source-locked against `g_us_iso_bank_stride_descriptor` in `src/theron/theron_v1_track02.c`, `docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md`, and the JP Rev 1 zero-image guard. Wired as CTest target `theron_v1_track02_descriptor_table` (PASS). The decoder is shape-driven only: it does NOT claim per-entry semantic types, dungeon-level binding, runtime loader handoff, or level-descriptor semantics — it only locks the byte-shape contract so future semantic work can build on it.
- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ Phase 4 - Rendering pipeline: viewport, tile renderer, palette, and UI chrome are wired into the Theron static library; rendering probes (`firestaff_theron_v1_viewport_renderer_probe`, `firestaff_theron_v1_tile_renderer_probe`) and the rendering integration test (`test_theron_rendering`) are built and green.
- ✅ Phase 5 - Mechanics implementation for movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds, with a 50-assertion mechanics hardening probe (`firestaff_theron_v1_mechanics_hardening_probe`) and a deterministic teleporter-chain probe (`firestaff_theron_v1_teleporter_chain_probe`).
- ✅ Phase 5 - Shop and world-serialization regressions: price-table guard (`test_theron_v1_shop_price_table`) and purchase-state round-trip (`test_theron_v1_world_serialize_purchase_state`) cover parser bounds and party-block atomicity.
- ✅ Phase 5 - Direct-launch path: hash-verified Track 02 loading without re-walking the data root is covered by `test_theron_v1_direct_launch` and the M11 handoff `test_theron_v1_m11_direct_launch`.
- ✅ Phase 5 - Launcher scan reuse: `test_theron_v1_launcher_scan_reuse` exercises the `M12_AssetStatus_Test*` helper path and proves the M12 launcher reuses the verified Theron path and hash on refresh.
- ✅ Phase 6 - Dungeon progression probe coverage.
- ✅ Phase 7 - Save/load coverage: `test_theron_v1_save_load`, `test_theron_v1_save_header_rejection`, and the `firestaff_theron_v1_track02_bank_probe` lock the save header, slot layout, and Track 02 bank signal contracts.
- ✅ Phase 8 verification suite wire-up: test_theron_v1_direct_launch, test_theron_v1_m11_direct_launch, test_theron_v1_launcher_scan_reuse, test_theron_v1_dungeon_progression, test_theron_v1_save_load, test_theron_rendering, test_theron_v1_save_header_rejection, test_theron_v1_shop_price_table, test_theron_v1_world_serialize_purchase_state, plus probes firestaff_theron_v1_teleporter_chain_probe, firestaff_theron_v1_mechanics_hardening_probe, firestaff_theron_v1_viewport_renderer_probe, firestaff_theron_v1_tile_renderer_probe, firestaff_theron_v1_track02_bank_probe, firestaff_theron_v1_track02_descriptor_table_probe are all wired into ctest and pass (17/17 dungeon progression, 9/9 save/load, 18/18 rendering, 3 NEW direct-launch + M11 + scan-reuse tests, 4 NEW viewport/tile/track02-bank/track02-descriptor probes).
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.
- ✅ Theron V1 lib link fix + mechanics + champions + combat probe (2026-06-17): new `src/theron/theron_v1_compat.c` provides compat shim definitions for combat symbols declared in `include/theron_v1_combat.h` but not defined in any .c file (theron_v1_champion_attack, theron_v1_champion_die, theron_v1_creature_ai_tick, theron_v1_creature_at, theron_v1_creature_spawn, theron_v1_creature_kill, theron_v1_creature_remove, theron_v1_creature_by_id, theron_v1_creature_count, theron_v1_creature_attack_champion, theron_v1_calc_attack_damage, theron_v1_calc_defense, theron_v1_modify_champion_hp/stamina/mana, theron_v1_creature_die, theron_v1_drop_loot, theron_v1_play_sound, theron_v1_sound_is_valid). Shims return safe defaults (0/NULL/no-op/THERON_COMBAT_MISS) and preserve V1 game state. The shims that DO mutate state (`modify_champion_hp/stamina/mana`, `champion_die`) clamp to valid ranges. This fix unblocks any consumer of `theron_v1_mechanics.o` (previously link-failed on undefined references). New headless probe `firestaff_theron_v1_mechanics_champions_probe` passes 68/68 (champions party_init + party_dungeon_entry_reset + party_dungeon_exit + get_champion + leader + HP/stamina/mana modification via shims + source evidence; mechanics move_party + turn_party + door_open/close + door queries + door_unlock_with_key + teleporter_resolve + altar_of_vi_resurrect + pool_use + alarm_trigger + trigger_activate + apply_post_move_effects + click_route + source evidence; combat champion_attack returns 0 + creature_attack_champion returns THERON_COMBAT_MISS + champion_die marks dead + creature_ai_tick no-op + creature_at returns NULL + HP/stamina/mana clamp + source evidence). Source-locked against THQUEST.ASM T500/T600/T700/T800/T900, ReDMCSB GROUP/COMMAND/CLIKMENU/GAMELOOP analogues, CSBWin/Resurrect Theron's Quest reimpl.
- ✅ 2026-06-22 Theron V1 shop purchase gate probe: new `firestaff_theron_v1_shop_purchase_gate_probe` (89/89) registered as CTest target `theron_v1_shop_purchase_gate_probe` with labels `tier4;theron;shop;purchase;gate`. Pairs with the existing `test_theron_v1_shop_price_table` and `test_theron_v1_world_serialize_purchase_state` by covering the narrower purchase-gate edges the unit test does not lock: (1) multi-champion slot targeting — purchase lands in the requested slot's inventory[0], other champions stay byte-identical; (2) sequential stock decrement chain — 3 buys → stock 3→2→1→0, 4th attempt reports THERON_SHOP_OUT_OF_STOCK with gold/stock preserved; (3) exact-gold purchase — gold==price drains to 0 with no underflow; (4) inventory slot allocation monotonicity — purchase lands at first empty slot (slot 5) when slots 0..4 are pre-filled; (5) stock=0xFF boundary depletion — 30 buys of stock=255 succeed (THERON_INVENTORY_SLOTS cap), 31st reports THERON_SHOP_INVENTORY_FULL with gold/stock preserved; (6) status-name round-trip — every THERON_SHOP_* enum maps to a distinct non-NULL string, plus out-of-range enum returns "unknown"; (7) source-evidence citation — string contains THQUEST + T560 + T800 + ReDMCSB markers. Source-locked against THQUEST.ASM T560 (item table) + T800 (champion persistence / gold field), docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5.3 (champion_gold offset + Theron-specific persistence), and ReDMCSB has no Theron shop source (DM1/CSB decompilation only).

### Theron V2.0 / V2.1 / V2.2

- ✅ Phase 0 V1 compatibility lock + Phase 1 V2 launch/profile separation: `theron_v2_phase_gate_pc34.c` (include/theron_v2_phase_gate_pc34.h) introduces a 16-domain classification (12 V1-source-locked + 4 V2-presentation-eligible) with per-domain `THERON_V2_PhaseGateDecision` (v1SourceLocked, v2PresentationAllowed, sourceAnchor, rule). V1-locked domains (TRACK02_BANK, BOOT_PROFILE, CHAMPION_PARTY, DUNGEON_PROGRESSION, MECHANICS, SAVE_LOAD, SHOP, TILE_RENDERER, VIEWPORT, WORLD_STATE, PALETTE, UI_CHROME) stay V1-locked regardless of V2 toggles. V2-eligible domains (PRESENTATION_MODE, TEXTURE_UPSCALE, FILTER_CONFIG, MODERN_SHAPES) require v2PresentationEnabled=1; FILTER_CONFIG additionally requires v2ConfigPersistenceEnabled=1 (stricter gate because filter writes are persistent state changes). Default config: V1-only, both toggles off. Ctest target `test_theron_v2_phase_gate_pc34` passes 220/220 (defaults, null-args, V1/V2-on behaviour, FILTER_CONFIG-persistence gate, v2_active, all 17 domain names, source-evidence, all-domain anchor, unknown-domain safety, Track 02 asset-hash pin). Headless probe `firestaff_theron_v2_phase0_v1_compatibility_lock_probe` passes 192/192. Headless probe `firestaff_theron_v2_phase1_launch_profile_separation_probe` passes 52/52 (launch gate, profile gate, Track 02 hash separation JP Rev 1 + US ISO MD5, cross-game hash separation Theron≠DM1≠CSB, V1-only default, headless safety). Source-locked against THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900, theron_v1_track02.c, theron_v1_boot.c, theron_v1_champions.c, theron_v1_dungeon_progression.c, theron_v1_mechanics.c, theron_v1_save_load.c, theron_v1_shop.c, theron_v1_tile_renderer.c, theron_v1_viewport.c, theron_v1_world.c, theron_v1_palette.c, theron_v1_ui_chrome.c, HuC6260/HuC6270 VDC/VCE datasheet, HuC6280 CPU datasheet, ADPCM audio codec, docs/source-lock/tqr_v1_phase{0,1,2}*.md, ReDMCSB CLIKMENU/COMMAND/MOVESENS.
- ✅ Theron V2 presentation-mode selection: `theron_v2_presentation_mode_pc34` module (include/theron_v2_presentation_mode_pc34.h, src/theron/theron_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the Theron V2 presentation runtime. `theron_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=theron). Fallback chain V22→V21 when the modern asset pack is absent. Three independent presentation-mode globals (DM1/CSB/Theron) verified by `t_independent_from_dm1_csb`. CTEST target `test_theron_v2_presentation_mode_pc34` passes 40/40, headless probe `firestaff_theron_v2_presentation_mode_probe` passes 23/23. Source-locked against ReDMCSB COMMAND.C F0359, CLIKMENU.C F0365/F0366, MOVESENS.C:475-538, THQUEST.ASM T400/T520/T560/T600/T700/T800/T900, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.1 texture upscale pipeline: `theron_v2_texture_upscale_pc34` (include/theron_v2_texture_upscale_pc34.h, src/theron/theron_v2_texture_upscale_pc34.c) provides the EPX 2x + bilinear + nearest + full V1→EPX→palette→RGBA pipeline for Theron's PC Engine CD V1 base (256x224 NTSC, 4bpp HuC6270 VCE). Theron-specific helpers: `theron_v2_upscale_ntsc_fullscreen` (256x224 NTSC native) and `theron_v2_upscale_dungeon_viewport` (192x160 letterboxed gameplay view, 4x3 letterbox, 24 tiles wide x 20 tiles tall). Wired into `theron_v2_presentation_mode_set()` so the EPX scale follows the active mode. CTEST target `test_theron_v2_texture_upscale_pc34` passes 28/28, headless probe `firestaff_theron_v2_texture_upscale_probe` passes 14/14. Source-locked against THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7, and the EPX/Scale2x algorithm (http://www.scale2x.it/).
- ✅ Theron V2.2 modern shape book: `theron_v22_shapes` (include/theron_v22_shapes.h, src/theron/theron_v22_shapes.c) provides the 4x3 (4 depth x 3 lateral) shape book parallel to DM1 V2.2 and CSB V2.2. 13 wall variants (D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C + DOOR + SECRET), 7 floor shapes (plain, cracked, mossy, pit, stairs_up, stairs_down, flooded — Theron-only). 11 builtin materials. Theron-only shapes beyond DM1: FIELD_TELEPORTER (THQUEST.ASM T700), FIELD_ALARM (T800 alert dispatch), SECRET_DOOR (T800 hidden passage), FLOODED (water/flooded squares), LIT_TORCH (4+ torch slots, not 4 like DM1), and THERON_V22_LIGHT_ALARM_PULSE (red pulse glow). CSB-equivalent helpers: `theron_v22_shape_for_teleporter`, `theron_v22_shape_for_alarm`, `theron_v22_shape_for_secret_door`, `theron_v22_shape_for_lit_torch`. Wired into `theron_v2_presentation_mode_set()` via `theron_v22_shapes_init()` on V22 entry. CTEST target `test_theron_v22_shapes_pc34` passes 41/41, headless probe `firestaff_theron_v22_shapes_probe` passes 16/16. Source-locked against THQUEST.ASM T400/T520/T600/T700/T800, HuC6260/HuC6270 VDC/VCE datasheet, include/theron_v1_world.h (THERON_SQUARE_* enum), tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.0/V2.1/V2.2 settings persistence in M12 menu config: extended `M12_Config` + `M12_MenuSettingsState` with `theronV2ScalePercent` / `theronV2BilinearEnabled` / `theronV2CrtScanlinesEnabled` / `theronV2CrtScanlineStrength` / `theronV2PaletteCorrectionEnabled` / `theronV2DitherCleanupEnabled` (same pattern as CSB V2, defaults 200% scale, 0 bilinear, 0 scanlines, 35 strength, 0 palette, 0 dither). Round-tripped through `M12_Config_SetDefaults` + the text Load + the text Save + the JSON Export + the JSON Import. New bridge module `theron_v2_settings_pc34` (include/theron_v2_settings_pc34.h, src/theron/theron_v2_settings_pc34.c) mirrors `csb_v2_settings_pc34`: `Theron_V2_Settings` struct, `theron_v2_settings_from_m12_config` / `theron_v2_settings_apply_to_m12_config` / `theron_v2_settings_apply_to_runtime` (pushes scale + bilinear into `theron_v2_upscale_init` + filter toggles into `theron_v2_filter_config_apply`). ctest target `test_theron_v2_settings_pc34` passes 23/23, headless probe `firestaff_theron_v2_settings_probe` passes 12/12. **Wire-up done:** `M11_GameView_OpenSelectedMenuEntry` reads `menuState->settings.theronV2*` and calls `theron_v2_settings_apply_to_runtime()` right before `M11_GameView_Start`. New `theron_v2_upscale_get_scale()` + `theron_v2_upscale_get_bilinear()` accessors let the wire-up probe verify the live runtime. Headless probe `firestaff_m12_v2_settings_wire_up_probe` covers both CSB + Theron (16/16 combined). **Filter config wired:** new `theron_v2_filter_config_pc34` module (include/theron_v2_filter_config_pc34.h, src/theron/theron_v2_filter_config_pc34.c) parallels the CSB filter config for the PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline. ctest target `test_theron_v2_filter_config_pc34` passes 24/24, headless probe `firestaff_theron_v2_filter_config_probe` passes 18/18. Source-locked against include/dm1_v2_settings_pc34.h, include/csb_v2_settings_pc34.h, include/theron_v2_texture_upscale_pc34.h, include/theron_v22_shapes.h, include/theron_v2_presentation_mode_pc34.h, include/config_m12.h, THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE.

## 2026-07-14 — Theron production initial-level capture gate

The production Soul Room entry now consumes the manifest-bound coalesced
Mednafen `$e009` receipt instead of permitting the earlier Stage 3/IRQ2
receipt alone. It rehashes Track 02, System Card, and transcript before
binding record `0x0b52` to the source-locked initial-level envelope. This is
only a fail-closed loader/media admission; no dungeon/object/visual semantics
are claimed. A positive result still requires a fresh authentic capture.
# 2026-07-14 — CSBWin EDBT_ObjectWeights runtime handoff

- Bound CSBWin `Mouse.cpp::GetObjectWeight`'s `EDBT_ObjectWeights` chest-base
  lookup to Firestaff's live ReDMCSB `DUNGEON.C F0140` container path. The
  DB11/EXPOOL record is consumed only while the complete appended tail matches
  its stored FNV receipt; absent records retain CSBWin's source default of 50,
  while altered, truncated, short, or out-of-range records cannot fall back.
- Extended `csb_v1_runtime_champion_load_attrs` with the original CSBWin
  `EXPOOL::Locate` key/hash/node layout, live child-content addition, and a
  changed-receipt rejection case.
# ✅ 2026-07-14 Theron PID-targeted Quartz host-input receipt

# ✅ 2026-07-15 Theron BIN/CUE Track 02 admission

The media classifier now records CUE Track 02's declared `MODE1/2048` or
`MODE1/2352` sector width and accepts either as one authentic Track 01/Track
02 pair. The scanner sends only 2352-byte data to the raw IPL receipt;
2048-byte CUE media remains on the existing verified ISO route. No sector
extraction, wrapper or fallback was added. Verification:
`firestaff_theron_media_classify_unit` and
`theron_v1_track02_cue_layout`.

# ✅ 2026-07-15 Theron 2048 ISO CUE startup handoff

M11 now validates and retains a Track 02 loader receipt only when the scanner
actually issued a valid raw `MODE1/2352` IPL receipt. A verified CUE-declared
`MODE1/2048` ISO therefore follows the normal Track 02 startup handoff and is
not reported as an invalid raw BIN. Verification:
`theron_v1_launcher_scan_reuse` and
`theron_v1_m11_launcher_handoff_boundary`.

# ✅ 2026-07-15 Theron ISO identity at Soul Room boundary

The boot-profile forcefield handoff now distinguishes raw BIN and ISO Track
02 variants. Raw BIN remains behind its authenticated IPL/IRQ2 capture gate;
a verified 2048-byte ISO retains its exact MD5 and source bytes through Soul
Room to the existing ISO semantic dungeon route. That route stays fail-closed
until original ISO bytes prove a first level/object handoff. Verification:
`theron_v1_m11_launcher_handoff_boundary` checks installed real media and
preserves the selected Track 02 identity through startup.
- ✅ 2026-07-15 DM2 M11 source render handoff: the live
  `m11_game_view` DM2 runtime route now calls
  `dm2_v1_boot_runtime_render_frame()` with no V2 callback after verified
  boot, so its dungeon frame consumes the source-owned G1 pose and GDAT
  materials instead of `dm2_v2_runtime_render_frame()`'s procedural viewport.
  The optional V2 HUD remains a decoded original-GDAT compositor and missing
  source data draws nothing. `test_dm2_v1_boot_profile_smoke` now locks the
  direct route: no V2 attempt, successful V1 render, real-material receipt,
  and zero core fallbacks.

# ✅ 2026-07-15 Theron Track 02 runtime bitmap provenance

The existing verified title, stage, Soul Room, and forcefield indexed bitmap
routes now carry their original Track 02 MD5 plus raw and MODE1 user-data
offset envelope into `Theron_V1_World`. A selected runtime level-bank receipt
copies that same source envelope, so a later consumer can require exact
source bytes instead of treating retained pixels as unowned data. The bind
rejects unknown/mismatched variants and incomplete spans. It still performs
no palette binding, RGB conversion, layout inference, object-table decoding,
or drawing. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.
# ✅ 2026-07-15 Theron authenticated CD-read runtime record

The independently authenticated Track 02 `$0b52` CD-read payload now enters
the runtime world as an opaque source receipt: canonical Track 02 MD5, raw
user-data offset, destination, whole-payload checksum, and the exact
post-envelope byte range/checksum. The receipt is published even while the
level route remains rejected, allowing a later captured game-owned consumer
to bind it without reopening media or treating copied bytes as unowned. It is
explicitly marked no-semantic-promotion: no level, object, palette, bitmap,
or visual behavior is inferred and no fallback is enabled. Verification:
Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 loader-envelope boundary

Runtime admission now derives the documented boundary inside the authenticated
`$0b52` CD-read record: the loader-provided initial envelope must begin at its
record-relative offset, match original Track 02 bytes and checksum, and end
exactly where the separately hash-verified opaque continuation begins. The
world receipt retains both spans only after these checks pass. This proves
source-byte boundaries and continuity, not level-grid, object-table, palette,
or visual semantics; the runtime remains no-draw without a captured game-owned
consumer. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 runtime boundary-byte retention

The runtime loader receipt now retains the actual authenticated initial
level-envelope bytes and their directly adjacent post-envelope bytes from the
original `$0b52` CD-read record. Both spans must fit the record, be adjacent,
and rehash to their loader-provided checksums before they are copied. This is
a source-owned boundary for a future captured level/object consumer, not an
object-table decoder: the continuation remains opaque and no palette, grid,
object, or visual semantics are promoted. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation-consumer boundary

The raw loader-trace intake can now admit a game-RAM byte only when its
original READ(6), FIFO-to-RAM, and game-owned consumer chain resolves to the
directly adjacent continuation after the authenticated `$0b52` envelope.
The receipt records its exact continuation-relative offset and source byte,
while rejecting preceding and out-of-range bytes. It remains deliberately
opaque: no object-table, level, palette, bitmap, grid, or visual semantics
are inferred. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation prefix receipt

The loader-trace route can now require a contiguous 12-byte prefix of the
authenticated post-envelope continuation from one ordered CD dispatch. Every
byte is independently tied to the original sector and one game-RAM consumer
chain; a split SCSI generation/LBA/dispatch is rejected. The retained prefix
is only a future capture anchor, not an object-table header or decoder.
Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation TII source binding

The provenance-marked Mednafen main-RAM-loader trace can now bind one original
`TII` transfer only when its source begins at `$3c80`: the continuation start
derived from the authenticated `$3800` sector receipt. The copied source span
is checksummed against retained original bytes and the capture must carry the
producer marker; unrelated `TII` rows are ignored. Destination content stays
opaque, with no object-table, level, palette, bitmap, grid, or rendering
claim. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 live TII capture intake

`capture_theron_mednafen_live_trace.sh` now writes the provenance-marked
main-RAM-loader trace beside the existing IRQ/CD/input traces. Its transition
receipt reports all observed `TII` rows and the subset whose source is `$3c80`,
the authenticated continuation boundary. Empty counts remain evidence of an
unreached original route; the script manufactures no trace or candidate.
Verification: `test_theron_v1_mednafen_live_capture_script.sh` passes; the
patch-shape gate passes and skip-cleans without `MEDNAFEN_SOURCE`.

# ✅ 2026-07-15 Theron Track 02 TII sidecar import

The continuation-transfer admission now accepts one explicit bounded
main-RAM-loader sidecar file and forwards its original text unchanged to the
strict TII parser. Missing, empty, oversize, and malformed sidecars reject;
the import does not create rows, bytes, or semantic fallback. This makes the
live capture producer directly consumable once authentic media reaches the
post-`$3800` transfer route.

# ✅ 2026-07-15 Theron Track 02 continuation execution handoff

The raw loader-trace route now binds a source-verified `$3c80` continuation
`TII` to a later main-RAM `JSR` only when the call target exactly equals the
TII destination. This demonstrates an original CD-byte-to-code stage handoff
without interpreting the copied memory as a level or object table. Duplicate,
wrong-target, or unmarked control rows reject. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 manifest descriptor boundary

Stage-three descriptor 0 now reaches the runtime loader gate as a strict
original-media boundary receipt. Firestaff retains the descriptor's three raw
words plus its derived Track 02 record, MODE1 raw sector, user-data offset,
2048-byte length, and FNV-1a hash. All fields must resolve back to the same
authenticated `$3800` Stage-3 sector before startup admission. The receipt is
deliberately non-semantic: it does not classify the descriptor or sector as a
level, object table, tile, palette, bitmap, command, or visual route. The
focused descriptor probe covers valid coordinates and rejection of malformed
MODE1/zero-selector records. Verification: Ninja probe, `test_theron_rendering`
18/18, and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 full descriptor-row handoff

The authenticated later `$e009` route now carries the complete raw Stage-3
descriptor row into the runtime handoff: descriptor ordinal, `word0`, `word1`,
selector `word2`, resolved Track 02 record, and the selected MODE1 user-data
hash. Firestaff independently derives those values from canonical Track 02
bytes before accepting the coalesced loader receipt; changed row bytes or a
changed selected sector reject the handoff. The row remains explicitly opaque:
no level, object table, tile, palette, bitmap, command, or visual semantics
are promoted. Verification: focused raw-handoff probe (skip-safe without the
authentic corpus), `test_theron_rendering` 18/18, and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor alias-table receipt

Descriptor-to-record admission now retains the selected raw selector's table
relationship: occurrence count, first and last descriptor ordinal, and an
FNV-1a hash over every matching `(ordinal, word0, word1, word2)` row. The
coalesced loader/CD receipt and runtime handoff both independently re-derive
this relation from the authentic Stage-3 manifest, rejecting changed aliases
or a mismatched selected row. These are table-identity facts only: aliases and
their ordering do not identify a level, object, tile, palette, bitmap, loader
command, or visual route. Verification: descriptor-correlation probe covers a
duplicated selector relation and rejection paths; `test_theron_rendering`
18/18 and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor source-span binding

Each admitted later descriptor record now keeps the exact six-byte big-endian
row span from the authenticated loaded Stage-3 MODE1 sector. Firestaff checks
the physical raw offset and FNV-1a against the three retained raw words before
the later sector may reach the runtime handoff. This closes the source-table
to-selected-sector byte boundary without interpreting any descriptor field,
target record, graphics, palette, object, level, or command grammar.
Verification: focused descriptor probe validates the byte span plus malformed
MODE1/zero-selector rejection; `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 copied-continuation termination receipt

The instrumented Mednafen main-RAM loader trace now emits HuC6280 RTS rows.
Continuation admission requires one source-bound `$3c80` TII, a later JSR to
its exact destination, and exactly one RTS whose PC lies inside the copied
destination span. The capture script reports RTS count for acquisition. This
proves only that original copied code reaches a termination instruction; it
does not observe a return target or promote level, object, palette, bitmap,
tile, command, or rendering semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18, `test_theron_v1_startup_save_resume_pc34`
258/258, patch-shape test skip-cleans without `MEDNAFEN_SOURCE`, and capture
script contract test passes.

# ✅ 2026-07-15 Theron Track 02 copied-continuation post-RTS receipt

The instrumented main-RAM loader trace now emits the first observed main-RAM
instruction after each captured RTS. Continuation admission requires that row
to reference the single RTS inside the source-bound `$3c80` TII destination
span and to land at the matching JSR return PC. The receipt retains its
physical PC and opcode alongside the already source-bound Track 02 transfer.
This proves control flow from copied original bytes back to the observed
return target only; it does not classify a descriptor, record, level, object,
tile, palette, bitmap, command, or visual route. Verification: focused
raw-loader probe (skip-safe without the authentic corpus),
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, patch-shape test
skip-cleans without `MEDNAFEN_SOURCE`, and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine-call receipt

When the authenticated post-RTS instruction is a HuC6280 `JSR`, Firestaff now
requires the immediately adjacent original main-RAM-loader trace row to agree
on its logical PC, physical PC, and immediate target. The new receipt carries
the earlier source-bound Track 02 TII/execution chain, so the call is tied to
copied original bytes without inventing a called-routine ABI or data format.
Missing, reordered, or changed call-site rows reject. This proves only a
control-flow target, not a descriptor, record, level, object, tile, palette,
bitmap, command, or visual route. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine termination receipt

The post-return routine-call receipt now requires one later main-RAM `RTS`
with a linked original `post_rts` row returning to the exact caller address.
Nested returns remain opaque and do not satisfy the receipt unless their
observed return address is the bound caller. This extends the authentic
Track 02 TII/copy/call/return control-flow chain without assigning any called
routine, table, record, level, object, tile, palette, bitmap, command, or
visual semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call receipt

After the authenticated post-return caller resumes, Firestaff now admits the
first subsequent main-RAM `JSR` row from the same original trace and retains
its exact physical call site and immediate target. The receipt nests the full
source-bound Track 02 TII/copy/call/return chain. It deliberately does not
identify the target routine, an ABI, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual route. Verification:
Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: genuine Mednafen 1.32.1 patch
dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-entry successor receipt

The Mednafen producer now records the next observed main-RAM instruction
after an authenticated caller-next routine entry. Firestaff requires the
exact entry logical/physical PC plus the successor logical/physical PC and
raw opcode, retaining the full source-bound Track 02 chain. A target that does
not continue through observed main RAM produces no receipt. This is execution
ordering only: no opcode, ABI, loader, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual semantics are
promoted. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor TII byte receipt

When the authenticated caller-entry successor executes HuC6280 `TII`,
Firestaff now accepts it only when its entire source interval lies inside the
already source-bound Track 02 continuation copy. The receipt retains exact
RAM source/destination coordinates, byte count, corresponding original source
coordinate, and FNV-1a checksum. This proves the observed caller path
re-copied known original bytes, without assigning them a loader, descriptor,
CD-read, table, record, level, object, tile, palette, bitmap, command, or
visual meaning. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja
focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor destination-call receipt

The first observed main-RAM `JSR` after an admitted caller-successor `TII`
must now call that transfer's copied destination. The nested receipt retains
the source-bound Track 02 interval and exact call site, proving a bounded
original-byte-to-execution chain. It does not classify the called routine or
bytes as a loader, descriptor, CD read, table, record, level, object, tile,
palette, bitmap, command, or visual route. Verification: genuine Mednafen
1.32.1 patch dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# Theron later-level resource-frame receipt (2026-08-06)

- ✅ Later-level runtime handoff now retains the authenticated `LE16(+2)-5`
  resource length and the exact user-data end offset of the framed bitstream,
  alongside the existing block/span hashes and per-level metadata.
- ✅ Focused real-media level-block and runtime-receipt tests pass for the
  bounded frame contract; no decompression, tile, map or object semantics were
  promoted.

# Theron authentic archive capture boundary (2026-08-06)

# Theron US roster label quarantine (2026-08-06)

- ✅ Audited the claimed US roster locator against the authenticated
  `TQUS02.bin`; the old offset is executable code, not a champion text table.
- ✅ Production retains the cross-checked numeric records needed by the
  forcefield handoff but compiles out unbound US names/titles; the named
  table is now explicit fixture/probe data only.
- ✅ Added null-safe champion initialization and kept the real JP roster
  cluster reader unchanged; production handoff, source-boundary, startup
  media, mechanics and fixture probes remain green.

# Theron JP champion record receipt (2026-08-06)

- ✅ Added a hash-gated reader for the authentic JP Track 02 cluster at raw
  offset `0x0B3D98`, covering all eight records and their newline/NUL framing.
- ✅ Decoded the real A–P nibble representation into HP/stamina/mana, seven
  attributes and 16 skill values, with regional-hash and mutation rejection.
- ✅ The receipt remains source-format evidence; it does not promote portraits,
  US labels or gameplay semantics.

# Theron TQTR capture-offset correction (2026-08-06)

# Theron real Track 02 bank reload hygiene (2026-08-06)

- ✅ The source-faithful world loader now clears the selected dungeon's level
  directory before loading a replacement Track 02 bank, so a shorter real
  dungeon cannot expose stale later-level records from a previous load.
- ✅ The regression reloads authenticated US DRATOR (8 maps) with real US
  SHADODAN (3 maps) and confirms only the three current levels remain loaded;
  the complete US/JP Track 02 object-chain census still passes.

# Theron startup font presentation gate (2026-08-06)

# Theron startup menu availability boundary (2026-08-06)

- ✅ Soul Room mirrors without an authenticated Track 02 roster record remain
  visible as `UNAVAILABLE` but are no longer selectable.
- ✅ M11 keyboard/controller focus skips unavailable mirrors, pointer hit-tests
  reject them, and stale focus cannot toggle one; the Forcefield remains an
  enabled route.
- ✅ M11 launcher handoff and direct-launch regressions pass with the real US
  Track 02 asset.

# Theron startup fallback quarantine (2026-08-06)

- ✅ Authenticated Track 02 media now blocks the legacy host border/text
  fallback before the graphics executor runs.
- ✅ M11 and boot-contract regressions verify that missing original startup
  graphics stay capture-gated instead of becoming synthetic UI pixels.

# Theron startup palette promotion gate (2026-08-06)

- ✅ Authenticated raw palette windows remain inspectable as source candidates,
  but no longer become a runtime palette without HuC6260 consumer evidence.
- ✅ Real US/JP palette-window regressions verify the candidate bytes while
  confirming that runtime presentation remains gated.

# Theron startup VDC/VCE presentation gate (2026-08-06)

- ✅ Candidate startup atlas pixels require an explicit presentation-route
  proof in addition to source media and palette state.
- ✅ M11 remains no-draw when only a palette candidate is present; the gate
  awaits a captured VDC/VCE destination and semantic route.

# Theron Track 19 raw-sector intake (2026-08-06)

- ✅ Track 19 inventory now accepts authenticated MODE1/2352 files by
  stripping only the 16-byte sector header before ISO-coordinate validation.
- ✅ The raw transport identity remains explicit; real object and later-level
  semantics are still not promoted without an original consumer trace.

# Theron JP startup roster real-data regression (2026-08-06)

- ✅ The startup-media regression now reads the authenticated local
  `TQJP02.bin` and verifies all eight source roster names and titles before
  they can reach the startup menu.
- ✅ The US path remains fail-closed because its real text consumer and
  champion-name payload are still unproven.

# Theron forcefield source handoff (2026-08-06)

- ✅ Interactive Soul Room → `ENTER FORCEFIELD` now consumes authenticated raw
  MODE1/2352 Track 02 through the source-faithful dungeon loader. Real map
  headers and bounded source records reach the live world without synthetic
  rooms or guessed host item mappings.
- ✅ Visual VDC/VCE capture remains separately gated. Focused M11 and real
  Track 02 loader tests pass: 59/59 and all seven US/JP dungeon blocks.

# Theron startup animation evidence boundary (2026-08-06)

- ✅ Documented the real startup media boundary: authenticated Track 02
  bitmap spans, atlas routes, font tiles and variant palettes are bound.
- ✅ Kept the original title/Soul Room animation consumer, frame table,
  VBlank cadence and VDC/VCE destination capture-gated. The M11 timing receipt
  is not presented as original animation parity.
- ✅ Removed the synthetic M11 8-frame/6-tick title timer and its state field.
  Authenticated startup now exposes one static title frame and accepts the
  menu immediately; no changing frame is claimed without source evidence.

# Theron complete static decompressor listing (2026-08-06)

- ✅ Expanded `docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`
  with the authenticated caller/output-size tail `$2386–$23a3` and the
  resource framing/variable-bit entry `$23ad–$243d`, using the real US ISO
  projection and the byte-identical JP bank span.
- ✅ The listing now shows the real six-byte resource-frame advance,
  destination-pointer table writes and widening `$0100` token contract. It
  still publishes no level, object, tile or palette semantics.

# Theron forcefield Enter retry boundary (2026-08-06)

- ✅ After a failed authentic Track 02 admission, M11 now keeps Enter bound
  to the forcefield action while the Soul Room shows `FORCEFIELD LOCKED`.
  This prevents the restored cursor from making Enter toggle a mirror and
  makes the capture gate actionable and visible without admitting fallback
  dungeon graphics.
- ✅ Added a regression covering the initial admission failure and a second
  Enter retry; both remain in the Soul Room with `level_loaded == 0`.

# Theron bounded `$2600` consumer trace (2026-08-06)

- ✅ The capture-only Mednafen patch now has a separate bounded trace for
  bank `$1f` logical reads in `$2600–$27ff`. The complete original-like US
  CUE replay produced zero rows, so no dynamic consumer, level record or
  object meaning is promoted.

# Theron authentic VDC/VCE screen-space capture (2026-08-06)

- ✅ A clean SIGINT shutdown of the instrumented Mednafen replay now emits
  the complete authentic US Track 02 VDC/VCE state: 65,536-byte VRAM and
  1,024-byte VCE snapshots, from the hash-verified ISO and real System Card.
- ✅ The production viewport already mounts both snapshots only through the
  explicit `FIRESTAFF_THERON_VRAM_SNAPSHOT`/
  `FIRESTAFF_THERON_VCE_SNAPSHOT` route. The real-capture regression reports
  154 BAT tile/palette pairs, 512 palette entries and 9,954 non-zero indexed
  pixels presented to M11; no inferred square/object meaning is published.
- ✅ `capture_theron_mednafen_live_trace.sh` now defaults to SIGINT for the
  bounded emulator shutdown, so the Mednafen snapshot hook runs on clean exit.
  The capture used SDL 2.32.70 through `sdl2-compat` with dummy video; it is
  authentic emulator memory evidence, not native Quartz/SDL2 evidence.

# Theron real main-RAM loader capture parser correction (2026-08-06)

- ✅ A fresh replay against the supplied hash-verified US Track 02 ISO and
  System Card produced a real Mednafen loader sidecar with the source `$2286`
  `TIA` witness followed by 13 block transfers, 24 RTS observations and 24
  post-RTS observations. The sidecar also produced 4,096 game-owned
  main-RAM-consumer reads and the executed HuC6280 `$2c54–$2c69` code window.
- ✅ Fixed `theron_v1_mednafen_main_ram_trace` so it accepts the complete
  instrumented HuC6280 transfer/control witness instead of comparing every
  later `TII`/return row to the first `TIA`. The parser remains opaque-only:
  no `$2600` consumer bytes, level/object semantics, VDC snapshot or runtime
  promotion are inferred.
- ✅ Verification: loader sidecar MD5
  `2827cb429d0b97f0e1fc26185a9bb28c` passes with `13/24/24`; consumer sidecar
  MD5 `9d19ad9b993f1853e868f381756eb1d0` passes with `4096` reads and the
  `$2c54–$2c69` code-window check. Capture files remain operator-local and
  no game data was added to the repository.

# Theron production fixture-symbol boundary (2026-08-06)

# Theron source roster survives forcefield admission (2026-08-06)

- ✅ Fixed a production startup data-loss bug where the forcefield handoff
  cleared the source-bound champion roster immediately before the authenticated
  Track 02 level-load gate. Real HP, skills and equipment now remain available
  even when dungeon promotion is correctly capture-gated.
- ✅ Added a regression through the runtime entry path using the US Track 02
  identity with deliberately invalid media: the capture gate still rejects the
  handoff, while Hakar's source roster records remain intact. Verification:
  `test_theron_v1_combat_runtime_source` and `git diff --check`. No game data
  was copied or committed.

# Theron production placeholder archive guard (2026-08-06)

- ✅ Extended the Theron production-archive regression so every inventoried
  fixture/compatibility module must have an explicit CMake exclusion and must
  be absent from the final `firestaff_theron` archive. This keeps synthetic
  startup, viewport, HUD and modern-art paths from re-entering through a broad
  source glob. The guard does not promote any unproven consumer.
- ✅ Verification: `test_theron_v1_production_archive_source_boundary` and
  `git diff --check` on a clean worktree from current `main`. No game data was
  copied or committed.

# ✅ 2026-07-11 Theron Track02 real-media compact-row layout receipt

- Extended the existing skip-safe, hash-verified Track 02 real-asset probe to read staged media and publish descriptor-anchor and per-level compact-row layout evidence: matching-anchor masks, row counts, raw-row hashes, table ordinals, and position-bound hashes.
- Kept the receipt non-promoting. It assigns no object semantics and does not affect runtime objects, Continue, synthetic menus, or palette promotion.
- Verification: focused Ninja and CTest Track02 startup-receipt targets.
- ✅ 2026-07-11 Nexus Saturn warning-media decoder: added an exact `RES*`
  directory reader and Sega DGT2 packed-pixel (`PP`) decoder. The local
  `WARNING.BIN` resource 0 now loads through its 256-entry BGR555 CLUT and
  240x96 byte-indexed plane; arbitrary `RES*` bytes remain rejected. The
  title path remains blocked because `TITLE.CG` has no proven header, atlas,
  or Saturn command placement, and no raw/guessed visual fallback was added.
  The focused startup-media gate exercises both the original warning decode and
  the real `TITLE.CG` rejection. Source: Sega Saturn/32X Graphic References,
  section 6 (DGT2 format).

- ✅ 2026-07-11 Nexus PRS3 MSB-first candidate audit: added a bounded,
  explicit MSB-first control-bit traversal alongside the retired LSB-first
  literal/back-reference trial grammar, retaining the observed big-endian
  PRS3 frame, declared output-byte target, opcode fields, and no-promotion
  rule. The optional hash-verified `MENU.BPK` probe disproves the MSB-first
  candidate across all 162 surfaces: zero exact and zero trailing completions;
  runtime/upload routing remains `blocked-prs3`. The synthetic regression
  proves the two bit orders diverge on a controlled literal stream and rejects
  unknown orders. Verified with Ninja `test_nexus_v1_bpk_surface_class` and
  `firestaff_nexus_v1_bpk_prs3_payload_evidence_probe`, plus direct real-media
  probe execution (89/89).

- ✅ 2026-07-11 Nexus PRS3 BE-framed exact evaluation: added a separate,
  diagnostic-only evaluator that begins after the observed BE frame word,
  accepts only the 161 directory-span-close real frames, bounds output to the
  declared mode-derived byte target, and records literal/back-reference
  command counts plus exact/trailing completion. The hash-verified local
  `MENU.BPK` result is identical for LSB-first and MSB-first control order:
  161 command failures, zero trailing completions, and zero exact
  completions; final entry 162 remains unvalidated because of its 530-byte
  BPK tail. No decoded bytes reach runtime and routes remain `blocked-prs3`.
  Verified with Ninja `test_nexus_v1_bpk_prs3_payload_evidence` and
  `firestaff_nexus_v1_bpk_prs3_payload_evidence_probe`; direct real-media
  probe passed 97/97.

- ✅ 2026-07-11 DM2 PC G1 pre-map extension boundary: `dm2_v1_dungeon_loader`
  now derives the standard `READ_DUNGEON_STRUCTURE` prefix through the
  declared DB-pool lengths, then publishes the bounded, untyped G1 extension
  preceding the proven trailing map-data block. The hash-verified DOS English
  `DUNGEON.DAT` proves prefix end `23826`, extension length `7841`, and map
  base `31667`. No extension bytes are assigned DB-pool or record-link
  semantics; `record_graph_complete` stays clear and boot continues to reject
  the partial world. Verified with Ninja
  `test_dm2_v1_dungeon_loader_first_map_gate`,
  `firestaff_dm2_v1_dungeon_loader_first_map_real_data_probe`, and
  `firestaff_dm2`; direct focused tests passed 55/55 and 32/32.

- ✅ 2026-07-11 DM2 PC G1 DB-pool placement audit: source-locked
  `c_record.cpp` confirms the standard ObjectID type/index and first-word
  link semantics, while `SkWinCore.cpp::READ_DUNGEON_STRUCTURE` confirms the
  normal sequential pool reader. The canonical DOS G1 file rejects applying
  that reader directly at the proven map tail: its declared count/size total
  is 16,884 bytes but the map-adjacent candidate span is 7,841 bytes. The
  real-data probe now locks that mismatch, so no accidental record alignment
  can promote bounded traversal or real map boot. The G1-specific pool-base
  transform remains the open blocker.
- ✅ 2026-07-11 DM1-006 F0168 text escape expansion: `F0508_DUNGEON_DecodeTextStringThing_Compat()` and the legacy scroll-style text-table decode now use all 32 PC 3.4 ReDMCSB `G0255` message/scroll, `G0256` symbol, and `G0257` inscription replacement entries. Code 30 selects `G0255` for messages/scrolls and the raw glyph-code `G0257` table for inscriptions; code 29 remains `G0256`. ReDMCSB anchors: `DUNGEON.C` globals `G0255/G0256/G0257` and `F0168_DUNGEON_DecodeText` lines 2280-2350. Verification: Ninja `test_memory_dungeon_text_scroll_pc34_compat` (28/28), `ctest -R '^memory_dungeon_text_scroll_source_lock$'`, and Ninja `firestaff_m10` passed.
- ✅ 2026-07-11 CSB-005 dungeon filter-location decode: added the CSBWin `Monster.cpp` `EDT_SpecialLocations` decoder for attack and level-specific/global movement filters. It unpacks `DSA.cpp` `LOCATIONREL::Integer` fields, movement-only party-level and maximum-distance bits, validates live dungeon bounds, and selects only the first DB3 actuator type 47 from the decoded square. This remains a selection boundary only: no saved DSA words are reinterpreted or executed, and no live monster decision changes yet. Verification: the focused 315-check phase-7 suite passes. A fresh Ninja configuration compiled the touched CSB sources; current Make and Ninja M10 builds stop in unrelated `memory_tick_orchestrator_pc34_compat.c` because `COMBAT_ACTION_APPLY_DAMAGE_PARTY` is undeclared.
- ✅ 2026-07-11 DM1 V1 M10 F0200/F0197/F0199 straight-line C37 visibility slice: M10 creature-reaction context no longer treats same-row/same-column party coordinates as automatically visible. It now resolves a loaded-DM1-tile straight line using the active group direction, `CREATURE_INFO` sight range, and the source blockers from `GROUP.C`: walls, closed fakewalls, and three-quarter/closed doors except the `DUNGEON.C G0254` Portcullis/Ra see-through types. The focused M10 regression drives a facing Vexirk C37 through clear, wall, closed-fakewall, opaque-door, and Portcullis cases, proving only the unobstructed/see-through paths enter ATTACK. Source anchors: ReDMCSB `GROUP.C F0200` lines 1344-1414, `F0197` lines 1175-1212, `F0199` lines 1238-1313, and `DUNGEON.C G0254_as_Graphic559_DoorInfo` lines 560-565. Verification: Ninja `test_memory_tick_orchestrator_f0303_skill_query_pc34_compat`, direct PASS, and focused CTest PASS 1/1. Honest scope: cardinal loaded-tile visibility at the M10 C37 decision boundary only; diagonal F0199 stepping, light/invisibility, per-creature multi-facing, and C37 physical move/retry application remain open.

- ✅ 2026-07-11 DM1 V1 authoritative viewport background boundary: with `assetsAvailable` from original DM1 `GRAPHICS.DAT`, `m11_draw_viewport_background()` now blits only the active map's exact floor/ceiling set. A missing or malformed exact pair leaves the already-cleared viewport untouched; it no longer replaces it with floor-set 0 art or procedural black/gray rendering. The explicit solid fallback remains only for asset-free/headless paths, where no original graphics are available. The local canonical PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT` pair was recognized as READY by `firestaff --data-dir ... --scan-data`. Verification: Ninja full `firestaff` build, scanner real-data probe, `test_m11_inventory_scroll_panel_render_pc34_compat` PASS 32/32, and the focused M10 LoS test still passed. Honest scope: viewport floor/ceiling source-miss behavior only; it does not claim complete real-asset viewport parity or remove explicitly asset-free test fixtures.
## What changed

- `include/theron_v1_cd_audio_availability.h`
  - Expanded `Theron_V1CdAudioAvailability` with source-locked failure
    states: `THERON_V1_CD_AUDIO_CUE_NOT_FOUND`, `CUE_PARSE_ERROR`,
    `LAYOUT_MISMATCH`, and `TRACK_FILE_MISSING`.
  - Expanded `Theron_V1CdAudioReceipt` with:
    - `track_count`, `audio_track_count`, `data_track_count`
    - `audio_directory`
    - 1-indexed `track_paths[1..19]`, `track_present[1..19]`,
      `track_is_audio[1..19]`
    - `unavailable_reason`
  - Changed `theron_v1_cd_audio_availability()` signature from a
    format-string comparison (`cue_format`, `local_format`) to a real
    source-locked intake (`cue_path`, `data_root`).

- `src/theron/theron_v1_cd_audio_availability.c`
  - Implemented a focused CUE parser that handles both quoted
    (`FILE "name.wav" WAVE`) and unquoted (`FILE name.wav WAVE`) forms,
    matching the real TQUS.cue / TQJP.cue syntax.
  - Resolves each declared track file relative to the CUE directory or an
    optional `data_root` override.
  - Allows `.ogg` fallback when the CUE names original `.wav` CD-DA files,
    because the locally staged original audio is supplied as OGG.
  - Handles the documented MyAbandonware-style split Track 02 alias
    (`TQUS02.iso` -> `TQUS02End.iso`, `TQJP02.iso` -> `TQJP02End.iso`).
  - Verifies the canonical Theron CD layout:
    - Track 01 AUDIO
    - Track 02 MODE1/2048 or MODE1/2352 (data)
    - Tracks 03-18 AUDIO
    - Track 19 MODE1/2048 or MODE1/2352 (data)
  - Returns `playback_allowed=1` only when all 19 declared tracks have
    readable local files and the layout matches the original CD.

- `probes/theron/firestaff_theron_v1_cd_audio_availability_probe.c`
  - Rewrote the existing format-string smoke probe into a source-locked
    integration probe:
    - Synthetic complete canonical layout with `.wav` files.
    - Synthetic `.ogg` fallback when CUE declares `.wav`.
    - Missing audio track file -> `TRACK_FILE_MISSING`.
    - Incomplete layout -> `LAYOUT_MISMATCH`.
    - Real-data test against `$HOME/.firestaff/data/theron/TQUS.cue` when
      present, verifying the staged original CD-DA corpus.

- `tests/test_theron_v1_cd_audio_availability.c` (new)
  - Unit test for the canonical 19-track receipt fields.
  - Verifies CUE-not-found, layout-mismatch, and track-presence invariants.

- `CMakeLists.txt`
  - Registered `test_theron_v1_cd_audio_availability` target and CTest
    entry next to the existing Track 01 CDDA handoff test.

## Source evidence

- Local original CUE sheets: `$HOME/.firestaff/data/theron/TQUS.cue` and
  `TQJP.cue` declare the 19-track CD layout.
- Locally staged original CD-DA audio:
  `TQUS01.ogg`, `TQUS03.ogg`, `TQ04.ogg` through `TQ18.ogg`, and JP
  equivalents (`TQJP01.ogg`, `TQJP03.ogg`).
- No synthetic audio playback fallback existed in `src/theron` before this
  change; the receipt is the required gate before any Theron audio output.

## Verification

- `cmake --build build --target firestaff_theron` succeeds.
- `cmake --build build --target test_theron_v1_cd_audio_availability`
  succeeds and the test passes.
- `ctest -R theron_v1_cd_audio_availability -V` from `build/` reports:
  - `theron_v1_cd_audio_availability_probe` PASS
  - `theron_v1_cd_audio_availability` PASS
- Note: the full `cmake --build build --parallel` is currently blocked by
  pre-existing conflicting-type errors in `dm2_v1_skproject_core.c` /
  `dm2_v1_dungeon_loader.h` that are outside Lane E scope.

- 2026-07-23 DM2-007 spell-effect timer handler bodies (Lane B, cycle 11):
  Bound the proven DM2 spell timer effect handlers that do not require
  unproven DB object or creature creation.
  Changes:
    * `include/dm2_v1_spell_timer_handlers_pc34_compat.h` (new):
      - Declares the handler dispatch table `dm2_v1_spell_timer_handlers` and
        the per-effect helpers for the timer types bound from
        `skproject/SKULLWIN/c_tim_proc.cpp`.
    * `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c` (new):
      - `DM2_V1_SPELL_TIMER_HANDLER_LIGHT` (`0x46`): implements
        `DM2_PROCESS_TIMER_LIGHT` (c_tim_proc.cpp:918-959), requeuing the
        timer while `remaining_seconds > 0` and clearing the request once the
        duration expires.
      - `DM2_V1_SPELL_TIMER_HANDLER_HERO_ENCHANTMENT` (`0x47`): sets/clears
        the hero enchantment flag slice (c_tim_proc.cpp:4111-4123).
      - `DM2_V1_SPELL_TIMER_HANDLER_ENCHANTMENT_POWER` (`0x48`): decays the
        enchantment power field each tick (c_tim_proc.cpp:4129-4163).
      - `DM2_V1_SPELL_TIMER_HANDLER_POISON` (`0x4b`): processes the poison
        tick on the bound actor (c_tim_proc.cpp:4165-4178).
      - Leaves `0x19` cloud, `0x1e` missile step, and `0x5e` summon
        fail-closed until their DB-record owners are proven.
    * `tests/test_dm2_v1_spell_cast_player_pc34_compat.c`:
      - Added five new test groups covering light requeue/expiry, hero
        enchantment flag mutation, enchantment power decay, poison decay, and
        source-evidence string for the new handler module.
    * `CMakeLists.txt`:
      - Added `src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c` to the
        `test_dm2_v1_spell_cast_player_pc34_compat` target sources.
  Source evidence:
    * `skproject/SKULLWIN/c_tim_proc.cpp:918-959` (DM2_PROCESS_TIMER_LIGHT).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4111-4123` (hero enchantment flag).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4129-4163` (enchantment power decay).
    * `skproject/SKULLWIN/c_tim_proc.cpp:4165-4178` (poison tick).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3980-4230` (dispatch matrix).
  Verification:
    * `cmake --build build --parallel` succeeded.
    * `test_dm2_v1_spell_cast_player_pc34_compat` 110/110 checks passed.
    * `test_dm2_v1_proceed_timers_pc34_compat` all checks passed.
    * `test_dm2_v1_spell_rune_lookup_pc34_compat` 38/38 tests passed.
    * `test_dm2_v1_spell_pc34_compat` all checks passed.
  Note: runtime wiring into `src/dm2/dm2_v1_runtime.c` was intentionally left
  out of this cycle because adding the new source to the standalone test
  targets that compile `dm2_v1_runtime.c` directly would widen the change
  beyond the proven handler bodies. The module is already compiled into the
  `firestaff_dm2` library via the existing `src/dm2/dm2_v1_*.c` glob.

- ✅ 2026-07-23 DM1 HUD real-material command admission: the production
  `dm1_v1_action_spell_render_command_admit_pc34()` boundary now rejects
  blank source-owned C009/C010/C011 surfaces and non-M653 font records before
  F0387/F0394 commands are published to M11. The focused regression covers
  detached C011, malformed dimensions, zero-filled line/font data, and a
  forged font graphic id. Verification: local PC34 `GRAPHICS.DAT` header
  read confirms C009=87x25, C010=87x45, C011=14x39; Ninja build plus
  `ctest -R '^dm1_v1_action_spell_render_command_admission_pc34_compat$'`
  passes 1/1. This is a fail-closed material admission improvement, not a
  substitute for the remaining Mac/app capture work.
- ✅ 2026-07-23 CSB C005 credits source-only presentation: the C202/F0442
  route now retains only the mandatory decoded GRAPHICS.DAT C005 surface and
  its source palette. Removed the generated "CHAOS STRIKES BACK / CREDITS /
  PRESS ENTER" substitute and updated the C001 phase regression to require
  CSB's distinct PRESENTS/CHAOS/STRIKES palettes. Verified by
  `csb_v1_startup_entrance_pointer_pc34_compat` (139/139).
- ✅ 2026-07-23 CSB C001--C005 CSBWin decoder audit: `ReadAndExpandGraphic(5)`
  clears `0x8000` and invokes `ExpandGraphic`, so C005 is an expanded
  four-plane page, not raw/not-expanded bytes. The startup loader now rejects
  C005 unless its complete decoder receipt reaches the record boundary and
  yields visible indexed pixels. The real-PC34 regression verifies the same
  path using `GRAPHICS.DAT` SHA-256
  `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942`;
  no text or image fallback is introduced.

- ✅ 2026-07-23 CSB C004/C002/C003 F0438 door-page tick contract: the
  Entrance consumer now accepts the first real opening raster only at source
  step 1, then requires every following page to advance one VBlank and one
  door position while retaining the same verified package/session hashes.
  It rejects stale host ticks, replayed pages, and skipped source positions;
  it does not synthesize any raster or replace the remaining F0128 interior
  viewport work. The focused test is registered with CTest. Verification:
  Ninja `test_csb_v1_startup_opening_door_tick_receipt_pc34_compat` build and
  the matching CTest entry pass.

- ✅ 2026-07-23 CSB F0128 Entrance runtime consumer: M11 now accepts a
  source-bound F0128 interior only when its original-material and raster
  receipts, source tick, and session generation agree. It replaces C004's
  224x136 viewport rectangle, then restores decoded C002/C003 door strips
  above it for closed and opening Entrance pages. M11 retains a private copy
  of the admitted indexed raster; missing or stale material cannot fall back
  to generated pixels. Verification: registered CTest
  `csb_v1_f0128_entrance_runtime_consumer_pc34_compat` passes.

- ✅ 2026-07-23 CSB C004 F0128 M11 producer handoff: the live Entrance path
  now advances the verified PC34 session to M11's source tick, copies the
  decoder-bound C004 `(0,33,224,136)` interior into M11-owned storage, and
  binds it through the existing F0128 consumer before real C002/C003 doors.
  `csb_v1_startup_entrance_f0128_m11_handoff_pc34_compat` opens the local
  hash-verified `GRAPHICS.DAT`, verifies every C004 copy byte, M11 storage,
  and the four-surface consumer result. This does not infer the pending F0439
  5x5 micro-dungeon material or introduce a synthetic replacement.

- ✅ 2026-07-23 CSB C005 credits/Entrance return admission: C005 publication
  now requires the live Entrance session, complete expanded GRAPHICS.DAT C005
  decode receipt, source credits palette, and one real raster surface. The
  real-PC34 startup-sequence regression verifies C005 then its C004/C002/C003
  return with the credits palette removed; no fallback text or panel is used.

- ✅ 2026-07-23 CSB C001 M11 title-frame admission: M11 now presents only a
  decoder-bound C001 host frame with one source raster, matching source step,
  and the consumed PRESENTS/CHAOS/STRIKES phase bit. The real-PC34 sequence
  regression locks C001 provenance and phase order; no host text fallback can
  substitute for a missing title phase.

- ✅ 2026-07-23 CSB C001 real timing capture: the real-PC34 GRAPHICS.DAT
  regression now captures and requires four distinct source rasters in
  PRESENTS, CHAOS zoom, CHAOS hold, STRIKES BACK order with the complete
  `0x0f` phase mask; no host text fallback is accepted.

- ✅ 2026-07-23 CSB C001-to-Entrance M11 receipt: the real PC34 sequence now
  proves the complete C001 title session immediately hands M11 the C004/C002/
  C003 closed Entrance plan, real three-source raster, and no fallback text.

- ✅ 2026-07-23 CSB F0247/F0219 live C14-to-F0128 handoff: a real runtime
  C05-chain receipt now supplies the resolved projectile identity to the next
  boot viewport frame. F0128 revalidates its actual C14 Thing-chain ownership
  and uses the source F0115/F0791 bitmap path; stale data and absent real
  material remain no-draw, with no marker fallback. Verification:
  `m11_csb_f0247_boot_projectile_frame_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 C14 C04 teleporter rotation: loaded object-scope
  teleporters now apply ReDMCSB MOVESENS.C F0263's packed relative
  direction/cell rotation to the authenticated active M10 projectile as well
   as raw C14. F0249 retains exactly the physical C48 owner at the rotated
   destination. Verification passed:
   `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 chained C14 C04 route: the existing source-backed
  F0263/F0249 path is now proven across two loaded object-scope teleporter hops.
  Relative direction/cell rotation accumulates in raw C14 and active M10 state,
   while the one physical C48 remains owned by that C14 at the terminal square.
   Verification passed: `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.
- ✅ 2026-07-23 DM1 HoC all-portrait F0115/C127 geometry regression:
  the real PC34 map-0 sweep now covers all 24 source C127 champion mirrors,
  derives each legal party pose from its packed wall cell, selects the actual
  portrait through the production route, and verifies that turning into a
  side/depth presentation cannot retain the prior ordinal. Verification:
   `dm1_v1_hoc_all_front_mirror_ordinals_pc34_compat` passed with installed
  `DUNGEON.DAT` and `GRAPHICS.DAT`.
  while the one physical C48 remains owned by that C14 at the terminal square.
  Verification passed: `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 F0249/F0267 C14 C02-to-C04 continuation: M10 now resolves a
  loaded open C02 door followed by an object-scope C04 target through the
   source F0263 packed direction/cell transform. The original C14 Slot material,
  kinetic/attack values, and one physical C48 owner continue to the target
   without synthetic projectile state. Verification passed:
   `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.
- ✅ 2026-07-23 DM1 HoC complete C127 viewport material route:
  M11 now binds every frame's live C127 projection to source material: D1C
  publishes the real C346/C026 pair, D1L/D1R consume C346 only, and D2+ is a
  clear-only decision with no synthetic mirror or portrait. The frame receipt
  is reset before each render. Real PC34 map-0 coverage sweeps all 24 front
  portraits and side/depth routes. Verification:
  `m11_dm1_hoc_real_mirror_viewport_material` passed.
  without synthetic projectile state. Verification passed:
  `test_dm1_v1_f0249_runtime_relocation_pc34_compat`.

- 2026-07-23 DM1 viewport F0134/F0135 material admission: added a fail-closed
  planar viewport consumer for ReDMCSB ACTIDRAW.C F0134 and FILLBOX.C F0135.
  It operates only on a caller-verified original material surface, preserves
  the source inclusive-box semantics, and rejects missing material without a
   synthetic fill. Verification passed:
   `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.
- ✅ 2026-07-23 DM1 original PC34 champion/group/timer byte gate: the
  fixture-free external-corpus F0435 -> F0433 -> F0435 test now requires
  decrypted C04 ACTIVE_GROUP allocation bytes, all four C02 M516 champion
  records, and the source C03 timer/event plus C04 timeline parts to preserve
  their exact admitted sizes and fingerprints. Internal fixture regression
  remains a semantic handoff check; it cannot stand in for external evidence.
  Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed.
  synthetic fill. Verification passed:
  `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.
- 2026-07-23 DM1 PANEL.C F0344 F0135 material consumer: added the real
  proportional food/water bar route on an admitted planar material. It uses
  the source `G2097_FoodOrWaterBarShadowOffset = 2`, fills the black shadow
  before the colored bar, and preserves F0344 red/yellow/base-color rules.
   Verification: `test_redmcsb_fillbox_blitfill_f0135_integration_pc34_compat`.

- ✅ 2026-07-23 DM1 PANEL.C F0344/F0351 source-bound health/stamina panel
  gate: M11's inventory champion-stat route now requires the current DM1
  session's source-bound M653 font after admitting the real C020 panel from
  `GRAPHICS.DAT`. A generic loaded font can no longer render health/stamina
  text onto original artwork. Verification:
  `m11_dm1_f0344_source_bound_champion_stats`.

- ✅ 2026-07-23 DM1 original PC34 ACTIVE_GROUP runtime identity gate: the
  external-corpus F0435 staging and candidate-to-runtime adoption receipts now
  retain a source-checked fingerprint of every live C04 ACTIVE_GROUP record.
  It includes the type-4 group Thing identity, full packed directions/cells,
  timing/flee fields, target/prior/home coordinates, and all four Aspect bytes.
  A mismatch or flattened PC34 sidecar fails closed before the corpus row can
  be admitted. The test remains corpus-only and reports `SKIP` without
  `FIRESTAFF_DM1_PC34_SAVE_CORPUS`; no synthetic save is accepted as evidence.
  Verification: Ninja/CTest `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed; the latter skipped
   honestly because no external corpus is staged.
- ✅ 2026-07-23 DM1 F0134/F0135 champion food/water material admission:
  M11 now treats the C12 alive-status fill together with the F0345 C020 panel
  and C030/C031 label blits as one fail-closed source transaction. The new
  DM1-owned receipt accepts only decoded `GRAPHICS.DAT` surfaces with exact
  original IDs and dimensions, fingerprints their pixels, and consumes the
  route without host text or generated panel fallback when any surface is
  absent. `m11_dm1_food_water_source_gate` verifies the receipt with local
  original PC34 media.
- ✅ 2026-07-23 DM1 F0134/F0135 production caller slots: the champion panel
  admission now binds each caller to its source slot and rejects stale or
  mismatched `GRAPHICS.DAT` material before it can enter the panel transaction.
  The real-data regression verifies loaded original material; no M11 fallback
  or F0115 scheduler path was changed.
- ✅ 2026-07-23 DM1 F0435 external-corpus global/party/map adoption gate:
  source-only PC34 corpus verification now retains a combined GLOBAL_DATA,
  party position, status-counter, C2 PARTY_INFO and M516 identity receipt
  across staging and candidate-to-runtime adoption. The fixture-free target
  admits no generated evidence and skips when no operator corpus is set.
  Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus` passed.
- ✅ 2026-07-23 CSB C002/C003 F0438 host-frame phase receipt: the
  source-bound door consumer validates the already-produced real
  `GRAPHICS.DAT` session/M11 host raster by opening step, tick, generation,
  Entrance palette, decoded C002/C003 records, and the exact C004/F0128/door
  source count. It never changes title or Entrance plan selection. Verification:
  source-bound fillbox, F0128 consumer, F0128 M11 handoff, and broad launcher
  boundary regressions passed.
- ✅ 2026-07-23 CSB C017/C040 M11 frame palette admission: the existing
  F0807 terminal host-frame gate now rejects a retained title or Entrance
  palette before M11 consumes the source-bound `GRAPHICS.DAT` C017/C040
  surfaces. The panel blit does not replay startup plans, preserving C002/C003
  composition. Verification: `csb_v1_m11_launcher_handoff_boundary` and
  `csb_v1_startup_real_sequence_pc34_compat` passed.

# ✅ 2026-07-13 Theron dynamic Track 02 CD_READ-to-RAM receipt: the
# ✅ 2026-07-14 Theron Track 02 later loader-to-local-RAM capture contract

# ✅ 2026-07-15 Theron main-RAM control-window read instrumentation

Added bounded CPU-read provenance for `0x1f01f7..0x1f01fb`, retaining logical
and physical reader addresses. The instrument does not classify the bytes or
infer any CDB, sector, level, or object semantics.

# ✅ 2026-07-15 Theron control-window System Card exclusion

Validated a real US Track 02 boot capture containing 64 reads of
`0x1f01f7..0x1f01fb`. Every recorded reader is System Card physical code/RAM
(`0x00xxxx`, `0x002xxx`, or `0x1fe0xx`); no reader is in the game-owned
`0x1f0000..0x1f7fff` range. The bounded verifier fails on a game-owned or
unclassified reader, so this capture cannot be promoted to a CDB/SCSI, sector,
level, or object-record link.

Verification: `test_theron_v1_main_ram_control_window_receipt` and
`verify_theron_main_ram_control_window_receipt.pl` against the authentic trace.

# ✅ 2026-07-15 Theron game-owned main-RAM window to SCSI receipt

Added bounded read provenance for `0x1f1000..0x1f1007`. In an authentic US
Track 02 capture, physical game code `0x1f0c88` reads all eight bytes before
the game-owned `0x1f0cc7` `$e009` dispatch, which is followed by SCSI
generation 2 at LBA 4165 for four sectors. The receipt proves this execution
ordering only: no FIFO destination, level layout, or object-record grammar is
assigned.

Verification: `test_theron_v1_main_ram_game_window_scsi_receipt`, Mednafen
patch dry-run, and the authentic capture verifier.

# ✅ 2026-07-15 Theron game-owned FIFO-to-RAM-to-reader intake gate

The Track 02 loader receipt now has a strict live-capture intake for a single
game-owned `$3840 -> $e009` dispatch: seven observed CDB writes must decode to
the following READ(6); its FIFO-origin byte must reach game-owned main RAM and
be read later by game-owned code from the identical physical cell. For the
verified US CUE coordinate, Firestaff rechecks the captured byte against
`raw_record = LBA - 3009` in the hash-verified Track 02 BIN. The result remains
an opaque byte-flow receipt, not a dungeon, grid, level, object, bitmap,
palette, or transition decoder. Verification: the focused raw-loader probe
rejects a mutated source byte and a CDB/LBA mismatch.

# ✅ 2026-07-15 Theron game-RAM initial-envelope correlation gate

An admitted game-RAM payload byte can now be joined to the source-locked Hall
of Records envelope only when its physical Track 02 sector and exact raw-sector
offset fall inside the authenticated envelope. The join deliberately uses the
IPL-derived physical `level_first_raw_sector`, not descriptor-relative record
`0x0b52`, preventing INDEX 01/file-sector coordinate confusion. It rejects a
pre-envelope byte and altered source media, and publishes no level grammar,
dungeon, object, grid, bitmap, palette, or transition semantics. Verification:
the focused US Track 02 raw-loader probe.

# ✅ 2026-07-15 Theron initial-envelope header capture gate

Firestaff can now retain the first twelve raw bytes of the source-locked
initial envelope only when twelve ordered game-RAM payload receipts share one
dispatch and READ(6) identity and cover the exact consecutive raw offsets.
The receipt stores the source bytes and FNV-1a hash only. It does not interpret
dimensions, the existing extension word, header grammar, level, dungeon,
object, grid, bitmap, palette, or transition semantics. Verification: the
focused US Track 02 probe rejects a split SCSI capture chain.

# ✅ 2026-07-15 Theron LBA 4165 raw Track 02 binding

Bound the four sectors requested by the game-owned window path, LBA
`4165..4168`, byte-exactly to raw Track 02 records `0x484..0x487` using the
observed `raw_record = LBA - 3009` coordinate. The verifier checks each full
2352-byte sector and its observed 32-byte prefix hash. This is a media
identity receipt, not a level/object classification or a FIFO destination.

Verification: `test_theron_v1_lba4165_track02_receipt` and the authentic US
Track 02 capture.

# ✅ 2026-07-15 Theron LBA 4165 FIFO origin receipt

Added byte-origin tracking through Mednafen's SCSI data FIFO. Authentic capture
proves Track 02 record `0x484` / LBA 4165 offsets `0..31` reach System Card
CPU `0xea9c` through `$1808`, with exact byte comparison. No game-owned RAM,
level, or object-record consumer is claimed.

# ✅ 2026-07-15 Theron generation-4 System Card boundary

Generation 4 is CDB `080010891100`: LBA `4233..4249` / Track 02 records
`0x4c8..0x4d8`. Its FIFO origin is System Card `0xea9c`; all four observed
`0x1f0256..0x1f0259` stores are written by System Card `0x000a52`. This route
is excluded from game-data semantics.

# ✅ 2026-07-15 Theron generation-7 FIFO/game-RAM ordering

Authentic capture proves byte-exact FIFO origin for LBA `4847..4851` / Track
02 records `0x72e..0x732`: each of the 10,240 bytes reaches the System Card
`$eb33` FIFO loop and is acknowledged through `$1802/$1803`. The complete
generation-7 FIFO window precedes game-owned `0x1f11xx..0x1f18xx` writes.
That is ordering only, not a byte destination or record semantic.

# ✅ 2026-07-15 Theron main-RAM CDB byte-consistency gate

The authentic main-RAM `$e009` dispatch receipt now decodes each READ(6) CDB
and rejects a mismatch between its LBA/count bytes and the emitted SCSI
command. This binds the game-owned dispatch route to the observed raw record
ranges without inventing a FIFO destination or record semantics.

# ✅ 2026-07-15 Theron later-generation FIFO capture filter

The reproducible Mednafen trace build accepts
`FIRESTAFF_THERON_FIFO_MIN_GENERATION=N`. It filters only provenance output
below `N`; emulated CD reads and RAM writes are unchanged. The authenticated
`N=8` capture omits the already-proved generation-7 FIFO traffic, but still
does not reach a later FIFO byte before timeout. No handoff is claimed.

# ✅ 2026-07-15 Theron guarded global-HID capture route

The Quartz helper can use a global HID route only after activating and then
rechecking the target's foreground PID. An authenticated run observed
`loginwindow` PID `622`, not Mednafen, so it failed before posting a key. This
is an environment receipt, not emulated input or a dungeon handoff.

# ✅ 2026-07-15 Theron main-RAM loader initialization exclusion

The post-`$e009` `0x1f10xx` write window is now fail-closed as loader
initialization: the authenticated writes are only `00`/`ff` sentinels from
the observed main-RAM writers. It cannot be promoted to level/object data.

Verification: focused initialization receipt test.

# ✅ 2026-07-15 Theron game-owned writer corpus negative receipt

The authentic USA Track 02 capture now has a strict, bounded negative corpus
receipt for every observed game-owned main-RAM loader writer. It contains 128
writes: 12 control-window writes at physical `0x1f01f6..0x1f01fb`, plus 116
`00`/`ff` initialization writes at `0x1f10xx`. All have
`dispatch_sequence=0`; the authenticated generation-7 `READ(6)` at LBA 4847
(Track 02 records `0x72e..0x735`) occurs only after that complete writer
corpus. These rows therefore cannot be the G7 loader or a G7 record consumer.
The verifier rejects a CDB-dispatched writer, a non-sentinel initialization
byte, an unclassified destination, and a changed corpus count. This is not a
global absence claim: a later game-owned FIFO/CDB reader or writer remains the
required positive handoff evidence. No level, object, palette, or visual
semantics were added.

Verification: `test_theron_v1_game_loader_writer_negative_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 game-loader record-route receipt

The authentic USA trace now fixes the post-G7 game-loader control boundary.
After G7, physical game-RAM `0x1f1840` continues to call `$e009` from logical
`0x3840`: dispatches 4, 5, and 6 have `A=20`, `X=ff`, `Y=04` and issue the
exact READ(6) CDB routes G8 LBA 4859 (record `0x73a`), G9 LBA 4855..4857
(records `0x736..0x738`), and G10 LBA 4858 (record `0x739`). This is the
verified loader entry/record route after G7. The trace patch emits the entry
only after disassembling HuC6280 opcode `0x20` with operand `$e009`, i.e.
`JSR $e009`. The trace still has no
FIFO-to-game-RAM destination or game-owned record reader, so no level, object,
palette, or visual semantics are assigned.

Verification: `test_theron_v1_post_generation7_loader_route_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 indirect CDB-parameter receipt

The post-G7 loader trace now proves a bounded ABI fact. Game code at physical
`0x1f1837` writes `ff/20/04` into physical `0x1f01e5..0x1f01e7` immediately
before dispatch 4, exactly shadowing `X/A/Y` at the `0x1f1840` `JSR $e009`.
Dispatches 4--6 keep that same register tuple but produce three distinct
authenticated READ(6) CDBs: `080012fb0100`, `080012f70300`, and
`080012fa0100`. The tuple is therefore not direct LBA/count encoding; it is
an indirect loader ABI whose additional parameter source and RAM consumer are
still unobserved. A new passive MD5-pinned CUE capture reached only the System
Card wait and contributes no loader route. No game-data, level, object,
palette, or visual meaning was inferred.

Verification: `test_theron_v1_post_generation7_cdb_parameter_receipt` and
the authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 parameter-window reader trace

Mednafen's authentic trace pipeline now records every physical read of the
post-G7 parameter-shadow window `0x1f01e5..0x1f01e7`, including its logical
address, value, and logical/physical reader PC. The receipt is bounded to 128
rows and is appended after all existing source-to-RAM provenance patches, so
it cannot alter CDB, FIFO, controller, or emulated input behavior. It is
fail-closed evidence only: the existing G8--G10 trace predates this reader
instrumentation, while a new passive MD5-pinned media run reached only the
System Card wait. There is therefore no claimed lookup, loader-table,
game-owned consumer, record-table, or semantic binding yet.

Verification: full `test_theron_v1_mednafen_controller_wait_trace_patch`
dry-run against Mednafen 1.32.1 source.

# ✅ 2026-07-15 Theron post-G7 parameter-thunk CPU receipt

The authenticated G8 trace now fixes the next game-owned control edge after
the indirect `$e009` ABI. Physical `0x1f184d` writes byte `1e` to executable
`0x1f1837`, then `0x1f1852` writes `20` to `0x1f1838`. Execution from
physical `0x1f1837` subsequently stages `04/20/ff` into the parameter window
before `0x1f1840` dispatches `$e009` and G8 reads LBA 4859. The verifier
rejects changed patch bytes, parameter-store ordering, and CDB ordering. No
CD-origin row writes the thunk bytes, no parameter-window reader was observed,
and no opcode, loader-table, record-table, level, object, palette, or visual
meaning is inferred from the two patched bytes.

Verification: `test_theron_v1_post_g7_parameter_thunk_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron generation-4 System Card CD-to-main-RAM receipt

The authenticated USA Track 02 generation-4 READ(6) now has a complete
CPU-provenance boundary. Its CDB reads the ordered 17-sector span LBA
4233..4249 (records `0x4c8..0x4d8`); all 34,816 raw data-port bytes are
checked for contiguous LBA/offset order. The observed FIFO values
`38/50/37/04` are read by System Card `$ea50`, written by System Card `$ea52`
to physical main RAM `0x1f0256..0x1f0259`, and the first three cells are then
read by low physical System Card code. The verifier rejects a game-owned
writer. This is a positive CD-to-main-RAM receipt, but it proves System Card
ownership only: it does not bind game code, a loader table, level data,
objects, palettes, or rendering semantics.

Verification: `test_theron_v1_generation4_system_card_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron byte-exact FIFO-to-main-RAM instrumentation

The Mednafen trace now retains the raw Track 02 LBA and byte offset that were
current at each queued FIFO read, and emits them with the later main-RAM
destination plus reader and writer CPU provenance. A verifier accepts such a
receipt only when its source lies in a preceding observed READ(6) range and
its destination is physical main RAM. This does not fabricate a handoff: the
new MD5-pinned headless USA capture stayed at the System Card wait and emitted
no FIFO-to-main-RAM receipt. A future runtime capture must supply the positive
row before any game-owned loader, level, object, palette, or visual claim.

Verification: `test_theron_v1_fifo_origin_main_ram_receipt`, the Mednafen
patch application/compile probe, and the negative headless capture.

# ✅ 2026-07-15 Theron FIFO-origin game-consumer gate

The trace now tracks a bounded set of raw-CD FIFO cells after they reach
physical main RAM. A consumer receipt is emitted only when a physical
`0x1fxxxx` game-code reader reads the exact same still-valid destination and
value; every later write invalidates that cell, including a same-value write.
System Card readers are excluded. The verifier requires the matching prior
raw LBA/offset receipt, so this cannot promote a timing correlation to a game
handoff. No authentic consumer row has been observed yet.

Verification: `test_theron_v1_fifo_origin_main_ram_consumer` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron main-RAM `$e009` return receipt

Each traced game-RAM `JSR $e009` now records an exact pending continuation at
the observed logical and physical `JSR+3` addresses. A return receipt is
emitted only when the HuC6280 executes precisely that continuation; unrelated
game instructions and an unmatched return are ignored. This extends the
loader route from call/CDB evidence to CPU continuity without assigning any
data or rendering semantics. No new authentic return capture is claimed.

Verification: `test_theron_v1_main_ram_e009_return_receipt` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron post-dispatch game-owned main-RAM write receipt

After authentic `$e009` dispatch, bounded tracing distinguishes writer
ownership. USA Track 02 capture proves game-owned code at `0x1f0cc9` and
`0x1f1173..` writes main-RAM state. It is not byte-linked to FIFO payload or
a proven level/object record, so no semantics or fallback is promoted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` writer-provenance receipt

FIFO destination receipts now retain the actual writer PC and physical PC.
Real USA Track 02 capture proves every observed `$e009` FIFO store is written
by System Card code (`0x000a52` or `0x000b35`), including stores addressed in
main RAM. Thus none qualifies as game-owned level/object data. The next route
must first prove a physical `0x1fxxxx` writer.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron G4 RAM consumer negative receipt

The HuC6280 read path now records exact reads of G4's materialized
`0x1f0256..0x1f0259` bytes. Real USA Track 02 capture shows their subsequent
readers are System Card physical code, including `0x002c1a..0x002c69`, rather
than game-owned main-RAM code. The G4 route is therefore explicitly blocked
from level/object promotion; no fallback or semantic inference was added.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` FIFO-to-main-RAM receipt

Dispatch-bounded FIFO tracing now ties real `$e009` SCSI data reads to strict
next-store RAM receipts. The USA Track 02 capture proves dispatch 0's
generation-4 bytes reach physical `0x1f0256..0x1f0259`; other captured
dispatches reach the System Card workspace. These are byte-transport facts
only: no level/object grammar, game consumer, or visual fallback is admitted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron main-RAM `$e009` to SCSI receipt

The HuC6280 trace now emits every physical main-RAM `$e009` call into the
PCE-CD trace. A fail-closed verifier requires exactly seven subsequent CDB
writes and one READ(6) SCSI command. A real USA Track 02 capture validates
32 such dispatch-to-record chains. This proves loader-to-record transport,
not game-owned destination, level, object, or visual semantics.

Verification: Mednafen patch dry-run, focused verifier test, and real SDL2
USA Track 02 capture.

# ✅ 2026-07-15 Theron parameterised main-RAM `$e009` receipt

The HuC6280 main-RAM trace now captures A/X/Y at each executed loader call.
Real USA Track 02 capture proves physical `0x1f1840` calls `$e009` after the
`0x1f1836` TII workspace transfer, including `a=20 x=03 y=02`. Parameters
vary across calls and remain opaque: no record, level, object, or visual
semantics are assigned.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 USA Track 02
capture.

# ✅ 2026-07-15 Theron main-RAM loader control receipt

Added a HuC6280-core trace patch that resolves executed PCs through active MPR
banks before recording bounded main-RAM `JSR` and block-transfer edges. Real
USA Track 02 capture records physical `0x1fxxxx` loader calls, including
`JSR $e009` at `0x1f0cc7` and `0x1f1840`. This proves control flow only, not
level, object, payload, or visual semantics.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 capture against
MD5-pinned USA Track 02 media.

# ✅ 2026-07-15 Theron all-generation Track 02 source-to-RAM receipt gate

The instrumented Mednafen build now carries an exact raw SCSI origin
(`generation`, `LBA`, and in-sector byte offset) through the pending FIFO read
and emits `pce_cd_origin_ram_receipt` only when that same byte is immediately
stored in physical main RAM. The receipt verifier rejects non-main-RAM
destinations and offsets outside the 2048-byte sector. It neither assigns
writer ownership nor record, level, object, palette, or visual semantics.

A fresh MD5-pinned USA CUE/System Card run without host input reached only the
System Card wait: no raw-sector SCSI transfer and no receipt were observed.
That negative result is deliberately not promoted to a game-data conclusion;
the next positive capture must show a game-owned consumer before any semantic
work may begin.

Verification: `test_theron_v1_origin_ram_receipt`, Mednafen patch dry-run,
and an instrumented authentic-media boot capture.

# ✅ 2026-07-15 Theron game-owned Track 02 FIFO-to-RAM writer gate

The all-generation receipt now observes the store at the HuC6280 write point,
so one trace row contains the raw sector generation/LBA/offset, FIFO reader,
physical main-RAM destination, and both logical and physical writer PCs. The
positive verifier accepts only writer and destination addresses in physical
game RAM `0x1f0000..0x1f7fff`; a System Card writer is rejected. This is a
transport/ownership gate only and publishes no record, level, object, palette,
or visual semantics.

The fresh MD5-pinned USA CUE capture posted real PID-targeted Quartz Return
pairs but Mednafen reported no SDL key event, then reached only the System
Card wait with no SCSI read or FIFO/RAM receipt. The failed delivery is kept
as a negative capture result, not replaced with injected controller state.

Verification: `test_theron_v1_game_owned_origin_ram_receipt`, full Mednafen
patch dry-run, instrumented Mednafen build, and the authentic-media capture.

# ✅ 2026-07-15 Theron PID foreground capture gate

PID-targeted Quartz delivery now requires the same foreground ownership proof
as the global-HID route. The helper activates the target, rechecks
`NSWorkspace`, and emits `quartz_frontmost_pid` only before posting a key.
The capture wrapper requires that receipt, so a `posted_to_pid` line cannot be
mistaken for SDL delivery from a background or login session.

The direct live check found the Mednafen target at PID `8739` while foreground
ownership remained with `loginwindow` PID `622`; it failed before posting. No
controller state, CD read, FIFO/RAM handoff, or Track02 semantics were
invented. A positive run still needs both real Aqua foreground ownership and
Mednafen's own SDL event receipt.

Verification: `swiftc -typecheck`,
`test_theron_v1_mednafen_live_capture_script`, and the direct live negative
foreground receipt.

# ✅ 2026-07-15 Theron foreground activation receipt refinement

The Quartz helper now records the result of macOS activation independently of
foreground ownership, and the capture wrapper requires `quartz_activation`
plus the exact foreground PID before it accepts a key-post attestation. A live
probe returned `activate=true` for Mednafen while `NSWorkspace` still reported
`loginwindow` PID `622`; activation success alone is therefore not promoted to
focus, SDL delivery, controller state, CD traffic, or a Track02 handoff.

Verification: Swift typecheck and
`test_theron_v1_mednafen_live_capture_script`.

# ✅ 2026-07-14 Theron Track 02 startup-grid positive route

The existing CD/MODE1 envelope and loader-semantic receipt now materialize one
positive route: Hall of Records level 0 only. It verifies the loader-selected
pose against the receipt and remains an explicit route boundary pending
startup-pose reconciliation with the older semantic handoff. The route
contains no object table, header-extension interpretation, transition, bitmap,
or fallback-visual claim; later dungeon requests reject. The focused Track 02
handoff probe checks the real-media route and its refusal of an unproven
dungeon ID.

# ✅ 2026-07-14 Theron later `$e009` capture correlation gate

# ✅ 2026-07-14 Theron later `$e009` production selector-coordinate gate

The production later-loader media receipt now derives the captured `$e009`
record from the authenticated Stage 3 descriptor coordinate base and accepts
it only when it resolves to an existing descriptor selector. It retains the
opaque selector and ordinal with the raw-sector receipt. A raw-sector-only or
synthetic media buffer cannot publish the receipt. This is still only an
executed loader-coordinate constraint: it assigns no descriptor format,
dungeon, object, palette, bitmap, or transition semantics. Verification:
`theron_v1_raw_loader_trace_stage3_sector` passes; the paired original-media
layout probe remains skip-safe until matching JP/US Mednafen traces exist.

# ✅ 2026-07-14 Theron later `$e009` raw-sector witness boundary

The selector-coordinate receipt can now be paired with exactly one
provenance-marked Mednafen SCSI raw-sector sidecar span whose bounded FNV-1a
matches the corresponding hash-verified Track 02 raw sector. The receipt
retains only the observed disc LBA, selector coordinate, and span fingerprint.
It does not claim that `$e009` caused that read, that both observations share
one capture session, or assign a payload format, dungeon, object, palette,
bitmap, or transition meaning. Noncanonical media, missing sidecars, duplicate
matching spans, and changed bytes reject.
Verification: `theron_v1_raw_loader_trace_stage3_sector` focused negative
probe; a positive result requires original JP/US media and captures.

# ✅ 2026-07-14 Theron later `$e009` complete-sector witness hardening

The raw-sector witness now accepts only a provenance-marked Mednafen SCSI row
that retains both FNV-1a fingerprints: all 2352 observed raw-sector bytes and
the existing leading 32-byte span. Firestaff compares both against the same
selector-resolved record in the hash-verified original Track 02 BIN; span-only
or malformed sidecars reject. This remains physical CD/media provenance only:
it does not establish `$e009` causality, shared capture-session identity,
payload format, dungeon, object, graphics, palette, bitmap, or transition
semantics. Verification: focused raw-loader CTest and Mednafen patch/capture
script contracts.

# ✅ 2026-07-14 Theron later `$e009` ordered raw-sector capture gate

The next Track 02 capture handoff now has a strict, source-only admission
contract. A future authentic JP or US coalesced Mednafen transcript must retain
exactly one variant-matched `$4090/$4093` loader row, followed by one later
`$e009` dispatch, exactly one complete 2352-byte raw-sector FNV witness, and
the matching `$e009` return. The verifier rejects split sidecars, reordered
rows, duplicate rows, malformed fingerprints, and unmarked transcripts. It
records only observation order; no destination, CD causality, payload format,
dungeon, map, object, graphics, or palette claim is introduced.
Verification: `tests/test_theron_v1_later_e009_raw_sector_order_trace.sh`.

# ✅ 2026-07-15 Theron post-`$3800` IRQ2-to-later-read ordering gate

The coalesced Track 02 receipt now requires an observed original Stage 3
`BRK $ff` return from `$3800` to `$3802` before it will accept a later
`$e009` dispatch. Firestaff checks those capture coordinates against the
hash-verified Stage 3 payload, then retains only the ordering fact. The gate
does not decode the later sector or promote level, object, bitmap, palette,
grid, or transition semantics. Verification: the corpus-bound raw-loader
handoff probe rejects missing or altered Stage 3 continuation coordinates.

Added a skip-safe, corpus-bound probe and Mednafen instrumentation for the
first post-stage-two HuC6280 `JSR $e009` envelope. A positive result requires
hash-verified JP and US raw Track 02 images plus matching instrumented traces;
the record must reconstruct from observed `CL/DL/CH`, remain in each raw-sector
range, resolve to the same existing stage-three descriptor selector ordinal,
and preserve one caller/return PC pair. This is only a bounded record/layout
and control-transfer correlation. It does not label the call as a payload
format, or publish a CD read, bitmap, palette, object, level, or gameplay
transition. Inspected historical US traces do not contain the new later
envelope, so no positive record has been claimed. Verified with an external
Ninja build of `firestaff_theron_v1_later_cd_read_layout_probe` and skip-safe
CTest registration.

The probe now additionally requires exactly one observed `$4090/$4093`
dynamic receipt in each trace, including its matching JP/US variant and
reconstructed `CL/DL/CH` stage-two record. A freestanding, duplicate, or
cross-variant later `$e009` row cannot be paired with authenticated media.

# ✅ 2026-07-14 Theron Track 02 route-receipt probe repair

The focused Track 02 handoff probe now constructs a complete hash-profiled
startup-media receipt before it exercises the existing media-gated bank
selection. This restores the real JP/US Hall of Records level-0 loader route
as a green target while retaining the Stage 3 `$4090 -> $4093` CD_READ receipt
as transport-only: it does not claim a later level, object layout, visual
decode, or transition.

# ✅ 2026-07-14 Theron Track 02 loader-pose reconciliation

The positive raw-CD Hall of Records level-0 path now preserves the existing
loader's first-floor/default-North pose across the candidate and loader-route
handoffs. The previous local passable-neighbor/East preference was removed
because it was not backed by the original CD or loader evidence. The focused
probe verifies the two real-media paths agree; it remains skip-safe without
hash-verified JP/US Track 02 images. The older seed-table semantic handoff is
still independently blocked on authentic media and is not composed here. This
does not infer an IPL spawn override, object table, transition, bitmap,
palette, or fallback.

# ✅ 2026-07-14 Theron Track 02 coalesced later-loader sector receipt

The later-loader handoff now has one media-bound receipt for a single original
Mednafen transcript. It requires the authenticated Stage 2 `$4090 -> $4093`
loader row, one later `$e009` dispatch, one complete 2352-byte raw-sector
fingerprint, and the matching return in that observation order. Both the
complete-sector and leading-span FNV-1a values must match the raw sector
selected through the existing Stage 3 descriptor coordinate in a hash-verified
JP or US Track 02 image. The opt-in corpus probe runs this check only when both
variants' coalesced traces are supplied. This records a loader-coordinate and
physical-media fact only: it assigns no payload format, dungeon, map, object,
graphics, palette, bitmap, or transition meaning.

# ✅ 2026-07-14 Theron Track 02 manifest-bound coalesced loader receipt

The opt-in JP/US coalesced-loader corpus probe now accepts each ordered
Mednafen transcript only through its own V2 capture manifest. It rehashes and
matches the exact raw Track 02, System Card 3.0, and trace paths before binding
the existing selector-resolved complete-sector receipt. A missing half-pair,
manifest, or System Card path fails the supplied-evidence gate. This records
only original-artifact provenance and loader/media coordinates; no payload
format, dungeon, map, object, graphics, palette, bitmap, or transition meaning
is assigned.

# ✅ 2026-07-14 Theron Track 02 manifest-required raw loader preflight

The positive raw-loader preflight now requires a V2 capture manifest and
rehashes the exact raw Track 02, System Card 3.0, and ordered Mednafen trace
against it before admitting the existing `$3800` media-span/Stage 3 receipt.
The shared loader-capture identity check rejects a missing manifest, a changed
trace, or a non-System-Card-3.0 hash. This is artifact provenance and transport
only; it assigns no payload format, dungeon, object, bitmap, palette source,
or decoder meaning.
# 2026-07-27 Theron hash-selected Track 02 media root

- ✅ 2026-07-27 DM1 champion HUD click repair. The full painted V1
  health/stamina/mana bar surface now opens the matching champion inventory,
  rather than accepting input only on the narrow right-edge source zone while
  the rest of the visible bar silently selected the leader. Name and hand
  routes remain unchanged; V2 portrait-card routing remains covered.
  Verification: DM1 inventory mouse-route runtime, V2 HUD interaction, and
  HiDPI champion pointer tests.

- ✅ 2026-07-27 DM1 V2.2 reviewed-art admission repair. Formatted Art Studio
  manifests are now parsed as JSON objects instead of line fragments, all
  V2.2 manifest/receipt roots are configured together, and alias-safe path
  joins support in-place path construction. A reviewed local pack passes the
  real material gate and renders eight source-backed cells; an unsigned
  cache remains fail-closed. Verification: V2.2 real-art material gate,
  per-mode material signatures, settings persistence, and source-lock gate.

Theron launcher campaign-media discovery now honours the caller's selected
known Track 02 MD5 when scanning a directory. A data root containing both US
and JP original releases is valid; the selected release remains launchable
instead of being misreported as ambiguous. The optional real-media test uses
the supplied root and selected MD5 to prove this without shipping game data.
- ✅ 2026-07-27 Theron Mednafen live-capture build repaired. Repaired the
  1.32.1 debugger trace patch so the core CPU/CD/input/sector instrumentation
  builds again, removed stale extension patches from the required build path,
  and made the local trace binary link a real SDL2 runtime with an embedded
  rpath. Verified against authentic US Track 02 plus System Card 3.0:
  Quartz-delivered Run input, two observed System Card calls, 25 CDIRQ events,
  and 31 raw-sector receipts. Dynamic dungeon-handoff rows remain deliberately
  unclaimed until an original run reaches them.
- ✅ 2026-07-27 Theron timed original-menu input capture. The authenticated
  Mednafen capture helper now supports ordered absolute input timings through
  `THERON_CAPTURE_HOST_KEY_DELAYS`. Verified a three-press Quartz Run sequence
  against authentic US Track 02 and System Card 3.0; the trace records every
  host SDL/key event and emulated port state. The current original route stops
  polling the PCE port before the scheduled presses, which remains explicit
  evidence rather than being misreported as a successful dungeon handoff.
- ✅ 2026-07-27 Theron Mednafen input-PC trace correction. Rebuilt the real
  SDL2-linked Mednafen 1.32.1 trace binary with CPU-PC provenance on every
  direct PCE input read/write and a configurable 4,096-per-direction cap.
  Authentic passive US CUE + System Card capture records 8,192 input
  transactions at System Card PCs `e4b7`/`e4c8`; the prior 128-row result was
  trace truncation, not a stopped-poll conclusion. Track 02 handoff remains
  blocked: this capture has no dynamic sector read or loader-consumer row.
  Verification: trace patch dry-run, full external Mednafen rebuild, SDL2
  runtime verifier, and authentic 55-second capture.
- ✅ 2026-07-27 Theron focused capture resolver. `capture_theron_mednafen_live_trace.sh`
  now waits up to ten seconds for Mednafen's own timeout/env descendant, then
  schedules host keys relative to capture launch instead of racing process
  creation. Script gate passes; a real 55-second capture attests four Return
  SDL events, PCE port `0000 -> 0008 -> 0000`, 31 raw-sector spans, and 56
  SCSI reads. It remains non-promotable because the initial input trace cap
  is reached before the host event.
- ✅ 2026-07-27 Theron post-key input-chain capture. Raised the default
  direct-input trace limit to 65,536 per direction and made PID Quartz
  delivery tolerate a background target while retaining a strict foreground
  requirement for global HID. Authentic US capture records 47,575 direct PCE
  transactions, 26,782 after the first host key, and direct `e4b7`/`e4c8`
  reads of port `0008`. Verification: Swift typecheck, shell/test gate,
  rebuilt SDL2-linked Mednafen, and 55-second authentic CUE/System Card run.
  No dynamic CD destination or game-owned PCECD reader appeared, so Track 02
  dungeon promotion remains blocked.
- ✅ 2026-07-27 Theron PCE input-result trace. Added a post-read trace hook
  after Mednafen applies PCE port semantics, rather than inferring result bits
  from host state. A real 28-second US capture with Return held records
  `raw=0008 -> value=3f` at `e4b7` and `raw=0008 -> value=37` at `e4c8`, plus
  4,796 subsequent PCE input transactions. This proves the observed input
  register result only; it does not assign a game command or promote Track 02
  data.
- ✅ 2026-07-27 Theron CD-to-RAM physical ownership trace. Added physical
  HuC6280 PC provenance to both CD-data reads and the matching RAM writes.
  Authentic input capture proves all currently observed candidates are System
  Card code `000a50/000a52` or `000b33/000b37`, including writes into
  `001fxxxx` main RAM. This closes the false inference that destination RAM
  alone proves a game loader; no game-owned CD consumer is promoted.
- ✅ 2026-07-27 Theron authentic multi-key boot capture. The live Mednafen
  capture harness now accepts one ordered absolute-time key sequence, keeping
  every element constrained to `return`, `i`, or `select`. Authentic
  `return@10,i@75,i@90` reaches the original Theron title menu and then real
  NEW GAME presentation, with six host key events and 8,910 subsequent PCE
  input transactions. This is boot/menu evidence only; no dungeon record,
  game-owned CD reader, or destination semantics are inferred.
# 2026-07-27 - Theron Mednafen loader-capture diagnostics

- Improved the authentic capture failure receipt with main-RAM `e009` dispatch,
  enter, data-read, and control-write counts. A raw-sector-only trace now
  states precisely that the missing proof is a game-owned PCE-CD data read.
- ✅ 2026-07-27 DM1 V1 door/wall-ornament source-lock maintenance. The
  viewport audit now follows the DM1-owned F0111 ornament planner after its
  coordinate sets and D2/D3 palette maps moved out of M11. It continues to
  verify the real ReDMCSB F0107/F0111 ordering, clipping and occlusion
  contract; `dm1_v1_viewport_door_wall_ornament_source_lock` passes.
- ✅ 2026-07-27 DM1 PC34 C70 save-event roundtrip. F0435 now reconstructs
  the signed `EVENT.B.LightPower` union for C70 rather than demoting it to
  generic cell/effect bytes, so a saved light-decay event can be written
  again by F0433. The PC34 export suite also verifies a materialized dungeon
  tail roundtrip and rejects an unproven C24 Fluxcage slot on the state-only
  path.
- ✅ 2026-07-28 DM1 real HoC orientation and champion-pointer regression.
  The registered `m11_dm1_hoc_orientation_runtime_pc34` CTest starts from
  the local original PC34 data, validates F0128 viewport material in all
  four directions and through live turn inputs, selects a real C127 mirror,
  resurrects the champion, and opens that champion's HUD inventory through
  the production pointer path. It also proves the shipped Hall's eight
  F0115 object candidates reach a real F0791 material draw, rather than
  needing synthetic floor or alcove art. The focused real-data
  HoC/object/alcove/save suite passes 4/4.

- ✅ 2026-07-28 DM1 default C140 save path. The live inventory SAVE control
  now has a regression that clears its test-only path override, creates the
  normal per-user `saves/dm1` directory, writes the save, and reloads it.
  This covers a fresh profile's former file-not-found failure mode.
- ✅ 2026-07-28 Compact runtime graphics popup. F10 now uses a narrow
  right-side panel and leaves the live viewport undimmed, so V1/V2.x mode,
  filter, palette, and scale changes can be judged immediately. Its input
  remains modal; the regression verifies the exposed viewport, compact close
  hitbox, and live setting changes.
- ✅ 2026-07-28 DM1 HoC object coverage and inventory-panel controls. The
  real PC34 HoC regression now requires every unique original object graphic
  from all eight ordinary candidates to reach an F0791 blit. C140/C141/C145/
  C011 are resolved before C081's broad inventory-panel route, so the
  visible Save, music, Zz and close controls cannot be swallowed. The
  source-owned save-disk menu is explicitly exercised before its save write.

- ✅ 2026-07-28 DM1 full turn-button feedback. Q/E, Home/End and controller
  turns now outline the complete 29x23 C013 turn cells; mouse hit geometry
  remains the original narrower C068/C069 rectangles.
- ✅ 2026-07-28 CSB title/Entrance source timing. ReDMCSB `TITLE.C:451-463`
  proves 60 VBlanks of PRESENTS, 20 CHAOS shrink frames, `Delay(20)` on the
  full CHAOS page, then `Delay(2)` on STRIKES BACK. The old 101-tick model
  held the final title frame for one VBlank. CSB now uses the correct
  102-tick timeline, and focused real-data title/Entrance regressions pass.
- ✅ 2026-07-28 DM1 V2.x current verification. Built the only previously
  absent registered V2 cursor-mask test binary, then ran the complete
  V2.0/V2.1/V2.2 CTest selection against local original DM1 data: 88/88
  passed. Coverage includes mode handoff, HUD/pointer routes, viewport,
  item/creature/spell/effect paths, resolution mapping, assetpack gates and
  real runtime presentation smoke.
- ✅ 2026-07-28 DM1 original PC34 save round trip. A real DOSBox
  `DMSAVE.DAT` now passes fixture-free F0435 -> F0433 -> F0435 admission:
  source bytes stage into a live world, a saved portrait reaches the active
  inventory panel, the complete tail is preserved, and exported bytes reload
  through the same handoff. C13 remains optional evidence, as it is in the
  C3 event stream; a valid C13-free save is no longer rejected for lacking a
  fabricated C13 lifecycle receipt. The external-corpus, handoff, and
  external-HoC runtime regressions pass against the supplied data.
- ✅ 2026-07-28 DM1 F0115 near-square consumer audit. Retired the stale
  TODO claim that D0/D1 object presentation needed a second M11 bridge.
  The active renderer already uses F0098 for source floor/ceiling material
  and F0115 C2500/F0791 for visible floor objects; the old isolated receipt
  has no production caller and must not be wired as a duplicate item blit.
  Real-PC34 floor-item and alcove runtime regressions pass.
- ✅ 2026-07-28 DM1 D0C C15 effect-order repair. The live F0115 receipt no
  longer filters fluxcage or rebirth C15 records before their source-specific
  consumers run. It preserves original C15 order while the renderer remains
  no-draw without an authenticated special bitmap. C14/C15 layout, projectile
  impact, D0C receipt, C15 runtime-capture, and projectile presentation tests
  pass.
- ✅ 2026-07-28 DM1 HoC F0115 presented-pixel gate. The real PC34 Hall sweep
  finds all eight original floor/alcove object graphics and now requires each
  F0791 destination rectangle to change after its exact source blit. This
  proves final framebuffer consumption rather than only a material receipt.
  The identical real-data sweep now passes in V1, V2.0, V2.1, and V2.2, with
  a real C127 mirror route in every presentation mode.

- ✅ 2026-07-28 DM1 V2 inscription preservation. V2.2 no longer suppresses
  the final ReDMCSB F0107/M648 repaint after V22 art. V2.0, V2.1, and V2.2
  now all prove exact original M648 glyph pixels, C10 transparency, and stale
  text invalidation with real PC34 wall text.
## 2026-07-28 DM1 C14/C15 final viewport consumers

- Closed the stale DM1 F0115 C14/C15 host-consumer follow-up. Real PC34
  runtime tests now prove a thrown object reaches the final C2900 material
  blit and an ordinary C15 explosion reaches the deferred final-pixel pass.
  Source identity, catalogue admission, material fingerprint, and fail-closed
  rejection remain enforced before either draw.

## 2026-07-28 DM1 V2 inventory controls

- Added runtime coverage for C141 music, C140 save-disk, C145 rest and C011
  close in V2.0, V2.1 and V2.2. Presentation selection does not make the
  visible DM1 inventory controls inert.

## 2026-07-28 DM1 HoC capture route

- The real-PC34 HoC regression now reports its selected source mirror route:
  wall `(14,2)`, party `(14,3)`, north, ordinal `5` for the installed corpus.
  This makes repeatable macOS/window capture possible without guessing a
  champion-mirror location.

## DM1 HoC viewport occlusion

- **DM1-VIEWPORT-001**: Fixed the corridor-through-wall artifact in the live
  M11 renderer. ReDMCSB F0128 draws center walls as part of each square before
  visiting nearer squares; Firestaff's deferred F0115 batch could otherwise
  paint deeper corridor content over a nearer wall. The final source-backed
  center-wall pass now restores the wall and then replays the D1C champion
  mirror route. Verified with the DM1 wall-ornament and inventory placement
  tests plus a clean `firestaff` Ninja build on 2026-08-05.
## DM1 source-data fail-closed wall rendering

- **DM1-VIEWPORT-002**: Removed the synthetic black rectangle used when a
  center wall bitmap could not be loaded. The ReDMCSB wall path now leaves the
  cleared/background pixels unchanged and reports the missing authenticated
  GRAPHICS.DAT material through the existing asset route. This prevents a
  missing asset from masquerading as a corridor opening or fabricated wall.
  Verified with the DM1 wall-ornament (`121/121`) and inventory placement
  (`156/156`) tests plus a successful Ninja `firestaff` build on 2026-08-05.
## DM1 centre-wall ornament restoration

- **DM1-VIEWPORT-003**: Prevented the final nearest-wall occlusion replay from
  erasing authentic centre-wall inscriptions and alcove material. The replay
  now restores only source-owned centre ornaments after the wall bitmap, then
  hands the live champion mirror route back to the renderer; side ornaments
  are not replayed across the occlusion boundary. Verified with the DM1 wall
  ornament (`121/121`) and inventory placement (`156/156`) tests and a clean
  Ninja build on 2026-08-05.

- ✅ 2026-08-05 Nexus palette source-lock correction: aligned the Phase 4
  rendering documentation with the actual fail-closed `STONE.BIN` loader.
  Short palettes clear and remain unavailable; they do not receive the old
  inferred `g_npal_default` colour table. Verified by the real-data DGN
  geometry readiness gate against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-08-05 DM2 actuator generator provenance hardening: removed the
  remaining live wall-mecha generator mutations. Creature generation no
  longer invents a fixed HP/base value or tick-derived direction, and item
  generation no longer allocates a generic DB item from actuator data alone.
  Both remain fail-closed pending the complete source `ALLOC_NEW_CREATURE` /
  `ALLOC_NEW_DBITEM` ownership chains. Verified by the focused actuator and
  runtime gates plus the mounted real-data startup, HUD, material,
  scene/weather and original-save-writer gates.
- ✅ 2026-08-05 Nexus rasterizer provenance cleanup: corrected the Phase 4
  source-lock record to describe the actual production boundary. Flat-color
  geometry, unsupported 3D assets, and missing surfaces/textures remain
  explicitly no-draw; the retired gray-billboard/placeholder claims are no
  longer documented as runtime features. Verified with
  `test_nexus_v1_dgn_material_raster`, the real-data DGN geometry gate, and
  `git diff --check`.
- ✅ 2026-08-05 DM2 unbound CCM timer hardening: an unresolved
  `DM2_THINK_CREATURE` body now consumes its source timer without re-queuing a
  coordinate-only creature retry. Live record pools and timer queues remain
  unchanged until the complete original CCM stream owns animation, movement
  and rescheduling. Verified by the think-creature, CCM-runtime and CAII
  reschedule gates.

- ✅ 2026-08-05 Nexus FACE.BIN production provenance gate: the low-level
  retail PRS3 structural/pixel diagnostic remains available for evidence, but
  `nexus_ui_load_face_record()` no longer promotes unproven PRS3 output or its
  64-entry per-frame palette into live startup UI. Production now records all
  portraits as blocked until an original Saturn capture authenticates pixel
  grammar, palette lane, and placement. Verified with the real FACE.BIN
  structural probe, updated Track 1 launch probe (57/57), and the focused
  Nexus build.
## DM1 combat-log source font guard

- **DM1-UI-001**: The normal verified DM1 catalog launch no longer renders
  the built-in mini-font when the original `GRAPHICS.DAT` font is unavailable.
  It now fails closed until the source font is bound; the mini-font remains
  available only for explicitly non-catalog diagnostic callers. This removes
  a synthetic production visual without changing the source-backed font path.
  Verified with a successful Ninja `firestaff` build and combat-log contract
  test (`5/5`) on 2026-08-05.

- ✅ 2026-08-05 Nexus SAL playback gate correction: real SAL tone decoding
  can now populate diagnostic receipts, but `nexus_sound_play_event()` and
  `nexus_sound_play_idx()` check the complete runtime receipt before invoking
  the tone trigger. Decoded bytes cannot bypass the unresolved SDDRVS/event
  ABI gate. Verified with the real-corpus sound runtime receipt test.
## DM1 HoC source item-name guard

- **DM1-HOC-OBJECTS-003**: DM1 item labels now require the authenticated
  ReDMCSB `OBJECT.C` M564 icon-indexed name stream. When that source table is
  absent or malformed, Firestaff leaves the label empty instead of presenting
  the legacy hand-written subtype catalog as if it were original data. The
  fallback catalog remains available only outside DM1 source-owned routes.
  Verified with a successful Ninja `firestaff` and real-alcove target build,
  plus `git diff --check`, on 2026-08-05.

- ✅ 2026-08-05 CSB scanner inventory clarity: `--scan-data` now labels
  `GRAPHICS.DAT` and `DUNGEON.DAT` explicitly as launch requirements, then
  recursively reports every other hash-catalogued CSB source medium it finds,
  including entries inside supported archives. This keeps the two-file launch
  gate intact while exposing verified `ANIMATE.*`, Hint Oracle, Utility Disk,
  `MINI.DAT`, and platform sidecars from the real data rather than relying on
  a small fixed list of loose filenames. The shared fingerprint test passes
  284/0.

- ✅ 2026-08-05 CSB Atari ST title cadence: the real `ANIMATE.SCR` M11
  handoff regression now proves that each 55 ms V1 tick becomes the correct
  accumulated 50 Hz source-VBlank count, never regresses, and reaches the
  `FTLCODE` handoff only at the script-derived terminal boundary. This guards
  against a title that advances too quickly. ReDMCSB `ANIM.C:67-72` and its
  VBlank waits establish the source timing; the extracted local Atari ST
  package passes the focused handoff test.
## DM1 source object icon parity

- **DM1-HOC-OBJECTS-004**: Added the missing ReDMCSB `OBJECT.C F0033`
  charged-Jewel-Symal branch. DM1 now resolves the source `G0237` Jewel Symal
  icon from its raw `JUNK.ChargeCount`, matching the original water/illumulet
  charged-item family instead of leaving the base icon selected. Regression
  coverage exercises the PC34 raw record and expects icon 11 for a charged
  Jewel Symal. Verification: `test_dm1_v1_projectile_explosion_render_pc34_compat`
  passed with all tests, plus `git diff --check`, on 2026-08-05.

## DM1 source fountain interaction

- **DM1-HOC-OBJECTS-005**: Reconnected the live DM1 C080 wall-click route to
  the ReDMCSB `F0601` fountain predicate. The current map's real
  `DUNGEON.DAT` wall-ornament table is now matched against `G0193` before a
  leader-hand object can be changed. Empty-hand drinking, charged waterskin
  filling and empty-flask to water-flask mutation now update the loaded
  runtime records; generic wall ornaments retain the sensor/drop path.
  Verified with a full `firestaff` build and the source fountain regression
  (`fountainInteractionInvariantOk=1`) on 2026-08-05.

## DM1 source wall ornament table correction

- **DM1-HOC-OBJECTS-006**: Corrected the DM1 PC34/I34E `G0194` wall-ornament
  coordinate-set table. Firestaff had used the ReDMCSB `MEDIA353` variant
  (`DUNVIEW.C:846-906`); PC34 uses the `MEDIA529`/`I34E` table at
  `DUNVIEW.C:932-1007`, including coordinate sets 7/8 for the real wall
  ornament family. The source graphic base remains `M615=259`, with F0791
  transparent colour 10 and G0198/G0199 palette maps unchanged. Focused
  G0194 and wall-plan tests pass after the correction. Real macOS pixel
  capture is still tracked separately in `DM1-HOC-OBJECTS-001`.
- ✅ 2026-08-05 CSB Atari ST executable-media inventory: corrected the
  `SWITCH.DAT` fingerprint to the bytes in the original hard-disk package and
  added hash identities for `ANIMATE.FTL`, `CHAOS.FTL`, and `FTLCODE`.
  ReDMCSB `COMPILE.H:609-620` identifies the three modules and `ANIM.C:94`
  makes the `FTLCODE` transfer explicit. They are reported as verified source
  media without changing the `GRAPHICS.DAT`/`DUNGEON.DAT` start gate. The
  fingerprint suite passes 294/0 against the extracted local package.
- ✅ 2026-08-05 Theron teleporter fail-closed hardening: unresolved legacy
  object-ID links and cyclic/overlong chains no longer report a successful
  transition or place the party at the clicked square. Transition and party
  state remain unchanged until a real terminal object record resolves;
  missing-target and cycle regressions now assert rejection.
- ✅ 2026-08-05 CSB Utility Disk CMP disk-format correction: replaced the
  synthetic 496-byte portrait layout with ReDMCSB's actual 508-byte `CMP`
  record. The decoder now reads the big-endian `Magic`, dungeon-id, platform,
  compatibility words, reserved words, name/title and the 464-byte portrait at
  offset 44. ReDMCSB `DEFS.H` defines the layout and `CEDT001.C F7000` writes
  exactly 508 bytes. The extracted original Atari ST `PORTRAIT/HALK.CMP`
  decodes as HALK, THE BARBARIAN; CMP import, portrait-handoff and title/import
  regressions pass without allowing a portrait-only file to invent party state.
- ✅ 2026-08-05 Theron legacy asset-parser cleanup completed: removed the
  unreachable THS4 sound parser body and its guessed marker constants from
  the implementation. The public diagnostic APIs remain explicit rejection
  seams; no Firestaff-only THG3/THS4 bytes can become runtime media.
- ✅ 2026-08-05 CSB Utility Disk portrait inventory and scanner repair: added
  hash identities for all 26 original Atari ST `PORTRAIT/*.CMP` files, whose
  508-byte disk format is established by ReDMCSB `CEDTDATA.C:394/397` and
  `CEDT001.C F7000`. The CSB report now uses media already materialized by
  the status scan instead of triggering a second recursive archive traversal.
  Archive materialization retains the real `ANIMATE.FTL`, `CHAOS.FTL` and
  `FTLCODE` modules beside the launch pair. A real loose-package scan shows
  those modules plus `SWITCH.DAT` and `MINI.DAT`; the fingerprint suite passes
  373/0.
- ✅ 2026-08-05 Theron Track 19 item-name binding: added a source-span reader
  for the US MODE1/2048 table at ISO offset `0x0E9271`. It validates all 69
  null-separated names against the verified catalog before returning any one
  label, and rejects truncation or byte changes. The real local `TQUS19.iso`
  passes the full table probe.
- ✅ 2026-08-05 Theron Track 19 level-label binding: added byte validation for
  the real US ISO selector table at offset `2112059`, covering `LEVEL  1`
  through `LEVEL 15`. The probe validates the complete table and rejects a
  changed label byte; this exposes labels only and does not invent maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB scanner sidecar visibility: `--scan-data` now searches
  beside the hash-matched loose `GRAPHICS.DAT` package (not only the selected
  data root) before reporting verified CSB media. The candidate inventory also
  recognizes Atari `ANIMATE.FTL`, `CHAOS.FTL`, `FTLCODE` and `MINI.DAT`.
  Archive-cache regression coverage now proves the three real Atari startup
  modules remain materialized beside the verified launch pair. Source-lock:
  ReDMCSB `ANIM.C:67-72,94`; verified against the real Atari ST archive.
- ✅ 2026-08-05 Theron Track 19 file-inventory binding: added a reusable
  file-backed receipt that authenticates the exact ISO hash/size and validates
  both real US metadata spans (69 item names and 15 level labels). The
  inventory exposes verification flags without admitting dungeon maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB Atari animation runtime chain: the authentic
  `ANIMATE.FTL`/`CHAOS.FTL`/`FTLCODE` trio now has its own hash-verified
  discovery and cache receipt. The modules must come from the same directory
  or archive and are never run as host binaries. Verified against the local
  Atari ST 2.0 directory using the original MD5. Sources: ReDMCSB
  `ANIM.C:67-72,94`, `COMPILE.H:609-620`, and DMWeb's Animation Script and
  animation-format documentation.
- ✅ 2026-08-05 CSB map-difficulty provenance: removed the invented
  champion-count percentage scale and its hard-coded three-champion default.
  A loaded CSB profile now takes the current map's authenticated `MAP.C`
  high-nibble difficulty from `DUNGEON.DAT`; a roster-only or failed handoff
  stays explicitly unbound. Runtime-image restore no longer revives the old
  synthetic multiplier. Source-lock: ReDMCSB `DEFS.H` `MAP.C`, `PANEL.C`
  F0337, `CHAMPION.C` and `PROJEXPL.C`; covered by CSB boot and save tests.
# 2026-08-06 Theron extended authentic replay receipt

- ✅ A 120-second Mednafen replay using the authenticated US Track 02 CUE,
  verified System Card and repeated Run/I input produced 54 SCSI reads but no
  game-owned post-startup Track 02 consumer, `$2600` handoff, or source-owned
  VDC/VCE destination receipt. The bounded main-RAM windows remain retained as
  loader evidence only; no level, object, tile, material, palette, HUD or
  viewport semantics were enabled.

# 2026-08-06 Theron text publication boundary

- ✅ Authentic Track 02 text codons remain decoded from the supplied US media
  for diagnostics, including their exact unresolved control-code markers.
  Production `theron_v1_world_load_dungeon_text()` now keeps the world text
  table empty when those markers occur, so candidate strings cannot become
  synthetic HUD, plaque or scroll text. The focused real-media regression
  passes and will reopen only after the original HuC6280 text consumer is
  identified.

# 2026-08-06 Theron source-index receipt integrity

- ✅ Track 02 source occurrences now retain their full 16-bit category index
  instead of an 8-bit field. This matches the 512-entry source-category
  bound and prevents later real records from being truncated or rejected.
  No category-local type was promoted to a host item index.

# 2026-08-06 Theron synthetic V2.2 asset quarantine

- ✅ The V2.2 modern-asset admission gate now requires
  `source_provenance="authenticated_track02"` in the manifest. The existing
  procedural and `gpt-image-2` Theron art pack is rejected by production and
  remains available only to fixture/reference inspection. The focused asset
  test passes 36/36.

# 2026-08-06 Theron US Track 02 descriptor receipt

- ✅ The production Theron source layer now reads all 53 six-byte level
  descriptor records from UD `0x619900` in the authenticated US Track 02
  MODE1 user-data stream. The focused test extracts the real BIN sectors,
  verifies the source-locked bytes and rejects mismatched tables. This closes
  descriptor-byte provenance only; it does not infer graphics compression,
  object IDs, tile-bank ownership, palette binding or dungeon handoff.

# 2026-08-06 Theron HuC6280 decompressor receipt

- ✅ Extended the authenticated US/JP bank-$1f disassembly receipt from the
  134-byte helper fragment to the full byte-identical `$23AD-$252A` routine.
  The 382-byte range covers the variable-bit reader, bank switches, literal
  output and back-reference path. It remains evidence only: the caller,
  destination and level-block contract are not yet proven, so no decoder was
  enabled in production.
# 2026-08-06 Theron split-ISO Mednafen capture intake

- ✅ The live Mednafen capture runner now handles the supplied retail CUE's
  CRLF and unquoted `FILE TQUS02.iso BINARY` spelling. When that authenticated
  MODE1/2048 member is absent but the production cache contains the exact
  `ceb02343868f80cec899e9b239aff2da` US ISO assembled from `TQUS19.iso` and
  `TQUS02End.iso`, the runner creates a private normalized capture CUE and
  replaces only Track 02. Track 19 and audio references remain from the
  original layout. This removes the missing-member/raw-BIN capture mismatch;
  it does not claim a game-owned dungeon consumer. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh` and
  `tests/test_theron_v1_mednafen_live_capture_script.sh` pass.
- ✅ 2026-08-06 DM1 GRAPHICS.DAT partial-surface quarantine: the legacy reader
  now rejects short LZW decodes and undersized output buffers instead of
  copying incomplete indexed pixels into a bitmap. The focused fail-closed
  regression and the real 713-record PC34 audit pass; no generated surface is
  admitted as a substitute.
# 2026-08-06 Theron Japanese split-ISO capture intake

- ✅ The live Mednafen capture runner now supports both regions. The supplied
  Japanese CUE's CRLF/okvoterade `FILE TQJP02.iso BINARY` member is normalized
  to the complete sibling `TQJP02End.iso` only after its authentic MD5
  `397039af02d50d15c70b74088eb8a1cb` is verified. `THERON_CUE` is accepted as
  the generic variable while `THERON_US_CUE` remains compatible. This extends
  only verified media intake; no JP consumer, dungeon, palette or viewport
  semantics are promoted. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh`, the live-capture
  script regression, and the real archive CUE transformation pass.
- ✅ 2026-08-06 F10 source-owned live graphics controls: Theron now routes its
  V2 filter changes through `theron_v2_settings` and persists the Theron slot.
  DM2/Nexus no longer mutate DM1 filter state from the popup; unsupported
  source-specific rows are explicitly locked while shared presentation and
  cheat/speed controls remain available. `m11_runtime_graphics_popup` passes.
- ✅ 2026-08-06 DM1 F0115 object identity quarantine: real floor-object and
  HoC alcove rendering now requires the source-owned raw PC34 `THING` record
  before resolving subtype or material. Missing raw identity produces no-draw
  instead of a candidate-derived wrong icon/name. Real F0115 floor pickup and
  alcove pickup-to-inventory tests pass against the PC34 corpus.

- ✅ 2026-08-06 DM2 FM Towns native startup-media gate: the HME-242 ISO reader
  now inventories the root `AUTOEXEC.BAT`, `SWOOSH`, `TITLE`, `TWANIM.EXP`,
  `SKULL.EXP` and `END` files as well as `DATA/`. It reads the original boot
  script in memory and requires the authenticated `SWOOSH -> TITLE -> SKULL
  -> END` route before boot accepts an FM Towns session. The real Japanese CD
  ZIP plus explicitly selected English GDAT companion regression passes with
  no game member unpacked to disk. This verifies the native animation/startup
  ownership and blocks partial media; it does not claim that TWANIM frame
  playback has been implemented.

- ✅ 2026-08-06 DM2 FM Towns animation-stream authentication: boot now checks
  the selected in-memory HME-242 `SWOOSH`, `TITLE` and `END` streams against
  the published retail MD5s before it accepts the AUTOEXEC animation plan.
  This binds the actual 18-layer swoosh and 224-layer/5-sound title corpus to
  the selected FM Towns CD rather than accepting name-compatible bytes. The
  M12 real-media regression verifies all three identities with the Japanese
  CD ZIP and English text companion, without extracting any game data to disk.

- ✅ 2026-08-06 DM2 FM Towns TWANIM stream-bound admission: the production
  boot owner now parses the selected, hash-verified root streams directly from
  the retained CD image using DMWeb's six-byte big-endian record framing.
  It requires the exact HME-242 inventories before exposing startup media:
  SWOOSH has 22 records/18 deltas, TITLE has 235/224 deltas plus one sound
  definition and five sound events, and END has 401 records/382 deltas across
  two matching animation phases. `test_dm2_fmtowns_m12_real_media` proves all
  three receipts from the user's original ZIP in RAM; no title frame is
  invented or rendered by this boundary.

- ✅ 2026-08-06 DM2 FM Towns TITLE IMG1 decoding: `dm2_v1_fmtowns_anim_stream`
  now replays HME-242 EN/DL records into the original packed 320x200 4bpp
  canvas directly from the selected CD stream. It follows SKWIN
  `ANIM_DECODE_IMG1` (0759:0330), including its original contiguous-stream
  boundary behaviour, while retaining strict whole-stream bounds. The
  real-media test locks first/final TITLE frame command counts and FNV-1a
  receipts (`c7ad2279`, `5ef57a09`) computed in RAM from the retail stream.
  This is a decoder and source receipt only; M11 palette/timing/presentation
  remains explicitly unclaimed until its own source-owned handoff exists.

- ✅ 2026-08-06 DM2 FM Towns TITLE M11 presentation: the selected HME-242
  `TITLE` member is retained only in RAM after its boot/profile MD5 and stream
  receipts pass. M11 decodes the original PL index/RGB4 palette, expands it at
  the indexed-render boundary, and presents the stream's packed 320x200 4bpp
  canvas instead of the PC static GDAT menu. EN/DL progression uses the
  SKWIN TWANIM Timer-A unit (`18*(1024-100)` microseconds) and each source
  display duration clamped to the original five-tick minimum. Input cannot
  reach SKULL's later menu until TITLE ends; rejected Towns media remains
  black rather than falling back to PC art. The opt-in real-CD M11 regression
  launches the selected Japanese ZIP plus authenticated English companion,
  verifies the first rendered frame and a source-timed advance; the focused
  `test_dm2_fmtowns_m12_real_media` also passes without unpacking game data.

- ✅ 2026-08-06 DM2 FM Towns TITLE sound-plan receipt:
  `dm2_v1_fmtowns_anim_stream_decode_title_sound` now retains the HME-242
  TITLE's real 12,862-byte signed SND2 PCM span and its five SO events from
  the selected CD buffer. The real-media regression locks the source offsets
  (14, 101790 … 492266), frame positions (14 … 131), sample FNV-1a
  `0b829ae7`, source volume bytes and `03e8` field. DMWeb identifies that
  frequency as invalid for this title; SKWIN `0759:0E33/0EF0` proves slot 1
  and fixed 5500 Hz instead. This is a read-only source receipt, not an SDL
  playback claim; no game member was unpacked or copied to disk.

- ✅ 2026-08-06 DM2 FM Towns SWOOSH M11 presentation: M11 now follows the
  real HME-242 `AUTOEXEC.BAT` ordering by presenting authenticated `SWOOSH`
  before `TITLE`. Its `AN` header is 0x0, so the IMG1 decoder takes the
  320x200 canvas only from SWOOSH's first EN record, exactly as SKWIN
  `ANIM_DECODE_IMG1` does. The retained stream/palette/frame buffer is reused
  for TITLE only after SWOOSH's 19 source frames finish on the Timer-A cadence.
  The real-CD M11 regression locks source frame-zero and first-delta output
  (13 and 59 indexed pixels), prevents early SKULL input, and reaches TITLE.
  No file is unpacked and no PC GDAT screen substitutes for either stream.

- ✅ 2026-08-06 DM2 FM Towns SKULL fallback fence (superseded by the verified
  IMG2 handoff): the temporary black completion state rejected PC GDAT as a
  platform substitute. M11 now presents only the selected HME-242
  `TITLE/0/dtImage+dtPalIRGB/4` IMG2 surface after TITLE, using its native
  local palette and `dt04/0` NEW GAME/RESUME rectangles. Native `SKULL.EXP`
  P3 execution, keyboard routing and continuation semantics remain closed;
  see `parity-evidence/dm2_fmtowns_startup_p3_gdat_boundary.md`.

- ✅ 2026-08-06 DM2 FM Towns CDDA mapping correction: removed the former
  hard-coded HMP→CDDA source literal. Boot now extracts the selected
  HME-242 `SKULL.EXP` in RAM and copies only its native 29-byte table at
  offset `0x3dac` into a bounded receipt. The real-CD regression locks the
  374,416-byte member, source offset and map lookup. Playback remains
  separately blocked until native SKULL execution and CDDA transport are
  joined.

- ✅ 2026-08-06 DM2 FM Towns CDDA coordinate correction: runtime CDDA
  dispatch now reads the live source party X/Y for the original 40-byte
  CD.DAT level-coordinate trigger table, and reevaluates only this route
  after a committed party step. It no longer probes a fabricated `(0,0)`
  cell. Missing source party state remains silent.

- ✅ 2026-08-06 Theron raw-BIN HuC6280 disassembly intake: the static bank-$1f
  receipt now verifies authentic `TQUS02.bin` and `TQJP02.bin` Track 02 files
  in addition to the ISO projections. Their real MODE1/2352 bank-window
  offsets and regional stage-2 handler hashes are bound by MD5/size/byte/FNV
  checks. The focused test passes all four authentic US/JP BIN/ISO sources;
  runtime consumer and semantic level/object/palette/tile/viewport handoff
  remain capture-gated.

- ✅ 2026-08-06 Theron forcefield-menu keyboard fix: M11's physical
  left/right arrow tokens (`STRAFE_LEFT/STRAFE_RIGHT`) now move Theron's Soul
  Room focus. Enter can therefore reach the FORCEFIELD action instead of
  appearing inert; the source-owned capture gate still prevents an
  unverified dungeon handoff.

- ✅ 2026-08-06 Theron real-data inventory: documented the authenticated US/JP
  Track 02 BIN/ISO files, the separate US/JP Track 19 ISOs and the materialized
  US split ISO, including size/MD5 ownership. The documentation explicitly
  prevents Track 19 bytes from being reused as Track 02 data and lists the
  remaining intentional placeholder/capture boundaries.
# 2026-07-31 Theron Track 02 quest-block extraction

- ✅ Added a source-data extractor for the seven 256 KiB quest blocks present
  in the verified US Track 02 raw BIN. Each block is reconstructed from
  MODE1/2352 sectors into contiguous 2048-byte user-data bytes and checked by
  an independent FNV-1a receipt in the bank probe. JP media remains explicitly
  unsupported until its corresponding block offsets are independently
  verified. This is real-data byte extraction only; it does not claim dungeon
  record, object-table, palette, bitmap, or runtime-render semantics.
  Verification: `theron_v1_track02_bank` and the clean-branch C11 syntax checks.
- ✅ 2026-07-31 DM2 V2 smooth viewport no-fabrication closure: removed the
  host-side pan and black-strip fill that ran after the real V1 viewport
  renderer. Smooth timing state remains available to input consumers, but no
  intermediate DM2 camera raster is known, so every presented frame remains
  the source-owned snapped V1 raster. References: SKProject
  `SKWIN/SkWinCore.cpp::DRAW_DUNGEON` and `DRAW_OUTDOOR_VIEWPORT`.
  Verification: V2 smooth movement 79/79, runtime binding 43/43, smooth
  probe 54/54, plus a byte-identical V1/V2 framebuffer comparison during an
  active smooth state in the hash-verified real-data DM2 M11 startup test.
# 2026-07-31 Theron JP Track 02 quest-block extraction

- ✅ Extended the real Track 02 quest-block extractor to the hash-verified JP
  BIN. The JP bank begins one raw MODE1/2352 sector before the US bank; all
  seven 256 KiB blocks are reconstructed from contiguous 2048-byte user data
  and independently checked against FNV-1a receipts from `TQJP02.bin`.
  The US receipts remain covered as well. This follows DMWeb's JP/USA
  PC-Engine CD split and seven-dungeon scope; it does not claim dungeon-record,
  object, graphics, or save-format decoding.
  Verification: clean C11 syntax checks, clean CMake target build, and
  `firestaff_theron_v1_track02_bank_probe` against both local real BINs.
- ✅ 2026-07-31 DM2 V2 unbound sky-colour closure: removed the procedural
  RGB gradients and fixed weather colours from the V2 lighting/outdoor helper
  APIs. ENVIRONMENT_DRAW_DISTANT_ELEMENT owns an outdoor image, palette and
  destination rectangle; time and weather alone cannot select original
  pixels. Unbound callers now receive DM2_V2_SOURCE_COLOR_UNAVAILABLE.
  References: SKProject SKWIN/c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT
  and skgdtqdb.cpp QUERY_TEMP_PICST/DRAW_TEMP_PICST. Verification:
  test_dm2_v2_lighting 64/64 and DM2/M11 build pass.
# 2026-07-31 Theron media-inventory false-promotion removal

- ✅ Raw Track 02 now proves startup/media ownership only. Removed the
  incorrect bitmap-, level-, and object-route promotion that treated an
  authenticated bank as if its dungeon decoder were already implemented.
  Downstream routes remain fail-closed until real consumer/decoder evidence
  exists, matching the bounded Theron status in `docs/DMWEB_REFERENCE.md` and
  TODO.md. Verification: `theron_v1_media_inventory_probe` passes.
- ✅ 2026-07-31 CSB startup fallback contract: removed the dead text and door
  fallback fields from the title/entrance plan, its old render commands, and
  the host rendering they could trigger. CSB startup now represents only the
  original C001–C005, C017, and C040; a missing source results in no draw
  instead of text or colored panels. Source: ReDMCSB `TITLE.C F0437`,
  `ENTRANCE.C F0438/F0441/F0806`; CSBWin `Viewport.cpp`. Verification: startup
  plan 139/139, boot handoff 501/501, the real-data C001–C005/C017/C040
  sequence, and the title-cadence probe pass.
- ✅ 2026-07-31 CSB startup render callbacks: removed the remaining executor
  API surface for door and text fallbacks. ReDMCSB `TITLE.C F0437` and
  `ENTRANCE.C F0441/F0806` now reach only concrete original surfaces through
  the title, door, opening-frame, and utility callbacks; host code can no
  longer attach a local replacement drawing. Verification:
  `test_csb_v1_boot_runtime_handoff` passes 501/501.
- ✅ 2026-07-31 DM2 inventory substitute closure: removed the reachable M11
  renderer that put authentic DM2 ObjectID icons into DM1 `GRAPHICS.DAT` slot
  rectangles and removed its matching DM1 click route. Keyboard and direct
  champion inventory commands now fail closed as well, leaving SKSave/DB
  ObjectID ownership untouched until the real DM2 inventory surface is
  bound. Source: SKProject `CHANGE_VIEWPORT_TO_INVENTORY`, with its
  `CHAMPIONS`/`INTERFACE_GENERAL` GDAT layout and event table. Verification:
  real-data `test_dm2_v1_m11_startup_profile_gate`.
# 2026-07-31 Theron alarm spawn fallback removal

- ✅ Removed the production alarm-trigger path that fabricated a Goblin for
  every creature spawner. The alarm still activates source spawners and emits
  its alarm event, but creature materialization now stays fail-closed until
  the real Track 02 object-tail/spawn table is decoded. Regression coverage
  verifies activation and no fabricated object (`52/52` mechanics checks).
- ✅ 2026-07-31 CSB startup asset types: removed the unused `fallback` source
  type and dead `fallback-original` alias from CSB graphics binding. Startup
  now accepts only verified `GRAPHICS.DAT` or verified `CSBgraphics.dat`;
  negative tests use the actual invalid type `NONE`. Verification:
  `test_csb_v1_boot_title_import_ui_gate_pc34_compat` passes 137/137 and
  `test_csb_v1_csbgraphics_runtime_binding` passes 83/83.
- ✅ 2026-07-31 CSB source inventory: corrected the mislabeled Lord Order
  type. `0x19` is ReDMCSB `DEFS.H:1364` C25_CREATURE_LORD_ORDER, not a
  placeholder, even though the original dungeons contain no such groups.
  Also updated TODO's stale note about the removed `fallback-original` alias.
  Verification: `test_csb_v1_monster_generator_gate_pc34_compat` passes.
- ✅ 2026-07-31 CSB title-capture cadence: real-data captures for V1, V2.0,
  and V2.1 now wait 14 seconds instead of 7, so they observe all four original
  palette phases after the PC3.4-bound CHAOS zoom. Game speed is unchanged.
  Verification: V1 title/entrance contract and V2.0/V2.1 capture tests pass
  against local PC3.4 data.
- ✅ 2026-07-31 CSB F0115 projectiles: removed the old 16×16 icon drawing that
  could replace ReDMCSB's perspective bitmap for thrown objects. A missing
  source-bound F0115 bitmap now results in no draw; only the verified
  perspective route can write projectile pixels. Verification:
  `test_csb_v1_viewport_phase3_rendering` passes 2655/2655.

- ✅ 2026-07-31 DM1 HoC candidate time-effects and endgame fallback gates:
  the live M11 idle route now proves ReDMCSB `CHAMPION.C F0331` excludes the
  selected C040 candidate from health/stamina/food/water mutation, then
  restores normal decay at the next due tick after confirmation. The related
  F0444/F0446 regression expectations were aligned with the existing
  source-only policy: missing original final-screen art draws no synthetic
  controls, while an available SDL backend may queue real SONG.DAT victory
  audio. Verification: `m11_starvation_runtime_source_lock`,
  `m11_action_stamina_runtime_source_lock`,
  `dm1_v1_hall_of_champions_pc34_compat`, and the real backed PC34 corpus
  roundtrip all pass.
# 2026-07-31 Theron relic-name correction

- ✅ Replaced the invented quest-item labels in progression, chapter-marker,
  and champion-item comments with the seven real Theron's Quest relic names
  documented by DMWeb: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza
  Armor, Soulcage, and The Retaliator. This changes presentation metadata only;
  item ordinals and the unresolved Track 02 placement/decode remain bounded.
  Verification: `firestaff_theron_v1_chapter_marker_probe` passes `65/65`.
# 2026-07-31 Theron seed-placeholder reduction

- ✅ Replaced the dungeon-1 progression fallback seed `313` with the real
  US/JP Track 02 initial-level seed `0x0108e938`. The unresolved dungeon 2–7
  fallback seeds are now zero rather than fabricated ascending values, so
  progression/save state cannot present guessed seeds as original data.
  Verification: `test_theron_v1_m11_direct_launch` passes; real US/JP Track 02
  probes bind the same initial seed at their verified raw offsets.
# 2026-07-31 Theron stale placeholder metadata removal

- ✅ Removed the retired Theron dungeon-seed fallback `313` from the boot
  profile; an unbound profile now starts at zero and only verified header or
  Track 02 handoff data may populate the seed. Updated the Track 02 source-lock
  table to mark dungeon 1 as `0x0108e938` (verified initial level) and dungeons
  2–7 as unresolved. Corrected the source-lock quest-item names to the seven
  DMWeb relics: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza Armor,
  Soulcage and The Retaliator. Verification: `test_theron_v1_m11_direct_launch`
  passes; `git diff --check` passes.
# 2026-07-31 Theron real door-state query

- ✅ Removed the remaining party-level door-state placeholder from
  `theron_v1_get_move_result()`. Hypothetical movement now reads the matching
  level door object's actual state, just like the committed movement path;
  missing door objects remain blocked rather than inheriting fixture state.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
# 2026-07-31 Theron real item pickup state

- ✅ Replaced the `THERON_CMD_TAKE` success-without-state placeholder. Known
  Track 02-independent object classes (potion, scroll, food, key, weapon and
  armor) now bind to the source-locked compact item IDs, enter the active
  champion's inventory, mark the level object picked up and recalculate load.
  Unknown/quest object classes remain rejected rather than receiving guessed
  IDs until the real Track 02 object table is decoded. Verification:
  `test_theron_v1_m11_direct_launch` passes and `git diff --check` passes.

- ✅ 2026-07-31 CSB V2.2 live-cache cleanup: M11 no longer populates the
  retired 3x3 CSB V2.2 shape cache during either CSB viewport path. Its
  hard-coded material parameters had no authenticated `DUNVIEW.C F0128`
  command, palette, clip or Thing-chain receipt and no consumer in the
  admitted compositor. Live CSB pixels can therefore reach V2.2 only through
  the command-local source-material route, while unsupported families remain
  V1. Verified with `test_csb_v1_viewport_phase3_rendering` (2655/2655),
  `test_csb_v22_inplace_draw_pc34` (57/57), and
  `test_csb_v22_shapes_pc34` (54/54).
# 2026-07-31 Theron locked-door inventory gate

- ✅ Removed the locked-door auto-unlock placeholder. `theron_v1_door_open()`
  now requires the active champion to carry the source-locked key item before
  clearing a real door's locked flag; absent keys leave the door closed.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
- ✅ 2026-07-31 Nexus STABG indexed-blit gate: `nexus_ui_render_stabg()` now
  refuses to copy retail palette indices into a framebuffer unless the same
  surface carries its verified source palette. This closes the remaining
  public wrapper path for unpaletted/synthetic HUD pixels; Saturn VDP
  placement remains a separate no-draw gate. Verification: Nexus startup-media
  and FACE real-data tests pass against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-07-31 DM1 timeline-dispatch stability re-verification: the former
  F0242/F0248/F0190/F0249 assert-crash cluster is stable on current main.
  The seven documented CTests pass once and in ten consecutive repetitions
  each (70/70): square-state dispatch, three F0248 launchers, fake-wall
  group deferral, and both F0190 killed-all handoffs. This closes only the
  stale crash report, not the broader original-runtime or pixel-parity work.
# 2026-07-31 Theron party-gold save binding

- ✅ Replaced the save-header gold placeholder with an explicit
  `theron_v1_save_to_slot_with_gold()` API. The real party round-trip test now
  supplies `party.gold`, the save header persists it, and slot metadata reads
  it back as `party_gold`; the legacy API remains a documented no-gold wrapper
  for callers without party context. Verification:
  `test_theron_v1_save_progress_roundtrip_pc34` and
  `test_theron_v1_m11_direct_launch` pass; `git diff --check` passes.
- ✅ 2026-07-31 Nexus SAL/MAP status correction: the audio path is no longer
  an empty placeholder. It retains verified source identity, bounded MAP
  windows, and the SAL container profile, but still marks the codec and
  Saturn event dispatch as untested and blocks playback. Comments therefore
  use `opaque/no-playback` instead of the misleading `STUB` label.
- ✅ 2026-07-31 Nexus water/fire movement results: the standalone movement
  path now returns `BLOCKED_WATER` and `BLOCKED_FIRE` instead of incorrectly
  collapsing both to `BLOCKED_WALL`. Item/rune ownership remains in the
  mechanics source and is not enabled by this correction. Verification: C11
  movement check against `firestaff_nexus`.
- ✅ 2026-07-31 CSB boot materialization gate: `csb_v1_boot_enter_game()`
  now reaches `RUNTIME_READY` only after loading a ReDMCSB byte-map dungeon
  and decoding its initial party pose. Missing materialized data and the
  retired 16-bit parser fixture fail closed at `ASSETS_READY`, clear the
  dungeon singleton and cannot bind M11's HUD or viewport. Verified with
  `test_csb_v1_boot_viewport_render_gate`, `test_csb_v1_boot_profile_smoke`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus stairs/step link: unregistered stairs no longer reuse
  coordinates or imply an adjacent level. `nexus_stairs_resolve()` returns
  explicit unresolved sentinels until a source-bound link is registered;
  registered links are unchanged. Verification: C11 checks for both
  unresolved and registered links.
- ✅ 2026-07-31 Nexus teleporter owner gate: mechanics now checks the
  teleporter link before mutating party position. An unregistered
  TELEPORT/TELEPORT2/TELEPORT3 blocks without movement; a registered link is
  dispatched unchanged. Verification: `test_nexus_v1_pit_teleporter_runtime`
  passes 44/44.

- ✅ 2026-07-31 CSB direct-loop source handoff: `fs_game_init()` now rejects
  absent or unmaterialized CSB media, just like the boot/M11 route, and
  `fs_game_load_assets()` consumes the boot-owned dungeon and party pose.
  The generic DM1 parser can no longer supply its fixed `(11,29)` start point
  to a CSB session. Verification: direct launch against
  `/Users/bosse/.firestaff/data/csb`, `test_csb_v1_boot_viewport_render_gate`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus HUD gold: M11 now sends the actual mechanics-state
  `gold_pieces` value to the HUD instead of always supplying synthetic zero.
  The field is updated by the source-bound gold-pile pickup path; fallback to
  zero is used only when the mechanics pointer is absent. Verification: full
  `firestaff` build and `test_nexus_v1_dgn_runtime_materialization`.
- ✅ 2026-07-31 Nexus HUD startup gate: launcher startup and save/resume
  production paths no longer use `force_active_for_test(1)` for the HUD.
  HUD rendering therefore requires the normal V2 presentation gate; test mode
  remains only for explicit integration tests. Verification:
  `test_nexus_v2_hud_runtime_integration` passes 9/9, and the full `firestaff`
  build passes.

- ✅ 2026-07-31 CSB runtime boot materialization gate:
  `csb_v1_runtime_boot()` no longer reports success with absent graphics, an
  unreadable/legacy dungeon or no decoded initial party pose. A failed retry
  clears the prior dungeon singleton and source paths before it returns.
  Verification: `test_csb_v1_boot_runtime_handoff`, including its missing
  source-media regression, plus boot-profile and viewport gate tests.
- ✅ 2026-07-31 Theron startup: the boot scanner now recognizes the actual raw
  Track 02 filenames `TQJP02.bin` and `TQUS02.bin` used in
  `~/.firestaff/data/theron`. They are hash-verified through the same existing
  directory gate; no new data files or fallback values are added.
- ✅ 2026-07-31 Nexus V2 production gate: launcher startup and save/resume no
  longer bypass the presentation gates for lighting, smooth movement, or touch
  runtime with test-only `force_active_for_test(1)` calls. V2 probes continue
  to activate the mode explicitly. Verification: `firestaff` build,
  smooth-movement probe 33/33, and touch-runtime probe 57/57.
- ✅ 2026-07-31 Nexus audio diagnostics: remaining `(stub)` labels for CDDA
  stop/pause/resume/fade are replaced with `opaque/no-playback`. Real SAL/MAP
  and CD tracks remain source-bound, but codec/driver and playback are still
  marked blocked. Verification: `test_nexus_v1_sound_runtime_receipt` passes.
- ✅ 2026-07-31 Theron startup seed: the startup receipt now connects the boot
  summary's dungeon seed to the verified initial Track 02 level header
  (`0x0108e938`) instead of leaving the no-header value at `0`. The real-asset
  probe verifies the seed, roster, and startup handoff.
- ✅ 2026-07-31 CSB graphics filename-fallback removal: runtime graphics
  discovery now requires a known CSB graphics MD5 for every version hint,
  including unknown/custom hints. A random `GRAPHICS.DAT`, `CSB.DAT` or
  `CSBGRAPH.DAT` can no longer become a live graphics binding merely because
  of its filename. The regression covers both selected and unknown hints;
  renamed authentic media remains discoverable through recursive hash search.
- ✅ 2026-07-31 CSB undefined monster-projectile gate: Grey Lord/Lord Order's
  documented ReDMCSB `GROUP.C` BUG0_13 path, and a missing RNG context, no
  longer create a synthetic Fireball. They return no source projectile, which
  the live runtime rejects before projectile creation. Normal authenticated
  creature attacks keep their original projectile selection.
- ✅ 2026-07-31 Nexus FONT256 DMWeb regions: the real S2D decoder now exposes
  named, bounds-verified byte windows for Map, Page/tilemap, Character
  Generator, Palette, and Attributes according to DMWeb's `DecodeFONT256S2D`.
  A retail check against `FONT256.S2D` verifies all five offset/size pairs;
  no glyph or menu semantics are claimed yet. Verification:
  `test_nexus_v1_font_s2d` passes.
- ✅ 2026-07-31 Nexus FONT256 Character Generator: a bounded API now copies
  DMWeb's 242 authentic 8x8/8-bit tiles from the CG region after its 16-byte
  prefix and rejects index/file-boundary overflow. Tile indices remain
  explicitly separate from glyph/menu semantics. Verification:
  `test_nexus_v1_font_s2d` passes against the local retail file.
- ✅ 2026-07-31 CSB M11 media-rehash gate: the M11 entry boundary now hashes
  the selected `GRAPHICS.DAT` and `DUNGEON.DAT` again and requires exact
  agreement with the boot profile's scanned receipt before any CSB pixels can
  be decoded. A file replaced after scan fails closed instead of inheriting a
  stale verified flag; the focused boot-profile test covers this regression.
- ✅ 2026-07-31 Nexus FONT256 Page/palette words: bounded APIs now read
  DMWeb's 4,096 big-endian Page/tilemap words and 256 big-endian BGR555 palette
  words from the real regions. The retail test verifies tilemap word 1 is
  `0x0002`, palette word 0 is `0x8000`, and index bounds; no glyph or menu
  meaning is inferred yet. Verification: `test_nexus_v1_font_s2d`.
- ✅ 2026-07-31 Nexus FONT256 attributes: a bounded API for the 242 authentic
  big-endian attribute words has been added from DMWeb's Attributes region.
  Tile attributes remain separate from the still-unproven glyph and menu
  semantics. Verification: `test_nexus_v1_font_s2d` passes against the retail
  file.
- ✅ 2026-07-31 Nexus HUD no-fake gate: the live DGN path no longer hardcodes
  V2 presentation flags to enable the procedural HUD. Without an authenticated
  retail widget/VDP placement receipt, the overlay remains closed; explicit V2
  integration tests can still enable it. Verification: full `firestaff`
  build, HUD 9/9, and DGN materialization test.
- ✅ 2026-07-31 CSB dead state-shim removal: deleted the unbuilt
  `csb_v1_game` skeleton, which exposed fixed `(5,5)`/`(0,0)` positions and
  marked DM1 import complete without loading anything. CSB now has only the
  verified `CSB_V1_RuntimeProfile`/dungeon/Utility ownership documented by
  the integration and source-lock references; no production caller used the
  retired API.
- ✅ 2026-07-31 Nexus viewport animated-material gate: Structure3 materials
  with `0x08xx` retain retail descriptor provenance but no longer use the
  first Structure2 image as an unproven static-frame substitute. The pixel
  route remains no-draw until the Saturn frame selector/VDP1 binding is
  verified. Verification: `test_nexus_v1_dgn_runtime_materialization`; the
  source-receipt test correctly skips without a staged Nexus directory.
- ✅ 2026-07-31 CSB Utility metadata-party removal: `get_party()` no longer
  reconstructs champion count, leader, and import provenance from free
  `reserved[]` metadata when the imported champion body is missing. The
  runtime receives only the full validated Utility party; the regression
  proves stale metadata cannot manufacture a launchable party.
- ✅ 2026-07-31 CSB file-dungeon fixture closure:
  `csb_v1_dungeon_load_from_file()` now rejects the retired 16-bit
  column-major fixture layout after parsing, clears its temporary ownership,
  and publishes only ReDMCSB-compatible one-byte square maps from a path.
  The explicit fixture regression proves the file boundary fails closed.
- ✅ 2026-07-31 Nexus MENU.BPK PRS3 source-lock correction: the runtime decoder is now documented against DMWeb `DMNDataFileDecoder.vbs::DecodePRS3`, including its LSB-first control bytes, literal/back-reference commands, 12-bit window, and `+18`/negative-window rule. The real local `MENU.BPK` corpus decodes all 162 PRS3 surfaces with zero failures. Remaining MENU work is pixel-mode/palette interpretation and authenticated Saturn VDP1 placement, not an undocumented compression algorithm.
# ✅ 2026-07-31 — Theron palette admission is source-gated

- Removed the synthetic default stone-gradient palette from the V1 palette state.
- An unbound palette now remains empty, so HUD/viewport code cannot receive manufactured colors before verified Track 02 data is loaded.
- Updated the rendering test to assert the fail-closed palette contract; focused suite passes 25/25.
# ✅ 2026-07-31 — Theron V2 HUD production path is asset-gated

- The boot/runtime path no longer draws the procedural V2 HUD overlay when the HUD widget manifest is missing, partial, or placeholder-only.
- Rendering now requires a complete manifest with real assets for every HUD slot; the local Track 02 BINs remain correctly limited to verified startup surfaces.
- Verification: `test_theron_rendering` 25/25 and `test_theron_v2_hud_overlay_pc34` 58/58.
# ✅ 2026-07-31 — Theron V1 chrome helpers fail closed

- Direct topbar, right-panel, and champion-slot helpers no longer emit procedural blocks, icons, or name bars without a verified runtime chrome bank.
- This closes the legacy low-level path as well as the master HUD compositor; the generic bar primitive remains available for source-backed callers.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup fallback no longer invents unknown seeds

- The legacy bounded fallback-room receipt now reports seed `0` when dungeon metadata is not verified instead of carrying the retired literal seed `313`.
- Verified Track 02 startup remains authoritative; this change only removes misleading metadata from the compatibility fixture path.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup no longer paints no-data placeholders

- Removed the production branch that enabled command-drawn synthetic title, stage, Soul Room, and forcefield graphics when Track 02 was absent.
- Startup now reports `NO VERIFIED TRACK02 GRAPHICS` and remains blocked until the real atlas route is present.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron V2.2 missing-shape API fails closed

- Removed the runtime checkerboard placeholder contract from `theron_v22_get_missing_placeholder()`; missing modern assets now return `NULL` with 0×0 dimensions.
- Updated the public contract and regression test. No production caller can receive invented missing-texture pixels.
- Verification: `test_theron_v22_modern_assets_pc34` 32 checks, 0 failures.
# ✅ 2026-07-31 — Theron boot scanner rejects unverified legacy files

- Removed the `GRAPHICS.DAT`/`DUNGEON.DAT` fallback search from the Theron boot scanner.
- Theron launch discovery now accepts only the hash-verified Track 02 media routes present in the real data corpus; unverified extracted files cannot become a launch source.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron legacy enter-game stub fails closed

- `theron_v1_boot_enter_game()` no longer reports success while leaving `theron_state` and `dungeon_data` unbound.
- The real Track 02 runtime handoff remains the only valid game-state transition.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron Track 02 bad-input routes deny fallback visuals

- Track 02 startup/object/level route receipt initializers now default `fallback_visuals_allowed` to `0` for unknown or malformed input.
- A caller must receive explicit verified route evidence before any visual permission can exist; bad input cannot grant placeholder rendering.
- Verification: `test_theron_rendering` 25/25.

- ✅ 2026-07-31 CSB viewport contract isolation: three more contract-only
  CustomBackgrounds modules (D1L/D1R first backdrop, floor/ceiling mask
  ordering and room-pass ordering) now compile exclusively into their focused
  tests, not `firestaff_m10`. Live viewport code retains only the source-bound
  room-slot/material path. Verification: focused regressions (74 + 563 + 86
  assertions) and complete `firestaff` link.

- ✅ 2026-07-31 CSBWin save-fixture isolation: the synthetic 14-shape
  CSBWin/DM1 save corpus and its convenience runner were removed from the M10
  loader-boundary module and public production header. They are now test-only
  support for the focused regression, boot-handoff regression and skip-safe
  verification probe; the runtime boundary accepts only caller-supplied save
  bytes. Verification: loader-boundary test 158/158, boot handoff 504/504,
  real staged-save probe 22/22 and complete `firestaff` link.
- ✅ 2026-07-31 DM1 HoC F0172 ornament correction: removed the
  Firestaff-only map-zero random-floor-ornament suppression. ReDMCSB
  `DUNGEON.C F0172` applies this path to every corridor map, and sensors then
  override its ordinal. The regression covers both a map-zero sensor ornament
  and a deterministic map-zero random ornament. Verification:
  `test_m11_overlay_command_queue_block` (192/192) and
  `test_m11_v22_shape_cache_pc34` (31/31).

- ✅ 2026-07-31 DM1 HoC F0172 sensor-zero correction: floor sensors now
  overwrite the random floor-ornament ordinal even when their source-owned
  `Remote.OrnamentOrdinal` is zero. ReDMCSB assigns that field
  unconditionally; zero suppresses a random grate or pressure plate instead
  of allowing it to leak through. Verification:
  `test_m11_overlay_command_queue_block` (193/193),
  `test_m11_v22_shape_cache_pc34` (31/31), and the installed PC 3.4 HoC
  runtime probe.

- ✅ 2026-07-31 DM1 F0115 alcove-object input binding: C080 now accepts the
  actual current-frame C2548/F0791 destination rectangle for a front alcove
  item, in addition to the original C05 ornament zone. This preserves wall
  sensor input while making a real rendered torch/object pickable. Verification:
  `test_m11_dm1_real_alcove_item_runtime_pc34` finds map 1 `(6,3,2)` in the
  installed PC34 corpus and successfully transfers the rendered object into
  the leader hand.

- ✅ 2026-07-31 DM1 F0115/C080 rendered floor-pile input: normal DM1 no
  longer uses four fixed, approximate floor-item click panes. Each successful
  PC34 F0115/F0791 object blit now publishes its exact final rectangle,
  source `THING`, and map square for the current frame; C080 takes the
  topmost clicked rendered object directly into the leader hand. Missing or
  occluded source material therefore cannot select an arbitrary neighbour
  from the thing chain. Verification: the real PC34 non-HoC F0115 runtime
  test clicks the returned material rectangle and confirms that a leader-hand
  object is produced; `test_m11_overlay_command_queue_block` remains 193/193.
- ✅ 2026-07-31 Nexus ITEM.IBS/viewport source chain recheck: the focused
  Structure1F provenance and spatial receipts, all 16-level retail DGN
  face/material admission, and runtime materialization pass against the real
  European corpus. ITEM.IBS 4bpp/palette ownership remains source-bound and
  no-draw; the only remaining viewport gate is authentic Saturn VDP1 capture.
- ✅ 2026-07-31 Nexus MENU.BPK palette boundary: DMWeb's 256-entry
  big-endian PALT trailer is now revalidated from the real `MENU.BPK`.
  Structure2 ABI, intake and PRS3/VDP1 consumer-evidence tests all pass;
  palette bytes remain source-bound but are not promoted to visible menu
  pixels until an authentic Saturn consumer trace is available.

- ✅ 2026-07-31 CSB viewport contract isolation: the unbound D0L2/D0R2
  F0111 partly-open-door and D1L/D1R F0108 floor/ceiling-ornament contract
  modules now compile exclusively into their focused tests, not `firestaff_m10`.
  They contain no authenticated bitmap decoder or runtime consumer, so keeping
  them out of M10 prevents their source-locked metadata from masquerading as a
  draw path. Verification: both focused tests and full `firestaff` link.
- ✅ 2026-07-31 Theron startup fallback boundary: confirmed M11 has no caller
  for the legacy synthetic-room API and uses only
  `theron_v1_startup_runtime_load_initial_level_verified_only()`. The helper
  and legacy loader are now explicitly documented as data-free fixture
  compatibility only; verified Track 02 with no semantic handoff remains
  blocked. Startup-flow `653/653` and rendering `25/25` remain green.
- ✅ 2026-07-31 Nexus startup/menu/HUD audit: real `TITLE.CG`, warning/gameover
  media, champion startup menu, `FONT256.S2D`, MENU.BPK no-draw handoff and
  the V2 HUD gate all pass their focused tests. The HUD integration's 9/9
  render assertions are test-only; production keeps the procedural overlay
  closed until a retail widget/VDP placement receipt exists.
- ✅ 2026-07-31 Nexus HUD provenance correction: removed the false claim that
  the procedural V2 overlay was sourced from retail `NEXUS.BIN`. The supplied
  corpus has no authenticated HUD widget surface; the module is explicitly
  diagnostic/test-only and production remains gated. HUD overlay 46/46,
  runtime integration 9/9 and `firestaff_m11` build pass.
- ✅ 2026-07-31 Nexus V2 provenance audit: corrected remaining lighting, touch,
  smooth-movement, phase-gate and title comments so absent `NEXUS.BIN` data is
  recorded as unavailable rather than presented as a retail source. ReDMCSB,
  DMDF/DGN and existing behavioral references remain cited; all production V2
  gates stay closed. Focused lighting 79/79, phase gate 240/240, smooth
  movement 27/27 and touch affordance 0 failures pass.
- ✅ 2026-07-31 Nexus launcher card audit: the modern M12 card renderer no
  longer permits any generated game-card motif branch to paint the Nexus card,
  even if a layout slot index is reused. Nexus startup/menu art therefore stays
  source-bound/no-draw until real Saturn placement is admitted; other game-card
  routes are unchanged. `firestaff_m11` rebuild passes.
- ✅ 2026-07-31 Nexus launcher status audit: removed the hardcoded `AVAILABLE`
  label from the legacy M12 card path. Nexus now reports readiness only from
  the verified asset-version match, like the other games; `firestaff_m12`
  rebuild and diff check pass.
- ✅ 2026-07-31 Nexus real FONT256 handoff: fixed the inverted
  `nexus_v1_font_s2d_decode()` success check in engine init. The supplied
  `FONT256.S2D` now reaches the engine's source-admitted state; the separate
  page-to-character glyph-render gate remains closed, so no guessed glyphs are
  emitted. Real Track 1 capture readiness passes 29/29, FONT256 decoder and
  startup-menu tests pass.
- ✅ 2026-07-31 Theron production combat boundary: removed the inferred
  creature/combat template table from `firestaff_theron`. Production now
  links explicit fail-closed symbols from
  `theron_v1_combat_runtime_noop.c`; the full inferred implementation is
  available only to the dedicated combat fixture target. Rendering `25/25`
  and startup-flow `653/653` remain green.
- ✅ 2026-07-31 Theron door regression: updated the combat fixture to place
  an authentic `THERON_ITEM_KEY` before attempting to open a locked door. The
  test now follows the source-bound key gate and passes 66/66.
- ✅ 2026-07-31 Theron shop-data boundary: removed the fixture-driven,
  source-unverified shop price-table helper from the production archive.
  Its focused test and purchase-gate probe still compile it explicitly;
  production cannot expose inferred shop prices or item ranges.
- ✅ 2026-07-31 Theron V2.2 viewport boundary: removed the placeholder
  3×3 cell-rectangle cache from the production Theron archive. Focused V2.2
  tests may still compile it explicitly, but live rendering cannot consume
  guessed viewport coordinates.
- ✅ 2026-07-31 Theron V2.2 material boundary: removed the inferred modern
  shape/material book from the production archive and replaced its init seam
  with an explicit blocked route. Focused V2.2 fixture targets retain the
  original shape implementation; live production cannot promote its guessed
  tints or geometry.
# Isolated the inferred Theron V2 HUD widget manifest/parser from production and added a no-op gate seam; procedural HUD pixels can no longer render in the verified runtime without a complete real asset manifest.

- ✅ 2026-07-31 Nexus champion provenance audit: the earlier 24-entry table
  was confirmed as fixture data and removed from the live path. The real
  `RLOWFIX.BIN`/`PLRD` handoff is recorded below; the 24-entry array remains
  storage capacity only.

- ✅ 2026-07-31 Nexus PLRD champion handoff: DMWeb's real
  `RLOWFIX.BIN` `RES*`/`PLRD` structure is now parsed in production. The
  European corpus supplies 20 records with Japanese `TABL`-decoded labels,
  HP/stamina/mana, attributes, levels, and source ordinals; the 24-element
  array remains storage capacity only. `test_nexus_v1_champion_plrd` passes
  against the local real file, and malformed/missing PLRD input fails closed.
- ✅ 2026-07-31 Nexus ITEM.IBS ordinal handoff: the source-owned category and
  weight bytes for all 243 real ITEM.IBS declarations now form the live item
  lookup boundary. PLRD equipment/backpack ordinals retain real declaration
  identity without reviving the old DM1 catalog; names, attack/defense and
  key/action semantics remain explicitly unavailable.
- ✅ 2026-07-31 Theron V1 UI chrome isolation: removed the inferred bars,
  labels and champion-slot pixels from the production archive. The public
  chrome API now fails closed through a no-op seam until the original Track
  02 UI bank is decoded; the old implementation remains fixture-only.
- ✅ 2026-07-31 Theron viewport admission wording: corrected the lifecycle
  and source comments to describe the palette as unbound, and removed the
  stale claim that facing could come from a world-tick surrogate. The
  viewport continues to accept only the authenticated party pose and blocks
  pixels until a source tile bank is bound.

- ✅ 2026-07-31 Theron tile-renderer isolation: removed the inferred
  square/depth tile table and rasterizer from the production archive. The
  diagnostic tile-renderer probe still compiles the implementation explicitly;
  production now returns no tile and preserves the framebuffer until a real
  Track 02 tile-bank handoff exists.

- ✅ 2026-07-31 Theron V2.2 local-art isolation: removed the modern-art
  manifest/cache and inplace rectangle renderer from the production archive.
  Their focused V2.2 tests retain explicit source compilation, but `firestaff`
  cannot promote local cache/manifest pixels into the runtime.

- ✅ 2026-07-31 Theron viewport mapping gate: blocked the duplicate viewport
  tile table even when a caller supplies an unverified atlas. The legacy
  fixture renderer is compiled explicitly by the rendering test; production
  now requires a decoded Track 02 square/depth/material mapping.

- ✅ 2026-07-31 Theron placeholder inventory: audited the champion-state
  initializer and recorded its default names/classes/stats as an explicit
  unresolved real-data gap. Existing save/fixture tests still depend on it;
  no production claim now treats those defaults as decoded Track 02 records.

- ✅ 2026-07-31 CSB SWSH F0904 receipt isolation: the palette-animation
  receipt accepts metadata only and has no runtime caller or SWSH command
  decoder. It now compiles only into its focused test, rather than M10;
  production cannot turn receipt facts into synthetic palette animation.
- ✅ 2026-07-31 Theron verified champion handoff: authenticated JP/US Track
  02 startup sessions now clear fixture-only 10-point stats, inventory and
  equipment defaults before runtime entry. Source-roster identity metadata is
  retained; undecoded numeric champion records fail closed instead of being
  presented as real data.

- ✅ 2026-07-31 CSB SWSH F0908/F0909/F0910 receipt isolation: the metadata
  chain for sound init, playback and release has no production caller. M11
  keeps using the real-byte `RedmcsbF0908_InitSoundPc34` path, while the
  receipt chain compiles solely into its focused test and cannot authenticate
  host audio as original SWSH data.

- ✅ 2026-07-31 CSB startup receipt isolation: F0436 palette fade, F0579
  entrance bitplanes and F0807 door-step helpers are metadata contracts with
  no product caller or original-pixel decoder. They now compile only into
  their focused tests; M10 cannot treat caller facts as title or entrance
  material. Live startup remains guarded by the authenticated runtime route.

- ✅ 2026-07-31 CSB F0797 entrance-layout receipt isolation: the 5×5
  micro-dungeon layout metadata had no product caller and now compiles only
  into its focused test. It cannot become a generic loaded-dungeon or viewport
  substitute; an actual entrance frame must still use its source-owned draw
  route and verified graphics material.

- ✅ 2026-07-31 Theron startup-receipt isolation: removed the explicit
  no-data placeholder receipt implementation from the production archive.
  The real-asset receipt probe and save/resume fixture compile it explicitly;
  `firestaff` cannot link placeholder startup labels or tokens.
- ✅ 2026-07-31 CSB F0440/F0902 startup receipt isolation: temporary-graphic
  byte-count and FTL-logo fact helpers have no runtime caller or decoder and
  now compile only into their focused tests. M10 can no longer substitute
  caller metadata for a verified decompressed member, logo bitmap or palette.

- ✅ 2026-07-31 CSB startup-boundary/ownership isolation: the F0474–F0490
  blocked-graphics receipt and F0886–F0905 ownership table have no runtime
  consumer and now compile only into their focused tests. Production continues
  through the verified archive/decoder path rather than treating a blocked
  receipt or an ownership string as graphics material.

- ✅ 2026-07-31 Theron runtime fallback isolation: the startup runtime no
  longer synthesizes a fallback room in the production build. That branch is
  compile-defined only for the startup-flow fixture probe; production remains
  unavailable until a decoded Track 02 level is bound.

- ✅ 2026-07-31 CSB F0906–F0925 primitive-inventory isolation: the raw
  function-number metadata table only reports dependencies and explicitly
  blocks execution. It now compiles solely into its inventory test, leaving
  M10 to the dedicated authenticated SWSH and Utility implementations.

- ✅ 2026-07-31 Theron legacy asset verification: the generic loader no longer
  reports success for an expected digest it cannot compare against an
  authoritative catalog. Hash-bound Track 02 boot remains the only admission
  route; the legacy API now fails with `TR_ASSET_ERR_HASH`.

- ✅ 2026-07-31 Theron chapter-marker gate: a verified media identity without
  decoded progression/save state now reports unavailable instead of fabricating
  Chapter 1 and `0/7` quest progress. Later dungeon hints remain unavailable
  until their real headers/names are bound; fixture-only profile projection is
  explicitly compile-scoped.

- ✅ 2026-07-31 CSB F0846–F0865 unmapped-boundary isolation: this range has
  no ReDMCSB callable and only reports a fail-closed admission receipt. It
  now compiles solely into its focused contract test, so M10 cannot mistake
  source-absence metadata for an executable runtime implementation.

- ✅ 2026-07-31 CSB F0986–F1005 graphics-boundary isolation: the function
  table documents local, foreign-platform and unbound helpers, then blocks
  every runtime route. With no product caller or decoder, it now compiles only
  into its contract test; live rendering continues through authenticated PC
  3.4 graphics material.

- ✅ 2026-07-31 CSB F1006–F1025 source-boundary isolation: this table only
  inventories local, existing-owner and foreign-platform symbols and blocks
  execution for all of them. It now compiles solely into its focused contract
  test; M10 retains only actual authenticated CSB consumers.

- ✅ 2026-07-31 CSB platform-helper isolation: the combined F1048/F1049/
  F1053/F1055/F1061 wrapper only exported a disabled alias and explicit
  Amiga fake-code no-ops, with no production caller. It is excluded from M10;
  source-faithful shared fail-closed boundaries remain available for their
  separate focused tests.

- ✅ 2026-07-31 CSB F1066–F1085 Amiga-boundary isolation: the table has no
  PC 3.4 product consumer and explicitly blocks every route. It now compiles
  only into its contract test; the separately owned, source-faithful Intuition
  vector boundary remains independent of this inventory.

- ✅ 2026-07-31 Theron champion handoff hardening: verified Track 02 runtime
  entry now clears fixture champion names, portraits, classes and party count
  in addition to default stats/inventory. Production cannot present the
  inferred roster until original champion records are decoded.

- ✅ 2026-07-31 CSB F1126–F1145 source-boundary isolation: this catalog only
  records local, foreign-platform and unbound symbols before failing closed.
  It now compiles solely into its contract test, so M10 cannot treat source
  labels as a substitute for an authenticated CSB input or graphics route.

- ✅ 2026-07-31 Theron SRM champion-name gate: real SRM body import no longer
  substitutes `Theron` or `Companion` when a champion name field is empty. The
  record is rejected until source name bytes are present.

- ✅ 2026-07-31 CSB F1186–F1205 ANIM-boundary isolation: the table is a
  DM1-owned ANIM inventory without an authenticated CSB stream or runtime
  consumer, and already blocks execution. It now compiles only into its
  contract test, preventing metadata from creating CSB UI or timing behavior.

- ✅ 2026-07-31 Theron SRM progression-only handoff: Continue now clears the
  fixture world party when an SRM contains progression but no champion body.
  It no longer invents a one-member Theron party from unrelated initialized
  state.

- ✅ 2026-07-31 CSB F1206–F1225 ownership isolation: the table only records
  ANIM platform/local status and admits no route. It now compiles solely into
  its contract test, keeping metadata from standing in for CSB palette, sound
  or allocation behavior.

- ✅ 2026-07-31 CSB F1406–F1445 unmapped-boundary isolation: ReDMCSB has no
  callable symbol in this range, and the table only reports a blocked receipt.
  It now compiles only into its contract test; local source labels cannot
  become a synthetic CSB entrance, startup or graphics implementation.

- ✅ 2026-07-31 Theron runtime-render asset gate: the frame facade now requires
  a non-NULL asset bundle and fails before viewport/UI presentation otherwise.
  Rendering remains source-admitted only; the focused rendering suite passes
  `25/25`.

- ✅ 2026-07-31 Theron startup receipt fixture isolation: verified Track 02
  receipts no longer copy the fixture mirror roster size or fallback-label
  count. Those values remain confined to the explicit no-data fixture receipt;
  real startup data cannot report synthetic roster metadata.

- ✅ 2026-07-31 Theron startup runtime test linkage: the save/resume contract
  target now compiles its fixture-only structured fallback entry explicitly,
  while production still links the no-fallback runtime archive. The focused
  suite is green at `325/325`.

- ✅ 2026-07-31 Theron startup menu metadata gate: absent decoded Track 02
  roster names no longer expose fixture portrait indices or classes in menu
  elements. The startup-flow probe remains green at `653/653`.

- ✅ 2026-07-31 Theron startup TODO audit: removed the stale claim that the
  structured save/resume receipt test had an unrelated failure. The corrected
  fixture-scoped linkage now passes `325/325`; HUD rendering remains blocked
  until a real Track 02 widget bank is decoded.

- ✅ 2026-07-31 Theron champion handoff fixture isolation: the production
  `enter_forcefield_with_roster` path no longer calls `theron_v1_party_init()`
  or inherits its synthetic stats, classes, and portraits. It admits only
  source roster names; the full mirror-table initializer is fixture-scoped.
  Startup flow remains `653/653`, save/resume `325/325`.

- ✅ 2026-07-31 Theron viewport tile-helper gate: production
  `theron_vp_tile_for_square()` now returns no tile until a real Track 02
  mapping is bound. The inferred table is compiled only into the explicit
  viewport fixture probe; verification passes `50/50` and rendering `25/25`.

- ✅ 2026-07-31 Theron menu portrait/class gate: decoded roster names no
  longer authorize inferred mirror-table portrait indices or classes in
  production. Those fields remain unavailable until their source records are
  decoded; fixture metadata is compile-scoped to the startup probe.

- ✅ 2026-07-31 Theron legacy asset no-data gate: `tr_asset_load()` no longer
  returns success or claims “using defaults” when the requested file is
  missing. It returns `TR_ASSET_ERR_NO_DATA`; rendering remains source-gated.
  Focused rendering passes `25/25`, startup/save-resume `325/325`.

- ✅ 2026-07-31 Theron legacy parse-error gate: discovered Track 03/04 data
  that fails its parser now returns `TR_ASSET_ERR_TR03`/`TR_ASSET_ERR_TR04`
  and releases the partially loaded bundle instead of reporting a successful
  asset load with fallback state.

- ✅ 2026-07-31 Theron runtime world-init gate: production boot and Track 02
  runtime inspection now use a zero-party world initializer. The legacy
  fixture initializer remains available to tests, but no default champion
  roster exists before verified source handoff.

- ✅ 2026-07-31 Theron level-header seed binding: `theron_v1_level_load()` now
  retains the authenticated Track 02 header seed in `Theron_V1_Level` instead
  of discarding it. No tile/object meaning is inferred from the seed; the
  viewport mapping gate remains closed.

- ✅ 2026-07-31 Theron seed regression proof: the real Track 02 level-handoff
  probe now asserts the retained `0x0108e938` seed directly on the loaded
  level, alongside the existing raw candidate checks.

- ✅ 2026-07-31 Theron opaque header-index binding: level load now preserves
  the Track 02 header's `0x0026` level-index value in a separate opaque field,
  without confusing it with Firestaff's internal 0-based level slot. The real
  handoff probe asserts it; result remains `fail=0` with one known ISO skip.

- ✅ 2026-07-31 Theron level fixture parity: the explicit no-data room helpers
  now populate the same seed/header-index fields as their serialized headers,
  keeping fixture inspection structurally honest without promoting fixture
  bytes into production semantics. Startup flow remains `653/653`.

- ✅ 2026-07-31 Nexus TEXT/TABL source-boundary cleanup: RLOWFIX.BIN TEXT
  offsets and the 216-entry DMWeb TABL code table are parsed from the real
  retail resource and exercised by `test_nexus_v1_champion_plrd`. The legacy
  heuristic ASCII/Shift-JIS scraper plus unauthenticated S2D text/glyph
  layout wrappers are excluded from `firestaff_nexus`; they remain available
  only to explicit diagnostic probes. No glyph, palette, menu, HUD or Saturn
  VDP1/VDP2 presentation is promoted by this change.
- ✅ 2026-07-31 DM2 SHOP_GLASS panel isolation: removed the remaining
  host-authored shop rectangle, English labels and empty-inventory fallback
  from the production shop module. Its render contract now clears the output
  and returns no-draw until the source-owned `WALL_GFX`/DB actuator chain is
  decoded. Verification: production link, shop admission regression and an
  executable-string check for the retired panel text.
- ✅ 2026-07-31 DM2 world/object fallback isolation: removed the inferred
  16-bit world builder and sequential thing-pool parser from the live path.
  `dm2_world_from_mem()` now requires the PC G1 byte-square loader, and the
  object model returns no records when the validated c_record chain is not
  available. Verification: complete production `firestaff` link and no
  compiler warnings in either changed DM2 source.
- ✅ 2026-07-31 DM2 V2 runtime/lighting isolation: removed the unattached
  smooth-camera, bloom and animated outdoor-state sources from the production
  archive and game loop. These local time/weather effects remain in explicit
  diagnostic targets only; live DM2 presentation stays on the authenticated
  V1 viewport and GDAT HUD path. Verification: production link, V2 probes,
  real-data DM2 startup gate and production-symbol check.
- ✅ 2026-07-31 Nexus real viewport gate rechecked: the Track 1 readiness
  probe drives the local English CUE/DM.BIN, real `LEV00.DGN`, `FONT256.S2D`
  and `SCORPION.MNS` handoff through `nexus_viewport_render`; 29/29 pass.
  The real viewport capture remains deterministic black until authenticated
  Saturn DGN/VDP1 material is admitted, with no procedural fallback pixels.

- ✅ 2026-07-31 CSB V2.2 synthetic shape-book isolation: removed the
  hand-authored material/PBR/geometry book from `firestaff_csb_v2`; its
  historical expectations remain explicitly test/probe scoped. Production now
  links `csb_v22_shapes_runtime_gate.c`, whose API reports zero materials and
  no shape parameters until a reviewed original-data binding exists. The
  runtime cache requires a non-NULL admitted material before activating a V2.2
  cell, so it retains source-owned V1/V2.1 pixels rather than inventing a
  fallback. Verified with the new `csb_v22_shapes_runtime_gate_pc34` test,
  the historical shape-book contract test, and a `firestaff` build.

- ✅ 2026-07-31 DM2 V2 companion/crafting/viewport isolation: removed the
  orphaned companion display, empty crafting catalog and host-timed smooth
  viewport helpers from production M10/V2 archives. The focused startup
  diagnostic retains its local copy, while the game executable contains no
  V2 companion, crafting or smooth-viewport symbols. Verification: complete
  production link, real-data DM2 startup gate and archive/executable-symbol
  checks.
- ✅ 2026-07-31 CSB V2.2 installed-state hardening: a launcher-set
  `installed` flag can no longer select modern art on its own. The V2.2
  source selector now rechecks the finished-art gate and every route's
  provenance before it returns `V2_MODERN`; otherwise it keeps the V2.1/V2.0
  fallback. The focused asset-pipeline test covers the forged-installed/no-art
  case.

- ✅ 2026-07-31 CSB V2.2 cache-admission hardening: a readable
  `v22_inplace_cache.bin` is no longer enough to overwrite an F0128 source
  command. The in-place blitter independently requires the finished-art
  material/provenance gate; fixture cache pixels remain invisible even with a
  matching source span and palette. The focused in-place test verifies the
  framebuffer stays source-owned.

- ✅ 2026-07-31 DM2 V2 HUD overlay-state isolation: removed the retired
  procedural overlay module from the production V2 archive. Its invented
  compass, gold, level and champion values no longer enter the live renderer;
  the GDAT HUD route retains only a visibility gate and can draw only
  authenticated `INTERFACE_GENERAL` records. Historical overlay code remains
  explicitly test-scoped. Verification: production link, 74/74 direct-overlay
  regression, real-data DM2 M11 startup gate and archive/executable symbols.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build, startup-flow probe and
  real-data startup receipt gate.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build and startup-flow plus
  save/resume probes.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 DM2 champion-stat bridge isolation: removed the unattached
  generic V1-to-V2 champion percentage bridge from the production V1 archive.
  It had no M11 consumer or authenticated session/palette handoff. Its focused
  regression remains explicit; live HUD stays on the source-owned GDAT route.
  Verification: production link, champion-bridge regression, real-data M11
  startup gate and archive/executable-symbol checks.
- ✅ 2026-07-31 DM1 original TITLE verification: repaired the standalone
  TITLE probe launcher after the source tree moved. The installed hash-locked
  PC 3.4 `TITLE` (12,002 bytes) now passes all 59 Greatstone mapfile-record,
  53-frame and two-palette-phase checks. The runtime TITLE palette and
  SWSH-to-C001 handoff probes also pass against the installed original
  `GRAPHICS.DAT`; no replacement title frame is used by these checks.
# ✅ 2026-07-15 Theron Track 02 transfer-destination call-entry receipt

The original Mednafen trace now admits the Track 02-derived TII destination
only when its bound JSR reaches an exact main-RAM entry row. The nested receipt
retains original byte-range and call provenance without classifying code or
data. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, raw-loader probe skip-safe,
and the capture contract pass.

# ✅ 2026-07-15 Theron Track 02 destination copied-byte receipt

The entered routine at the Track 02-derived TII destination now requires its
observed opcode to equal the exact first source byte copied from `$3c88`.
The receipt retains copied and original source addresses while leaving routine,
level, object, palette, bitmap, and rendering semantics unclassified.

# ✅ 2026-07-15 Theron Track 02 copied-entry successor receipt

The first observed successor after the copied destination entry now has to
remain inside the same TII destination span and match its corresponding
original byte (`$3c89`). This extends the byte-to-execution chain without
assigning instruction, record, dungeon, object, palette, bitmap, or rendering
meaning.

# ✅ 2026-07-15 Theron Track 02 copied-entry second-successor receipt

Mednafen now emits a second source-owned successor row after a main-RAM call
entry's first successor. Firestaff admits it only when it remains inside the
same copied TII span and matches original Track 02 byte `$3c8a`. This proves a
third bounded byte-to-execution observation, not instruction role, control
semantics, CD-record selection, dungeon data, or visual meaning.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA receipt

The Mednafen main-RAM loader trace now emits HuC6280 `BRA` control rows. The
Track 02-derived entry admission requires opcode `0x80`, its exact copied
displacement byte, and the emulator-computed target to agree. The receipt
records only this bounded control transfer; it does not classify the target as
loader code, a record selector, dungeon data, object data, palette, bitmap, or
rendering behavior.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target execution receipt

The raw loader trace now records a target row only when Mednafen actually
fetches the exact target computed by the source-bound copied-entry `BRA`.
Firestaff retains the target opcode solely as opaque control-flow evidence and
requires the source PC, source physical PC, target and executed main-RAM PC to
agree. This does not assert loader, CD-record, dungeon, object, palette,
bitmap, or rendering semantics.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target JSR receipt

The Mednafen trace now binds the first observed `JSR` after an executed
copied-entry BRA target to that exact target's main-RAM control path. Admission
requires the preceding target receipt and ordered trace rows. The JSR target
is retained as opaque control evidence only, without any assertion about a CD
record, loader routine, dungeon data, objects, palette, bitmap, or rendering.

# ✅ 2026-07-15 Theron Track 02 post-BRA JSR CD-record receipt

Firestaff can now admit a strict control-to-media join: an executed post-BRA
JSR must write the CD data register, then a canonical READ(6) and FIFO-origin
row must select a byte matching the hash-verified Track 02 sector at the
observed LBA. The resulting record coordinate remains opaque provenance, not
a loader name, level, object table, palette, bitmap, or rendering claim.

# ✅ 2026-07-15 Theron Track 02 CUE startup contract

The Track 02 launch resolver now follows the same CUE shape that the Theron
media classifier exposes to startup/menu code: `FILE`, `TRACK`, `MODE1`, and
`INDEX` keywords are accepted case-insensitively, and a CUE must contain
exactly one Track 02 `INDEX 01` before its BIN/ISO payload can be mounted.
This keeps real `MODE1/2048` ISO CUE media launchable while rejecting partial
or ambiguous CUE metadata. No dungeon, object, bitmap, palette, or fallback
semantics are inferred. Verification: `test_theron_v1_track02_cue_layout`,
`test_firestaff_theron_media_classify`, and
`test_m12_theron_missing_track02_popup_gate` pass.

# ✅ 2026-07-16 Theron Track 02 raw-only initial-envelope intake

The `$0b52` initial-envelope loader intake now carries the authenticated
Track 02 media variant and admits the complete-payload handoff only for the
JP/US raw BIN variants. ISO byte lookup remains an inspection boundary, but a
`MODE1/2048` ISO cannot reuse a raw-BIN loader/object-table route or become a
synthetic dungeon substitute. Verification: `theron_v1_track02_loader_intake`
and `theron_v1_raw_loader_trace_initial_level_handoff` pass.

# ✅ 2026-07-16 Theron Track 02 loader semantic gate

The real `$0b52` loader handoff now carries a hash-covered semantic-gate
receipt beside the full payload, initial envelope, and post-envelope bytes.
It exposes real byte availability while keeping dungeon-record,
object-table, bitmap, palette/RGBA, and fallback-visual promotion explicitly
blocked until an original consumer proves them. Verification:
`ctest --test-dir build-local-ninja -R
'theron_v1_track02_loader_intake|theron_v1_raw_loader_trace_initial_level_handoff'
--output-on-failure` passes.

# ✅ 2026-07-16 Theron post-$3800 consumer semantic gate

The Track 02 loader intake now exposes a separate post-`$3800`
consumer-trace gate. It promotes dungeon-record, object-table, bitmap,
palette, and source RGBA availability only when the original same-capture
consumer trace matches the already rehashed loader payload, level-envelope,
and post-envelope checksums. Synthetic dungeon/object/bitmap/palette
promotion and fallback visuals remain hard blockers. Verification: strict
compile of `theron_v1_track02_loader_intake.c` and focused
`theron_v1_track02_loader_intake` coverage for positive source admission,
stale checksum, missing consumer, synthetic, fallback, and pre-promoted-gate
rejections.

# ✅ 2026-07-16 Theron bounded Track 02 route after session handoff

The Theron runtime-admission surface now has a post-session-handoff bounded
Track 02 route receipt. It consumes the admitted US raw Track 02 FIFO
session handoff plus a route receipt carrying corpus evidence, then preserves
the capture mask, no-fallback semantic role mask, startup-level anchor,
blocked object-table anchors, blocked non-startup-level anchors, and route
hashes. It remains runtime-capture-required and refuses exact object/level
semantic promotion, object-table admission, level admission, payload
semantics, visual semantics, and fallback visuals. Verification:
`cmake --build build-local-ninja --target
firestaff_theron_v1_runtime_admission_probe`, `ctest --test-dir
build-local-ninja -R '^theron_v1_runtime_admission$' --output-on-failure`,
and focused `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 decoded-route render proof producer

Theron runtime admission now constructs `Theron_V1RuntimeTrack02RenderAssetProof`
from decoded Track 02 route receipts instead of probe-filled proof fields. The
producer accepts only the same admitted US Track 02 consumer session with
matching level/object/all-dungeon route hashes, decode-ready non-startup level
and object-table receipts, a complete startup bitmap atlas, promotable palette
window evidence, nonzero decoded hashes, and no synthetic/fallback visual
flags. This is a fail-closed producer contract; real ISO/BIN/CUE capture still
has to provide the decoded receipts for broader non-startup dungeons.
Verification: `firestaff_theron_v1_runtime_admission_probe`,
`ctest -R '^theron_v1_runtime_admission$'`, and focused `git diff --check`
passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon-only consumer grammar gate

Added a narrow post-$3800 object/dungeon consumer grammar gate to
`theron_v1_track02_loader_intake`. It consumes the same real loader payload
boundary as the existing semantic gate, but admits only object-table and
dungeon-record grammar provenance when the same-capture original trace proves
both consumers and the payload/envelope/post-envelope checksums match. Bitmap,
palette, RGBA, runtime handoff, fallback visuals, and synthetic promotions are
explicitly rejected on this route. Also repaired the Theron raw-loader final
bind against the current startup-media receipt by reading the Soul Room raw
route spans directly from the receipt fields instead of the removed helper
type. Verification: direct focused C11 build/run of
`test_theron_v1_track02_loader_intake` passed, strict syntax-only checks for
the touched header/source/test and raw-loader source passed, and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon consumer byte-window binding

The Track 02 post-`$3800` consumer gates now require concrete same-capture
object/dungeon evidence before accepting the existing consumer markers. The
trace facts must carry nonzero dungeon/object consumer PCs plus payload-window
offsets, byte counts, and checksums that match the already verified initial
level envelope and post-envelope object-candidate slice from the real `$0b52`
loader read. The narrow object/dungeon grammar receipt retains those PCs and
windows while keeping field decode, bitmap, palette, RGBA, runtime handoff,
synthetic promotion, and fallback visuals closed. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched Theron header/source/test passed, and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track02 consumer-to-CD-read coordinate binding

The post-`$3800` Track 02 consumer facts now bind object/dungeon evidence back
to the exact raw loader/CD-read handoff before either the narrow grammar gate
or the broader consumer semantic gate can open. The facts and receipts retain
the `$0b52` record's user-data offset `$114`, destination `$3800`, and 2048-byte
payload size alongside the existing payload, level-envelope, post-envelope,
consumer-PC, and byte-window checksums. Mutated loader destination, payload
size, record-local offset, object window, or dungeon window evidence all fail
closed, with bitmap/palette/RGBA/runtime/fallback visuals still blocked on the
object/dungeon-only route. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched intake header/source/test passed, and targeted
`git diff --check` passed. At that point the wider
`theron_v1_runtime_admission.c` syntax check still remained blocked by the
missing `Theron_Track02NonstartupContainerIndex` API closed below.

# ✅ 2026-07-16 Theron Track02 nonstartup container-index blocker closure

The missing `Theron_Track02NonstartupContainerIndex` API is now defined and
implemented as an opaque, fail-closed real-data bridge. It is built from the
existing hash-gated nonstartup sector receipt and indexes only verified,
contiguous user-data windows from real raw Track 02 data whose receipt already
marks them opaque and promotion-blocked. The index records descriptor entry,
raw offset, user-data offset, byte count, and hash evidence for later
object/dungeon consumer binding, but it does not decode object tables, levels,
bitmaps, palettes, text, runtime state, or visuals. Runtime-admission syntax
and object compilation now pass again without admitting fallback visuals.
Verification: strict syntax-only checks for `theron_v1_track02.h`,
`theron_v1_runtime_admission.h`, and `theron_v1_runtime_admission.c` passed;
`src/theron/theron_v1_runtime_admission.c` object build passed; focused C11
`test_theron_v1_track02_loader_intake` build/run passed; and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 multi-level runtime handoff gate

Theron Track 02 now has a level-transition/runtime-handoff gate above the
object gameplay state. The new handoff requires same-capture trace proof for
source and target level selectors, target level byte count/hash, target object
runtime-state hash, party-placement binding, and object-pool state binding.
`theron_v1_runtime_publish_track02_level_transition()` then installs the target
level, publishes that level's verified object pool, places the party at the
target level start pose, clears the pending stairs transition, and invalidates
runtime media. This path deliberately stays separate from the older
bitmap-complete dungeon route so real level/object state can advance without
promoting unproven palette/pixels. Dungeon runtime admission, dungeon draw,
synthetic dungeon/object data, and fallback visuals remain denied. Verification:
Ninja built `firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object gameplay-state handoff gate

Theron Track 02 now has a second gate after object placement: object gameplay
semantics. It accepts compact object-table rows only when the same-capture trace
proves the supported runtime kind set, flags low bits as object state, argument
as quantity, preserved flags, and a runtime-state hash. A separate world handoff
then mutates only the selected loaded level's object pool, removes stale objects
for that level, preserves objects from other levels, updates thing count/current
level, and invalidates runtime media. It still denies dungeon runtime admission,
dungeon draw, bitmap/palette/RGBA promotion, synthetic objects, and fallback
visuals. The runtime-admission probe wires this into the optional real
object/dungeon HuC6280 trace path; plain real CUE/BIN remains fail-closed source
proof without such a trace. Verification: Ninja built
`firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object placement-state gate

Theron Track 02 now has a fail-closed object-placement state receipt after the
level/object loader-route proof. It consumes the verified compact object table
and same-capture route trace, binds selected dungeon/level rows, table checksum,
level mask, row hashes, first-row x/y/level/flags/argument bytes, and a placement
state hash. It deliberately keeps object-kind gameplay semantics under review and
does not allow world object publish, runtime admission, dungeon draw, bitmap/
palette/RGBA promotion, synthetic decode, or fallback visuals. The runtime
admission probe's optional object/dungeon HuC6280 trace branch now carries the
full chain to placement state and parses the object table from the real Track 02
container window. Verification: Ninja built `firestaff_theron_v1_runtime_admission_probe`
and `test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
the direct runtime-admission probe passed both default and local US-CUE real-media
runs; syntax checks and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette source-window gate

Theron Track 02 now has a fail-closed bitmap/palette source receipt above the
proved multilevel runtime route. The receipt consumes only a verified
level-transition runtime result, binds the same Track 02 record and
source/target levels to palette raw/user-data offsets, palette checksums,
bitmap atlas route facts, and a combined source hash, and rejects hash drift,
pixel-output claims, M11 render admission, dungeon draw, and fallback visuals.
No bitmap decoder, palette decoder, pixel output, synthetic visual, or M11
render promotion was added. The acute integration break from the new helper
name was fixed by using the existing `theron_v1_runtime_mix_hash` helper, and
`ninja -C build/ninja-dm2 firestaff` now completes. Verification:
`ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette decode-vector gate

Theron Track 02 now has a positive decode-vector receipt after the
bitmap/palette source-window gate. The receipt consumes the source-bound
record/level route plus the real US Track 02 bytes, re-decodes the HuC6260
4bpp palette window, builds the indexed startup bitmap atlas from the same
media, and admits only exact checksum/route/tile/nonzero-pixel agreement. It
retains the first palette word/RGB triplet, atlas route geometry, first source
bitmap offsets, and first decoded pixel-row hash as proof vectors. The result
sets palette decode, bitmap decode, and pixel output verified, but keeps M11
runtime consumption, M11 rendering, dungeon draw, and fallback visuals closed.
No guessed decoder, fallback image, host upload, or dungeon render promotion
was added. Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 M11 Soul Room runtime consumption

Theron now binds the positive Track 02 bitmap/palette decode vector to a
production M11 runtime-consumption receipt for the verified Soul Room level-0
surface. `theron_v1_world_runtime_media_for_level()` now returns the retained
Soul Room surface for level 0, so the existing live `Theron_RuntimeLevelMedia`
path can select it through `THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL`. The new
M11 consumption receipt requires the real world runtime-media surface to match
the decode vector's Soul Room route bit, offsets, geometry, route checksum,
tile count, and nonzero-pixel count, then verifies exact 1:1 placement and
clip bounds before allowing host presentation. Checksum drift, bad host bounds,
scale changes, missing world media, non-Soul Room routes, dungeon draw, and
fallback visuals all remain fail-closed. The real US-CUE probe now builds the
production startup media receipt from the real Track 02 bytes, binds it into a
live world, and proves the M11 Soul Room consumption receipt from that world.
Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# Theron V1 source-locked CD-DA track routing receipt (Lane E, cycle 10)

Closed TODO.md item (5) under the 2026-07-11 Theron original-media
synthetic-path audit: implemented a source-locked CD audio track routing
receipt that gates any future Theron V1 audio output on original CUE
metadata and locally staged CD-DA tracks.

### 2026-08-08 — archived entries

- ✅ 2026-07-27 Theron CDDA host-consumer correction
- ✅ 2026-07-22 Theron boot runtime input/idle facade
- ✅ 2026-07-23 Theron boot startup host-receipt apply facade
- ✅ 2026-07-23 Theron boot startup action/state-receipt apply facade
# 2026-08-10 — source roster stats survive missing US text consumer

- Fixed the authenticated startup handoff so missing/invalid optional US
  roster text no longer aborts or clears the real Track 02 champion records.
- Source-bound stats and skills remain available; display names remain absent
  until the text consumer is proven. T900 equipment semantics remain gated.
- Verified with `test_theron_v1_combat_runtime_source`.
# 2026-08-10 — bound category-4 group count in live admission

- Kept the source monster materializer within the four authenticated health
  words of a Track 02 category-4 record in both validation passes.
- Corrupt or future records can no longer make the live-creature bridge read
  past the source health array; the original RNG/AI path remains gated.
- Verified against the real US/JP dungeon corpus and the production combat
  bridge.
- ✅ 2026-08-20 Theron original PC Engine SRAM boundary: an authentic
  2,048-byte Mednafen `.sav` is now classified by exact size, `HUBM`, and the
  observed `DMS-SG.NNN` marker. The real-data test accepts the byte-identical
  Japanese/American Save Disk baseline and rejects the local 136-byte text
  dump. No body semantics or progression are claimed; the authentic file is
  empty after formatting and is used only to derive the next authentic save
  delta.
- ✅ 2026-08-20 Persistent original save for external Theron: the verified
  Mednafen path now binds `filesys.path_sav` to region-separated
  `~/.firestaff/saves/theron-original/us|jp`. This preserves the original raw
  `HUBM` Backup RAM between Firestaff launches without `.tqsv` conversion and
  prevents the US and Japanese editions from sharing a save directory.
  At first launch, a lone name-matching legacy Mednafen `.sav` is also migrated
  only if it passes exact 2 KiB `HUBM`/`DMS-SG` classification; existing
  destination data is never overwritten, and ambiguous candidates are
  rejected.
- ✅ 2026-08-20 ADPCM playback capture: Mednafen instrumentation now has a
  separate 4,096-record sidecar for `$180D/$180E` with CPU PC, physical MPR PC,
  control value, ADPCM address, read position, length, rate, and the actual
  stop→play transition. This complements the already byte-exact FIFO→ADPCM-RAM
  transport without inventing a gameplay sound ID.
- ✅ 2026-08-20 authentic Theron graphics diagnostics: the current Mednafen
  patch chain was rebuilt from clean 1.32.1 source against real SDL2 after
  correcting the consumer and provenance hunk context. A cold start from the
  complete US disc and System Card 3.0 produced, in one session, 161 raw-sector
  spans, 51 SCSI commands, 161 sector bindings, 25 CD IRQs, and two byte-exact
  CD→RAM origin receipts. Visual inspection then showed that VRAM/VCE alone
  are insufficient for correct replay. The capture producer now also saves
  concurrent HuC6270 registers, and Firestaff's tile base, GRB333 channels,
  shared BG color 0, and unmapped tile indices follow the hardware. The new
  capture pairs are not admitted by the product hash gate until
  `HDR/VDR/MWR`-bound 320×200 geometry is complete; no incorrectly rendered
  image is published.
# ✅ 2026-08-20 Theron Track 02 doors separate source data from runtime state

- Fixed the native Track 02 loader, which previously wrote the source record's
  `type` bit (wood/iron) to the door runtime `state` and wrote the thing
  position into the same low flag bits as `LOCKED`, `BROKEN`, and generic
  object mutations. This could make authentic iron doors partly open and some
  position variants locked or broken at load time.
- Authentic doors now start closed. Position, material, ornament, opening
  direction, button, destructibility, and bashability are preserved losslessly
  in a separate metadata area; no new lock, damage, or sound rules are inferred.
- `theron_v1_track02_dungeon_loader` compares each materialized door against
  the exact 4-byte record in the world's provenance ledger for all seven US
  and seven JP dungeons, and rejects aliases with runtime flags.
- Also moved the teleporter's and actuator's two-bit thing position from the
  generic `PICKED_UP`/`OPENED` bits into the same separate source-metadata
  field. Raw category-3 actuators are now published as
  `THERON_OBJTYPE_SOURCE_ACTUATOR`, not as the unrelated fixture type
  `BUTTON`. The real-data test counts and checks every materialized
  teleporter and actuator in both regions.
- Fixed the parallel teleporter decoder so `ldest` retains bits 8–13 from the
  record format. The real corpus has a maximum destination of 7; a bounded
  decoder test also covers a six-bit value without fabricating a runtime
  destination.
- Door and teleporter routes now select control objects by authentic type,
  not by whichever object happened to be first at a coordinate. The same
  six-bit field now also survives packing and unpacking in the teleporter
  runtime.
- Replaced the TAKE route's generic "first object on the tile" behavior with
  an ordered walk through materialized source records. On verified Track 02
  levels, it selects the first carryable occurrence not yet picked up, then
  applies the existing strict raw-record, property, and ledger gates. The
  real corpus contains 402 US and 402 JP carryables behind an earlier record;
  each dungeon tests one such occurrence through source inventory and a drop
  round trip.
- The M12 pickup receipt now publishes the exact source slot that was filled,
  and M11 retains only this explicit selection. P/DROP returns the record to
  the party's validated tile through the existing lossless source-inventory
  API. Selection is cleared after a drop or active-champion change; a generic
  fixture slot or missing selection still cannot mutate a verified Track 02
  world. The real-data round trip uses the same boot-input facade as the
  product.
- ✅ 2026-08-20 atomic Theron VDC capture: the official Mednafen 1.32.1
  source was verified with SHA-256
  `de7eb94ab66212ae7758376524368a8ab208234b33796625ca630547dbc83832`,
  the complete Firestaff patch chain was built in isolation against real SDL2
  2.30.9, and an authentic Cocoa run produced 65,536 VDC records plus a
  simultaneous 64 KiB VRAM, 1 KiB VCE, 512-byte SAT, and HuC6270-register
  capture. The producer's atomic footer is present. The parser now preserves
  real HuCPU time regressions as diagnostics and uses the contiguous sequence
  as ordering evidence; semantic publication remains disabled without the
  same session's CD→RAM consumer join.
- ✅ 2026-08-20 native atomic graphics gate: Firestaff's Theron viewport
  requires VRAM, VCE, HuC6270 state, SAT, and the complete VDC I/O trace as
  one hash-closed bundle. All 25,890 VWR commits and all 8,816 affected VRAM
  words are verified before loading. The old four-file function always
  rejects; the CLI requires `--theron-vdc-io`, and the real 320x200 bundle
  presents 63,923 source pixels, including 85 SAT sprite pixels, through M11
  without dungeon semantics.
- ✅ 2026-08-20 cold atomic US control: one complete original startup produced
  161 raw-sector spans, 51 SCSI reads, two byte-exact Track 02→RAM
  transport receipts, 32 game-code calls to `$E009`, and 31,794 VWR commits.
  All 24,576 affected VRAM words match the simultaneous 256x240 dump. The
  receipts are still System Card-owned and therefore do not establish the
  remaining source semantics.
- ✅ 2026-08-20 game-owned `$3840 → $E009 → $2800` binding: a new cold run
  from the authentic 19-track US disc preserved the loader copy
  `$201E → $20F8` (8 bytes), raw parameters
  `01 00 00 28 00 03 FF 01`, READ(6) generation 5 for LBA 4257, and the
  asynchronous return to `$3B36`. At the next real `$3840` dispatch, the
  complete 2,048-byte payload was in physical main RAM at `$1F0800` (`$2800`).
  Its 32-byte FNV `2723167f` and full FNV `33a90342` match the MODE1 user data
  in hash-verified US Track 02 record `$4E0` byte for byte. Firestaff now has a
  fail-closed C binder and an optional real-data test that also rejects
  corrupted copies of the real sector and capture row. The parameters' high
  bytes and the payload's gameplay meaning intentionally remain uninterpreted;
  this binds source and destination, not dungeon semantics.
- ✅ 2026-08-20 game-owned consumer chain for the first `$2800` payload: a
  dedicated sidecar preserves the asynchronous return `$3840 → $3B36`, five
  ordered main-RAM reads, and the next `$3840` dispatch. Game code at
  `$37E2/$37E9/$37F7/$37FC/$3802` read `$2D13..$2D17`, i.e. offsets
  `$513..$517` in the bound block, with values `F9 02 04 00 20`.
  The fail-closed binder checks addresses, physical MPR mapping, ordering,
  reading PC, and every byte against authenticated US Track 02 record `$4E0`.
  The real-data test also rejects a tampered copy of the actual read row.
  The provenance fields were invalid, and the gameplay meaning of the five
  fields remains unknown; the receipt proves only a real game-owned consumer
  chain.
- ✅ 2026-08-20 source-bound E009 consumer code: a new cold original run
  captured main RAM `$37C8..$383F` at the first asynchronous return. All 120
  bytes match the authenticated US Track 02 file across a real raw-sector
  boundary: 56 bytes from record `$4C4`, user offset `$7C8`, and 64 bytes from
  record `$4C5`, user offset `$000`. FNV-1a is `048e8620`. The byte-exact
  HuC6280 routine at `$3806` loads base `$2803` and performs a three-step
  shift/add with constant 6. Together with the first real consumer address,
  this proves the relation `$2803 + 6 × $D8 = $2D13`. The call trace also
  preserves `$D8` at `$36D2 → $37D8`. The C binder rejects modified code both
  in the capture and on the real media. `$D8` and the following five bytes
  still have no assigned gameplay meaning.
- ✅ 2026-08-20 next E009 parameter block: the same authentic RAM sidecar
  preserves the ordered write chain from the consumer routine to work area
  `$201E..$2025`, the subsequent game-code writes, and the TII instruction
  `$3836: $201E → $20F8` with length 8. The resulting block before the next
  `$3840` call is byte-exact `00 20 00 10 00 06 F8 FE`. A separate fail-closed
  receipt checks the write sequence, logical and physical addresses, writing
  PC, and all eight TII results; the real-data test rejects a modified
  parameter write. The bytes' SCSI/record meaning remains unknown.
- ✅ 2026-08-20 generation 6 CD→VDC binding: a separate, generation-tagged
  sidecar from the same cold original run preserves four VDC settings and
  then exactly 8,192 unique data-port writes, double-logged in the same way
  as the producer's VDC hook. After deduplication, the byte stream exactly
  matches MODE1 user data from READ(6) LBA 5018–5021, i.e. authenticated US
  Track 02 records `$7D9..$7DC`; FNV-1a is `4859675d`. The writer is game
  routine `$EB35` (physical PC `$000B35`), and the ports alternate
  `$0002/$0003`. The preceding VDC writes select register 0, set MAWR to
  `$1000`, and select register 2. A dedicated same-session dump immediately
  after the generation's final write corrects the initial deduplication
  model: the real VDC path writes every 16-bit source word twice. The stream
  therefore fills 8,192 VRAM words `$1000..$2FFF`, byte-exactly matching the
  doubled Track 02 source words, with FNV-1a `9b9f7361`. The new fail-closed
  binder requires the preceding parameter receipt, generation-6 CDB, VDC
  settings, complete write order, and every real media byte. The real-data
  test rejects both a modified VDC row and a modified Track 02 copy. It also
  corrupts a copy of the actual VRAM dump. Image meaning and parameter-field
  semantics remain closed by this transport receipt.
- ✅ 2026-08-20 first source-bound BAT presentation: a second cold original
  run froze VDC/VRAM exactly after generation 7's 4,302 port records. A new
  fail-closed replay starts from the verified generation-6 snapshot, replays
  2,048 real VWR commits, and matches the generation-7 snapshot's BAT byte
  for byte (`593edd45`; full VRAM `1f64dae1`). VDC state shows a 64×32 BAT,
  a 256×240 active area, and the background enabled (`CR=$0088`). All 960
  active 32×30 cells reference source-bound tile indices `$110..$187`, 60
  unique indices in total. Source-bound 4-bpp decoding yields exactly 61,440
  indexed pixels, of which 2,848 are nonzero, with FNV-1a `c5899c5d`. The
  same generation-7 snapshot binds VCE (`f12861c5`) and palette group 0
  (`ebc22165`) to every indexed pixel; the resulting 16-bit color-word image
  has FNV-1a `31866f25`.
  A 512-byte CDRAM snapshot `$104600..$1047FF` also binds the active BAT
  writer code to authenticated Track 02 record `$4D0`, user offset `$600`.
  The unchanged source spans cover 435 bytes, and the differing positions
  are genuinely modified operand/work fields. The dynamic code is also
  joined to the existing static Stage 2 receipt for `$466B`: the byte-bound
  routine selects the VDC VWR register and rewrites operands for the HuC6280
  `TIA` instruction at `$468C`. At runtime, the original zeroed fields become
  source `$47E0`, alternating VDC destination `$0002`, and length `$0040`.
  The generated 32-byte BAT row at `$47E0` has FNV-1a `da633f05`, and its
  actual copies are part of the already verified generation-7 replay. The
  code snapshot's FNV-1a is `3e3745f7`. The real-data test rejects a modified
  generation-7 VRAM copy. This verifies consumption of authentic Track 02
  bytes by the display pipeline; palette colors and image gameplay meaning
  intentionally remain uninterpreted.
- ✅ 2026-08-20 later generation-49 graphics transport: the continued cold
  original session binds READ(6) LBA 4622–4633 to twelve authentic MODE1
  sectors, Track 02 records `$64D..$658`. Exactly 24,576 media bytes with
  FNV-1a `01551f76` appear in order in 49,152 double-logged VDC payload
  records after the byte-exact MAWR=`$1000`/VWR setup. A total of 49,160
  generation-49 records are verified. The new fail-closed binder requires
  the preceding generation-7 presentation chain and rejects a modified real
  media byte. A 90-second original capture also showed that generation 51 is
  a long active drawing loop, not a bounded one-time load: more than 449,000
  VDC records were observed. The first natural 1,035-record frame loop starts
  at global sequence 174,721; an atomic snapshot after its first complete
  iteration, generation-51 row 100,755, is retained only as raw display
  hardware evidence. Its background and SAT show an in-progress transition
  with no sprite pixels, so no title, menu, or gameplay semantics are
  attributed.
  A second fail-closed receipt now requires that same atomic snapshot and the
  authenticated Stage 2 receipt in one verification chain. Through the first
  complete loop boundary, it verifies 100,755 ordered generation-51 records:
  4,288 writer rows within `$466F..$4693`, 25 within `$4934..$4942`, 4,120
  within `$50F1..$5110`, and 4,096 exactly at `$5110`. The simultaneous full
  file hashes are VRAM `87fbe859`, VCE `8682b5d5`, and SAT `4d7705c5`. The
  real-data test rejects a corrupted copy of the atomic snapshot's VRAM. This
  proves disc transport → Stage 2 drawing code → atomic display hardware,
  but not what the transition depicts.

- ✅ 2026-08-20 actual Track 02 source for the file-selection prompts: a
  separate fail-closed receipt verifies all three US copies of `WHICH FILE DO YOU
  PLAY?` and `WHICH FILE DO YOU LOAD?` directly in the hash-verified raw file.
  They are in MODE1/2352 records `$4EA/$4EC/$4EE`, at raw sector offsets
  `$1AE/$0FA/$0FA`; the prompts' FNV-1a hashes are `ef1550ad` and
  `aa654403`, respectively. The test corrupts one byte in a copy of the real
  file and requires rejection. Generation 51's graphics instead come from
  `$64D..$658`, so the image and text source have deliberately not been
  combined without a same-session capture of the CPU's text reads.
  The source receipt now has its own real-data test,
  `test_theron_v1_file_select_text_source_real_data`, which runs directly
  against `TQUS02.bin` and no longer depends on the larger generation-5–51
  capture bundle. A local run against the real file passes; the test honestly
  skips when the path is not configured.
  Three new cold original sessions also show that no READ(6) covers
  `$4EA/$4EC/$4EE` and that the exact ASCII prompt is never present in PCE RAM
  banks while file selection is displayed. After deterministic Button I at
  frame 3600, the original passes through `$4698/$511B` bulk transfer and
  then a tight main-RAM loop around `$3C2A..$3D2B`. This is discovery evidence
  for an encoded glyph/text path, not enough to establish text-consumer
  semantics.

- ✅ 2026-08-20 stable file-selection phase and atomic VDC consumer: a new cold
  original session with late Button I (`frame 8520`) exited the
  `$4698/$511B` transition. At `frame 8580`, the CPU reads state
  `$2059=01` at `$41A8` and a source stream through `$5139..$515E` and
  `$5561..$55D0`. The latter reads real bytes from physical bank
  `$0D1D58..`; the first observed bytes are
  `02 00 70 00 40 00 10 02 80 01`.
-  A second atomic original session joins the same CPU window to VDC:
  `$5110` performs 1,024 VDC port writes after setup at `$4934..$4942` and
  `$50FB..$5109`. The parameter routine simultaneously reads
  `$4DE3/$4DE4 = 3D 7D` and `$4DE9/$4DEA = 49 7D`; the CPU trace also shows a
  read from `$7D58` at `$514B`. This verifies runtime→VDC consumption for
  the encoded path. The media record that populated `$0D1D58` has not yet
  been identified, so prompt and glyph semantics remain closed.

- ✅ 2026-08-20 corrected media→RAM→VDC chain for file selection: review of
  the combined Mednafen patch stack found two `WriteMap` calls per logical
  write. The build chain now has an explicit single-write correction, and
  new cold original sessions were run. READ(6) generation 12 reads LBA 4668,
  raw Track 02 record `$67B` (`fcc73c77`). Its first 195 user bytes
  (`efad54b3`) are written by the System Card at `$EA9E` to
  `$0DDC5B..$0DDD1D`, is copied byte-exactly by game code `$3446` to
  `$0D1D3D..$0D1DFF`, read at `$514B`, and reaches VDC routine `$5110`.
  The corrected target frame has 512 actual `$5110` port writes and 13 setup
  writes, 525 total. The previous counts, 1,024/1,035, were double-write
  artifacts. A new real-data test rejects both a modified Track 02 byte and
  a modified loader row.
- ✅ 2026-08-20 The 525 file-selection VDC rows are now replayed as actual
  hardware writes. The 13 setup rows select VWR and set MAWR to `$0800`; the
  following 512 bytes form exactly 256 VRAM words `$0800..$08FF`, with hash
  `a8007f15`. A new atomic snapshot taken immediately after the final byte
  shows that the region
  is byte-identical to the VDC's internal 512-byte SAT, also `a8007f15`. BAT
  contains no references that would make `$80..$8F` pattern indices. The SAT
  has 18 non-empty entries and 46 empty entries. The real-data test rejects
  a changed VWR byte or SAT byte. This proves SAT staging, but not the
  meaning of individual sprites or the screen.
- ✅ The 18 SAT entries are decoded from hardware using the same HuC6270
  rules as Firestaff's existing authentic SAT renderer. All use PN `$0210`;
  entries with a height of 64 pixels address sprite patterns `$108..$10F` in
  VRAM bytes `$8400..$87FF`. The eight pattern blocks are byte-identical, and
  the entire region has hash `ba5526c5`. Twelve SAT entries are within the
  visible 256×240 area; six are at y=240 and outside it. The sprites have not
  yet been identified.
- ✅ Frame 8580 is now composed from the atomic VRAM, VCE, and SAT image using
  exactly the same HuC6270 rules as Firestaff's existing renderer. It has
  61,440 pixels, 43,087 non-empty source pixels, 18,511 final background
  pixels, and 24,576 final sprite pixels across 40 unique source indices.
-  The indexed image hash is `7622aee1`; the same image represented as raw,
  authentic VCE words has hash `8f1cf573`. All sprite pixels use source index
  `$101`, VCE color `$0000`, within x=32..223 and y=0..239. A changed SAT or
  VCE byte is rejected. This proves the rendered hardware image but does not
  attribute any bands to file selection, prompts, or any other screen meaning.
- ✅ Two new authentic end frames bracket the VDC update: frame 8579 has BYR
  `$00E9`, while frame 8581 has `$00E8`. VRAM (`832b4d13`), VCE
  (`5376a91b`), and SAT (`a8007f15`) are byte-identical. At frame 8580, game
  code `$4993` selects BYR and `$4999/$499F` write `$00E8`. The finished
  source image hash changes from `af183e0d` to `7622aee1`, and the VCE color
  hash changes from `68fe4a69` to `8f1cf573`. Exactly 12,644 pixels change
  within x=32..223 and y=64..175. All 24,576 sprite pixels remain unchanged;
  only the background behind them scrolls by one row. A new real-data receipt
  rejects modified pre-frame VRAM and requires explicit end markers for both
  frames.
- ✅ The file-selection BYR producer and stop condition are now bound using an
  authentic code image, RAM image, and two write traces from the same
  deterministic button sequence. Game code `$4993` reads `$2210/$2211` and
  writes the value to VDC BYR. The self-modifying signed loop at
  `$4175..$41A1` writes `$2210` through `$4184`: `$F0→$60` in 144 steps at
  eight-frame intervals. The control pair `$47BC/$47BD` simultaneously
  counts down from `$0090` to `$0000` through `$4A7B/$4A80`; the final RAM
  step occurs at frame 9666 and the zero stop at 9668. The next observed
  write in both traces is initialization at 10310, delimiting the stop. A
  real-data test checks all 144+144 steps and rejects modified code or a
  modified stop row. Receipt fields for screen semantics remain explicitly
  closed.
- ✅ The same authentic control trace now also binds the continuation after
  the first stop. At frame 10632, `$47BA/$47BB` are set to `$0040`. Code at
  `$4B0C..$4B23`, with writers `$4B1B/$4B20`, decrements the pair exactly 64
  times at ten-frame intervals and reaches `$0000` at 11274. The next reset
  boundary is frame 11578. The real-data test requires the full sequence and
  rejects a corrupted second stop row. The control trace alone establishes
  no screen meaning.
- ✅ The authentic JP Track 02 spawn source block is now decoded without a
  US fallback.
  `TQJP02.bin` is hash-verified before reading the pointer table at UD `$273818`,
  zone records at `$273858/$2738D7/$273902/$273929/$273950`, and the exact
  Shift-JIS roster prefix at `$2739EF`. Pointer-entry region word `$2780` is
  retained instead of the US value `$278A`; the zone records are byte-
  identical between editions. The world binding stores the JP source, while
  the category consumer still returns `$FF` for JP. The real-data test runs
  both US and JP BINs, requires their respective regional records, and rejects
  a corrupted copy. Production startup now also binds the correct regional
  source before user-data normalization. The boot receipt reports
  `theronSpawnSourceAuthenticated=1` and variant 1 for JP or 2 for US; both
  complete raw-BIN startups are verified against the original files.
- ✅ The region-specific static spawn consumers are also source-bound.
  The US routine's 269 bytes at UD `$0870E5` have FNV-1a `eb241d19`; the
  relocated JP counterpart at `$0868D2` has `7dc1e453` and retains its own
  call and data addresses. Receipts require the correct full-file MD5 and
  reject a modified original file. `runtime_execution_proven` and
  `category_semantics_proven` remain zero, so static code cannot replace a
  Japanese execution trace.
- ✅ The older generation-6, generation-49, and generation-51 receipts are now
  fail-closed because their VDC row counts came from the double-write build.
  Media coordinates may be used as discovery clues, but runtime admission is
  reopened only after new single-write captures.
- ✅ 2026-08-20 generations 6 and 7 were recaptured and reopened with the
  corrected single-write build chain. Generation 6 writes each of the 8,192
  authentic bytes from Track 02 `$7D9..$7DC` once to VRAM words
  `$1000..$1FFF` (`4859675d`). Generation 7 makes 1,024 VWR commits within
  2,151 BAT rows and reaches the presented boundary after 36 additional
  register writes: 2,187 rows total, CR=`$0088`. The active 32×30 background
  references 124 source-bound tiles `$110..$18F`; its indexed image has
  61,440 pixels, 3,373 nonzero pixels, and hash `2c2cfb4d`. The new real-data
  test verifies the full generation-6→7 chain and rejects modified
  generation-7 VRAM. Generations 49 and 51 remain fail-closed.
- ✅ 2026-08-20 generations 49 and 51 were also recaptured with the
  single-write build chain. Generation 49 binds READ(6) LBA 4622 to the
  24,576 real user-data bytes in Track 02 `$64D..$658` (`01551f76`) and
  exactly 24,580 VDC rows: four setup writes followed by one write per media
  byte to VRAM words `$1000..$6FFF`. Generation 51's corrected atomic frame
  boundary is row 54,842, sequence 94,434, with VRAM `de27fc7e`, VCE
  `88629e93`, SAT `4d7705c5`, and CR=`$0048`. Separate real-data tests reject
  modified Track 02 media and a modified VCE palette, respectively. The old
  double-write counts 49,160 and 100,755 are no longer used for runtime
  admission.
- ✅ Track 02 object metadata is now bound to the owning dungeon's real bank
  in both US and JP. Name counts are 80/65/69/69/67/63/66; all name and type
  code spans are hash-verified, and each bank retains its own 66×6 property
  table. JP Drator has the genuinely different table hash `6c4d1386`; the
  other verified tables have `b97787ef`. Pickup, drop, and save validation
  use the object's source dungeon and reject a missing or changed source bank.
- ✅ The production build no longer links the historical static 66-row
  catalog. Track 19 instead verifies exactly 396 real ISO bytes with FNV-1a
  `b97787ef`, and the negative real-data test requires a changed table to be
  rejected. The old catalog remains only in explicit compatibility and
  analysis fixtures.
- ✅ The US edition's readable Track 19 catalogs are also disconnected from
  production. Runtime reads the 69 real object names from the verified
  685-byte span (`5be5602d`) and the 15 real level labels from the 135-byte
  span (`7f7d9f67`). Modified ISO bytes are rejected; checked-in plaintext
  lists are used only by historical fixtures.
- ✅ Track 19's regional 69-byte type-code tables are now verified and
  preserved in the runtime bank: US `$0E9226` (`21533bb5`) and JP `$0E9266`
  (`f9c3eabb`). Negative real-data tests modify a type-code byte and require
  rejection. The tables are byte-identical to the corresponding edition's
  dungeon 4 bank in Track 02. World binding also compares the complete
  396-byte property table and then opens a strict dungeon 4→Track 19 index
  mapping. Objects from other dungeons are rejected, as are changed type
  codes or property bytes.
- ✅ Live inventory inspection uses the proven Track 19 path for Sarmon
  objects. The check follows the real pickup record's dungeon, type code, and
  property row. The other six dungeons continue to use their own authentic
  Track 02 names because their tables do not have the same positional
  relationship to Track 19.
- ✅ Four checked-in US plaintext catalogs for Track 02 are disconnected from
  production: UI status, save menus, level/quest text, and action names. They
  have no production consumer and duplicate bytes available in real media;
  their separate historical fixture test remains. The static 41-entry
  cost/secondary table was also removed from the production archive because
  only the fixture-only compatibility combat uses it.
- ✅ Twelve more disconnected Track 02 fixture modules are no longer linked
  into the production library: champion/combat/dungeon/HUD text, font glyphs,
  complete object names, level labels, action parameters, class base values,
  class skill values, creature names, and dungeon descriptors. An exact
  symbol audit shows that no production file calls their exports; their
  explicit historical fixture tests remain. Active roster, spawn, item-ID,
  and level-data consumers are unaffected.
- ✅ The US edition's complete roster is now decoded directly from the
  authentic 5-bit stream in Track 02. The exact 360-byte span
  `$0B46C8..$0B4830` has FNV-1a `39d95c9e` and contains eight records with
  names, gender, HP, stamina, mana, seven attributes, and sixteen skill
  values. An incorrect region hash, changed record structure, and changed
  bytes even in the still-unresolved title fields are rejected. Title control
  codes are not published.
- ✅ Forcefield handoff now uses regional original records for both US and
  JP. The production party starts empty; selected names create identity slots
  only, and the regional roster reader atomically fills all stats and skill
  levels. The older DMWeb/C table is disconnected from the production archive
  and remains only in explicit fixture tests.
## Authentic regional quest-artifact name binding (2026-08-21)

## Extended authentic capture limit (2026-08-21)

- The VDC producer now has a backward-compatible, runtime-selectable limit of
  65,536–2,097,152 records. The three earlier atomic original captures still
  verify unchanged, with zero VRAM differences.
- The graphics boundary can now also freeze the original's 8 KiB main RAM.
  One real click/control pair shows 14 RAM differences without any graphics
  difference. This result is preserved as negative evidence and does not
  establish movement, position, or direction semantics.
- A new authentic button/control pair now shows a presented dungeon change
  with 27,430 differing pixels. The original Button I edge is bound to
  `$28B8=$01` at `$44E5`, and the same session binds the quarter-turn to
  `$203F: 1→2` and `$2944/$2948: 1→2` through captured routine
  `$D900..$D92E`. The previous assumption of forward movement is therefore
  corrected.
- Command capture is now fail-closed: exactly 65,536 ordered RAM writes,
  a final boundary row, a genuine 64 KiB code-bank snapshot, and two 8 KiB
  main-RAM snapshots. Extra records after the boundary and incomplete
  sidecars are rejected. The longer VDC image remains analysis data because
  the CPU-port trace does not yet replay internal DMA.
- The opposite original capture queues type `$01` at `$7B/$8F` and changes
  `$203F`, `$2944`, and `$2948` from `1` to `0`; a long control pair differs by
  27,226 pixels. Types `$01/$02` are now byte-bound to left/right. M11's four
  rotation inputs use these original types, while other types are rejected.

- Bound all seven quest-artifact labels to their exact dungeon-local Track 02
  name-table entries: indices `41, 63, 45, 43, 44, 43, 7` for dungeons 1
  through 7. These are presentation indices, not yet retrieval-event IDs.
- Added a fail-closed source accessor and a world accessor.  Both require an
  authenticated regional item-name bank and return raw ASCII (US) or Shift-JIS
  (JP) bytes without substituting a host label.
- Verified all fourteen names directly against `TQUS02.bin` and `TQJP02.bin`.
  Missing, invalid, or empty source entries are rejected.
- Added `test_theron_v1_track02_quest_item_names`, a compact real-media test
  that can run independently of the full application build.
- Startup chapter inspection and layout now use the live world's bound name
  banks. Authenticated US ASCII is shown directly (for example
  `SHIELD DEFIANT`); authenticated JP Shift-JIS remains explicitly
  unavailable rather than being mislabelled as UTF-8 host text.
- The production text-gate test loads both real BIN files and proves those two
  regional outcomes. Calling the older API without a world remains
  fail-closed and cannot reintroduce the removed static US catalog.
- A second raw-data census proves why these indices must not drive gameplay:
  dungeon 1 has no ordinary carryable record at its label index in either
  region, while other dungeons have between one and eleven same-index records.
  The US counts are `0,1,2,2,1,3,2`; JP counts are `0,11,2,2,0,3,2`.
  Quest completion therefore remains gated on the still-unrecovered original
  retrieval consumer rather than being inferred from an ordinary pickup.
- Release-mode CMake verification now keeps assertions enabled for the two
  real-media decoder/loader tests. The quest-name census, production chapter
  text gate and full US/JP dungeon-loader test all pass together.
- A rebuilt `firestaff` executable also passes both complete raw-media startup
  checks: `theron_v1_raw_bin_runtime_boot` (US) and
  `theron_v1_jp_raw_bin_startup` (JP).
- Added a production decoder for the complete regional retrieval-message
  banks. US uses seven `05 03`-prefixed, NUL-terminated records at
  `UD 0x27713D` (331 bytes, FNV `4777d500`); JP uses seven
  `81 96`/`81 97`-framed Shift-JIS records at `UD 0x27696D` (364 bytes,
  FNV `cb874921`). Both are read from the real BIN and reject any altered
  span. The decoder deliberately leaves `retrieval_event_relation_proven=0`:
  text framing alone does not identify the gameplay event that selects a
  retrieval message.
- The full real-data dungeon loader now installs that regional retrieval bank
  in the live world. A new index-only raw accessor exposes the seven
  authenticated records without calling them gameplay events; bind and read
  both fail closed if event ownership or host rendering is asserted before it
  is proven. The full US/JP loader, raw startup, and compact decoder tests pass
  with the rebuilt application.
- Added an authenticated US/JP campaign-mask source receipt. The common
  336-byte
  routine is byte-identical at US UD `$071800` and JP UD `$071000` (FNV-1a
  `72af456f`) and proves original RAM `$267C`, low-bit mask `$7F`, preserved
  high bit, and the load-time `ORA`/store sequence. Seven 304-byte
  dungeon-local windows additionally bind the real `01 02 04 08 10 20 40`
  ordinal-to-bit table and their own `ORA $267C`/`STA $267C` sequence before
  loading another code resource and jumping to `$4000`. Altering either code
  class is rejected for both retail BINs. The receipt explicitly records that
  no artifact-collection relation has been proven; `$267C` is therefore only
  classified as campaign/dungeon state.
- The campaign-mask receipt is now installed in the live world on every full
  real-media dungeon load. Its binder rejects any source that asserts an
  artifact-collection relation, so the authenticated `$267C` evidence cannot
  silently enable the host's older quest-item model.
- The same receipt now authenticates the seven four-sector post-dungeon
  programs reached by that transition. US Track 02 INDEX 01 begins after 225
  raw sectors and JP after 224, so relative CD blocks `$3C7 + 4*n` resolve to
  raw records `$4A8 + 4*n` and `$4A7 + 4*n`. Each program stores its fixed
  ordinal 0–6 at `$2701`; programs 0–5 store `$09` at `$2700`, while the final
  program stores `$0F`. This proves the ordinal handoff into the post-dungeon
  program, not yet its relation to retrieval-message record `$4EE`.
- Authenticated the shared 17-sector post-dungeon program for both editions
  (US FNV-1a `113c8278`, JP `834bede1`). The original `$8243` routine copies
  `$2700/$2701` into `$2780/$2781`, and the common handler forwards the seven
  bytes beginning at `$2781` into its local selection table. The production
  decoder now rejects altered shared-program bytes and exposes these two
  ordinal-transport proofs without claiming an artifact or retrieval event.
- Captured the retail US forward-panel click `$8A/$8F` as original command
  type `$03` with an exact 65,536-write main-RAM trace. The command starts at
  party coordinates `$02/$03`, resolves destination `$03/$03`, and commits
  both bytes through the original `TII $20B4,$2040,$0002` at `$C1FA`
  (observed write PC `$C203`, physical `$0DC203`). The UP runtime route now
  uses `$03` and the existing real Track 02 tile/object collision model.
- Captured the remaining retail movement-panel commands: `$04` at `$98/$A5`
  is right, `$05` at `$8A/$A5` is backward, and `$06` at `$7B/$A5` is left.
  The original `(command-3+direction)&3` calculation binds their relative
  directions. `$05` commits `$02/$03->$01/$03`; `$04/$06` are blocked from
  this real savestate and never write `$2040/$2041`. M11 now routes A/D and
  DOWN through these original types while preserving facing direction.
- Replaced the stale Theron four-way/no-strafe host assumption. A/D now emit
  `M12_MENU_INPUT_STRAFE_LEFT/RIGHT` and reach original `$06/$04`, while arrow
  Left/Right and keypad 4/6 retain turn semantics. The existing host bridge
  test and runtime-input test pass. The real US/JP Track 02 playability probe
  now exercises each `$03..$06` command over an authentic floor edge and an
  authentic wall edge, preserving facing in every case: 143/143 pass.
- Historical result, superseded by the later source-evidence gate: that version
  of the real-data mechanics probe converted authenticated 2352-byte raw
  sectors through the verified MODE1 user-data bridge, loaded the real AKUTUBA
  dungeon, and reported traversal over a stair edge. The current mechanics
  implementation rejects stairs on authenticated Track 02 because their
  direction, destination map, and arrival pose remain unbound. The later
  regional probe verifies this fail-closed behavior; see the open stair work
  in `TODO-theron.md`. Do not treat the old 145/145 result as current stair
  traversal parity.
- Removed the mechanics probe's unrelated four synthetic champions, fixed
  stat values and 1,000-gold seed. Real movement now runs with an empty party
  state plus only the authenticated map/start pose; regional roster and save
  receipts remain the sole production owners of champion state.
# 2026-08-21 Theron real-door mutation boundary

- ✅ Authentic US/JP Track 02 doors no longer receive the synthetic fixture
  model's immediate `OPEN`/`CLOSED` mutation. Mechanics verifies the exact
  source occurrence, four-byte record, and level header, then closes the
  mutation path until the original T900 button/actuator consumer is captured.
  The real-data probe finds an actual door edge in the complete dungeon corpus
  for both regions, verifies blocked original movement, and confirms that
  direct open/USE preserves the closed state. Isolated fixtures retain their
  explicit test model.
- ✅ The loader now binds the already-verified Track 02 identity to live door,
  teleporter, and actuator objects as well. Previously their exact bytes were
  present only in the provenance ledger, so a runtime object could not prove
  which real record it represented. The corpus test now compares source
  dungeon, level, coordinates, ref, index, category, position, and the full
  raw record for every placed control object in US and JP.
- ✅ The source gate for real doors survives world save/load. Every US/JP
  dungeon containing a door is now serialized from its complete real world,
  restored against the same authenticated level bank, and checked to ensure
  that source identity and raw record are unchanged and direct host-open is
  still rejected.
- ✅ Source reference `0x0000` is no longer an invalid host sentinel. In
  Track 02 it is a valid packed reference for category 0, index 0, and
  position 0; SARMON's real door corpus contains this case. Provenance is now
  determined by a full occurrence match against the ledger, even when the
  packed reference happens to be zero.
# 2026-08-21 Theron startup seed uses the real level header

- ✅ Startup's semantic handoff no longer requires the disproven synthetic
  descriptor table `313/414/527/632/749/856/967`. The runtime seed now comes
  from the hash- and anchor-approved candidate's real 12-byte header
  (`0x0108e938` in both US and JP). Descriptor entry 0 is retained only for
  diagnostics and may be zero-filled or malformed without rejecting the
  authentic level. The real-data probe verifies positive handoff, the runtime
  receipt, and blocked fallback for both regions.
- ✅ The media-free `theron_v1_startup_enter_forcefield()` can no longer
  publish a zeroed champion record as "Theron" in the production library. It
  returns `NOT_READY` without changing the Soul Room or party. The real
  runtime path continues to use `_with_roster()` and then atomically binds the
  US/JP records' HP, stamina, mana, attributes, and skills before level handoff.
- ✅ Production handoff from the forcefield is now one transaction through
  `theron_v1_startup_enter_forcefield_with_track02_roster()`: the verified
  region, selected portraits, names, and US/JP stat records must all succeed
  before flow or party state is published. On failure both are restored
  exactly. The older media-free forcefield APIs return `NOT_READY` in the
  production library and remain only for explicit fixture compilation.
- ✅ The dead compact host-object publisher in startup entry has been
  removed. It could only be built from the disproven descriptor-object table
  and created objects from `kind/flags/argument` without a Track 02 occurrence
  or raw record. Production startup now uses only the full-dungeon loader for
  real objects; the separate authentic level header may publish a level but
  never fabricate an object layer.
# 2026-08-21 Original `$50` consumer gate

- ✅ The hash-bound original capture now also verifies the complete ordered
  main-RAM consumer trace. The authentic `$50` window contains no reads in
  `$2600–$27ff`, so an ordinary viewport-zone hit test can no longer be
  accidentally published as door or T900 evidence. A missing consumer trace
  is rejected and all semantic publication remains closed.
- ✅ The command-bound Mednafen producer now logs only main RAM `$2000–$3fff`,
  and the capture gate rejects both addresses outside the window and windows
  that reach the 65,536-read limit. A research run of the original `$03`
  against the real US door ended after 55,078 reads and produced 96 reads in
  the source region. This result still does not publish door semantics
  without a targeted mutation at the consumer point.
- ✅ Targeted research mutations have narrowed what this window does not
  prove. HuC6280 interrupts during block transfers can retain the block
  instruction's reported PC, and mutations of `$271b–$272b`, `$27af–$27c8`,
  and the door-like work byte `$2098=$81` bound neither the real record nor
  the movement outcome. Documentation therefore no longer calls the 96
  time-windowed reads causal evidence of a T900 consumer.
- ✅ The capture receipt now separates source-region reads in the time window
  from reads after the original `$2905` dispatch at `$d34d`. The real `$03`
  run has 96 reads before dispatch and 0 after it. Interrupt/rendering work
  can therefore no longer count as movement or door-consumer evidence.
- ✅ A separate data-only trace compared the same real door position against
  a verified open control at `(1,3)→(2,3)`. The original's first path
  difference is the exclusive X boundary at `$4fbb–$4fbd`: the door
  position's target X `5` is blocked against boundary `5` before Y/tile
  checks, while the control's target X `2` continues. Mutating the later
  banked difference `$10→$20` did not change position. Forward collision is
  therefore classified as a map boundary, not T900 or door-opening evidence;
  real doors remain fail-closed.
- ✅ The original marker route has now been captured at another real point.
  `$79/$62` writes hover command `$2911=$74`, queues `$2905=$74`, and ends
  after 69,208 main-RAM writes. A complete data-only trace of 72,693 reads
  was compared with the same point from a verified open control. A horizontal
  hover sweep shows `$50` at X `$00–$6f` and then a snap to `$79/$78` with
  `$74`; the code therefore belongs to the right column, not the front cell.
  The route changed no source-bound door record and is published only as an
  original command, not as view, door, or T900 semantics.
- ✅ Theron's older V1 click matrix no longer publishes nine host-created
  320×240 rectangles as original data. They also did not match the original's
  authentic 320×200 image. V1 zone queries are now empty and fail-closed until
  the real rectangle table or a complete boundary trace is available. The
  modern V2 overlay's presentation zones remain and are explicitly not V1
  data.
- ✅ The Track 02 loader no longer lets category-0 records overwrite the map
  stream with host-created door tiles. The map byte's own type must already be
  a door; otherwise the bank is rejected. A full run of all seven authentic
  US and seven JP banks shows that every real door record satisfies the gate.
- ✅ The Track 02 loader's TAKE/DROP regression now uses Theron's authentic
  regional champion record from the same real US or JP BIN. The test
  previously created an empty world and then tried to perform an inventory
  transition without an active champion. All fourteen real dungeon banks now
  pass both the item round trip and door gate without a test champion.

# 2026-08-21 Track 02 teleporter's real OPEN attribute

- ✅ An isolated original run distinguishes two real Akutuba pads. Tile
  `(0,0)` has map byte `B8` and moves the party to `(2,3)` on map 0. Tile
  `(2,1)` has `B4`; an authentic step north lands on the tile but leaves the
  party on map 0. The difference is bit `0x08` in the map byte, the same OPEN
  bit specified by the dungeon format, not the teleporter record's `scope`.
- ✅ The world bridge now preserves the exact raw map byte for every Track 02
  tile. The loader requires a category-1 record to already be on a real
  teleporter tile and can no longer overwrite map geometry.
- ✅ Source-bound runtime uses the map's OPEN bit as the teleporter state. An
  active `B8` pad follows its real coordinate link; an inactive `B4` pad is
  passable floor and starts no transition. The record's two-bit `scope` is
  kept separately as source metadata and is no longer aliased to runtime
  state.
- ✅ The non-mutating movement query now uses the same OPEN gate as the
  mutating path. A closed `B4` pad is reported as ordinary passable movement,
  not teleportation; Akutuba's real `(2,1)` tile tests both calls.
- ✅ The regression runs all seven authentic US and all seven JP dungeon
  banks, verifies the map/record gate, and tests both Akutuba routes. The full
  Track 02 loader, cross-route mechanics, touch matrix, and world
  serialization pass. The older broad combat fixture still has its known
  fail-closed failures for unauthenticated combat, spell, and door consumers;
  no such fixtures were opened merely to make tests pass.
- 🔒 The closed Akutuba tile also has a real floor-party actuator, raw
  `feff0300a4078018`: type 3, `OnceOnly`, `SET`, `RevertEffect`, delay 15,
  target `(2,3)`. The original step on `B4` does not consume it because
  `RevertEffect` reverses the additional event to false. The record must not
  be used for automatic host teleportation.
- ✅ The original OPEN consumer is now code-bound. `$50F6` fetches the map
  byte; `$C240–$C247` requires teleporter type `A0`, and `$C24C–$C251` tests
  `AND #$08`. A closed pad goes to the ordinary movement exit; an open pad
  continues to record decoding at `$C27A`.
- ✅ `$C27A–$C2D8` fetches destination level, X, and Y directly from the real
  six-byte record and copies the coordinates with the original
  `TII $20B4,$2040,$0002`. Firestaff's Track 02 path now follows the same
  contract: coordinate destinations need no fabricated endpoint object, but
  a destination that is itself a teleporter requires its real category-1
  record.
- ✅ The full corpus contains 170 open teleporters in each region. Ten US and
  ten JP links land directly on a closed teleporter. These cases previously
  chained onward and invalidated the whole move; the chain now terminates at
  the passable destination, matching the original's repeated `$C24C` gate.
  All twenty real links run through the runtime resolver in the regression test.
- ✅ The save/load regression now also selects a real open teleporter in each
  of the fourteen US/JP dungeon banks. The raw six-byte record, OPEN state,
  separate scope, coordinate link, and source map's OPEN bit must survive,
  and the restored world must produce the same resolver destination. The test
  preloads the same authentic Track 02 world as production; it creates no
  replacement map.
- ✅ The JP Rev. 1 Akutuba `(0,0)` regression now separately pins its
  authenticated Track 02 record to M0 `(2,3)` and moves a real-roster party
  there through Firestaff's original-command host path. This agrees with the
  isolated original Akutuba route captured on 2026-08-21; it does not assert
  JP-specific metadata consumers or derive movement from the latest GUI
  screenshots. The authentic JP dungeon-loader CTest passed three consecutive
  runs on trv2.
- ✅ The same regression also checks the authentic JP landing neighborhood
  around `(2,3)` against the map tiles used by the original US movement
  capture. A trial that applied US post-landing command outcomes as JP
  expectations failed, so that cross-edition behavioral assertion was
  removed; the failure is not treated as a JP retail semantic difference.
- 🔒 A research mutation of only Akutuba's real `$0E8AE8` from `B4` to `B8`
  made the original use the existing record's map-1 destination `(6,0)` and
  then read the new map's data. It was used only to try to reach the button
  door. Direct pose/cache mutations produced conflicting tile pointers and
  are therefore not published as door or actuator semantics.
- 🔒 The exact physical original address of the transition record is now
  identified as `$0E9591`: raw record `05 0c 06 f0 00 01` is decoded by the
  original as map 1, destination `(6,0)`. The research instrumentation
  changed only the record's coordinate and rotation bits. The original
  teleport routine then produced consistent states at `(2,3)` facing north
  and `(2,1)` facing south, without direct pose or cache patches.
- ✅ The real US Track 02 map verifies the button door at Akutuba map 1
  `(2,2)` as map byte `9c`. Its real category-0 record is
  `fe ff e0 01`: wood, opens upward, has a button, and is destructible and
  bashable. This is source-data and placement evidence, not yet runtime
  mutation evidence.
- 🔒 The original UI trace in front of the door shows authentic hover commands
  and hit points, including `$65` at `$78/$38`. Reproduced button presses,
  however, either reached no command consumer or other UI commands (`$03`,
  `$6c`, `$7d`); no run changed the door record.
  Firestaff therefore keeps source-bound doors fail-closed until the real
  button consumer and its exact state transition have been captured.

# 2026-08-21 Track 02 pits' real OPEN gate

- ✅ Original movement code `$C332–$C347` classifies map type `$40` as a pit
  and enters the fall route only when levitation is zero, `OPEN` (`$08`) is
  set, and `IMAGINARY` (`$01`) is zero. A closed, imaginary, or levitated pit
  continues without a fall. Both the movement query and mutating runtime path
  now use this gate.
- ✅ The complete real corpus verifies 165 closed/passable and 132
  open/fail-closed US pits, and 166 closed/passable and 130 open/fail-closed
  JP pits. At least one real movement outcome of each kind is run per
  available dungeon bank. The same open original tile is also tested with
  levitation and must then be passable, as specified by the `$AB` gate. No
  constructed pit tiles or fallback maps are used.
- 🔒 An open, real pit is still blocked before damage and level fall. The
  original gate is proven, but the T700 stat mutation and destination
  transition consumer are not. The fixture pit's older damage behavior is
  therefore not used on a source-verified level.

# 2026-08-21 Live startup and Continue preserve authentic dungeon data

- ✅ Production startup for verified US and JP raw BINs uses the same
  full-dungeon loader as Track 02 transitions. The boot test now checks the
  running world, not just a loader receipt: Akutuba must have four loaded
  levels, four verified source headers, and non-empty preserved source tiles.
  Both authentic regional startups pass.
- ✅ Continue no longer resets `level_loaded` and thereby makes already
  decoded Track 02 maps inaccessible. With a verified boot profile, the saved
  dungeon bank is loaded atomically from the exact same Track 02 file before
  the restored world is published. The save contributes only progression and
  party state; it creates or serializes no replacement geometry.
- ✅ If Track 02 cannot be read or verified, or does not contain the saved
  level, Continue fails without partially overwriting the live world.
  Synthetic fallback dungeons remain disabled.

# 2026-08-21 Authentic actuator corpus

- ✅ All seven dungeon banks in both US and JP are now censused directly from
  their authentic category-3 records. The regions contain the same 1,109
  placed actuators, including 393 floor type 3 (party), 46 floor type 6
  (monster generator), and 57 wall type 127 (champion mirror). 286 records
  have `OnceOnly`, 116 have `LocalEffect`, and 70 have `RevertEffect`. The
  four effects SET/CLEAR/TOGGLE/HOLD are distributed 632/158/103/216. Type,
  effect, and total counts are regression-locked against real data.
- 🔒 The census does not authorize target mutation. The original step did not
  consume Akutuba's real floor type-3 actuator on the closed `B4` teleporter
  tile. On addition, `RevertEffect` makes the condition false; on normal
  removal, F0276's occupancy check returns before inversion. Effect/target
  dispatch must still be bound before `SOURCE_ACTUATOR` may change doors,
  generators, or its own flags.
- ✅ The standalone category-3 decoder test no longer uses a constructed
  actuator. It was replaced with the exact US/JP record at Akutuba M0
  `(2,1)`, source reference `0c05`, index 5, raw bytes `feff0300a4078018`.
  The test locks the full decode: type 3, `OnceOnly`, `SET`, `RevertEffect`,
  delay 15, no `LocalEffect`, and target `(2,3)`.
- ✅ The same test now undefines `NDEBUG` before `<assert.h>`. The Release
  build previously removed both checks and loader calls that were inside
  `assert`, continued with uninitialized data, and crashed. The real Track 02
  decoder test now actually runs and passes.

# 2026-08-21 Real projectile and cloud pools

- ✅ The Track 02 reader's second item stream previously stopped incorrectly
  after category 10, even though the authentic category-14 and -15 tables
  follow categories 11–13, which have zero record width. The reader now
  processes all 16 categories. In US Akutuba, the 60 projectile records and
  50 cloud records exactly fill the final 680 bytes before the dungeon-text
  source offset.
- ✅ All seven real US and JP banks must now contain and decode exactly 60
  projectile records and 50 cloud records. Each record's raw payload is
  checked as non-empty and passed through the portable source decoder. The
  previously constructed decoder records are replaced with the real first US
  Akutuba records `ffff101001000000` and `ffff2e0e`, also compared directly
  with the tables read from `TQUS02.bin`.
- 🔒 The tables are authentically loaded and decoded, but are dynamic record
  pools, not map placements. No projectile or effect is created without the
  original allocation, timer, and hit consumer.

# 2026-08-21 Real monster and party-actuator contexts

- ✅ The standalone category-4 test no longer uses a constructed monster
  record. It has been replaced with US Akutuba index 0,
  `fefffeff0a013b002500240025002004`, and is compared directly with the
  authentic Track 02 table before `chested`, type, position, four health
  words, flags, group size, and direction are checked.
- ✅ All 393 real floor type-3 records in each region are now classified by
  their actual map tile. 210 are on floor, 92 on pits, 8 on stairs, and 83 on
  teleporters; of the teleporters, 7 are open and 76 closed. All have thing
  position 0. The SET/CLEAR/TOGGLE/HOLD effect distribution 190/66/40/97,
  91 `OnceOnly`, 12 `LocalEffect`, and 19 `RevertEffect` are also
  regression-locked and identical in US/JP.
- 🔒 This context disproves a general host rule that a closed teleporter by
  itself blocks a party actuator. Therefore, the B4 case cannot open or close
  type-3 dispatch without the original event condition.

# 2026-08-21 Real event gate for type-3 records

- ✅ The actuator word layout now follows the original's shared DM/CSB
  format: two-bit `Effect` in bits 3–4, separate `RevertEffect` in bit 5, and
  `LocalEffect` in bit 11. The previous three-bit interpretation invented
  effects 4–7 and incorrectly called `LocalEffect` `inactive`.
- ✅ The F0276 gate exists as a pure Theron function before target dispatch.
  It checks for a live party, addition/removal, direction when the record's
  value is 1–4, `RevertEffect`, HOLD resolution to SET/CLEAR, and that
  `OnceOnly` may be consumed only after actual dispatch.
- ✅ The regression uses the authentic Akutuba record above. The party
  stepping onto the tile causes no dispatch and does not consume the record.
  Normal removal is also silent because `partySquare=1` is stopped by the
  occupancy check before `RevertEffect`. Only a call context that explicitly
  reports an empty tile can invert removal into SET. An empty party is stopped
  before inversion, as in the original. Tests also force assertions on in
  Release mode so real Track 02 calls cannot be optimized away.
- ✅ The gate does not mutate the target directly. Movement events go through
  the serializable queue below, and target types whose original consumer is
  now bound are applied only on the correct tick.

# 2026-08-21 Authentic actuator events in the live world

- ✅ Ordinary party movement now publishes F0365/F0276 events in the
  original order: removal from the old tile before addition to the new one.
  The direction check uses the party's actual facing, not the absolute travel
  direction of the last step. The full queue is checked before movement, so a
  full queue leaves both position and actuators untouched.
- ✅ The event preserves dungeon, level, source reference and index, source
  tile, remote target or local tile, resolved SET/CLEAR/TOGGLE effect, sound
  flag, delay, and absolute `due_tick`. `OnceOnly` is cleared only after
  F0276 has actually published an event. The raw source record is unchanged.
- ✅ The queue is included in the world hash and save version 15. Full
  round-trip, a one-byte-short tail, atomic deserialize failure, and version
  13 compatibility pass. Older saves get an empty queue; real source objects
  are still loaded from Track 02.
- ✅ Akutuba's authentic B4 route is tested through the real movement
  function. Both stepping onto it and the normal step off are silent and leave
  its `OnceOnly` record active, matching F0276's ordering. A separate
  authentic direction record in Formicia, raw `feff830080008029`, is queued
  when the party turns to the correct direction and locks the real publication
  path.
- ✅ The stair path now runs the addition pass on the entered stair tile
  before changing levels. The authentic group in dungeon 2, map 4, `(0,6)`
  queues its three simultaneous SET records in source order: `0c0a`, `0c3a`,
  `0c39`. No fabricated destination event runs after the level transition.
- ✅ The actuator's final word is now also decoded according to its local
  union. Remote records retain cell/X/Y; `LocalEffect` retains the full
  12-bit `Multiple` value. A later local consumer therefore need not recreate
  information from target coordinates. The separate category-3 interface's
  9-bit Data field was also widened from eight to nine bits.
- ✅ All 393 real floor-party records per region are classified before
  dispatch. Twelve are local and have exactly `Multiple=176`. The 381 remote
  records hit all eight raw map families. A shared Thieves record at M0
  `(5,5)`, `feff030078154050`, points outside the map to `(1,10)` and must
  therefore remain fail-closed. US/JP distributions are regression-locked.
- ✅ The queue is now permission to change authentic category-0 doors and
  category-1 teleporters when the full source identity still matches. Before
  mutation, the consumer verifies the raw category-3 record, dungeon, level,
  source tile, ref/index, target, effect, delay, and local metadata. Sarmon's
  real `0cd6`, raw `feff030040074068`, opens the teleporter at `(1,13)` only
  after delay 14. Drator's real `0c04`, raw `feff0300d0008018`, resolves
  TOGGLE to opening and animates the door at `(2,3)` from state 0 to 4 over
  four ticks after delay 1.
- ✅ Both target tests verify that Track 02 source bytes remain unchanged.
  The teleporter's mutable OPEN bit belongs to its authentic live object; the
  door animation state similarly belongs to the category-0 object and follows
  the original SET=close/CLEAR=open ordering.
- ✅ Pits and fakewalls now use a sparse runtime layer over the unchanged
  Track 02 source map. Only each respective OPEN bit may differ: `$08` for a
  pit and `$04` for a fakewall. The layer is included in the world hash and
  save version 17 together with the sparse object word below; versions 15/16
  are read with their older empty tails and version 13 tests still pass.
  Dungeon reload clears only runtime records for the reloaded bank.
- ✅ Sarmon's authentic fakewall record `0c9b`, raw `feff030090004068`,
  toggles target `(1,13)` from `$c0` to `$c4` after one tick. The world's tile
  query then sees passable floor while `source_tiles` still contains `$c0`.
  The same real world is saved and restored with the sparse record intact.
- ✅ Sarmon's authentic pit record `0cea`, raw `eb0c0300c8078080`, runs
  CLEAR after delay 15 and changes target `(2,16)` from runtime `$48` to
  `$40`. The movement query sees the closed pit as passable without
  overwriting source byte `$48`. Drator's stair group also confirms that SET
  on two already-open pits is idempotent before the level transition.
- ✅ All remote targets are now also classified against their authentic thing
  chains. In US, 186 of 201 wall targets and 86 of 114 corridor targets have
  no thing at all; the corresponding JP counts are 185 and 86. Exact category
  outcomes for all eight map families are regression-locked. Empty
  wall/corridor events are now consumed as the original F0248/F0245 no-op
  rather than filling the queue. Drator's stair group tests the path with its
  empty wall target `(8,1)`.
- ✅ The 57 wall/corridor hits on category 3 are also classified by actual
  actuator type and are identical in US/JP. Wall types 0/1/2/3 have four each,
  type 4 has three, type 5 has sixteen, and type 15 has one. Corridor types
  1/2 have one each, type 3 has eight, type 6 has seven, and type 7 has four.
  Thus exactly seven corridor generators need F0245 group materialization
  and reactivation; the others must not incorrectly use the generator path.
- ✅ The F0248 consumer now retains only wall types actually handled by the
  original: gate/countdown, launcher types, and endgame. In the real target
  corpus this means sixteen type-5 gates and one type-15 launcher. The twenty
  hits on wall types 0–4 are consumed as authentic no-ops instead of remaining
  queued and filling the queue.
- ✅ Category-2 text now has a sparse, serializable runtime word that may
  differ from the raw record only in the Visible bit. The complete US/JP
  corpus has exactly one remote corridor hit on text: Sarmon's `0c41`, raw
  `feff030000004048`, sends SET to text `0803`, raw `feff0902`. The text word
  is already `$0209` and Visible is already 1, so the authentic effect is
  idempotent and leaves no fabricated runtime value.
- ✅ Wall type 5 now uses the original F0730 AND/OR gate: the incoming cell
  bit is mutated by SET/CLEAR/TOGGLE, the low nibble is compared with the
  reference nibble, `RevertEffect`, HOLD, and `OnceOnly` are respected, and a
  hit publishes the gate's own delayed successor. Sarmon's authentic chain
  from `0ca7` at `(12,11)` to gate `cc78` at `(5,3)` is byte- and identity-
  locked; it forwards CLEAR to `(8,6)`. Gate words and queue survive
  save/load deterministically without overwriting the Track 02 raw record.
- ✅ The only remote hit on wall type 15 is Shadodan source `0c59` to
  launcher `4c65`: the event addresses cell 0 while the launcher is in cell 1.
  F0248's required cell match is now bound and regression-tested, so the chain
  is consumed as an authentic no-op and creates no projectile.
- ✅ All twelve local floor records have `Multiple=176`. F0270 preserves the
  encoded value, but F0271 mutates only for CLEAR 1 and TOGGLE 2; the XP path
  is the separate value 10. All twelve real events now run through the
  consumer and are consumed without mutation, matching the original default.
- 🔒 Corridor generators remain queued until their mutable category-3 and
  group/RNG consumers are source-bound. Events in these real chains are
  retained. The seven real hits are now identified: four target Drator's
  `0c38`, and one each targets Formicia's `0c57`, Thieves' `0c70`, and Demon's
  `0c64`. Six have fixed generation field 1; Formicia's has 12 and requires
  the original randomized group count. All also need the still-uncaptured
  `$4644/$4667` return for direction and HP. Thieves' invalid target remains
  fail-closed with no side effects.
- ✅ Production-adjacent regression tests no longer require synthetic combat,
  spells, locked doors, altars, or hunger/thirst drain when their authentic
  consumers are missing. Instead they verify fail-closed behavior and that
  world state is not mutated. `theron_v1_combat_mechanics`,
  `theron_v1_cross_route_mechanics`, and `theron_v1_srm_classifier` pass. The
  SRM test requires a dated status marker without pinning an old date. The
  explicitly linked champion and startup probes also regained their missing
  real source modules and build again without relinking fixture tables into
  the production archive.
- ✅ The generators' next source boundary is now byte- and state-locked. Four
  independent authentic 8 KiB RAM images have the same `$28b9-$28bb` state
  `1a 62 29`; this is exactly seed 0 after two `$4667` steps, and the next real
  return is `$9d`. The seven reachable generator hits also verify the full
  raw records for Drator `0c38`, Formicia `0c57`, Thieves `0c70`, and Demon
  `0c64`. A source-bound plan distinguishes the six fixed one-member groups
  from Formicia's randomized 1–4 with bound 4, but still publishes no
  direction or HP until the original's complete RNG call order is bound.
- ✅ The Mednafen trace now locks each real RNG call at physical
  `$0d0667/$0d067e`, with the return address read from HuC6280 stack page
  `$2100`. An authentic state autoload produced 26 consecutive entry/return
  pairs: return owner `$4647` five times, `$464d` nine times, and `$cca4`
  twelve times. Each return state is exactly the next entry state. The capture
  script validates the chain and correctly handles a missing optional
  command-consumer file.
- ✅ All seven real corridor-generator events can now be resolved uniquely
  to their authentic Track 02 generator records in the production world. The
  resolver verifies the event's eight-byte record, effect, delay, sound, and
  target coordinates, and returns only source identity. The event remains
  queued; RNG, timer, HP, direction, and monsters are not yet published. The
  regression also locks the real event coordinates and raw records for
  Drator, Formicia, Thieves, and Demon.
- ✅ RNG capture format v2 now records the physical return owner and the
  original `$B0/$B1/$B3-$B6/$B8/$BA/$BB`. A real smoke capture produced 18
  complete pairs and bound `$4647→$0d0647`, `$464d→$0d064d`, and
  `$cca4→$0d8ca4`. This provides sufficient context for the next event-bound
  capture but does not yet claim that this run triggered one of the seven
  generators.
- ✅ The original post-dungeon dispatcher has now been run against the
  authentic US disc and the same real savestate for ordinals 0–5. Campaign
  byte `$267c` becomes exactly `01, 02, 04, 08, 10, 20`; Drator is therefore
  bound by original evidence to bit 1, without a fabricated progression
  table. Ordinal 6 uses the special `CMP #$06` branch and is not published as
  a dungeon bit. These runs do not yet load Drator's bank, so generator
  RNG/spawn remains correctly closed.
- ✅ A known Track 02 identity is no longer sufficient to admit a fabricated
  byte buffer. Runtime requires the entire raw medium to match the declared
  MD5; any mismatch blocks both level publication and fallback graphics.
  The real `TQUS02.bin` test also passes for the source map, objects, four
  levels, authenticated spawn source, and seven dungeon-owned name tables.
- ✅ Startup-flow and save/resume tests now run their seven dungeon paths
  against the real `~/.firestaff/data/theron/TQUS02.bin` rather than allowing
  a fabricated Track 02 buffer to stand in for verified media. A false MD5
  declaration is tested separately and must fail identity checks without
  publishing a level or fallback graphics.
- ✅ The full CTest-labeled Theron suite builds and passes: 53 of 53 tests.
  Two Theron viewport probes now link the original Track 02 font module,
  three standalone asset-scanner tests link the complete Theron archive, and
  the transition-receipt test runs its assertions in `NDEBUG` builds instead
  of crashing after side effects were compiled out.
- ✅ The generator path now has an explicit event-bound runtime evidence
  receipt. It matches both the category-3 event's and type-6 generator's exact
  eight-byte records against the real world's Track 02 index and retains the
  source-bound plan. All seven authentic US events reach the source join; a
  complete receipt verifies only the captured Drator event's runtime witness
  and does not authorize Firestaff-native creature materialization.
  Same-run evidence of the `$4644/$4667` return, physical call owner, and
  generator consumer is required for witness verification. Smoke traces and
  modified generator bytes are rejected.
- ✅ The RNG producer now saves the full authentic 8 KiB main RAM and 32
  bank-mapped instruction bytes around every physical return owner. A real
  Drator run produced 156 complete contexts and bound the previously unseen
  owners `$CC33`, `$CC55`, and `$DA5A` to their exact calls, masks, and
  subsequent writes. UP/RIGHT probes did not change the RNG sequence, so
  these data provide stronger original evidence but do not yet enable
  generator materialization.
- ✅ Bytes 6–8 of the generated Drator row are now bound to the original's
  authenticated Track 02 code at logical `$C852`, raw offset `$A1612`. The
  exact 27 instruction bytes show the copy to `$B5/$B6/$B4`, the two-bit mask
  to `$BB`, and the final call to `$51F8`. Because consumption of this row was
  not observed in the same session, its bytes remain opaque in the
  materialization receipt and are not published as coordinates or direction.
  Firestaff therefore creates no host monster from them. Type, local
  coordinates, HP, and timer remain closed.
- ✅ A new authentic bank-dispatch trace shows that the call to `$5D58` after
  row construction goes via `$45E3` to physical `$0E0AF5`. The target routine
  updates spatial boundary and pointer tables but provides no evidence for
  type, HP, or timer. A separate bounded instruction trace also captures an
  earlier logical `$C852` in the other physical bank `$0DC852`. Logical
  addresses therefore cannot be conflated across banks; the position receipt
  remains explicitly static-source evidence until consumption of the same
  row is observed.
- ✅ A focused original trace from physical `$0D07B7` now captures the first
  generated row's full lifecycle within 967 instructions: construction start,
  sorting, final toughness write, copy, publication, first read, and unlink.
  The production receipt requires the eight exact logical and physical PC
  addresses and their sequence numbers; a changed sequence number is
  rejected. This binds group writing to one authentic run but still does not
  establish host type, local map position, or HP.
- ✅ The category-4 monster layout is corrected to DMBUILDER's complete
  `ITEMS` contract: `item.c:getItem()` skips a generic two-byte link before
  the 14-byte `dm_monster` payload. Firestaff now reads `next_ref` at byte 0,
  `chested` at byte 2, type/position at bytes 4/5, HP at bytes 6–13, and the
  flag/direction word at byte 14. Three authentic following objects recur in
  the US census (`640/2189`). The seven strings AKUTUBA…DEMON are also
  reclassified as dungeon labels; static groups retain their real raw type
  value without a `+1` rewrite or a false spawn category.
- ✅ The alternate source-occurrence path in
  `theron_v1_combat_runtime_source.c` now follows the same contract. It
  publishes the category-4 record's raw type byte unchanged and leaves spawn
  category `$FF` without a verified runtime join. The regression requires an
  authenticated category-3 witness to be rejected when only the old
  type-0→zone-0 mapping would have bound it.
- ✅ Firestaff's portable world snapshot version 18 now preserves the full
  authentic category-4 ledger. This is the `theron_v1_world_serialize` format
  for byte-exact world round trips in the library and tests, not the
  original's production format `slotN.tqsv`; the separate Continue path first
  loads Track 02 and then applies its between-dungeon save. The snapshot
  therefore preserves more than just monsters that happened to be alive on
  the current level when the world was serialized.
  Each 55-byte record stores source identity, dungeon/level/tile,
  `chested`, raw type/position/count/direction, HP words, and the exact
  16-byte record. Deserialization checks bounds and raw size atomically. A
  real-data test serializes Akutuba's real US ledger, clears the destination
  copy, reads every field back byte-for-byte, and then switches to another
  real level where the live group is rebuilt from the saved ledger.
  Version 18 also rejects a save if decoded type, position, `chested`, count,
  direction, HP, or flags contradict the accompanying 16-byte record, or if
  the same dungeon/level/source-ref/source-index occurs twice; such a late
  failure leaves the active world unchanged. All 165 monster groups in the
  real US campaign also pass the same save/deserialize/level-switch regression.
  Save versions 1–17, which lack this section, retain the hash-verified ledger
  already loaded from Track 02 by Continue; version 18 atomically replaces it
  from the snapshot.
- ✅ The original's complete `DMS-SG.001` record is now bound from real US
  Track 02 code and authentic PC Engine Backup RAM. HUBM size `$01A9`, the
  `$E04E` read, and `$E051` write prove a `$0199`-byte data area with three
  `$88`-byte slots and the original selected-slot index. Shared code binds the
  index through `$0198 → $278C → INY → $42B8 → {0000,0088,0110}`; a new
  authentic run also produced `$278C=00/$42B8=01` and reproduced BRAM hash
  `ffabc8…`. Firestaff therefore rejects indices outside 0–2 and decodes only
  the selected slot. Firestaff preserves all 409 bytes, and the real-data test
  cross-binds the original instructions, record hash `0ce6b7ba`, the
  previously proven `$86`-byte writer, and main memory from the same session.
  Only campaign bytes have gameplay semantics; other fields remain closed.
- ✅ The complete real-data regression for the original Backup RAM was run
  separately against installed `TQUS02.bin`, `TQJP02.bin`, the authentic
  Akutuba `.bram` file, the same session's 8 KiB main RAM, and the real Save
  Manager code page. It verified the layout, decoded Theron's maximum values
  and all 20 skill values, and cross-bound the three-slot record to the
  original read/write code. The run passed without modifying game data. It
  still does not show that a real gzip-based Save Disk `.srm` can be imported;
  that format is separate and remains open work.
# ✅ 2026-09-30 Theron public status corrected to match evidence

`docs/FIRESTAFF_GAP_LIST.md` no longer marks the rendering pipeline,
mechanics, or seven-dungeon progression as fixed. These rows now distinguish
the passing bounded data/runtime tests from unverified original presentation
and the remaining stair/exit, pickup/use, combat, and chapter-completion
consumer gaps. `TODO-theron.md` retains those gaps as open work.

Verification: documentation diff review and `git diff --check` passed. No
runtime behavior changed.

# ✅ 2026-10-02 CUE-selected Theron CDDA handoffs

Added an explicit verified-media handoff for any CUE-declared Theron audio
track while retaining the Track 01 wrapper used by the title lifecycle. The
new handoff still requires a recognized Track 02 edition and the matching
Track 02 CUE entry. Generic CDDA stream entry points now start/pump/stop the
selected handoff; Track 01-named functions remain compatibility wrappers.
This plumbing does not infer gameplay event routing. The optional
authentic-media regression resolves Tracks 03–18 from the operator-supplied
JP Rev. 1 CUE and starts each selected stream through SDL's dummy output,
requiring Vorbis decode and queued audio sectors.

Verification: built `test_theron_v1_track01_cdda_handoff` on `trv2` and ran it
three times against the authentic JP Rev. 1 CUE and original sibling BIN/audio
files; all three runs passed. Each run decodes and queues Tracks 03–18 and
retains the Track 01 stream regression. Gameplay event mapping and automatic
in-game selection remain open in `TODO-theron.md`.

# ✅ 2026-10-02 Authentic Track 02 CD-play candidate census

`test_theron_v1_hw_config cd_play_track` now scans both authentic raw editions
when present. On trv2, the US BIN yielded two heuristic code-region `$E03F`
call-site candidates in sectors 1224 and 3095; the JP BIN yielded candidates
in sectors 1223 and 3094. The preceding-byte scan reports parameter `$0E` for
all four. This is static byte-pattern evidence, not proof that `$E03F` is a
CDDA playback API or that these sites execute for a gameplay event. The test
and hardware summary now call them candidates rather than proven playback.

Verification: built the hardware-configuration test on `trv2` and ran the
filtered census three times against the authentic US and JP Track 02 BINs; all
six region-results matched. Runtime caller, event ownership and actual audio
start remain open in `TODO-theron.md`.

# ✅ 2026-10-02 Original 7z Track 01 audio handoff

M11 now reads the exact US/JP archive CUE and its declared raw Track 01 BIN
after the paired Track 02 edition has been verified by hash, then independently
checks Track 01 against its known regional SHA-256. The bounded memory-backed
CDDA stream queues the original 2352-byte sectors; no game data is extracted or
cached to disk. Other archive variants and unpaired or substituted audio remain
fail-closed.

Verification on trv2: authentic `theron_v1_us_7z_direct_boot` and
`theron_v1_jp_7z_direct_boot` both reported `theronTrack01CddaReady=1`;
`theron_v1_track01_cdda_authentic_archive` queued US and JP original sectors
through both file- and memory-backed stream paths; and
`theron_v1_m11_launcher_handoff_boundary` passed. These checks prove title
Track 01 availability/startup only. Gameplay CDDA selection and a verified
manual emulator save/load round trip remain open in `TODO-theron.md`.

# ✅ 2026-10-03 Regional authentic Track 02 door and teleporter coverage

Split the Track 02 door/teleporter real-media test into independent US and JP
CTest cases. Each case verifies its selected Track 02 BIN hash before loading
all seven dungeon maps. Missing default media now skips only that region; a
provided but missing or wrong-edition override fails instead of silently
falling back to another path. This test change does not establish original
door-opening or teleporter runtime behavior.

Verification on `trv2`: both regional CTests passed three repeated runs using
the installed original US and JP BINs. Valid explicit overrides passed in
three loops without default media; cross-region media and a missing explicit
JP override failed admission; absent default media independently returned
CTest skip code 77 for each region.

# ✅ 2026-10-03 Regional authentic Track 02 dungeon-map verification

Split the map regression into independent US and JP CTests. The JP case now
runs its hash-gated seven-dungeon offset/layout checks and the 170-stair-class
tile census even when no US media is installed. The US case retains support
for the regular raw BIN and an explicitly supplied CloneCD-derived Track 02
BIN, with separate expected hashes. Missing default media skips only the
selected region; explicit unreadable or wrong-edition overrides fail closed.
CTest compilation now undefines `NDEBUG` for this assert-based test so Release
builds retain the checks.

Verification on `trv2`: both CTests passed three repeated runs against the
authentic installed US and JP Track 02 BINs. Explicit US and JP overrides
passed in three loops without default-media discovery; wrong-region JP media,
a missing JP override, and a missing CloneCD override were rejected. With
default media hidden, both selected regions independently returned skip code
77. The built Release test binary references `__assert_fail`, confirming its
assertion checks remain enabled. These data checks do not establish original
stairs, transitions, or later-dungeon runtime behavior.

# ✅ 2026-10-03 Regional authentic Track 02 champion-roster coverage

Split the champion-roster real-media check into independent US and JP CTests.
Each invocation verifies only its selected authentic BIN; missing default
media skips that region, while a non-empty unreadable or wrong-edition override
fails. Assertions are explicitly enabled for Release builds. The JP live-party
source-record binding remains part of the JP invocation, and portrait artwork
or T900 behavior is not inferred.

Verification on `trv2`: both regional tests passed three repeated runs against
the installed authentic US and JP BINs. Valid explicit overrides passed with
an isolated HOME; cross-region and missing explicit paths failed; missing
default paths independently returned CTest skip code 77. The Release test
binary references `__assert_fail`. These receipt and numeric roster checks do
not establish complete Theron champion gameplay parity.

# ✅ 2026-10-03 Regional authentic Track 02 level-descriptor coverage

Split the authentic level-descriptor receipt into independent US and JP
CTest cases. Each selected raw BIN is normalized and authenticated against its
own expected digest; wrong-edition and explicitly unreadable overrides fail,
and only a genuinely absent default path skips that region. Permission and
other I/O failures fail. Media-independent descriptor-table assertions run as
a separate static CTest. Input seeking, allocation, read, and close failures
are checked directly. Assertions remain enabled in Release builds.

Verification on `trv2`: the US and JP tests each passed three repeated runs
against installed authentic media. Explicit authentic overrides passed with
default discovery isolated; wrong-region and missing explicit paths failed;
missing defaults independently returned CTest skip code 77. The Release
binary references `__assert_fail`. This validates descriptor receipts only,
not dungeon loading or gameplay transitions.

# ✅ 2026-10-03 Authenticated regional Track 02 thing-data coverage

Split the thing-data regression into independent static, US BIN, JP BIN, and
opt-in CloneCD raw CTests. Each real-media case authenticates the complete
selected image against its region-specific expected digest before decoding
all seven dungeons. A non-empty explicit media path is authoritative and
cannot fall through to another installed edition. Only missing default BINs
or the unconfigured optional CloneCD input return skip code 77; unreadable,
wrong-region, and other non-missing failures fail closed. The real-media reader
now checks seek, size, overflow, read, and close results, and map tile
flattening is bounded before copying into its fixed buffer. Assertions remain
enabled in Release builds.

Verification on `trv2`: the 14 available roster, spawn, door, map,
level-descriptor, and thing-data CTests passed three repeated loops against
authentic US/JP media. The optional CloneCD CTest correctly skipped because no
explicit CloneCD raw image was configured. Independent missing-default tests
returned 77; missing explicit paths and wrong-region explicit media failed.
This validates source tables and record decoding, not complete original
object semantics or in-game parity.

# ✅ 2026-10-03 Authenticated Theron level-data-block source coverage

Split the level-data-block verification into static, US BIN, JP BIN, US
CloneCD raw, US Track 19 ISO, and JP Track 19 ISO CTests. Each media test
authenticates the exact selected file against its source digest before reading
it. Explicit overrides are authoritative; only absent defaults may skip.
Raw-sector and ISO file reads now validate seek, size, allocation, read, and
close results. Release builds keep the assertion checks enabled. The static
decoder fixture remains an algorithm-boundary test only and does not stand in
for real game data.

Verification on `trv2`: static, US/JP BIN, and US ISO cases each passed three
repeated runs against authentic installed files. CloneCD raw and JP Track 19
ISO cases skipped because their optional source files were unavailable. The
missing-default cases returned 77; explicit-missing and cross-region paths
failed. The Release test binary references `__assert_fail`. This validates
prologue, metadata, and resource-receipt boundaries, not decompressed level
contents or original-game semantics.

# ✅ 2026-10-05 PCE Fast scripted-input reference capture

Added an opt-in bounded replay-input producer to the Mednafen PCE Fast
reference build. It parses at most 32 frame-indexed key events, applies them
only to controller port 0, and writes event/apply receipts to the isolated
capture trace. The live-capture shell regression passed three consecutive
loops. A fresh `-j1` instrumented build on `trv2`, with PCE Fast and the
bounded main-RAM snapshot enabled, completed and advertised both `pce` and
`pce_fast` modules.

Three isolated runs restored the authentic Japanese Rev. 1 Ak-Tu-Ba emulator
state and replayed four directional inputs. The independent receipt verifier
confirmed all four event frames were applied and followed by original
controller-port reads, including four polls outside the System Card wait
addresses. Each overall capture correctly remained blocked at
`transition=missing` because no dynamic CD-to-RAM receipts were present. This
is reference-input instrumentation evidence only; it does not prove that the
gameplay loop consumed the inputs or establish movement, map identity, or
complete Theron support. The original media, System Card, state, and traces
remain outside the repository.

# ✅ 2026-10-03 Theron Japanese gameplay boot and load-menu observation

Using stock Mednafen 1.32.1 with the authentic Japanese Rev. 1 disc and
System Card in a new isolated profile, held RUN for four seconds at the
System Card prompt, started NEW GAME in FILE_1, selected Ak-Tu-Ba, and entered
the real first-person dungeon view. After a graceful emulator exit, the fresh
profile contained a 2-KiB title-specific persistent-RAM image. A second
Mednafen process reused only that SRAM directory and a separate empty
save-state directory. Selecting LOAD GAME, FILE_1, and YES returned to the
Ak-Tu-Ba dungeon-selection map. A later hash audit found the BRAM byte-identical
to the known empty menu-only JP image (`de8e415730226a1f0e39666b1ea291b6abec07bcaeb7223dc33ea01a71f89eaa`),
and the after-load map capture byte-identical to the after-New-Game capture.
Thus the authentic title-to-dungeon route and a load-menu observation are
verified, but no non-empty native campaign save or restored progress is proven.
No BIOS, disc image, SRAM, state file, or screenshot was added to the repository.
The separate F5/F7 reload below is emulator-state evidence, not a native save.

# ✅ 2026-10-03 Theron gameplay save-state reload

With authentic Japanese Rev. 1 media and System Card under stock Mednafen
1.32.1, the fresh isolated `pce_fast` profile held RUN for four seconds and
used Button I (Space; SDL scancode 44) through NEW GAME, FILE_1, Akutuba, and
the original Japanese introduction to reach the first-person dungeon view.
F5 followed by clean emulator shutdown produced a 229,965-byte `.mca` file.
A second isolated `pce_fast` profile loaded the byte-identical state with F7;
after one ordinary W input, the HUD and Akutuba wall/floor view were rendered.
The captured restored frame differs from the original first-person capture in
1,384 of 786,432 pixels. State, screenshots, and media remain outside Git;
the state SHA-256 is `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`.
The accompanying 2-KiB BRAM hash equals the known empty menu-only JP image, so
this proves an emulator gameplay-state reload, not native campaign progress,
party-coordinate/pose parity, or complete Theron gameplay parity.

# ✅ 2026-10-03 Theron regional and original-media regression loops

After syncing to `main` at `501ccdb`, the focused 21-case regional Theron CTest
selection passed three consecutive runs: 18 tests executed against authentic
installed media and passed; optional CloneCD raw and JP Track 19 ISO cases
skipped because those files were not configured. The static cases also passed.
Eleven original-media startup, archive, scanner, and CLI checks then passed
three loops, with only the converted ISO boot and authenticated-capture CLI
cases skipped for unavailable inputs. A preceding first broad-label attempt
was incomplete because the isolated build had omitted auxiliary probe targets;
its missing-binary failures were not counted as code/test passes. The selected
tests do not establish full Theron gameplay parity or close the remaining
retail-mechanics gaps.

# ✅ 2026-10-03 Authenticated teleporter preview and resolver fail-closed gate

On authenticated Track 02 levels, a teleporter map tile no longer receives a
legacy preview result when its source coordinate-link record is absent or
unmarked. The mutating resolver also refuses to reinterpret a packed
coordinate word as a legacy object ID on an authenticated level. Unauthenticated
fixture worlds retain their compatibility path. This is a data-integrity gate,
not a claim about retail handling of damaged or incomplete records.

Verification on `trv2`: the authentic US/JP Track 02 dungeon-loader CTest
passed three consecutive runs. Both regional cases used their real maps and
headers while fault-injecting an absent record and a removed coordinate-link
marker; preview and movement both remained blocked without changing state.
`theron_v1_combat_mechanics` also passed three runs, preserving legacy fixture
coverage. Optional ISO subchecks skipped because those images were not staged.
This does not establish teleporter scope, facing, rotation, sound, or broader
original-runtime semantics.

# ✅ 2026-10-03 Authentic regional stair-attribute source census

Extended the hash-authenticated US/JP Track 02 dungeon-map test to pin the
low-nibble occurrence count for each of the sixteen stair-class attributes
across all seven dungeons. On trv2, an isolated build of the test and map
decoder with Clang passed three consecutive loops for each authentic region.
The US vector totals 171 tiles and the JP vector totals 170. This is source
inventory only; it does not assign direction, destination level, arrival pose,
or stairs runtime semantics. Stair movement remains blocked on authenticated
levels pending the original consumer or a source-bound runtime capture.

# ✅ 2026-10-03 Theron emulator and stair-status evidence reconciliation

Corrected the historical movement entry that described stair traversal as
current parity. The production mechanics path and current real-data probe keep
authentic Track 02 stairs blocked until direction, destination map, and arrival
pose are source-bound. Also corrected the JP load-menu interpretation: its
2-KiB BRAM matches the known empty menu-only image, so native campaign-save
persistence and restored progress are not proven. The separate F5/F7 reload is
emulator-state evidence, not a native save. The US cold-start note remains
limited to that specific attempt.

Verification on `trv2`: a fresh Release build from commit `9ac13263d` completed
the `firestaff_theron_v1_mechanics_playability_probe` target with `-j1`. The
`theron_v1_mechanics_playability` CTest passed three consecutive runs against
the authentic US and JP Track 02 BINs. Each region reported 215 passes, zero
failures, and zero skips; its 39 US and 42 JP approachable stair cells remained
transactionally blocked. This verifies the documented fail-closed boundary,
not retail stair traversal semantics.

# ✅ 2026-10-05 Keep static `$E03F` candidates separate from CDDA track semantics

Renamed the Track 02 `$E03F` byte-pattern census API and its receipt so they no
longer claim to extract a CDDA track map. The nearby `LDA #$0E` / `STA $FF`
pattern remains available as raw candidate evidence only. The source-locked US
stage-2 disassembly shows that byte sequence in a caller, but does not establish
the meaning of `$FF`, gameplay-event ownership, or a runtime audio selection.
Gameplay audio dispatch remains disabled pending same-session caller, playback,
and source-byte evidence.

Verification: `test_theron_v1_hw_config` passed three loops on `trv2` against
the authentic US and JP Track 02 BINs; the test still counts source candidates
and false positives without assigning track semantics. No synthetic media was
used.

# ✅ 2026-10-06 Initial three-sector JP Tier-5 prefix audit (superseded scope)

Verified authentic US and JP Track 02 image hashes and extracted the first
three JP stage-two sectors (6,144 user bytes; SHA-256
`e955531cc2c7bab6c910302ba4966ff12308c856fc37874529866ee2fbc92e0c`). This
prefix scan found no encoded direct JSR or HuC6280 BSR target for the seven
US Tier-5 addresses. It was not the complete stage-two payload and did not
exclude dispatch-table/indirect roots elsewhere. The same-address JP and US
`$52a2..$52c8` bytes differ, but that comparison alone does not establish
routine boundaries or regional parity. The scope and conclusions were
corrected by the complete 17-sector audit below. No synthetic media was used.

# ✅ 2026-10-06 Bind the JP 17-sector record-selector static path

Extracted all 17 JP stage-two sectors (34,816 user bytes) from authenticated
`TQJP02.bin` in three repeated passes. Every pass produced SHA-256
`bc9ff3922dad71f2cd24afeaaf09747c64db15245eb7f9c454dbb76bd4e12d67`; the
source image SHA-256 is
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`. An
independent audit confirmed the payload hash and all expected bytes against
the authentic image.

Added source-locked MAME HuC6280 disassembly and the optional-authentic-JP
test `stage2_jp_record_selector_52xx_flow`. It binds eighteen windows spanning
`$4fea..$506c`, `$508b..$5097`, `$50f5..$5111`, `$5141..$51c0`,
`$5208..$5237`, `$525e..$53d8`, `$5555..$5609`, `$5669..$5670`,
`$5685..$56e1`, `$5800..$5960`, `$5966..$5984`, `$5984..$5989`, and
`$5989..$599e`. The test locks all 14 pointers in `$5810..$582c` and checks
the table-selected entry bodies' static JSR/BSR targets. The pointer at
`$5814` is `$5895`, selected by the indexed dispatch when the runtime
selector is 2. Static flow then reaches `$58d2: JSR $4fea`, `$505a: JSR
$50f5`, and `$5101: JSR $525e`.

Corrected the address interpretation: `$52a2` is the high operand byte of
`JSR $567a` at `$52a0`, not a JP instruction entry; execution continues at
`$52a3: JSR $5669`. Runtime selector/branch values, helper semantics,
gameplay behavior, and regional parity remain unproven. No synthetic media
was used.

2026-10-07 follow-up: corrected the earlier `$524e..$527d` range, which came
from a raw-sector skip that was 512 bytes too far into the authentic image.
The correctly mapped JP bytes `$524e..$525d` are `E6 0F 60 18 A5 0E 69 40 85
0E 90 02 E6 0F 60`; HuC6280 decoding confirms the `$5258` BCC target is
`$525c`. The authentic US same-address bytes differ, so no regional behavior
is inferred. Two purported BSR edges at `$5260` and `$526a` were removed;
those addresses are operands/instructions in the separate `$525e` transfer
decoder, not BSR sites. Corrected JP/US hashes and the listing are recorded
in `docs/source-lock/theron-disassembly/theron-jp-stage2-record-selector-52xx-huc6280.asm`.

Verification: rebuilt `test_theron_v1_stage2_disassembly_chain` from the
current full source tree on `trv2` with `--parallel 1`, then ran
`FIRESTAFF_THERON_TEST_JP_STAGE2_SELECTOR_ONLY` three times against the
authentic JP Track 02; all passed. This is static source-byte and edge
coverage only. Runtime selector values, branch outcomes, helper semantics,
gameplay behavior, and regional parity remain open.

2026-10-07 follow-up: added an explicit edge assertion for the JP `BNE` at
`$528c`, which statically targets `$5292`. MAME `unidasm -arch h6280` decodes
the authentic JP bytes as `D0 04`; the runtime branch outcome remains
unobserved. The current full-source test target rebuilt on `trv2` with
`--parallel 1` and passed three loops against the SHA-256-attested authentic
JP Track 02. This adds static control-flow evidence only, not gameplay or
regional parity.

# ✅ 2026-10-06 Audit JP `$3114` static call-edge coverage wording

An independent read-only audit re-extracted all 16 newly bound JP helper
spans from authentic `TQJP02.bin`, checked the corresponding US same-offset
hashes, and independently disassembled the JP windows with MAME HuC6280.
Listed instruction bytes and static branch/BSR targets matched the source.
The non-BIOS static call graph rooted at `$31b3` is covered by those spans
and the three earlier bound targets `$5237`, `$5251`, and `$551a`; `$5ceb` is
inside the bound `$5ca7` window, and `$e063` remains the BIOS boundary.

Corrected the listing to state that `$53e8` branches directly to `$543e`,
while nested BSR edges reach `$547d` and `$5498`. This establishes static
byte and edge coverage only, not dynamic indirect calls, helper semantics,
runtime behavior, or Theron's Quest gameplay parity. No synthetic media was
used.
