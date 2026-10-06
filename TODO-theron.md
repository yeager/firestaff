# Firestaff TODO — Theron's Quest

Reviewed 2026-10-06. Only open work is listed here.

## 2026-10-06 — qualify the JP saved-state probe

- 🔒 A bounded replay on `trv2` loaded the previously captured JP Akutuba
  dungeon state under the original JP Rev. 1 CUE and SysCard 3.0, using the
  corrected PCE Fast instrumented binary. The autoload receipt confirmed the
  state load. Four logical `$4ed4` observations had physical PC `$000d0ed4`
  and bytes `0d 8a 2d`, not the authenticated `$20 $2e $3a` call at the
  `$00100ed4` stage-two mapping. No `$4ec9` lookup or `$3ab7` execution was
  recorded. This state therefore does not cover the desired helper path; do
  not treat a logical-PC-only hook hit as stage-two execution. Repeat with a
  contemporaneous screen/state receipt and require the physical mapping plus
  source-byte signature before interpreting any helper trace. The candidate
  probe now records MPR2 as well as MPR1; the rebuilt object and emulator
  reproduced the same four nonmatching JP observations in a second bounded
  run, with the added bank field visible in the trace.

## 2026-10-06 — characterize the stage-two helper's behavior

- 🔒 Authentic US and JP cold-start traces agree on selector roots `$00..$04`
  (`$4d86`, `$4dc5`, `$4e04`, `$4e43`, `$4e82`), now byte-locked against both
  original Track 02 images. Root counts vary with key/runtime. The remaining
  US sample recorded 60 successful searches (final candidate matched key,
  carry clear) and no miss/exhaustion case; key/count/result semantics remain
  unnamed. Earlier probe rows that read PCE Fast X/Y/P through stale
  `HuCPU.X/Y/P` fields are invalid; the hook now records the active interpreter
  locals. A strict receipt showing a scripted event mask consumed after the
  final replay event is still missing; current traces do not establish a
  gameplay state change.

- 🔒 `$4ec9` is the function entry; authentic stage-two bytes place the
  `JSR $3a2e` call-site at `$4ed4`. The opt-in `pce_fast` probe now targets
  `$4ed4`, links only the immediately following `$3a2e` instruction, and
  records caller/target physical PCs and MPR1. Both call and linked-target
  rows have bounded budgets. The helper lives outside the opcode loop to
  avoid C++ goto-over-initialization errors; trace-file discovery is
  initialized once. The corrected PCE Fast object and executable compiled and
  relinked on `trv2`; the complete updated patch chain passed three loops.
  Three earlier captures replaying the authentic JP Akutuba F5 state each
  recorded eleven controller events followed by controller reads at `$44c1`,
  zero System-Card poll reads, `controller_poll_boundary=verified`, and
  `game_or_non_system_card_poll_boundary=observed`. Those runs probed the
  function entry and did not identify the actual call-site execution.
- ✅ Authentic JP and US cold-start traces each recorded 32 linked
  `$4ed4 → $3a2e` executions. Both mapped the target through MPR1 `$f8` to
  `$1f1a2e`; caller PC `$4ed4` mapped to `$00100ed4`, and the call bytes were
  `20 2e 3a`. Each target's first 64 bytes matched its same-session 8 KiB
  BaseRAM snapshot at offset `$1a2e`; the US and JP target windows also match
  each other byte-for-byte. These prove regional parity only for the tested
  cold-start route. The authentic-media Track 02 byte test passed three CTest
  loops on `trv2`.
- 🔒 The linked captures and bounded `$3a2e..$3a9f` byte/dataflow decode do
  not assign game meanings to the records or copied fields, nor prove how
  every caller uses the result. The strict scripted-input receipts for both
  captures blocked because the final event had no later controller read
  exposing its mask; do not claim these replays changed game state.
- ✅ Four authentic JP cold-starts with original Track 02 and System Card
  reached `$40dc`. The first 32-row logger saturated on repeated ID `$00` /
  target `$41c5` at `MPR2=$80`; a transition-filtered probe then preserved the
  bounded output and showed repeated ID `$00` transitions between target
  `$41c5` at `MPR2=$80` and `$ba9c` at `MPR2=$82`. Both captures applied all
  three requested replay events, but the strict input receipt blocked because
  the final event had no later controller read with its scripted mask. These
  rows establish dispatch-point observations, not handler completion or
  gameplay effects.
- ✅ The transition trace exposed and corrected an earlier target-physical-address
  calculation: it used `MPR2` for targets outside that logical segment. The
  current patch derives the target bank from `MPR[target >> 13]` and records
  table physical address plus entry bytes. The complete patch-only chain and
  PCE Fast object compile/relink pass on `trv2`; the authentic capture with
  the corrected bank calculation confirmed `$41c5` through MPR `$80` at
  `$001001c5`, and `$ba9c` through MPR `$6a` at `$000d5a9c`; raw table bytes
  were `c5 41` and `9c ba`. Earlier `$ba9c` physical fields from the second
  capture remain invalid. All six input receipts still block because the
  final scripted event lacked a later controller read exposing its mask.
## 2026-10-05 — continue the ID `$2c` helper decompilation

- ✅ Rooted `$4ec9` and `$4f31` in the authentic US and JP Rev. 1 stage-two
  image. `$4ec9`'s bounded path and `$4f31`'s indexed pointer-table read are
  byte-locked by `test_theron_v1_stage2_disassembly_chain`; the tracked da65
  listing already decodes both roots. The adjacent `$4ef4` helper, its direct
  `$4be2` caller and conditional `$4c17` caller are also locked. `$4f11`'s
  three-byte table-entry writer is included in the same raw-media test.
- ✅ Bounded the authenticated US `$8000` entry callee's pointer/dataflow
  through `$45a6`, `$4696`, and `$48fc`. The raw-media receipt binds the
  `$8000/$45a6` pair and its call sites; `$4696` is traced as multiplication
  and `$48fc` is independently bound in both editions. Structure-field
  meanings and the dynamic `$3ab7` lane remain unresolved; the US-only
  `$8000/$4696` receipts do not resolve MPR1 at `$3a2e`.
- ✅ Traced the separately bound `$4696` helper as unsigned 8-by-8 shift-add
  multiplication, with inputs `$0e` and `$10`, and 16-bit result `$0f:$0e`.
  The evidence is the authentic US Rev. 1 `$4696` byte receipt and its
  `$8000` call path; the input/result fields have no assigned game meaning.
- ✅ Bound the loop bytes from `$48ec` through the RTS at `$4900` independently
  against authentic US and JP Track 02. The code clears two VDC registers and
  decrements the `$01:$00` pair to zero; the source value's meaning and purpose
  remain unproven.
- ✅ Decompiled dispatch ID `$4b` at `$4aca` through its local `$4ae4` pair
  checker. A new raw-media assertion locks the 45-byte path, including the
  conditional cursor replacement/skip, against authentic US and JP Track 02.
  The stream structure and comparison purpose remain unassigned; see the
  source-lock dispatch-table entry.
- ✅ Bounded dispatch ID `$51` and the adjacent ID `$52` helper variant within
  the authentic stage-two image. Raw-media assertions lock both entry paths
  and the `$4d0e..$4d78` helper bodies for US and JP; field meanings and
  called-helper effects remain unknown.
- ✅ Traced dispatch ID `$4d`'s two operand reads, `$4c30` call, and shared
  four-byte cursor tail at `$49e8`; the exact 19-byte path is locked to US and
  JP Track 02. Operand meaning and called-helper effects remain unknown.
- ✅ Added independent US/JP raw-media locks for dispatch IDs `$0b..$0f`,
  their shared `$41f8` operand reader, indexed store/add/subtract/increment/
  decrement instructions, and the `$40f9`/`$40f5` cursor tails. `$2780` entry
  meanings and retail stream execution remain unassigned.
- ✅ The linked authentic JP and US cold-start receipts and same-session
  BaseRAM snapshots provide a bounded byte decode of `$3a2e..$3a9f`. The 114
  bytes match verbatim in the raw Track 02 BINs at US offset `2872366` and JP
  offset `2870014`; the `$3965` helper also matches raw media in both regions.
  `test_stage2_runtime_helper_media_source` source-locks these slices and
  passed three authentic-media CTest loops on `trv2`. This proves parity only
  for the tested cold-start route, not full caller conditions or game-level
  meanings. See
  `docs/source-lock/theron-disassembly/theron-stage2-bytecode-dispatch-table-20261005.md`.
- 🔒 `$4ec9` branches around the four-byte copy when carry is set and copies
  on carry-clear. Whether that control result represents failure, absence, or
  another game-level condition, and what the fields represent, remain open.
- 🔒 `$3879` is still below the loaded stage-two window and undecoded.
- 🔒 Both US and JP linked captures applied all three replay events, but strict
  input verification blocked because the final event had no subsequent
  controller-port read exposing its mask. Do not treat those captures as proof
  the input changed game state. Earlier F5-state recaptures and their
  controller-port evidence remain separate, route-limited observations; see
  the source-lock document for capture distinctions.

## 2026-10-05 — source-lock the stage-two bytecode dispatch table

- ✅ Recovered the 85-entry `$410d..$41b6` indirect-jump table from authentic
  US and JP Rev. 1 raw Track 02 BINs. All entries match across regions and
  target the `$4000..$7fff` stage-two address span. The source-lock assembly
  now represents them as words, and the raw-BIN receipt checks every target.
- 🔒 The map provides addresses only. Valid stream indices, remaining
  per-handler operands/advancement, gameplay meanings, and the real-resource
  caller remain unbound. The disassembly now decodes from all 85 table roots
  without assigning command semantics. Indices `$00..$07` now have table-word
  and bounded-root source locks, including shared comparator `$4203`; their
  static comparison/cursor-transfer trace is not a valid-stream proof.
  Indices `$22/$24` and adjacent `$25/$26` roots and bounded helper/cursor
  windows are also byte-locked to both editions. ID `$22`'s `$4240..$4252`
  helper is now included through its `$37d8` and `$383e` call sites; those
  callees, helper effects, and runtime selection remain unknown. ID `$37`'s
  `$4814` helper now has a bounded carry-propagation trace for its `$37d4`
  three-byte source plus stream offset `$02`, before staging `$2800` and
  selectors `$1e/$25` for `$383e`; source bytes `$37d4..$37d6` are still
  unbound.
  Indices `$0b..$10` have indexed-byte/nested-cursor traces; the `$09` return
  byte, `$0a` root, and `$10` nested cursor handler are now byte-locked in both
  authentic editions.
  Index `$28`'s conditional path, Y-offset 1 test, Y-offset 2 fetch, regional
  helper operand, and cursor tail are now byte-locked for both editions.
  Index `$12`'s saved-cursor indirect-call path is also source-locked for both
  editions; its selected target remains unobserved. Index `$08`'s local helper
  and cursor tail are also byte-locked for both editions; helper branch effects
  remain unresolved. Indices `$13/$16` and their shared `+1` tail are now
  byte-locked; `$3ab7` effects remain unresolved. Indices `$14/$15` plus their
  shared operand reader and cursor tail are now also byte-locked. The
  `$17..$1b` fixed-argument roots and their common reader/tail are byte-locked
  in both authentic editions; their callee effects remain unbound. Indices
  `$20/$21`'s fixed-argument roots and shared continuation are also locked;
  callee semantics remain unknown. The naturally bounded `$1e` root/helper
  and adjacent `$1f` root are now byte-locked in both editions, without
  assigning operand semantics. IDs `$1c/$1d`, complete roots `$4345/$4497`,
  and their neighboring `+5`/`+7` cursor-step stubs are also byte-locked in
  both authentic editions. Their repeated `$1c` reads and `$3ab7` calls do not
  establish stream structure or helper effects. Dispatch ID `$24` at table
  index `$29`, its `$43dd` bounded root, and `$4403` polling helper are now
  byte-locked in both authentic editions; helper meaning and runtime
  selection remain unknown. ID `$23` at `$42fb` and its US/JP `$56af/$5729`
  call operand are also locked separately in both authentic editions; the
  selected helper's effects and actual stream use remain unknown. ID `$27` at
  `$45ca` is now byte-locked through its `$40f5` cursor tail; pointer purpose,
  `$3ab7` effects, and stream selection remain unbound. ID `$2a`'s `$4409`
  entry and `$4415` helper are byte-locked in both editions; the expansion-ROM
  effects, `$4b3c` table meaning, and runtime selection remain unknown. ID
  `$2d`'s overlapping `$468f` poll/cursor path is also byte-locked in both
  regions; counter meaning and retail stream execution remain unbound. ID
  `$2e` now has US/JP byte locks for its complete `$46ca..$4749` main branch,
  alternate BIOS path, local pair loop, and operand reader. BIOS/callee
  effects, bank mapping, table contents, and retail selection remain open.
  ID `$2f` at `$4794` is now byte-locked through its `$40f5`
  cursor tail; both callee effects and actual stream selection remain unknown.
  ID `$30`'s overlap window `$47a6..$47c4` is also byte-locked in both
  regions; BIOS effects and retail stream selection remain unknown. ID `$31`
  at `$47c5..$47d2` is byte-locked separately in both editions; `$e0d8`
  effects and actual stream selection remain unknown. ID `$32` at
  `$47d3..$47f2` is also locked in both regions, with its condition meaning,
  BIOS effects, and retail stream selection still unresolved. ID `$33` at
  `$469d..$46b7` is byte-locked in both editions through its `$4101` tail;
  store destinations' roles and stream execution remain unproven. ID `$34`
  at `$44bd..$44e6` is also locked in both editions; selector/argument meaning,
  `$3ab7` effects, and retail selection remain open. IDs `$4e/$4f`'s signed
  branches are now explicitly calculated to converge at `$49d8`, the ID `$4c`
  terminal `JMP $40f5` (`+2`) tail. Authentic US and JP test loops pass; the
  `$49e1`, `$4ce1`, and `$4c17` callee effects and actual stream selection
  remain unknown.
  None of these paths is bound to an executed stream. The authentic
  `$6800` data has a 13-pointer prefix whose targets all begin with mapped
  dispatch IDs. Selector `$00`'s candidate has a bounded cursor walk through
  `$6f28`, with recursive targets `$73b2` US / `$73b4` JP. That target begins
  with ID `$1d`, six operands, and ID `$09` (`RTS`), giving a static return
  path if entered. After returning, the outer cursor reaches `$41` roots at
  `$6f23` and `$6f26`; the latter targets `$7470` US / `$7472` JP. Both nested
  streams reach `$09`/`RTS`. The outer `$2a` at `$6f29` maps to `$4409`, whose
  rooted handler reads `$1c+1` (`$00` in both regions), indexes `$4b3c` to set
  `$f8=$03`, BCD-increments that value to `$fc=$04`, sets `$fb=$80/$ff=$83`,
  calls `$e012`, and jumps to `$40f5` (which adds two to `$1c`). Since the
  external calls' effects on `$1c` remain unknown, the eventual outer-stream
  continuation, command meaning, and external effects are not bound.
  Selector `$01`'s target has a bounded walk through `$6c70`, with embedded
  pointers `$78ea/$73b2` in US and `$78ec/$73b4` in JP. The
  `$73b2/$73b4` recursion has the same bounded static return path;
  `$78ea/$78ec` point to matching HuC6280 routine bytes. Selectors `$02/$03`
  embed `$74f2` in US and `$74f4` in JP, also matching routine bytes. The
  runtime `$201c` vector and whether those latter routines execute remain
  unknown; the authenticated stage-two payloads contain no direct absolute
  store encoding to `$201c`, but indirect writes, DMA, and external setup are
  not excluded. Runtime selector choice and complete continuations remain
  unproven. Selectors `$04..$0c` now have conditional cursor walks through
  their rooted `$12` calls. The newly rooted `$7446/$7448` stream contains
  eight `$14/$15` operand pairs and reaches `$09`/`RTS`; suffixes at
  `$7464/$7466` and `$7470/$7472` do likewise. Selector `$0c` reaches a
  static `$01` comparison chain over `$2781`, with pointer branches to
  `$686d..$692d`; all seven 32-byte candidate blocks reach `$09` if their
  calls and counter polls return. Root `$468f` for `$2d` clears `$3b33` and
  polls until it exceeds the stream operand; `$89e7` increments `$3b33` only
  when the byte popped at `$89e2` has bit `$20` set. The `$8975` gate tests
  that bit separately; the visible stack setup loads from `$0000`, not the
  processor status, and a shared producer is unproven. Other clear/wait sites
  use this byte in the US image at `$503d/$5048`, `$7539/$753c`,
  `$7549/$754c`, `$7733/$7736`, and helper `$88a6`. JP's `$753x/$773x` roots
  are two bytes later and its `$50xx` path differs; no causal path from these
  sites to this poll is proven; helper `$88a6` has direct callers at
  `$8862,$8877,$88b1,$88d9` in both editions.
  Selector `$0b` also reaches the poll. Runtime selector choice, counter update
  path, wait completion, indirect-call targets, and gameplay meaning remain
  unproven.
  Regional call behavior at indices `$2b`, `$23`, `$35`, plus `$28`'s regional
  pointer immediate in helper `$43b5`, remains unassigned. ID `$35`'s
  `$46b8..$46c9` handler and `$5e4d/$5e7d` regional call operand are now
  source-locked against authentic US and JP data. ID `$36`'s two-byte
  stream handoff at `$4361..$4372`, fixed `$14` helper selector, and `$40f9`
  tail are also source-locked. ID `$37`'s root at `$480a` and local helper
  `$4814..$4841` are now byte-locked; `$383e` effects and runtime selection
  remain unresolved. ID `$38`'s three-byte handoff at `$47f3..$4809`, fixed
  `$15` selector, and `$40fd` tail are also locked; callee effects and runtime
  selection remain unresolved. ID `$39`'s `$4842` entry and local helper
  `$4be7..$4bff` are now locked, including their visible `$4f31/$37a0` call
  chain; field meanings and runtime selection remain unknown. ID `$3a`'s
  `$485f` `JMP $40f1` stub is locked as the `+1` cursor path; executed-stream
  selection remains unproven. ID `$3b`'s `$447f` branch root is locked and
  reuses the independently locked `$4483` helper; continuation and stream
  selection remain unresolved. ID `$3c`'s `$4862..$4894` conditional BIOS
  window and shared `+1` exit are now locked; BIOS effects and stream execution
  remain unproven. ID `$3d`'s `$489f..$48ab` branch window is also locked;
  it loops into the ID `$3c` tail, so its full behavior remains unresolved.
  ID `$3e`'s `$48ac..$4900` handler/helper window is now locked through its
  `$40fd` cursor tail; condition outcomes and field meanings remain unproven.
  ID `$3f`'s `$4901..$490f` indexed transfer is also locked up to the next
  distinct ID `$26` table target `$4910`; the transferred data's meaning and
  execution remain unproven.
  ID `$40`'s 88-byte branch/polling window `$491b..$4972` is source-locked up
  to ID `$42` at `$4973`; selector meaning and helper effects remain unknown.
  The uncovered ID `$41` root `$42be..$42d2` is now byte-locked to both
  editions through its cursor save/call/restore and `$40f9` tail; helper
  effects and runtime selection remain open.
  ID `$42`'s `$4973..$4993` conditional stores and `$40fd` (`+4`) tail are
  now locked against both editions; field meanings and branch execution remain
  unknown.
  ID `$43`'s `$4995..$49aa` source-copy path and `$40f5` (`+2`) tail are also
  locked against US and JP data; field roles and stream execution remain open.
  ID `$44`'s `$45eb..$45ef` call/branch root re-entering the locked `$45d7`
  continuation is now source-locked; runtime selection remains unproven.
  ID `$45`'s `$49ab..$49b3` subroutine/BIOS handoff is now source-locked to
  the next dispatch target `$49b4`; callee effects and stream selection remain
  unknown.
  ID `$46`'s `$49b4..$49ba` relative-helper/callee handoff is also locked up
  to ID `$47` at `$49bb`; called-routine effects remain open.
  ID `$47`'s `$49bb..$49d2` three-byte setup and shared `$49e1..$49e7` reader
  are now locked; field roles and stream execution are still unknown.
  ID `$48`'s `$4a5e..$4a80` paired-selector loop and `$40f9` (`+3`) tail are
  now source-locked; loop-count meaning and runtime selection remain unknown.
  ID `$49`'s `$44eb..$4517` selector split and `$4514` join are locked, and
  its alternate `$459f..$45c9` continuation now has a US/JP byte lock through
  the shared `$4105` (`+7`) cursor tail. Its zero-selector helper `$4446` is
  also byte-locked in both editions. The `$4f48` subhelper now has a separate
  US/JP lock proving its `$4f31` table-pointer call and two-byte copies to
  `$4ec3/$4ec4` and `$4d79/$4d7a`; caller `$4bd2` is now byte-locked through
  its conditional `$4ef4`/`BRK` handoff. Selector meaning, `$4f5e/$4ef4`
  effects, `$4c17` effects, and runtime selection remain unresolved.
  ID `$4a`'s `$4a81..$4a9f` paired-call loop is now independently locked to the
  authentic US and JP images, stopping before ID `$4b` at `$4aca`; immediate
  operands and runtime selection remain unassigned.
  ID `$4c`'s `$49d3..$49da` shared-reader/call/terminal-jump handoff is also
  locked to both editions; `$4bd2` effects and runtime selection remain
  unknown. ID `$4e`'s `$4a3b..$4a41` shared-reader/call/relative-branch prefix
  is locked to both editions, ending before ID `$51` at `$4a42`; the shared
  destination's continuation and runtime selection remain open. ID `$4f`'s
  `$4a14..$4a1a` shared-reader/call/relative-branch prefix is also locked to
  both editions, ending before ID `$50` at `$4a1b`; its helper effects and
  runtime selection remain unknown. ID `$50`'s `$4a1b..$4a3a` staged operand
  and helper handoff is now locked to both editions, ending before ID `$4e` at
  `$4a3b`; field meanings and helper effects remain open.
  ID `$53`'s `$49fb..$4a13` three-byte staging and `$4c3f` handoff is also
  locked to both editions, ending before ID `$4f` at `$4a14`; field meanings
  and helper effects remain unknown.
  ID `$54`'s `$484e..$485e` short staging path through `$40f5` is now also
  locked to both editions; value meaning and `$4f5e` effects remain open.
  ID `$2b`'s `$4b00..$4b0e` MPR-write helper is now locked identically in US
  and JP, alongside its caller and shared helper prefix; banking purpose and
  the regional helper effects remain open.
  ID `$11`'s US/JP target is now rooted at `$5e27/$5e57`: its
  overlapping TII descriptor and two relative callees are source-locked, and
  the bounded callees do not directly access `$3b33`. This does not prove the
  candidate reaches ID `$11` or close the counter-poll producer gap. ID `$2c`
  is source-locked through its `$3ab7` call and fixed five-byte cursor step;
  `$4ec9` and `$4f31` are now bounded and byte-locked. `$3a2e` and `$3ab7`
  remain below this stage-two image; `$3a2e` has a US/JP raw-Track-02 source
  lock and bounded runtime decode, but no game-level semantic contract.
  `$3ab7` remains undecoded.
  See
  `docs/source-lock/theron-disassembly/theron-stage2-bytecode-dispatch-table-20261005.md`.

## 2026-10-05 — prove replayed PCE masks at the controller data port

- ✅ The scripted-input verifier now requires each event frame's requested
  mask to appear in port 0's controller-register read and to match the
  active-low value returned by the selected direction/button bank. Same-frame
  mixed-bank inputs require successful readback from both banks.
- ✅ A fresh authentic JP Rev. 1 PCE Fast replay delivered four scheduled
  directions to port 0's selected direction bank, with matching raw masks and
  active-low return values. The capture used the instrumented Mednafen build
  and hash-checked retail disc and System Card; its private trace remains
  outside Git.
- 🔒 This only proves controller delivery and a non-System-Card polling path.
  It recorded no command-buffer writes, CD IRQs, or authenticated CD-to-RAM
  receipts, and it did not bind the active map. Door, movement, and T900
  behavior remain gated on a same-session original consumer capture.
- 🔒 A no-input replay of the same authentic Ak-Tu-Ba state recorded `$203F`
  cycling `01 -> 02 -> 03 -> 04 -> 01` at approximately the same write rate
  as the replay with directional input. Do not interpret the snapshot tuple
  `$203F-$2041` as a stable heading/pose until its game-owned consumer is
  identified; see the source-lock capture note.
- 🔒 A separate authentic JP replay read button I and button II on the
  selected port-0 button bank, but recorded no command-buffer writes or
  CD-to-RAM handoff. With active map and front tile still unresolved, this
  does not prove that either button is a retail no-op or bind door/T900
  behavior.

## 2026-10-05 — preserve unresolved Track 02 tile family 7

- ✅ Authentic US and JP map-loader regressions retain every family-7 raw tile
  byte while publishing a distinct unresolved square value. The authentic
  census is 82 US and 78 JP tiles; one real floor-approachable occurrence per
  region verifies both movement preview and the original-command host path
  fail closed without changing world state.
- 🔒 The source-format inventory identifies the packed family value but does
  not establish retail collision, rendering, or interaction behavior. The
  host's no-entry policy is defensive only; do not label these tiles as retail
  walls or claim parity until an original consumer is captured.

## 2026-10-05 — add scripted input to PCE Fast reference captures

- ✅ The opt-in Mednafen reference build can now replay bounded, frame-indexed
  PCE Fast controller inputs on port 0 and emit separate event/apply receipts.
  The live-capture regression passed three consecutive loops. A fresh
  instrumented build on `trv2` with PCE Fast and the bounded RAM snapshot
  enabled completed with `-j1` and advertised both `pce` and `pce_fast`.
- ✅ Three isolated restores of the authentic Japanese Rev. 1 Ak-Tu-Ba
  emulator state each replayed four scheduled directional inputs. The
  independent input verifier reported 4/4 events applied, 4/4 event frames
  followed by original controller-port reads, and four non-System-Card poll
  reads. Inputs were supplied to the reference emulator only; no game or
  campaign data was changed.
- 🔒 The capture transition remains `missing`: these polls do not prove that
  Theron's original gameplay loop consumed the inputs, moved the party, or
  selected an active map. No CD IRQ/raw-sector/authenticated CD-to-RAM
  receipts were present in the replay. Continue with the open Track 02
  runtime-consumer join below; do not promote gameplay or map semantics.

## 2026-10-05 — route authentic host keys to the selected PCE core

- ✅ The Mednafen capture harness now selects `pce_fast.input.port1` when
  `pce_fast` is selected instead of configuring only the regular PCE port.
  For host-driven input, it mirrors the already configured PCE scancodes into
  PCE Fast's per-button settings as temporary command-line overrides; it does
  not edit the operator's profile. Replay and host-input captures also default
  to the larger bounded controller-read trace so startup polling cannot fill
  the old 65,536-read limit before later scheduled keys. The focused script
  regression passed three loops.
- ✅ A full instrumented Mednafen build completed on `trv2` with `-j1`, and the
  complete patch set applied in three isolated patch-only loops. A private
  authentic Japanese Rev. 1 cold-start capture then recorded 296 PCE reads of
  RUN (`raw=0008`) and 21,210 reads of Button I (`raw=0001`) after 30 host SDL
  key events, confirming that this controller profile reached PCE Fast's
  controller-port reader. The bounded trace also recorded 11,949 BaseRAM
  reads and captured the executing instruction bytes for each sampled reader.
- 🔒 This capture still has zero CD IRQs, non-System-Card CD reads, raw-sector
  spans, SCSI reads, or authenticated CD-to-RAM receipts; its transition is
  `missing`. `$2031` was read only as part of two block copies, including a
  later `TII $2000,$2700,$0080` at `$4009`; this does not identify an active
  map or level consumer. The final `$2031=73` and zero party coordinates remain
  System Card state, not dungeon state. Keep gameplay and startup-level
  readiness closed until a same-session authentic map/level consumer join is
  captured.

## 2026-10-05 — read BaseRAM from the authentic Akutuba state

- ✅ Replayed the previously captured, authentic JP Akutuba gameplay state
  (`.mca` SHA-256
  `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`)
  through the instrumented PCE Fast core. Its bounded 8 KiB RAM snapshot
  retained `$2031=02`, `$203F-$2041 = 01 02 03`, and `$20DA/$20DB = 01/00`.
  The 20-second replay produced 3,440 BaseRAM consumer reads; instruction-byte
  sidecars decoded frequent readers at `$B9FC` (`TIA $2062,$02,$0020`) and
  `$A1B7` (`TII $287F,$2883,$0014`).
- 🔒 None of the sampled consumer reads accessed `$2031`, `$203F-$2041`, or
  `$20DA/$20DB`; the instruction windows therefore do not bind those state
  fields to an active map identity. The restored state also produced no CD IRQ,
  raw-sector read, or authenticated CD-to-RAM receipt, so the strict transition
  gate remained blocked. This is emulator-state and reader-code evidence only;
  the `.mca` is not a native campaign save, and no map, pose, or gameplay
  semantics are promoted.

## 2026-10-05 — keep PCE Fast transition evidence module-correct

- ✅ The live-capture validator now requires same-instant HuC6270/VDC
  snapshots only for Mednafen's `pce` module. `pce_fast` rejects accidental
  PCE-only VDC sidecars and reports those artifacts as unavailable rather
  than failing on a snapshot that core does not produce. Transition bytes
  are read from the selected core's bounded 8 KiB RAM snapshot.
- ✅ On `trv2`, an isolated `pce_fast` restore of the existing authentic JP
  Ak-Tu-Ba state passed the module-specific snapshot gates. Its receipt reads
  `$2031=02`, `$203F-$2041 = 01 02 03`, and `$20DA/$20DB = 01/00` from the
  captured PCE Fast snapshot, and marks VDC data unavailable. The short
  no-input state restore has no CD IRQ or authenticated CD-to-RAM receipt,
  so the overall capture correctly remains blocked at the later dynamic-data
  gate. The focused capture-script test passed three consecutive loops.
- 🔒 This fixes evidence collection only. Neither the restored pose nor the
  level/bank bytes identify the active Akutuba map or prove an input consumer;
  keep the production first-floor/North pose provisional and exact startup
  level readiness closed until the same-session map join is captured.

- 🔒 2026-10-05 PCE Fast reader instruction follow-up: the bounded consumer
  trace now retains eight same-session instruction bytes from Mednafen's
  mapped fast-read page for each logged BaseRAM access (only after the existing
  bank, global-count, and per-byte limits pass). Patch-only and full `-j1`
  builds succeeded on `trv2`; the authentic Ak-Tu-Ba state restore again passed
  the PCE Fast snapshot checks and stopped at the expected missing dynamic
  CD/IRQ receipts. `da65` decoded high-frequency captured reader windows as
  HuC6280 block transfers (`TIA`, `TII`, and `TDD`), including `$B9FC`
  (`TIA $2062,$02,$0020`) and `$A1B7` (`TII $287F,$2883,$0014`). This
  identifies how those observed reads execute, not which transfer supplies
  the active map or level; no runtime map join or gameplay behavior is
  promoted from these bytes.

## 2026-10-04 — join the original startup pose to its active dungeon map

- ✅ A fresh authentic JP Rev. 1 `pce_fast` run again reaches the title after
  holding RUN for four seconds, then enters New Game → Ak-Tu-Ba using ordinary
  Space/Button I input. A private raw 8 KiB BaseRAM snapshot records
  `$2031=02`, `$203F-$2041 = 01 02 03`, and `$20DA/$20DB = 01/00`.
- ✅ The opt-in Mednafen reference instrumentation now traces both PCE BaseRAM
  bus callbacks and direct HuC6280 zero-page stores when MPR1 maps BaseRAM. The
  capture logged `$2040=02` and `$2041=03` at the same instruction (`PC=$C2AF`,
  physical `$0DC2AF`), and `$20DA=01` at `PC=$CC00`, physical `$0D4C00`.
  `$203F` runtime updates ended at `01` (`PC=$5800`, physical `$0D1800`) in the
  recorded trace, matching the final byte. `$20DB` remained zero with no
  nonzero write observed. The `$2031` trace reached its per-offset sample cap;
  its final byte is `02`, but its final writer is not proven.
- 🔒 This is emulator-side writer evidence only. It does not bind `$2031` to
  the active level/map or join the captured coordinate/direction to the exact
  Track 02 map consumer. Keep the production first-floor/North pose provisional
  and exact startup-level readiness closed until that same-session source join
  is captured and verified.

## 2026-10-03 — keep parser fallback poses out of exact-level readiness

- ✅ The startup route's candidate match still validates that the parsed
  first-floor coordinate is passable and matches the placed party, but that
  check no longer increments `semantic_level_count` unless the pose has
  runtime-capture provenance. The sparse regression route remains available
  as capture evidence while reporting zero exact levels.
- 🔒 No current authentic new-game capture is joined to the active map and
  level consumer, so no production level receives runtime-capture pose
  provenance. Exact level semantics correctly remain unavailable until that
  source join is implemented and verified.

## 2026-10-03 — bind the initial party pose to an original runtime capture

- 🔒 The authenticated candidate parser and full-dungeon decoder expose a
  `start_x/start_y` chosen by a first-floor scan and default `start_dir` to
  north. A new provenance field labels that value as a fallback, and the
  source lock/test labels no longer present it as a proven retail spawn.
- 🔒 A fresh authentic JP Rev. 1 `pce_fast` session now confirms that holding
  RUN for four seconds reaches the Theron title menu; ordinary Space input
  proceeds through FILE_1 and Ak-Tu-Ba to a first-person dungeon view. A
  bounded 8 KiB raw main-RAM snapshot records `$203F-$2041 = 01 02 03` in
  that session. However, the snapshot is not yet joined to a source-authenticated
  level/bank and exact Track 02 map cell from the same session. Do not promote
  the parser's provisional first-floor/North pose based on this capture.
- 🔒 The same snapshot also has `$2031=02` and `$20DA/$20DB=01/00`. `$2031`
  was source-interpreted as level 2 in a separate Drator capture, but applying
  that identity to this Akutuba session would put `(2,3)` on a wall in JP map
  2; the coordinate is open floor in JP Akutuba map 0. The authentic JP map
  test now locks both bytes to preserve this ambiguity, and the US/JP full-map
  regression verifies every decoded start pose remains labeled as a fallback.
  The bank bytes are provenance only. Capture the active map/level consumer or
  loaded-map pointer in the same session before selecting either map or
  promoting the pose.
- The new-game run's native 2 KiB BRAM matches the known empty menu-only image;
  it is not a campaign save. Emulator `.mca` autosave round-trip was verified
  separately from an existing authentic gameplay state, but this does not
  create new native campaign progress. Keep savegame support deferred until
  authentic game-owned save behavior is captured and bound.

## 2026-10-03 — exercise every captured movement command on authentic campaign maps

- ✅ The US/JP hash-gated mechanics probe now runs all four original command
  types (`$03`–`$06`) over source-loaded floor and wall routes in all seven
  authentic dungeons. Each attempt starts from a copy of the loaded world and
  asserts destination/facing on floor or atomic pose retention at walls.
- 🔒 This verifies Firestaff's command mapping against authentic campaign map
  bytes; it does not join the separate original save-state pose to a raw
  Track 02 coordinate, prove regional original-runtime captures, or unlock
  doors, stairs, combat, or other unresolved interactions.

## 2026-10-03 — keep creature-spawn source evidence regional

- ✅ Replaced the combined spawn-source media probe with separate US and JP
  invocations. Each validates one edition's exact Track 02 BIN identity and
  source records. Missing default media skips only its region; an explicit
  unreadable override or cross-edition image fails. On trv2, both authentic
  regional CTests passed three loops, and the wrong-edition, explicit-missing,
  and independent absent-media checks behaved as required.
- 🔒 These assertions bind authentic regional source bytes and source-consumer
  identity only. Retail runtime spawning, category meaning, RNG effects, and
  resulting creature stats remain unverified.

## 2026-10-03 — regional authenticity for the shared UI glyph bank

- ✅ The existing 120-glyph 8×6 viewport bank is byte-identical to authentic
  US Track 02 at UD `0x09A000` and JP Rev. 1 Track 02 at UD `0x099800`.
  `theron_v1_track02_font_glyphs_real_data` now validates both edition hashes
  and reads both source spans through the bounded raw-sector/user-data mapper.
- ✅ On trv2, the focused font, quest-item-name, production text-gate and
  seven-dungeon real-media tests passed three consecutive loops against the
  authentic US and JP BINs. The JP CUE ISO projection was not staged and its
  subcheck reported `SKIP`.
- ✅ US and JP font checks are separate CTest cases, so missing regional media
  skips only its own check. Direct missing-US and missing-JP media probes each
  returned CTest's configured skip code independently.
- 🔒 This verifies the common glyph asset in Firestaff's source-only Latin
  viewport text helper. The viewport/font module is excluded from the
  production library; this does not prove Japanese kana glyphs, the retail
  text consumer, or complete regional text-rendering parity.

## 2026-10-02 — original raw Track 01 audio from US/JP 7z discs

- ✅ M11 now binds Track 01 directly from the selected hash-verified US or JP
  7z Track 02 edition. It reads the matching archive CUE and its exact
  CUE-declared raw Track 01 BIN into bounded memory, then starts the existing
  CDDA stream from original 2352-byte sectors. No archive members are written
  or cached to disk. Other 7z layouts, loose Track 02 files, and unpaired audio
  remain closed.
- ✅ Extended the memory-backed CDDA lifecycle to queue authentic raw sectors
  as well as the existing bounded OGG input. The authentic US/JP archive test
  verifies both the filesystem-backed and memory-backed stream with original
  Track 01 data.
- ✅ On trv2, the `theron_v1_jp_7z_direct_boot`,
  `theron_v1_us_7z_direct_boot`, `theron_v1_track01_cdda_authentic_archive`,
  and `theron_v1_m11_launcher_handoff_boundary` CTests passed. Both archive
  boot tests reported `theronTrack01CddaReady=1`; the audio test queued raw
  sectors from both original editions. This establishes authentic title
  Track 01 availability and startup only—not audible device output, gameplay
  CDDA command selection, or a game-owned event-to-track mapping.
- 🔒 The isolated US attempt below failed to reach a playable dungeon or
  produce a game-native progress save. It showed the authentic Track 02
  copy-protection message at UD `0x26C348` ("This disc only works on the SUPER
  CD-ROM2 SYSTEM") after a four-second RUN hold. Its fresh 2 KiB BRAM is not
  evidence of campaign progress. The capture hash-checked and passed the
  normalized System Card 3.0 path as `pce.cdbios`; because the CUE's first track
  is AUDIO, Mednafen's `pce.cpp` GE-CD BIOS dispatcher selects that standard CD
  BIOS setting. This US failure is limited to that capture: the later
  authentic JP New Game route to the Akutuba selection map and first-person
  dungeon is recorded in `DONE-theron.md`. A later BRAM hash audit found that
  image byte-identical to the known empty menu-only JP save, so the observed
  load-menu return does not prove native campaign-save persistence. Preserve the US
  capture outside Git and do not import its BRAM. The Mac remains locked and
  the separate Nexus capture on trv2 was left untouched.
- The instrumented capture, including a read-only replay from the existing
  authentic US save-manager state, recorded RUN as PCE input `raw=0008`; every
  controller-read PC remained in the System Card `$E4xx` path. No dungeon input
  consumer or transition was observed. The strict capture rejected the run
  because it did not produce a valid bounded VDC snapshot; its trace and BRAM
  snapshot remain private on trv2 and are not parity proof.
- A second isolated headless cold-start used the authenticated US CUE and
  hash-verified System Card 3.0 with scheduled RUN at PCE input frame 480,
  held for 240 frames. The instrumentation verified that the input was
  consumed and followed by a controller read at System Card PC `$E4B7`, with
  no non-System-Card controller poll. It read 25 authentic raw sectors,
  beginning at LBA 3234, but recorded no CD-to-game-RAM data transfer; the
  strict capture therefore stopped at its missing-origin gate. The delayed
  240-frame RUN input (nominally four emulated seconds) still did not reach a
  dungeon and created no valid game save. Its trace and BRAM remain private at
  `/home/trv2/work/firestaff-theron-run-save-capture-20261002/pce-scheduled-run-20261003/`.
- A separate `pce_fast` GUI attempt confirmed that a 4-second RUN hold can
  reach the authentic JP Rev. 1 title screen; button I opened the real file
  menu, New Game/File 1 reached the seven-dungeon map, and the first campaign
  route displayed Ak-Tu-Ba, its Japanese narrative, and the in-dungeon
  first-person wall view. Captures remain outside Git. This proves a retail
  JP gameplay session can reach Akutuba, not complete gameplay parity.
- A second clean, task-private trv2 profile repeated that route with RUN held
  for four seconds and reached the same authentic JP Akutuba view. Its
  screenshots expose no party coordinates or item state. The attempt therefore
  does not prove that the `(0,0)` teleporter was entered or establish any
  teleporter field semantics. Its screenshots remain outside Git under
  `/home/trv2/firestaff-theron-evidence/emulator-created-20261003/teleporter-capture-01/`.
  No native BRAM progress or usable Mednafen state was created. Keep the
  teleporter resolver and preview unchanged until an ordinary-input capture
  yields game-owned before/after state or authenticated source binds the
  behavior.
- A third task-private run confirmed the user-requested four-second RUN hold,
  the JP New Game/File 1 route, and a first-person Akutuba view. Follow-up
  inspection corrected its Button I note: Mednafen input values are SDL
  scancodes, so value `44` is Space, not `KP_3`. In a new isolated profile,
  Space visibly advanced New Game to FILE_1; the source-backed JP route then
  reached the Akutuba map, Japanese introduction, and first-person dungeon
  view. This still supplies no party coordinates, movement, item, or
  teleporter evidence. Capture-03 images and emulator files remain private
  outside Git.
- The follow-up capture is saved privately at
  `/home/trv2/firestaff-theron-evidence/emulator-created-20261003/capture-space-run-20261003/theron-jp-akutuba-first-person.png`
  (SHA-256 `1364c217ba5e6c0f14d8dc00e709504e2e2660ac2b69ca337d671dfe7f593cdc`).
  It was taken from the authentic JP Rev. 1 CUE/System Card run after a
  four-second Return/RUN hold and ordinary menu input. The PCE Fast per-module
  override had to be loaded from `pce_fast.cfg`; a global override alone was
  insufficient. The preceding GUI attempt did not create an F5 state or a new
  persistent-RAM file; the separate successful state capture and its empty
  BRAM result are recorded below.
- The real-data loader regression now pins the JP Rev. 1 Akutuba `(0,0)` raw
  teleporter record to M0 `(2,3)` and runs Firestaff's turn-left/forward host
  movement path against authentic JP Track 02 and map bytes. It passes three
  consecutive runs on trv2. This is regional source-data/host-path parity,
  consistent with the isolated original Akutuba route recorded in
  `DONE-theron.md` on 2026-08-21; it does not upgrade the failed recent GUI
  attempt into new evidence, nor establish JP-specific scope/rotation/sound or
  a screenshot-based movement trace. Those consumer details remain governed
  by their existing source-lock boundary.
- Do not apply the captured US `(2,3,north)` post-landing movement outcomes to
  JP as retail evidence. The authentic JP map neighborhood is now recorded,
  but a test experiment showed that asserting the US result through the
  current JP host path fails; source-bound JP command/position capture is still
  required before any regional behavior change.
- The JP native 2 KiB BRAM written by that run has SHA-256
  `de8e415730226a1f0e39666b1ea291b6abec07bcaeb7223dc33ea01a71f89eaa`, equal
  to the fresh menu-only BRAM produced earlier; it remains an empty save, not
  campaign progress. That run's private `.mca` was initially treated as
  unusable because the first restored frame showed only the HUD and hand
  pointer. A later independent F5/F7 test below supersedes that save-state
  conclusion; neither state is a native save or evidence of campaign progress.
- Follow-up inspection of the same private state capture found
  `snap/private-restored-after-up.png`: after an authentic Up input, the
  screenshot shows the Theron HUD, hand pointer, and Akutuba first-person wall
  and floor. This establishes that an input can redraw the restored gameplay
  view, so the state is useful for continued emulator capture. It does not
  identify party coordinates or prove that the Up command moved the party.
- A separate stock Mednafen 1.32.1 capture repeated the four-second RUN boot
  with the authentic JP Rev. 1 CUE and System Card. In its isolated profile,
  the PCE Fast module loaded keyboard bindings from its own `pce_fast.cfg`;
  SDL scancode `44` is Space, the Button I input that advanced New Game,
  FILE_1, Akutuba, and the Japanese introduction to the first-person dungeon
  view. Pressing F5 then closing that profile wrote a 229,965-byte `.mca`
  state (SHA-256 `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`).
  A second isolated `pce_fast` profile copied that exact file and loaded it
  with F7. Its initial frame showed the HUD with a black viewport; after a
  normal W input, the authentic Akutuba wall/floor view was redrawn. The
  before/after screenshots differ in 1,384 of 786,432 pixels. This verifies a
  useful original-gameplay state round trip, but not party coordinates, pose
  parity, movement distance, or new native BRAM progress. The state and both
  screenshots remain private outside Git under
  `/home/trv2/firestaff-theron-evidence/emulator-created-20261003/capture-space-run-20261003/`.
- One earlier GUI attempt wrote Mednafen's shared global config on clean
  shutdown. Subsequent capture used a task-private Mednafen base directory;
  preserve original game media and all existing profiles.

## 2026-10-03 — bind the four-second startup press to original gameplay

- The external authenticated JP capture harness replayed RUN at frame 480 for
  240 frames (four seconds), followed by eight short Button-I events. Its input
  trace records RUN `raw=0008` at the System Card controller poll `$E4B7` and
  the full scheduled sequence, but the authentic JP CUE/System Card run still
  produced no CD-to-game-RAM origin receipt, no party pose, and
  `transition=missing`. It did not reach a gameplay consumer; do not promote
  the BIOS-only input evidence.
- A separate attempt to load the 229 KiB GUI-created Akutuba `.mca` through the
  instrumented Mednafen `pce` core failed: the state contains `pce_fast`
  sections that `pce` does not recognize, and the required VDC section was
  absent. The state remains useful for GUI captures after input, but this
  cross-core failure is not evidence of a retail save defect. A matching
  instrumented `pce_fast` core or a `pce`-created gameplay state is still
  needed for a same-session pose/consumer capture.
- These experiment traces and the original state remain outside Git. No JP
  teleporter, movement, item, or save-state semantics changed.

## 2026-10-02 — authentic US BRAM reaches Firestaff's M11 Continue route

- ✅ On the isolated trv2 build, `test_theron_v1_pce_bram_real_artifact`
  passed against the installed authentic Akutuba-complete US Backup RAM and
  authentic JP empty-save artifact. `theron_v1_m11_real_bram_continue` also
  passed (CTest 1/1) using the authentic US Track 02
  (`f23601102138f87c33025877767ebf76`) and 2 KiB BRAM
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). The protected test copy retained the
  same MD5 after the run. The test drives Firestaff's M11 APIs through its
  Continue path, source-backed Drator Soul Room entry, and native movement
  checks. It is labeled `no-synthetic` and `real-media`.
- This is authentic-media evidence for Firestaff's host path, not execution
  of the retail Theron code. It does not demonstrate that the retail game
  created a new save, capture an original-engine dungeon session, or prove
  full gameplay parity.
- The isolated Mednafen profile's BRAM is still byte-identical to the
  existing authentic save. Emulator capture remains open pending an unlocked
  Mac and a visibly confirmed dungeon view; when resuming, hold RUN for 3–5
  seconds as the user specified before following the source-locked Drator
  route.
- Corrected the startup save/resume probe's stale citation: it now treats
  THQUEST.ASM T800 as a routine label only and keeps field semantics gated by
  the authenticated DMS-SG.001 evidence, rather than attributing save behavior
  to T080/T800 from the generic resume gate. Both startup save/resume checks
  pass.
- Built all missing binaries for the Theron-labeled CTest selection on trv2;
  all 80 entries then completed with 72 passing and 8 skipped for unavailable
  authentic captures, archives, converted ISO or operator inputs. There were
  no failing entries. Authentic-media US/JP boot, scan, data and M11 Continue
  checks ran against the installed game files; this does not establish retail
  engine parity beyond those bounded host-path assertions.

## 2026-10-02 — separate Firestaff TQSV state from retail Theron saves

- Corrected save-format provenance across the save/progression interfaces:
  eight-slot `.tqsv`, its 64-byte header, XOR transform and checksum are now
  labeled Firestaff host/interchange behavior, not original retail fields.
  T800 is retained as an original routine label, not evidence for those host
  champion reset/persistence rules.
- Updated source evidence and string-contract tests to point at the
  authenticated `DMS-SG.001` Backup RAM writer/restore body and its bound
  fields. The gzip community `.srm` Save Disk body remains separately opaque
  and non-launchable; no data was synthesized or imported.
- Reconciled the champion, progression, boot, SRM, shop and startup comments
  with that boundary: Firestaff policies and DMWeb summaries are identified
  as host behavior or secondary evidence, not unqualified retail semantics.
  Host round-trip tests are labeled as such and no longer cite T800 as proof
  of retail inventory or champion persistence.
- Marked the older 2026-08-20 BRAM uncertainty as superseded: the later
  source lock proves the two slot-tail bytes are padding and binds all 134
  writer bytes, while the production restore remains limited to the proven
  Theron fields and campaign value.
- A fresh emulator dungeon checkpoint is still unverified. Mednafen state
  files exist, but the Mac remained locked, so I could not verify the foreground
  window or confirm their in-game location. Repeat with RUN held for 3–5
  seconds, follow the source-locked Drator route, then capture and reload only
  after the active dungeon view is visibly confirmed. The isolated 2 KiB BRAM
  still matches the authenticated Akutuba-complete artifact
  (`ffabc8d19b0915d4d9632a7ae2e90a97`).
- `git diff --check` and C11 syntax-only checks pass for the changed runtime
  and test files. On an isolated trv2 clone of the exact current main
  revision, the V2 phase-gate test passes 231/231; CMake-flagged object builds
  pass for the save/load, progression-roundtrip, and startup-resume sources
  and tests. Full link/CTest for those save paths remains open because the
  target graph pulled in the broad shared M10 library; that unrelated build
  was stopped before completion. The local macOS CMake check also remains
  unavailable because its SDK linker rejects the `arm64e.x1` architecture.

## 2026-10-02 — authentic combat integration media is an explicit CTest skip

- `theron_v1_combat_runtime_source` now returns CTest's configured skip code
  when either authentic regional Track 02 BIN is unavailable. Missing-media
  startup/combat integration can no longer appear as a green test that only
  exercised data-independent gates.
- Focused local CTest passed with authentic US and JP Track 02 BINs
  (`f23601102138f87c33025877767ebf76` and
  `b7afb338ad31be1025b53f9aff12d73a`); a separate run with both paths
  unavailable was reported as `Skipped`. The authentic US Track 02 ISO
  cross-check was also supplied and hash-verified by the test.
- This corrects test reporting only. Combat/action semantics remain
  source-gated, and no save data or synthetic gameplay data was introduced.

## 2026-10-02 — authentic launcher scan reuse and regional disassembly checks

- Replaced the Theron launcher scan-reuse test's synthetic media/hash fixture
  with the installed authentic `.firestaff/data/theron` files. It verifies the
  detected Track 02 identity and required-file marker against the actual file
  bytes, then loops three launcher refreshes to check reuse without repeating
  full-root hash scans. The test correctly skips when authentic Theron media
  is unavailable; it does not claim scanner coverage in that environment.
- Made the HUC6280 source-disassembly test skip an unavailable regional input
  instead of failing the whole test. On trv2, the authentic raw BIN checks ran
  and the unavailable JP ISO was reported as a skip; no ISO parity is claimed.
- Updated the startup wall-block harness's evidence assertions to match its
  current THQUEST/phase-2/C240 citations and explicitly labeled its data-free
  8x8 host model as not proving retail collision/gameplay parity.
- On trv2, built the affected targets and ran all 290 selected `theron_*`
  CTest entries with two workers. There were no failures; 19 entries were
  reported skipped for unavailable capture, archive-tool, ISO, or operator
  inputs. The corrected launcher scan-reuse test passed against authentic
  installed media. This regression result does not close the original-runtime
  gameplay and rendering gaps below.
- On `main` at `a0bd259`, trv2 configured the project and built the application
  plus affected Theron test targets. Six
  focused CTest entries passed, including authentic-media scanner, loader,
  CDDA, and disassembly checks; the hardware-configuration executable also
  passed all six checks against installed authentic media. This targeted
  regression run does not claim full-game parity.

## 2026-09-30 — remaining coordinate-teleporter parity

- The authentic US AKUTUBA M0 route from `(1,0,north)` through the active
  `(0,0)` pad to `(2,3)` commits through Firestaff's mutating movement-command
  API and transition executor. The real-media movement corpus now exercises 41
  eligible routes each from US and JP Track 02, including four cross-level
  routes and eight closed-pad terminal arrivals per region. The real-media
  census finds 14 active-chain roots in each region: four floor-approachable
  chains end at walls and block unchanged, one chain with a floor approach
  ends at a special square and is deferred, and nine chains have no adjacent
  floor approach. No unresolved chain with a direct floor approach occurs in
  either current retail BIN; see `DONE-theron.md`. On all 91 active links per
  region with an adjacent authentic floor, the read-only preview now agrees
  with Firestaff's mutating original-command API on a cloned world and leaves
  the source world unchanged. Special-square behavior and routes without a
  floor approach,
  direct wall-target behavior in the original runtime, and original source-owned
  runtime consumers remain unverified. The JP full-CUE capture
  still did not reach an authenticated game-owned data consumer, so it adds no
  independent original-runtime gameplay evidence.
- The read-only preview and mutating resolver now both fail closed when an
  authenticated Track 02 teleporter tile lacks its matching coordinate-link
  record. The resolver also rejects a non-Track-02 object on an authenticated
  level instead of reinterpreting its packed coordinate word as a legacy
  object ID. Legacy object-ID fixtures remain available on unauthenticated
  worlds. On `trv2`, the authentic US/JP BIN loader corpus passed three
  consecutive runs; each regional case fault-injected a missing record and a
  missing coordinate-link marker while retaining its authentic map/header.
  The legacy combat-mechanics fixture passed three runs. These checks establish
  fail-closed host consistency, not retail behavior for malformed source data.
- Active coordinate links retain the encoded record metadata, but the Firestaff
  resolver and preview do not yet apply its party/item scope, rotation,
  absolute-facing mode, or sound fields. The original-runtime capture documented
  in `DONE-theron.md` proves one controlled coordinate/rotation mutation and
  its resulting party facing only; it does not establish every field
  combination or scope effect. The authentic US and JP census now finds the
  same 25 `(scope, rotation, absolute, sound)` combinations among 170 open
  teleporter occurrences in each region across all seven dungeon banks; the
  real-media test locks each count. Firestaff's normal movement regression
  reaches 41 floor-approachable floor/closed-pad routes per region across 15
  of those tuples; that is a capture-candidate inventory, not evidence of
  field behavior. The real-media test now also sends each reachable
  active-to-closed target through a floor approach, read-only preview, and
  original movement command, verifying the committed destination pose for
  eight routes per region. This remains Firestaff-path consistency, not
  original-runtime proof or field-semantics evidence.
  Next, capture ordinary-input before/after party and item state for
  representative cases or bind the missing branches in the authenticated
  Theron routine. Until then, keep these semantics unsupported and update the
  mutating resolver and read-only preview together only from Theron-specific
  evidence. Generic DM/ReDMCSB behavior is not sufficient.
  Neither the production packed-coordinate path nor the legacy object-ID
  compatibility path maps its transition to the generic
  `THERON_SOUND_TELEPORT` sample. The original sound event/sample ownership
  remains unverified; this conservative gate does not imply that retail
  teleporters are silent.
- Capture preparation is incomplete: the authenticated US gameplay savestate
  used by the prior command captures is not available as a gameplay state.
  The older authenticated US `.mc0` has matching state, US CUE, and System
  Card identities, but a new instrumented replay selects the original
  `DMS-SG.001` backup-RAM manager (`$42B7/$42B8` overlay values `0f/01`), not
  an active dungeon consumer. Scripted controller events were observed at an
  original CPU poll, but produced no source-backed movement or teleporter
  transition. Treat this `.mc0` as save-manager evidence, not a gameplay
  savestate. The checked-in replay still has no panel-click event. Do not
  mutate a record or RAM to manufacture a gameplay result. Recover an
  authorized authentic gameplay state or establish a fresh source-bound route
  first; then add PC-attributed party/item and sound-register observation
  before drawing field semantics.
  The user confirmed on 2026-10-02 that no authentic in-dungeon savestate is
  available. A new isolated local Mednafen session used the US 19-track CUE,
  System Card 3.0 (`ff1a674273fe3540ccef576376407d1d`), and the original
  Akutuba-complete BRAM (`ffabc8d19b0915d4d9632a7ae2e90a97`) in a private
  profile. Real controller inputs reached the original title and campaign
  map. Mednafen wrote two authentic emulator states: title checkpoint
  (`a3b436167a28a48c511859909ee4ea7e`) and campaign-map checkpoint
  (`f7183e146f181189ae1470643a949092`). The title state was reloaded to the
  title, and the map state was reloaded to the campaign map. Loading the
  existing file showed Tower of Drator and other continuation chapters as
  available, but the session did not enter a dungeon. The BRAM remained
  byte-identical after shutdown, so no new original-format game save or
  in-dungeon state was produced. Keep both `.mc0` states and screenshots in
  the operator's private Firestaff work directory outside Git; do not claim
  gameplay capture or native-save creation from these states.
  A later isolated 180-second trv2 `drator-generator` replay used the
  authenticated US CloneCD CUE, System Card (`ff1a674273fe3540ccef576376407d1d`),
  and instrumented Mednafen (`9889ef7e2361d2bce69fcae327bf5f9c`). Of five
  planned controller events, only RUN at frame 9600 and I at frame 12000 were
  applied; both were read only by the System Card poll at `$E4B7`. The capture
  had no non-System-Card poll, raw-sector span, authenticated CD-to-RAM
  receipt, or transition (`transition=missing`); strict capture rejected the
  three unobserved events. Mednafen emitted a 2 KiB `HUBM`/`DMS-SG.001` BRAM
  snapshot (MD5 `dbdedb0ec809227b289c2bc5b18b9c9d`), but it differed in only 30
  byte positions from the existing authentic campaign BRAM and no game-owned
  poll or gameplay transition was observed. Do not treat or import this as a
  newly created gameplay save. The capture remains outside Git at
  `/home/trv2/work/theron-save-attempt-20261002/capture/`.
  A read-only trv2 check on 2026-10-02 found no other Mednafen state or movie
  files in its configured save directory or Firestaff work trees. The SSH
  session had no display, and `:0` was unavailable; this does not authorize
  starting a capture against a shared display.
  On 2026-10-03 a later isolated trv2 `pce_fast` session used the authentic JP
  Rev. 1 CUE and System Card. Holding RUN for four seconds reached the title;
  the route continued to a visible Akutuba dungeon view. A screenshot taken
  after an autosave restart shows the first-person dungeon corridor, and the
  private profile now contains the 229,390-byte emulator state
  `theron-jp-akutuba-dungeon-capture.mca`. This is emulator-state evidence,
  not a native game save. A manual F5-save/input/F7-restore round trip for
  that state is still unverified. The existing profile has autosave enabled;
  repeat verification only in a new isolated profile with autosave disabled.
  The current trv2 graphical session is locked, so do not send input to it or
  claim manual state-load verification until an authorized display is
  available.

## 2026-09-30 — explicit US menu target still does not reach the title route

- A ten-minute isolated trv2 replay used the authentic US CUE (MD5
  `63dbd2fab613b2e8030ff4e44b978a39`), Track 02
  (`f23601102138f87c33025877767ebf76`), System Card
  (`ff1a674273fe3540ccef576376407d1d`) and campaign BRAM
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). The final 2 KiB BRAM snapshot was
  byte-identical to the configured authentic save. The signed US menu-route
  target `7549` was explicitly supplied alongside `drator-generator` and the
  original scripted `run@9600:90` input.
- The input event was applied and followed only by a System Card controller
  poll at `$E4B7`; no non-System-Card poll or Drator route-hook receipt appeared.
  The run recorded 115 CD IRQs, 25 raw-sector spans and four SCSI READs, plus
  one `$E009` dispatch/return but zero `$E009` data reads and zero authenticated
  CD-to-RAM receipts. The capture ended `BLOCKED` with `transition=missing`.
- Supplying the authenticated route target did not advance the cold start to
  the title/menu path. This remains negative startup evidence, not Drator or
  gameplay proof. Private traces remain on trv2 under
  `/home/trv2/work/theron-stair-capture-20260930/capture/`.

## 2026-09-30 — authentic JP full-CUE capture still stops before Track 02 consumer

- On trv2, an isolated 120-second run used the complete hash-verified JP
  Rev. 1 CUE, its Track 02 BIN (`b7afb338ad31be1025b53f9aff12d73a`), and the
  already-installed System Card (`ff1a674273fe3540ccef576376407d1d`). The
  instrumented Mednafen binary's MD5 was `e69796b508d306844ffdee039858b4a1`;
  capture scratch and traces remain outside Git.
- The run observed 115 CD IRQs, 24 raw-sector spans, four SCSI reads and 24
  sector bindings. The scripted RUN was applied and read only at the System
  Card poll `$E4B7`; the game/non-System-Card poll gate remained unobserved.
  Stage two recorded two calls and one return, but the authenticated `$40A4`
  → `$E00F` call had no `$40A7` return. Strict verification rejected it.
- There were zero authenticated CD→RAM origin receipts, zero dynamic CD_READ
  transactions, and no direct RAM-provenance sidecar. The single `$E009`
  dispatch had zero data reads. The VDC I/O trace reached its 2,097,152-row
  cap, so it is incomplete. This capture supplies no level/object semantics,
  gameplay transition, or rendering evidence; the original consumer gap
  remains open.
- A second isolated 120-second JP replay varied only the scripted event to a
  90-frame `run@9600` hold. It again observed 115 CD IRQs, 24 raw-sector
  spans, four SCSI reads, 24 sector bindings, no authenticated CD→RAM origin,
  and a single `$E009` dispatch with zero data reads. The event was consumed
  only at System Card poll `$E4B7`; no game/non-System-Card poll was observed.
  The longer hold therefore did not advance the capture boundary. Both runs'
  traces remain private under their separate trv2 work directories. Do not
  interpret this repeated startup path as evidence for JP world data or
  gameplay parity; next work needs an independently source-bound original
  title/menu entry route or a proven consumer trace.

## 2026-09-30 — preserve the semantics gap for the regional 64-word block

Authenticated US and JP Rev. 1 Track 02 tests now verify the same 64 words at
their edition-specific offsets (`0x1DA890` and `0x1DA0BC`) and byte-identical
preceding 16-word span. This establishes source identity only. Both raw-word
APIs now avoid assigning class/stat or XP/rank meaning; the blocks' purpose and
original runtime consumer remain open. Do not restore semantic naming until
original code or runtime evidence binds those meanings.

## 2026-09-30 — keep original-consumer test fixtures out of `/tmp`

The main-RAM consumer-trace test requires `TMPDIR` to point to a task-specific
test directory. Its authentic parser-only trv2 CTest passed and left that
directory empty. This removes a test-harness `/tmp` fallback only; original
Track 02 consumers and end-to-end Theron gameplay remain open below.

## 2026-10-01 — preserve authentic US/JP rank prefix bytes

The 15 JP Rev. 1 rank records at UD `0x89333` are authenticated as
byte-identical in visible text to the US table at UD `0x1C9B6B`. Both editions
now expose bounds-checked raw-record accessors, and the real-media tests compare
all returned record bytes against the authenticated BINs. The six custom prefix
bytes remain semantically opaque: rank-icon presentation, an original in-game
display consumer, and unrecorded progression slot 15 remain open.

## 2026-09-30 — keep Theron public status evidence-bounded

The gap list now labels rendering, mechanics, and seven-dungeon progression
as partial rather than fixed. Existing tests establish bounded data/runtime
paths, not original in-game presentation or end-to-end stair, pickup/use,
combat, and chapter-completion behavior. Continue the original-consumer work
below; do not promote these rows until authentic evidence closes those gaps.

## 2026-09-30 — distinguish BIOS-only controller polls

- ✅ `verify_theron_scripted_input_consumption.sh` now reports the PC of the
  first authentic controller-read witness and distinguishes the observed
  System Card polling sites `$E4B4/$E4B7/$E4C5/$E4C8` from a non-System-Card
  poll. The regression test accepts a generic non-System-Card witness but
  explicitly keeps a `$E4C8`-only trace at
  `game_or_non_system_card_poll_boundary=not_observed`.
- ✅ `tests/test_theron_v1_mednafen_live_capture_script.sh`, both shell syntax
  checks, and `git diff --check` pass. This improves evidence classification
  only; it does not change runtime behavior or prove title/menu input.
- 🔒 The authentic cold-start trace still shows the RUN event only at BIOS
  polling sites and no title/menu poll PC. Resolve why instrumented Mednafen
  remains on the System Card path before treating it as a gameplay capture.

## 2026-09-30 — isolate Theron menu-route input instrumentation

- ✅ The Mednafen 1.32.1 research patch previously ORed
  `TheronDratorGeneratorInputMask()` into every gamepad update, even when the
  Drator route was not selected. Its route-stage counters start at zero, so
  ordinary instrumented boots received unsolicited RUN pulses. The input
  producer is now gated on the explicit `FIRESTAFF_THERON_MENU_ROUTE=drator-generator`
  setting. The full research patchset applied to a fresh authentic source
  archive, the instrumented binary built serially on trv2, and the focused
  live-capture script test passed.
- 🔒 A real-media startup with the new binary, the authenticated JP Rev. 1 CUE
  and System Card, and host RUN at 25 seconds reached Mednafen's original
  controller poll. Capture stopped because the bounded VDC snapshot validator
  did not produce its required atomic receipt; therefore this is only an
  input-isolation check, not proof of menu artwork, gameplay, or Theron parity.
  A 105-second run recorded only 11,783 VDC writes and remained in the
  System Card input loop. Repeating with the input-read bound raised to
  1,048,576 observed the host RUN at `$E4C8` (`raw=0008`, `value=37`) after
  497,855 trace rows, but still recorded no authenticated Track 02 handoff;
  its VDC trace stopped at 40,354 writes before the 65,536 boundary. The
  controller poll is verified, while title selection and game launch are not.
  Both failed traces were reviewed and then removed with their isolated
  `/dev/shm` build artifacts; no media or trace payload entered Git.

The low-level campaign-mask projection now preserves raw bit 6 without
projecting it onto Demon completion. The original ordinal-6 capture stalls
before the completion write, so Demon completion semantics remain open; the
regression's `0x40` value is not a valid BRAM Continue state because the
source-locked restore routine rejects masked campaign values >= 7.

The runtime keeps the authenticated `$267C` campaign-completion mask separate
from quest-item collection: bits 0–5 project only to dungeon-completion state,
while bit 6 remains raw and does not complete Demon. Host item helpers can no
longer mark a dungeon complete, and exhausting stage selection cannot claim
quest completion. This closes a false host-side parity path only; the original
T900 pickup/retrieval consumer and final-stage completion event remain unbound.

The mixed-region data-directory regression for authentic JP Rev. 1 CUE
selection is closed by binding the Track 01/02 pair to the requested Track 02
identity. Bounded hashing now accepts single-image CUE slices in RAM. Track 01
availability is verified, but the original gameplay CDDA selection/event
consumer remains open below.

The static Track 02 scan finds two code-region `$E03F` call-site candidates per
authentic US and JP raw BIN. The earlier scanner mislabeled a nearby
`LDA #$0E` / `STA $FF` byte pattern as a track parameter. The US source-locked
stage-2 disassembly shows that sequence immediately before the `$E03F` call in
the handler at `$4339`; `$FF`'s role and the caller's gameplay-event ownership
are not established. The scanner and test now report only a nearby byte
pattern, not a CDDA track mapping. Keep gameplay audio dispatch disabled until
same-session caller-PC, playback-start and source-byte provenance are captured.

The world-level quest-item helper rejects invalid dungeon IDs, wrong-dungeon
bits and duplicate collection. It now also refuses to mutate a source-header-
verified level while the original T900 pickup consumer remains unbound, so a
fixture helper cannot synthetically unlock a real Track 02 exit. This keeps
production fail-closed; it does not bind authentic object occurrences to the
original pickup transaction or close the quest-item gameplay gap below.

The production chapter marker can now recover US artifact display names from
the hash-verified Track 02 retrieval-message records when the parallel item
name bank is unavailable. It can also extract all seven JP artifact spellings
from their authenticated regional retrieval records using the checked CP932
converter. This is only a launcher text fallback: the original
pickup-to-retrieval event remains unbound.

The production text gate now verifies that every decoded US/JP retrieval
record is byte-identical to its ordinal slice in the authenticated Track 02
message span and checks the source selector provenance fields. This proves
ordinal-to-retail-text binding only; pickup identity, possession, T900 state,
and campaign completion remain unbound.

The original-consumer capture marker verifier now correlates each required
consumer read with one unique earlier FIFO-origin receipt by sequence,
generation, source LBA/offset, logical and physical RAM destination, and byte
value. Its regression rejects mismatched or later receipts. This strengthens
the admission check but does not make any current capture pass or prove the
consumer's game semantics.

The old US dungeon-lore accessor no longer carries separate paraphrased
story strings; it returns the US Track 02 story records and their control
bytes from the shared source table. A real-media gate compares all seven
embedded records against the authentic US image byte-for-byte and guards their
source offsets/lengths. A dynamic, region-authenticated Track 02 story consumer,
JP story selection, and original presentation remain open.

The US skill-name API now shares the authentic 15-record text table at
UD `0x1C9B6B`; its real-media test verifies all records, including custom
prefix-glyph bytes for the six `MASTER` ranks. Progression slot 15 has no
verified text record, so the API returns no label there rather than repeating
`ARCHMASTER`. Both US and JP Rev. 1 Track 02 rank records are now verified at
their regional offsets; the 64-entry experience table does not establish a
record for progression slot 15. Original rank-icon presentation remains
unverified.

## Remaining work, ordered by the end-to-end playability dependency

The repository has real-media startup, source-data loaders, bounded mechanics,
and an authenticated US Continue-to-Drator route. That is not complete Theron
support: the original game-owned Track 02 consumer and quest-item transaction
remain the upstream blockers. Continue the following work in order; do not
replace absent original evidence with fabricated item, map, visual, or save
data.

1. **Track 02 runtime and object consumers (US and JP).** Close the source-LBA
   → game-owned RAM → executing consumer → source-record → reproducible input
   transaction chain. Current loaders and mechanics expose authentic bytes and
   bounded routes, but stairs, quest artifacts, pickups/use, dynamic creature
   generation and combat, and chapter completion still lack their corresponding
   original consumers. One US Drator `0c81` generator event now has a
   same-transaction runtime-witness receipt for its raw row, first consumer and
   unlink lifecycle. That receipt does not translate the row into a native
   creature and leaves type, HP, timer, map-local position, AI and JP parity
   unproven; it is evidence only, not generator support.
   Earlier authentic RUN replays exhausted their 65,536- and 131,072-read
   controller traces before the scheduled event's later poll could be observed.
   The research capture now defaults scripted plans to the maximum supported
   1,048,576-read bound and requires both a nonzero event-apply row and a
   post-event `$1000` controller-port read. A fresh authentic US cold-start
   passed that poll gate with the first later read at sequence 505,423; it does
   not prove title selection or gameplay. The full capture remains blocked by
   the strict VDC boundary gate (49,350 writes versus 65,536 required), so no
   transition receipt was emitted. Continue closing original game-owned
   consumers; a controller poll alone is not a Track 02 consumer.

   2026-09-29 authentic Linux-X11 capture loop: the JP Rev. 1 retail CUE
   (Track 02 MD5 `b7afb338ad31be1025b53f9aff12d73a`) with the hash-verified
   System Card produced 115 CD IRQ callbacks, 24 raw-sector spans and four
   SCSI read commands with 24 sector bindings. It reached the original
   `$40a4 -> $e00f` second-stage call but emitted no matching `$40a7` return
   and no authenticated CD-to-RAM origin receipt. The strict stage-two
   verifier therefore rejects this capture. A second cold start used the
   authentic US CUE and all 19 track files extracted from the operator's
   original archive into private trv2 scratch; Track 02 matched MD5
   `f23601102138f87c33025877767ebf76`. That run observed one CD IRQ, no raw
   sector spans, no stage-two calls and no authenticated CD-to-RAM receipts.
   Mednafen logged the CUE's unsupported `CATALOG` directive but continued
   opening its TOC; the capture does not show whether that warning affected
   boot. Both runs recorded host Run events followed by controller polling,
   which does not prove that the original game consumed the input. The raw
   traces remain outside Git at
   `/home/trv2/work/theron-x11-auth-capture-20260929/capture/`; these negative
   transport receipts authorize no level or gameplay semantics.
   A later read-only trv2 readiness check on 2026-09-29 found an accessible
   X.Org display on `:0`, `/home/trv2/.Xauthority`, Mednafen, `xdotool`, and
   authentic US/JP Track 02 media. The SSH environment itself has no `DISPLAY`
   set. That readiness check did not start an emulator; the existing
   transport-consumer and strict VDC evidence gates remained open.
   A new read-only check on 2026-09-30 found no Mednafen or Xorg process, no
   `DISPLAY` in the SSH environment, and `xdpyinfo :0` could not connect. Do
   not launch a capture against the shared display until a fresh readiness
   check confirms it is available.

   2026-09-30 capture preparation: an isolated workspace on trv2 now contains
   a fresh instrumented Mednafen 1.32.1 build from the official source archive
   (SHA-256 `de7eb94ab66212ae7758376524368a8ab208234b33796625ca630547dbc83832`).
   The build used the repository patch set and one build job; all eight runtime
   instrumentation markers are present, and the live-capture, controller
   patch, and palette patch checks pass against the clean source tree. This is
   capture-tool readiness only, not runtime/game evidence. The authentic
   System Card image (required MD5 `ff1a674273fe3540ccef576376407d1d`) is not
   staged in the trv2 Mednafen or Theron-data directories, so no capture was
   launched. The binary and source remain outside Git in the task-specific
   trv2 workspace.

   A later isolated headless JP Rev. 1 run used the authentic CUE and
   hash-verified Track 02 (`b7afb338ad31be1025b53f9aff12d73a`) plus the
   authenticated System Card (`ff1a674273fe3540ccef576376407d1d`). A
   360-second `run@9600:90,i@11000:8,ii@13000:8` replay produced three
   scripted-input events, each followed by an original CPU controller poll;
   all 1,048,576 bounded input-read observations were retained. The runtime
   emitted 115 CD IRQ callbacks, 24 raw-sector spans, four SCSI READs and 24
   sector bindings. It also emitted one `$E009` dispatch/return and five TII
   transfers, but zero `$E009` data reads, zero byte-exact FIFO destinations
   and zero authenticated CD-to-RAM receipts. The strict capture therefore
   returned `BLOCKED` at its 360-second bound with `transition=missing`.
   This is a fresh negative transport/runtime receipt, not a Track 02
   consumer, dungeon-entry, or gameplay proof. Raw output remains outside Git
   under `/home/trv2/firestaff-theron-auth-capture-20260929/capture/`; the
   capture used the dummy video driver and did not access the shared `:0`
   display or another agent's evidence directories.

2. **Complete a real dungeon transaction and progress save.** Bind the quest
   artifact's name, object occurrence, pickup, exit, and next-chapter handoff
   to original gameplay consumers. The authentic US DMS-SG.001 container,
   134-byte writer body, restore direction, and current/max-stat and skill
   experience meanings are already byte-bound in
   `docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md`.
   The `$267C` campaign-completion mask is kept separate from quest-item
   collection; the original pickup consumer and a changed in-game save
   transaction remain unproven. The authentic Continue regression now checks
   the real Akutuba-complete Backup RAM record independently: campaign bit 0
   marks Akutuba complete in Firestaff's progression state, while
   `quest_items_collected` remains zero and quest completion stays false. The
   source-locked ordinal capture proves this campaign bit but not the original
   pickup consumer or wholly independent state semantics in the original game.
   Generic host item-mask restore now leaves campaign completion unset; only
   the separately authenticated campaign-byte projection may mark stages 1–6
   complete. This is a Firestaff fail-closed rule, not proof of the original
   game's pickup or save transaction. Legacy snapshots that predate the
   campaign byte may resume only their saved current stage; they do not infer
   campaign completion or unlock other stages.
   Firestaff now has a source-gated in-memory encoder for Theron's persistent
   runtime fields and the selected original record. An unchanged Continue
   round-trips the authentic Akutuba-complete artifact byte for byte. There is
   now also an atomic explicit-path writer, tested against that artifact, but
   no authenticated stage-completion transaction calls it yet. Production
   Continue now discovers `theron-original.bram` in the selected save root
   ahead of the untouched baseline and fails closed if that user file exists
   but is corrupt. Wire the writer to proven gameplay progress and verify the
   changed result by reopening it in the original runtime before claiming an
   in-game original-format save workflow. The available authentic JP SRAM is
   empty, so JP Continue with progress remains unverified.
3. **Original presentation and event output.** Join game-owned Track 02 bytes
   and consumers to VDC/VCE screen ownership, text, portraits, CDDA selection,
   and ADPCM/SFX events. The admitted US capture is a bounded screen-space
   frame, while JP boot captures remain System Card/startup evidence; neither
   proves gameplay presentation. Track 01 playback alone does not establish
   gameplay audio selection. The live production route is
   `M11_GameView_StartTheron()` → `theron_v1_boot_startup_launch_alloc()` →
   `theron_v1_startup_runtime_load_initial_level()` → the Theron draw branch
   in `M11_GameView_Draw()` → `theron_v1_boot_runtime_render_frame()`. Level
   startup stays closed without a source-bound semantic handoff; its
   synthetic room fallback is fixture-only. Production viewport code in
   `src/theron/theron_v1_viewport_runtime_noop.c` can present an authenticated
   screen-space VDC/VCE capture, but has no source-bound square, object, or HUD
   renderer. The separate `tr_render_dungeon()` stub in
   `theron_v1_tile_renderer_runtime_noop.c` has no callers and is not the live
   rendering seam. Do not wire inferred tile/depth mapping into either path:
   first bind authentic gameplay VRAM/VCE/BAT state to the source-owned
   map/object consumer for both regions, then implement and capture the
   production drawing path. Installed authentic Track 02 BINs alone do not
   supply those semantics. The research capture runner now has PID-bound Linux
   X11 host-key delivery alongside its macOS Quartz route, with profile-derived
   key mappings and a Mednafen input-grab receipt gate. Shell/static regression
   checks pass, but trv2 currently has no active display and this new X11 route
   has not yet produced an authenticated game capture; it proves no gameplay
   semantics.
4. **Broader mechanics and completion tests.** Once each original consumer is
   bound, verify later-level transitions, objects, doors/actuators, combat,
   spells, inventory, chapter progression, and save/resume against both
   authentic regional media. V2 asset/effect/movement verification follows
   source-backed V1 gameplay and must use real assets when available.

   Firestaff's bounded inventory-input regression now exercises 431 authentic
   TAKE/DROP cases in each regional edition across all seven dungeons. Of 23
   authentic category-local raw-type-zero objects per region, 18 had an
   adjacent source floor for this route and five remain unexercised because
   that approach square is unavailable. This tests Firestaff's source
   provenance and input plumbing only; it does not establish original T900
   selection, reachability, or quest-item semantics. See the corresponding
   `DONE-theron.md` entry for the per-dungeon counts.

   The five deferred raw-type-zero scrolls share the same authentic source
   refs and coordinates in both regions (`5c00`, `1c02`, `5c03`, `1c02`,
   `dc00`). Their current Firestaff map projection places them next to only
   pits, walls, a secret wall, a door, or teleporters—not an ordinary floor
   approach. The original selection and movement/input sequence for these
   special-square occurrences remains unbound; do not mark them unreachable
   or make them collectible by assuming the projected tile enums are original
   T900 behavior.

   The all-level authentic census also finds no direct Track 02 coordinate
   teleporter link to any of the five cells. At the same coordinates on other
   loaded maps, Firestaff's projection shows only floor or wall, not a
   same-coordinate pit. These negative joins rule out those two simple route
   explanations in the current data projection; they do not identify the
   original pickup path or rule out another transition/input consumer. The
   neighboring authentic actuator records at D4/L1 and D5/L2 decode to target
   coordinates `(3,13)`, `(4,13)`, `(0,0)`, and `(4,13)`, none matching the
   adjacent deferred scroll cell. This only describes the record layout; the
   original actuator consumer and any indirect route remain unbound.

The sequence is dependency guidance, not a smaller completion target: the user
requested complete Theron support.

2026-09-29 stair census: the authentic probe now checks all 171 US and 170 JP
stairs across all seven dungeons. Of these, 39 US and 42 JP have an adjacent
ordinary-floor approach; all 81 query and original-command paths remain
blocked without mutating party pose, world tick, transition metadata, or queued
actuator-event count. This is regression
coverage of the source-evidence gate, not traversal support. Recover the
original consumer or an authenticated runtime transaction that binds each
stair attribute to direction, destination map and destination pose before
implementing transitions. The regional totals are explicit test assertions so
a broken/empty approach selector cannot masquerade as passing coverage. See
`DONE-theron.md` for exact test scope.

2026-10-03 regional stair-attribute source census: the hash-verified map test
now asserts all sixteen low-nibble occurrence counts independently for US and
JP Track 02. The US vector totals 171 and the JP vector totals 170. All values
occur in both regions, but the counts bind bytes only—not direction, target
level, target pose, or runtime behavior. The authentic transition consumer or
an ordinary-input runtime capture is still required before enabling stairs.

2026-09-30 authentic stair-hosted actuator census: the Track 02 real-media
test now lists every floor-party actuator whose occurrence is on a verified
stair tile. The eight US and eight JP occurrences are byte-identical across
the hash-verified regional files. Three linked records share one Drator stair
square; the remaining five occur on four other squares. This source census
selects future normal-play capture targets but binds none of the raw stair
attribute to a traversal direction or destination. See `DONE-theron.md` for
the regional hashes and full test result.

For a stair promotion, start from a normal playable original US or JP session
and enter an authentic-map stair through ordinary game input; do not edit
party coordinates, map tiles, or transition RAM to manufacture the event. Join
the raw source tile and its coordinates to pre/post dungeon, level, facing and
party-position bytes, the command dispatch, the executing HuC6280 PC and MPR
mapping, and any Track 02 read that supplies the destination level or arrival
pose. Capture the destination's actual arrival square and original screen for
the same transaction. Repeat for distinct stair attributes and both editions
before generalizing a direction/destination rule. Existing forward/backward
movement captures from the Akutuba start state do not cross a stair and cannot
authorize this promotion.

2026-09-27 authentic regional runtime input-state regression: the JP Rev. 1
and USA raw-BIN startup tests now compare a no-motion baseline with individual
native commands against the hash-locked regional Track 02 files; JP also
checks its Track 19 bank. Both editions enter Akutuba at party pose `(1,0,0)`;
`right` changes facing to `(1,0,1)`, and `down` moves to `(1,1,0)` while
advancing the Theron source tick from 0 to 1. The existing multi-input runs
remain covered. The JP real-media regression now also isolates left turn
(`(1,0,3)`) and the blocked forward step at the authentic Akutuba spawn
(`(1,0,0)`, tick unchanged). This proves Firestaff's input path mutates
source-map-backed runtime state and respects that source-map boundary for both
editions; it is not an original-game comparison and does not establish full
movement, collision, transition, rendering, or gameplay parity.

2026-09-27 authentic US raw-BIN movement extension: the USA regression now
isolates left turn and blocked forward movement at the same authentic spawn,
matching the JP checks. Both keep pose `(1,0,0)` with tick unchanged when the
forward destination is blocked; left turn changes facing to `(1,0,3)` without
advancing the tick. `theron_v1_raw_bin_runtime_boot` passes against the real
hash-verified `TQUS02.bin`. This closes a regional test gap only; it does not
prove original-game movement parity or later-level behavior.

The same workspace also rebuilt and reran `theron_v1_mechanics_playability`
against the installed authentic US and JP Track 02 BINs: 215 checks passed,
with no failures or skips. Each region exercised source-backed floor movement
and wall blocking in all seven dungeons. The probe explicitly keeps unresolved
stairs and real door interactions blocked; this is a regression check of the
bounded native mechanics, not original-game semantic evidence.

2026-09-27 local fresh-build verification: after rebasing the verified
Theron changes onto GitHub `main` at `ecf45acbe`, a clean macOS CMake
configuration and the application plus save-path targets build successfully.
The complete Theron-labelled CTest set finishes with 62 passed and six
skipped because those tests' specific operator-owned capture or regional-media
inputs are not staged in this local data view. Authentic media-backed routes
that are staged passed; no media was synthesized. This validates the current
local source and regression suite only; it does not close the remaining
gameplay or original-runtime gaps listed above.

2026-09-27 source-backed inventory-name receipt: selected carried Track 02
items now resolve their raw name only when champion/slot identity, source
origin, inventory type and the exact source property record still validate.
The authentic US/JP dungeon-loader test confirms the name is preserved by a
real-data pickup and rejects a mutated property record. `firestaff` and the
dungeon-loader target build locally; the real-media dungeon-loader and JP
raw-BIN startup CTests pass. This lookup currently feeds the M11 boot-probe
receipt only. It does not render item names or establish original inventory
UI/gameplay semantics, which remain open under the runtime-consumer and
presentation gaps above. The broader configured Theron selection was not
counted as passing because its build directory lacked several probe binaries.

2026-09-26 trv2 clean-tree verification: a fresh archive of source commit
`2756e9800aadb23d26b39d853921bf71021d3658` contained tracked project files
only; no game media was copied to the build host. The project, `firestaff`,
and the selected Theron test targets configured and built successfully on
trv2. Six CTest cases passed against the authentic media already present in
trv2's user data directory: `theron_v1_pce_bram_real_artifact`,
`theron_v1_m11_real_bram_continue`, `theron_v1_track02_full_item_names`,
`theron_v1_combat_runtime_source`, `theron_v1_jp_later_dungeon_runtime`, and
`theron_v1_startup_real_asset_receipt`. This verifies those save/import,
source-name, combat-boundary, JP-record and startup-receipt gates on the
current source snapshot; it does not prove a complete campaign, original
quest-item transactions, or original-format BRAM export.

2026-09-28 authentic inventory transaction regression: the focused Track 02
loader test executes source-backed TAKE and DROP against real Akutuba objects
in both US and JP media. Each region verifies 71 item transactions, including
all six category-local type-zero records, preserving source occurrence, origin
and property-row bytes through inventory transfer. Raw type zero is represented
as internal compact inventory ID 126; the provenance receipt retains its
authentic raw zero. Altered source or property bytes are rejected before
inventory mutation. On trv2, the focused loader target built and both
`theron_v1_track02_dungeon_loader` and `theron_v1_inventory_id_mapping` passed
2/2 against hash-verified TQUS02/TQJP02; verbose output reported
`type-zero records tested/deferred: 6/0` for each edition. The test temporarily
suppresses earlier co-located occurrences only to select each authentic object
and restores their flags afterward. The test also routes all six type-zero
records through M12 pickup, carried Track 02 name lookup, inventory-slot
selection and drop, verifying the input/name plumbing and same-occurrence
return against the authentic US/JP objects. This verifies Firestaff's bounded
inventory handoff, not original T900 pickup/UI semantics or quest-item
collection transaction.

2026-09-28 full registered Theron CTest selection on `trv2`: all 71 Theron
tests passed after building the missing test executables and `firestaff`.
Eight optional tests skipped for unavailable capture/archive/ISO inputs. The
regional edition scanner accepts the hash-verified JP Track 02 BIN chosen from
its CUE sibling set, and the JP runtime regression prefers that CUE-paired
Track 02 when available; both fall back to authentic standalone media. This
is broad Firestaff test coverage, not original-runtime gameplay or complete
campaign parity.

2026-09-28 source-backed inventory swap integrity: source-level inventory
swaps now require each compact champion ID to match the raw type in its
authenticated Track 02 occurrence, in addition to the existing record and
property checks. A mismatch is rejected before either slot or provenance can
move. The real-media dungeon-loader regression corrupts the compact ID and
checks rejection for authentic Akutuba occurrences in both regions, then
byte-compares both slot values and provenance records to verify that the
rejected swap changes neither. On macOS, the app and focused loader targets
built successfully; CTest passed both
`theron_v1_track02_dungeon_loader` and `theron_v1_inventory_id_mapping`. The
loader reports 71 TAKE/DROP cases per region, with all six type-zero records
tested and none deferred. This is Firestaff source-integrity coverage, not
evidence for the original T900 inventory-swap consumer or complete gameplay
parity. A fresh Linux build on `trv2` also produced `firestaff` and both
focused test targets; the two tests passed 2/2 against the authentic US/JP
Akutuba data, with the same 71 per-region TAKE/DROP cases and zero deferred
type-zero records.

2026-09-25 authentic JP Rev. 1 CD availability: the production CUE receipt
passes locally and on trv2 against the complete user-provided CUE and its
nineteen original sibling BIN files. It verifies the canonical layout (17
AUDIO and two MODE1/2352 data tracks) and readable backing files for all 19
tracks. The availability regression can optionally run the same receipt
against an operator-supplied authentic CUE; no media is created or
substituted. This proves complete-disc file/layout availability only. Track
01 remains the only CDDA stream connected to the title lifecycle. The generic
verified-media handoff can now select CUE-declared audio tracks 03–18, but no
original gameplay event-to-track/ADPCM/SFX mapping is bound, so these tracks
are not automatically started during gameplay.
`theron_v1_track01_cdda_handoff` also passes locally with this JP CUE and starts
the original raw Track 01 through SDL's dummy output.

The rebuilt local Firestaff executable also passes
`test_theron_v1_jp_cue_runtime_boot.sh` against this complete authentic JP
CUE: it reaches the native Akutuba runtime and accepts six movement inputs.
This verifies that specific source-backed JP startup/runtime path, not later
dungeon transitions or full original-game parity.
The same executable passes `test_theron_v1_jp_raw_bin_startup.sh` against the
original JP Track 02 and Track 19 files in `.firestaff/data/theron`, including
the source-backed movement and JP Track 19 item-name checks.
`theron_v1_jp_later_dungeon_runtime` also passes locally against the authentic
regional Track 02 files, checking source-backed records across all seven JP
dungeons. Original game-owned transition semantics remain unverified.
The authentic combined USA RAR direct-boot regression passes locally as well;
it resolves the original `TQUS19.iso` + `TQUS02End.iso` Track 02 members in
memory without extracting the archive. Its CUE handoff regression also passes
against the original RAR and starts the original Track 01 audio stream through
SDL's dummy output.

The CD availability parser also rejects duplicate track numbers and any
missing number in the canonical 1..19 CUE sequence. Its duplicate-declaration
regression and the authentic JP Rev. 1 receipt both pass; malformed CUEs can
no longer receive a ready receipt solely because their maximum track number
is 19. Raw `.bin` audio extents must also be non-empty and divisible into
complete 2,352-byte CDDA sectors, and no track may resolve to an empty file.
The duplicate, empty-file and partial-sector rejection tests and the authentic
19-track JP receipt all pass.

2026-09-25 authentic JP full-disc replay audit: the private
`theron-authentic-jp-full-disc-20260925` capture uses the hash-locked Rev. 1
Track 02 (`b7afb338ad31be1025b53f9aff12d73a`) and System Card
(`ff1a674273fe3540ccef576376407d1d`). Its transition receipt records 131,072
input transactions, 24 raw-sector spans, 25 CD IRQ callbacks, one game-main
`$E009` dispatch, zero authenticated CD-to-RAM receipts, no transition, and a
65,536-byte VDC snapshot plus 65,536 VDC I/O writes. The paired main-RAM
consumer sidecar has 512 reads in `$2600-$27FF`; all are zero-valued BIOS
`$CB22` reads. This is authentic-media negative evidence, not a gameplay or
dungeon handoff. The local Mednafen save directory contains no JP non-empty
BRAM/save artifact, so a JP continue-state replay is not presently available.
Capture and sidecars remain ignored local scratch and are not tracked.

2026-09-25 authentic US viewport parity: the real Akutuba VDC/VCE bundle
replays to Firestaff's 320x200 source-only frame. The optional real-capture
test now calls the same `theron_vp_init_from_data_dir()` admission route as
M11; it previously called the generic unbound initializer and never exercised
the bundle. With `THERON_EXPECTED_SOURCE_BMP` pointing at the operator-local
original Mednafen frame and `THERON_VRAM_CAPTURE_BMP` at Firestaff's output,
the two BMPs match byte-for-byte (SHA-256
`b6fc9a8c0ade4a92716ef2514dacf9337eb2cf585d62f36ac91382d4cf766d16`). This
proves only this captured US screen, not gameplay semantics or JP rendering;
the reference image remains local.

The Mednafen CD-state parser now accepts this negative replay's four mandatory
instrumentation markers: the optional CD-transfer marker is emitted only when
a destination candidate exists, so requiring it rejected captures precisely
when no such candidate was observed. The parser still requires every requested
raw sector and SCSI binding to match and keeps semantic publication blocked;
the real JP sidecar passes at 24/24 sectors and bindings. The authenticated
VDC-I/O verifier independently replays the exact snapshot boundary: 30,453
VWR commits write 12,544 words, all 12,544 match the captured 64 KiB VRAM
snapshot, with zero mismatches. Visual inspection of the captured 256x240 VDC
frame shows the PC Engine CD-ROM System Card screen, not Theron game graphics.
Its 512 init-only `$2600` reads are zero-valued BIOS `$CB22` reads; the capture
has no authenticated CD-to-RAM receipt and no game transition. Do not promote
this bundle into the Theron runtime or a game screenshot. A trial admission
and CLI pass were reverted after the visual check exposed this source mismatch.
The correct production boundary remains fail-closed until a game-owned capture
is joined to the Theron media and consumer. Capture and sidecars remain local.

2026-09-25: Fixed the production M11 boot path so explicit CLI-provided
VRAM/VCE/VDC-state/SAT/VDC-I/O files are passed as one authenticated bundle to
the native viewport. Previously the CLI accepted the five paths but
`theron_vp_init_from_data_dir()` ignored them and booted without the capture.
The viewport now prefers that explicit bundle and fails closed on a partial or
invalid override rather than discovering an unrelated screen. The CLI
real-capture regression and the real VDC viewport regression both pass against
the locally supplied authentic Track 02 BIN and hash-locked capture. The full
Theron label suite also passes (68 tests, six skipped for unavailable
capture/media inputs); this is capture wiring only and does not close the open
dungeon, UI, or gameplay semantics.

The same build and 68-test Theron label suite also completed on Linux `trv2`
with no failures (21 capture/media-dependent skips in that host's staged data
view). Its authentic USA CloneCD ZIP startup/runtime route, USA raw-CUE route,
and JP CUE route passed there. The larger skip count reflects that host's
distinct data staging and is not a synthetic-data substitution.
After explicitly staging the same five authentic, hash-locked capture files
in a temporary `trv2` test directory and pointing the CLI test at the actual
US Track 02 BIN in that host's data view, both
`theron_v1_vram_trace_real_capture` and
`theron_v1_cli_authenticated_capture` passed on Linux as well.

2026-09-25: Removed the data-free synthetic first-room probe and its fabricated
stair assertion. Current dungeon evidence comes from the authenticated
regional source-dungeon and mechanics regressions; those pass with the real
US/JP Track 02 files. No product behavior was enabled by removing the probe.

2026-09-25 local regression audit: all 278 tests selected by the `theron_`
CTest name prefix completed without a failure; 18 returned the configured
skip status because their original-runtime captures or other required local
inputs were not configured. With the authentic US/JP Track 02 files in the
local data directory, the regional level-descriptor, level-block, dungeon-map
and thing-data checks passed, covering all seven map groups and their source
records. The US Track 02 file-select text-source test was also rerun against
the locally available hash-verified BIN and passed. These source-data tests do
not prove that the original game renders or consumes those strings.

2026-09-25 local revalidation: the supported instrumented Mednafen build now
uses the official SDL 2.32.10 headers/runtime pair and starts against the
authentic US full CUE, System Card 3.0 and local Mednafen state. The
signature-bound research hook reaches the original post-dungeon dispatcher at
`$DE38` and records the injected ordinal, but the 45-second run ends with one
unmatched RNG entry at `$4667`; the capture verifier rejects it and emits no
transition receipt. This is source-execution evidence only, not proof that the
game selected that ordinal or completed a dungeon transition. The isolated
VDC-I/O replay parser accepts the authentic trace and exactly matches 9,728
written VRAM words, while semantic publication remains blocked.

2026-09-25 local authentic-state replay: rebuilt a temporary full US CUE from
the supplied archive's original track order, its original OGG CDDA members,
the hash-verified Track 02 ISO (`ceb02343868f80cec899e9b239aff2da`), and the
user's authentic Track 19 ISO. With System Card 3.0, the hash-verified
Akutuba-complete Mednafen state (`f17f377df210b4a3ae904a13fb85a7f0`), and
instrumented Mednafen (`f3fa332485bc3074e70ffbc3c4bf9a9d`), the 45-second
capture records 40,980 input transactions, one CD IRQ, no non-System-Card CD
reads, no raw-sector spans, no authenticated CD-to-RAM receipts, and no
game-owned `$E009` dispatch. Its same-session 8 KiB save-manager code page
matches the previously authenticated page byte-for-byte (MD5
`6b520314faa729149a91556488a421c4`); the captured 2 KiB BRAM remains identical
to the original artifact (`ffabc8d19b0915d4d9632a7ae2e90a97`). The real-BRAM
regression passes when supplied this capture's main RAM and code page. This
revalidates save-field/source correspondence only; the state replay has no
source-sector join or level transition and does not open gameplay semantics.
Raw captures and the temporary normalized CUE stay local under ignored
`.codex-scratch/`.

2026-09-25 cold-start input replay: a 120-second run against the authentic US
MODE1/2048 CUE, hash-verified Track 02 ISO and System Card 3.0 applied five
scripted controller events and confirmed their wire masks (`RUN=0x0008`,
`I=0x0001`) in the original HuC6280 input reads. It read 25 authentic raw
sectors across four SCSI commands and entered `$E009` once, with 24 register
writes but zero game-owned E009 data reads, zero authenticated CD-to-RAM
receipts, and no level/party publication. Repeating I later in the same cold
boot did not advance the source loader. This is negative transport evidence;
the capture and its raw sidecars remain local under ignored `.codex-scratch/`.

2026-09-25: The authentic US CUE from the combined local archive now retains
its hash-bound CUE provenance through M12→M11, binds the exact sibling Track 01
audio and loads the regional Track 19 metadata bank from that source directory
even when Track 02 was materialized into cache. The CDDA handoff accepts the
original 44.1 kHz stereo PCM inside a validated RIFF/WAVE container and starts
the bounded SDL stream without queuing container bytes; authentic JP raw BIN
CDDA and the archive's OGG transcode are also covered. The real US CUE M12→M11
handoff passes; lower-level authentic CUE checks use real media, not substitute
audio.
The combined US/JP RAR now launches directly when external archive tools are
explicitly enabled: Firestaff reads the authentic `TQUS19.iso` and
`TQUS02End.iso` members in bounded memory, verifies their concatenated
Track 02 digest, and extracts no game data. The authentic Japanese 7z also
launches by member hash, even when a different real ISO is placed under the
expected loose-file name. These real-media startup tests prove media admission
and the title/startup route only; they do not prove dungeon runtime or parity.

The full authentic Japanese Rev. 1 CUE from trv2 was exercised locally without
changing that checkout: Firestaff reaches its bounded Akutuba route, the
source-only loader accepts all seven dungeons (34 maps, 2,269 objects), and
Track 01's original raw CDDA starts through the SDL audio stream. The complete
authentic US CUE also starts its original PCM/WAVE Track 01 stream. The CDDA
regression recognizes source layouts from the CUE-declared Track 02 sector
width and validates RIFF/WAVE format and data bounds before playback. This does
not establish the original emulator's game transition or full
audio/presentation parity.

2026-09-25: The broad M12 inventory no longer treats the catalogued JP Rev. 1
Track 02 ISO digest as launchable. The supplied file is a hash-matching,
zero-filled 149-sector stub and the strict Track 02 intake already rejects it
as empty source content. A real-media regression covers the inventory gate;
the authenticated US and JP raw-BIN startup routes and seven-dungeon loader
remain green. This does not close Japanese ISO acquisition or runtime parity.

2026-09-25: Removed the entire inferred square-to-tile table from the viewport,
including fixture-only wall, floor, door, portal, pool and stair indices.
Both production and tests now refuse to infer atlas ownership from a Track 02
square type. Real US/JP Track 02 loader checks and the US raw/ISO comparison
pass; the authentic square/material consumer and original stair transitions
remain open.

Firestaff's product runtime is native. Emulator instrumentation may be used
to acquire evidence, but emulator launch and BIOS/System Card dependencies
are not product features.

The supplied Japanese Rev 1 Track 02 has been checked directly by
`theron_v1_track02_level_data_blocks`: its raw sectors bind the seven
described level blocks, their shared prologue and per-level metadata, while
mutated bytes are rejected. The native source-dungeon test also loads all 34
maps across those seven dungeons and requires their real map headers, thing
directories and dungeon-local property tables. This is source-data
verification only; it does not promote uncaptured transition, presentation,
save or item-action logic. The assembled authentic US MODE1/2048 ISO is now
byte-compared with the raw US user-data stream after its 225-sector pregap;
the source-only loader reproduces all 34 maps and 2,269 source-object records
across the seven dungeons with identical tile grids and property provenance.
The supplied US CloneCD ZIP is also a native source owner: its `.ccd` and
bounded `.img` Track 02 slice reach the title/startup route directly in memory
without an emulator, BIOS, extracted game tree or fallback graphics.
On trv2, the full ZIP regression now passes direct launch, keyboard selection
of Original and Modern, the mouse-only card route and the normal launcher
handoff into a source-backed runtime level. Its wait tokens were shortened so
the three clicks fit inside the phase-A test window; no game-data substitute
or fallback was involved.
The 149-sector JP Rev. 1 `TQJP02End.iso` matches its known hash but the
supplied bytes are entirely zero-filled. It is not usable dungeon content;
both the source loader and campaign-media launch intake explicitly reject it.
The authentic JP CUE-projected ISO is byte-identical to the authentic raw
Track 02 user-data stream after its 224-sector INDEX 01 prefix. The native
runtime restores only that zeroed coordinate prefix and loads all seven
source dungeon banks using the existing JP BIN decoder; the CUE ISO test
verifies 34 maps and 2,269 source objects without manufacturing raw-sector
spawn records. Dungeon-transition, visual, combat and item-action parity
remain open.

The JP raw-BIN M11 and full-CUE startup regressions now send six native
movement commands through authentic Akutuba and require the resulting party
pose `(direction=2, x=3, y=0)`. The raw-BIN route starts with two champions;
the CUE route starts with one. This is bounded host-runtime movement on the
real Japanese map; it does not establish parity against a Japanese
original-runtime capture, JP Continue, or movement behavior in later dungeons.

The authentic US CloneCD ZIP menu-to-runtime regression now also sends the
same six native movement inputs on Akutuba and requires the resulting party
pose `(direction=2, x=3, y=0)` with three source-backed champions. This
extends the bounded movement check to the real US launch route; it does not
establish visual/gameplay parity or movement behavior in later dungeons.

The raw-US-BIN mouse-card regression now uses the same bounded inter-click
waits and phase-A window as the passing CloneCD mouse route. On trv2 it passes
through the original/platform/presentation cards while preserving the verified
raw Track 02 route; this corrects the test timing only and does not change
product input behavior.

The registered `theron_v1_jp_later_dungeon_runtime` regression now binds and
checks both authentic regional raw Track 02 files separately. On trv2, the JP
and US sources each passed all seven source-dungeon handoffs (34 maps and
2,269 source objects), including per-map header/property provenance and
dungeon-local thing-directory verification. This closes a regional test-coverage
gap only; the receipt still explicitly keeps visual capture, original
transition, combat and item-action semantics gated.

The combined US/JP RAR's CUE names `TQUS02.iso`, while the archive carries the
authentic US logical Track 02 across `TQUS19.iso` and `TQUS02End.iso`. The
native resolver now recognizes that exact source composition by content hash;
the separate authentic `TQUS02.bin` remains preferred when present. Original
CD-runtime transition, presentation, combat and later-dungeon parity remain
open. No substitute game data has been generated.

- Bind the verified Japanese Rev 1 Track 02 source dungeons to captured
  transition and save consumers. Regional champion records and source-backed
  pickup/drop are bound, but broader item-use semantics remain gated; the
  current all-seven dungeon loader remains source-only.
  A fresh 2026-09-23 JP Rev 1 cold-start replay authenticated the raw Track 02
  sectors (`b7afb338ad31be1025b53f9aff12d73a`) and emitted 24 raw-sector spans,
  but no authenticated CD-to-RAM receipt, zero main-RAM E009 data reads, and
  no dungeon-state handoff. A second replay using the existing opcode-gated menu
  research hooks did not match any hook signatures and reached the same
  boundary. A third replay delivered 11 planned PCE inputs through frame 5800
  and again produced 24 raw-sector spans, zero authenticated CD-to-RAM
  receipts and no dungeon-state handoff. A fourth replay delivered 30 planned
  PCE inputs through frame 9600 and again produced 24 raw-sector spans, zero
  byte-exact origin-RAM or authenticated CD-to-RAM receipts, one game-owned
  `$E009` dispatch, zero `$E009` data reads, no command-buffer consumer reads,
  and no dungeon-state handoff. Its bounded PCE input-result trace recorded
  6,235 reads of controller register `$1000` returning `0x37` (scripted Run)
  against the neutral `0x3f`; this confirms hardware-level input delivery,
  not a game action or transition. The private capture remains on trv2; its
  raw media and trace files are not part of the repository. The Mednafen
  transport receipt parser now identifies JP and US Track 02 hashes
  separately, but that tooling change does not make these captures gameplay
  or source-consumer witnesses.
  A fresh 2026-09-24 US replay used the authentic raw Track 02
  (`f23601102138f87c33025877767ebf76`), System Card 3.0, and the existing
  2 KiB SRAM (`ffabc8d19b0915d4d9632a7ae2e90a97`). It delivered a documented
  RUN/D-pad/Button-I sequence; the PCE input trace observed active-low RUN,
  direction, and Button-I reads. The run emitted 25 raw-sector spans and one
  game-owned E009 dispatch/entry, but zero E009 data reads, zero authenticated
  CD-to-RAM receipts, and no dungeon-state handoff. Thus controller delivery
  is now evidenced, but menu selection and level loading are not; the private
  capture remains on trv2 and does not promote gameplay semantics.
  Two additional 2026-09-24 JP Rev 1 cold-start captures used the authentic
  regional CUE/Track 02 and System Card 3.0 with Mednafen's instrumented
  frame-scheduled PCE input. The first valid four-event plan was applied over
  131,072 input transactions; its real Xvfb frame showed the System Card UI,
  not Theron gameplay. A second plan applied Run from frame 1 and held later
  Run inputs; its captured frame was black. Both runs still produced 24 raw-
  sector spans, one game-owned E009 dispatch/entry, zero E009 data reads, zero
  authenticated CD-to-RAM receipts, and no dungeon-state handoff. These are
  negative startup/input observations only. Their raw traces and screenshots
  remain private on trv2 and are not promoted as public game captures.
  A 2026-09-26 recheck of the authentic `jp-scripted.trace` bundle confirmed
  the same boundary: its transition sidecar binds the real JP Rev. 1 Track 02
  and System Card hashes, but records zero non-System-Card reads and zero
  authenticated CD-to-RAM receipts. Its VDC state is a 32×32 BAT at 320×240,
  outside the currently admitted 64×64/320×200 screen contract; the real
  capture viewport test rejects it. Do not admit this boot/System Card screen
  as Theron graphics or infer JP gameplay from its pixel data.
  A further 2026-09-24 JP Rev 1 X11 capture used the Linux profile's actual
  `command.toggle_grab` binding (Ctrl+Shift+E; the earlier attempt incorrectly
  sent Ctrl+Shift+G) and waited through the documented eight-second BIOS
  startup window before sending RUN. The instrumented PCE register trace then
  observed controller value `0x0008` during RUN, confirming gamepad delivery;
  nevertheless the frame became black and the final receipt still had 24 raw
  sectors, zero authenticated CD-to-RAM receipts, zero E009 data reads and
  `transition=missing`. This improves the input-delivery diagnosis only; it
  does not establish title/menu selection or gameplay. The trace remains
  private on trv2.
  Two further 2026-09-24 US captures used the authenticated full CUE, System
  Card 3.0 and the original Akutuba-complete 2 KiB Backup RAM image
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). One enabled the opcode-gated Drator
  menu route; the other also enabled the exact title-wait RUN hook. Neither
  research hook logged a match. Both captures emitted 25 authentic raw-sector
  spans and one game-owned `$E009` dispatch/entry, but zero `$E009` data reads,
  zero authenticated CD-to-RAM receipts, and `transition=missing`. The output
  BRAM remained byte-identical to the input. These runs confirm that the
  authenticated save is present in Mednafen but do not establish original
  title/menu selection, Continue, dungeon entry, or gameplay. The private
  traces remain on trv2 and are not promoted as runtime evidence.
  Two further 2026-09-24 cold US replays scheduled their only scripted RUN at
  the documented System Card wait frame 9600, using the same authenticated
  CUE, System Card 3.0 and 2 KiB BRAM
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). The 240-second run emitted the input
  event at line 196,615 of 196,618, so it ended immediately after RUN and did
  not test the follow-on menu. A 600-second follow-up confirms the event was
  applied and continues emulator execution to its timeout; its output BRAM is
  byte-identical to input, yet it still emits only 25 raw-sector spans and
  115 CD IRQ callbacks, with zero authenticated CD-to-RAM receipts, one
  `$E009` dispatch/entry but zero `$E009` data reads, no Drator-route hook
  matches, and `transition=missing`. Extending capture time past the scheduled
  input therefore does not recover the route. Neither run establishes title
  selection, dungeon entry, or gameplay. The private traces remain on trv2.
- Validate the production Continue action end-to-end with authenticated
  dungeon-entry capture. The native M11 route is now verified with authentic
  US Track 02 and a 2 KiB Akutuba-complete Backup RAM artifact: it admits slot
  0, restores all seven attributes and all 20 temporary/persistent
  skill-experience pairs against the decoded body, skips completed Akutuba,
  selects the next available Track 02 champion and loads dungeon 2, level 0,
  through Soul Room and forcefield. The original-emulator transition has not
  yet been captured for parity. Japanese Rev. 1 CUE media now reaches its
  source-backed Akutuba runtime on trv2, but JP Continue remains unverified.
  A separate authentic 2 KiB JP HUBM/DMS-SG.001 save candidate was found
  there (MD5 `dbdedb0ec809227b289c2bc5b18b9c9d`); its selected slot's campaign
  byte is zero, so it does not establish saved progression or qualify as the
  progressed-save evidence needed for JP Continue. No progressed authentic JP
  Backup RAM capture is currently staged. The same authentic run also
  verifies a native three-step movement route through floor tiles in the real
  loaded map; movement parity beyond that bounded route remains open.
  Production now ignores the Firestaff-only `.tqsv` container; it remains
  available solely to fixture/tooling targets and cannot substitute for the
  original T080/T800 save consumer. The original writer layout is now bound
  byte-for-byte as 1 + 6 + 7 + 6×20 bytes. The writer has now been proven to
  read only `$267C` after loading a slot and to overwrite `$267D..$2701` from
  live RAM; it is not their restore consumer. The separate US and JP restore
  routines are now byte-bound in all seven regional dungeon blocks and copy
  every section back to their region-specific live-RAM columns. Their
  downstream consumers now identify Theron's three maximum vitals, seven
  maximum attributes and all 20 temporary/persistent skill-experience pairs.
  Both regional Stage 2 routines also prove that the final two bytes of each
  `$88`-byte slot are cleared transport padding and never enter the `$86`-byte
  gameplay restore. The proven body fields now apply transactionally to an
  authenticated roster-owned Theron while companions, inventory, equipment,
  position and loaded media remain unchanged. Production startup now detects
  the verified real Backup RAM artifact (or an explicit
  `FIRESTAFF_THERON_BRAM_PATH`) and the explicit Continue action uses this
  transactional route.
- Capture and decode original bitmap, palette, text, ADPCM and remaining audio
  ownership for production presentation; Track 01 raw CDDA/WAVE/OGG is now covered,
  while fallback visuals remain disabled.
- Verify JP and US runtime, save and later-dungeon behavior separately. The
  authentic US MODE1/2048 ISO now reaches the source-backed Akutuba
  forcefield handoff; this does not establish complete campaign, gameplay,
  visual or transition parity. Do not infer JP offsets or gameplay semantics
  from US media.
# Firestaff TODO - THERON

## 2026-08-20 — Japanese raw Track 02 reaches the game runtime

- ✅ Complete original CUEs with authentic WAV audio tracks are now accepted
  by the same strict 19-track gate as archive-backed OGG materialization.
  Previously, the media classifier counted only OGG files and therefore
  rejected a complete WAV/ISO disc as if its audio tracks were missing. The
  US, Japanese, combined-RAR, and wrong-region rejection paths are verified
  against real media.

### Track 19 names in the native runtime

- ✅ Object names are no longer read from a single hard-coded table for
  dungeon 7. Each of the seven dungeons has its own 66-entry table in Track
  02, and all 14 regional tables are now read directly from the authentic US
  and JP files. Each table's offset, length, and FNV must match before it is
  bound to the live world.
- ✅ A source-bound object with a verified property record can look up its
  name using the object's dungeon and `source_item_type`. This gives the same
  index the correct dungeon-local name. Entry 64 in dungeon 6 is genuinely
  empty in both regions and remains empty rather than being filled with
  fallback text.
- ✅ The adjacent 66-byte table was misclassified as a global category map
  derived from Demon. The authentic tables instead contain dungeon-local
  type codes and differ across every quest block and between US and JP.
  Production now preserves all 924 type codes with their own FNV values and
  uses the thing record's actual category 5, 6, 7, 8, or 10 for property
  binding. Thus every real materialized item other than chests receives its
  authentic 6-byte property record.
- ✅ Native startup now reads all 69 authentic Track 19 names from exactly
  the region identified by the hash-verified Track 02 file. US text is stored
  as source ASCII and JP text as unchanged Shift-JIS bytes; a mismatched file
  hash, region, or table span is rejected. US and JP startup tests require
  the correct name bank to be present in the live world.
- 🔒 Track 02 objects use their own dungeon-local namespace. There is no
  simple index mapping to the differently sized 69-entry Track 19 table.
  The Track 19 bank can therefore only be read using an explicit Track 19
  index. JP bytes are not sent to the host text renderer; the original T900
  consumer and a verified Shift-JIS glyph path remain open.

### Authentic VDC geometry for capture replay

- A new simultaneous HuC6270 snapshot from an authentic US dungeon save
  state has `BXR=0000`, `BYR=0000`, `MWR=005a`, `HDR=0327`, and `VDR=00c7`.
  This proves a 64×64 BAT and a 320×200 active background, not the previously
  hard-coded 256×224/32×28 view.
- ✅ Firestaff's tile base, GRB333 channels, shared BG color 0, and unmapped
  12-bit tile indices now follow HuC6270/HuC6260. The production gate
  requires VRAM, VCE, `.vdc-state`, and 512-byte SAT data from the same
  snapshot, and renders the `MWR/HDR/VDR/BXR/BYR` geometry without crop or
  fixture assumptions.
- ✅ SAT replay follows the original hardware's 64 entries, maximum of 16
  sprite parts per scanline, SAT priority, BG priority, transparency,
  16/32/64-pixel height, 16/32-pixel width, and H/V flipping. The PCE's used
  9-bit palette indices are losslessly compressed to M11's 8-bit surface;
  the real frame uses 52 source entries and contains 211 verified sprite
  pixels.
- 🔒 This authentic screen capture is a verified frame, not a dynamic native
  graphics engine. Full game-driven VDC/SAT updates in the internal Track 02
  runtime still require the later graphics/object consumer; until then, the
  complete original runtime remains available through Mednafen.
- ✅ The verified internal graphics stage-2 chain now also includes the real
  bank-2 frame dispatcher `$4215..$424B`, its coordinate updater
  `$4417..$4552`, and the complete `$42DB..$43A1` routine with local
  subroutines `$4358` and `$4386`. The caller's JSRs bind `$42DB`, `$4417`,
  `$424B`, and `$458E`; nested calls bind the already verified `$43A1`,
  `$43D6`, and `$4552` bodies. The authentic banked graphics/VDC dispatcher
  `$4943..$49FA` is also bound, including MPR save/restore and all nine
  absolute subroutine targets. Its direct `$49FA..$4A09` dispatcher in turn
  binds rendering paths `$4A09` and `$4A84`. Bodies `$4A09..$4A84` and
  `$4A84..$4B24` are now bound together with shared address calculation
  `$4B24..$4B3C`. `$491F..$4932`, which enables the VDC control bit before
  the second strip, is also verified. In addition, scroll consumer
  `$4BB0..$4C0D` is bound to the real `$220C/$220D/$2210/$2211` registers, as
  is `$56DE..$571A` with its 1 KiB TIA transfer from `$58E0`. `$50F1..$5111`
  also binds a 512-byte VDC write from `$4EF1`, while `$5E2B..$5E81` binds
  the original register 6/7 dispatcher. `$5CE4..$5D1C` binds the producer's
  real initialization and clearing of the 1 KiB buffer at `$58E0`. The
  direct `$5111..$533D` family is also bound: eight 32-byte entries, call
  site `$533D`, local targets `$517A/$519F`, and all 15 real handler
  addresses in the `$51E8` table. The secondary `$533D..$555E` family also
  binds its 16-entry target-minus-one table and every handler body.
  `$55EF..$560B` and the overlapping entry points
  `$55F4/$55FF/$5617/$562A/$563D` are also verified; the direct `$563D`
  stream is distinguished from the main flow's BCC operand. Its signed BBR4
  branch to mid-entry `$55C8` is bound along with the entire `$55B6..$55E0`
  range. A total of 4,899 original bytes (14.1 percent of the stage-2 image)
  are now exactly bound. `$3B75` is runtime data, not a code gap. Additional
  stage-2 routes and a native consumer are still needed before Firestaff can
  run the graphics chain internally, but the nine direct targets of
  dispatcher `$4943` and this dynamic graphics subchain are now byte-bound.

- ✅ The native loader now keeps a door record's actual material type and
  two-bit thing position separate from runtime state and mutation flags.
  Previously, iron could be misread as an opening step, while positions 1/2
  could be mistaken for locked/broken states. Every real door across all
  seven US and seven JP dungeons is now checked against its exact 4-byte
  record, starts closed, and retains material, position, ornament, opening
  direction, button, destructibility, and bashability in separate metadata
  fields. 🔒 This does not enable lock/key behavior, door damage, or sound;
  their T900, combat, and ADPCM consumers remain open.
- ✅ The same source-position field is now separate from runtime flags for
  teleporters and actuators. Positions 1/2 can no longer make a real control
  record `PICKED_UP` or `OPENED` on load. Category-3 actuators also use a
  neutral source type instead of incorrectly aliasing the fixture type
  `BUTTON`. Teleporter destination levels are decoded from the format's six
  bits; the current real US/JP corpus uses levels 0–7. 🔒 Actuator link effects
  and sound remain gated until the original consumer is verified.
- ✅ TAKE now follows the authentic ground-reference chain to the first
  unpicked carryable source record on the tile. The corpus contains 402 such
  US records and 402 JP records that are not first in a chain behind a
  control, chest, or other object; these were previously unreachable because
  the generic host path checked only the first object on the tile. Each
  dungeon now has a real-data test for picking up, inventorying, and dropping
  one such non-first record.
- ✅ Door and teleporter consumers now search a tile explicitly for their
  source type instead of letting the chain's first arbitrary object own the
  control semantics. The current US/JP corpus has zero order conflicts for
  these two types, now measured as a regression boundary. The runtime mask
  for teleporter destination levels also uses the format's full six bits.
- ✅ Native US/JP now loads authentic Track 01 CDDA from a matching,
  hash-verified full-disc archive. 🔒 Gameplay SFX remain separate and await
  the authentic ADPCM event/sample consumer.
- ✅ `USE_ITEM` is connected to the runtime's existing front-door command
  for isolated fixtures. Real Track 02 doors are now identified by their
  exact source records and remain closed; the generic host model can no
  longer invent an immediate `OPEN` transition in real game data. 🔒
  Button/actuator flow, key selection for locked doors, and other inventory
  use await the authentic T900 consumer.
- ✅ A successful source-pickup receipt now selects exactly the inventory
  slot that was filled. `DROP`/P may only return this fully verified raw
  record to the party's validated tile; missing selection, fixture ID,
  incomplete property data, or a champion change closes the path. The US/JP
  real-data test drives M12 input through pickup and drop for a non-first
  chain record in each dungeon.
- ✅ Carried-object provenance now also retains the original dungeon, level,
  and tile coordinates through floor objects, inventory, DROP, and save/load.
  This is required because the real US corpus has 204 pairs with otherwise
  identical source identities on different origin tiles. Runtime position
  and source origin can no longer be confused or swapped during a second
  pickup cycle.
- ✅ `I` now selects the next source-backed inventory slot for the active
  champion. This makes real carried items available to P/DROP even when M11's
  temporary selection is lost on resume; generic ID-only slots are skipped,
  and the DROP mutation still requires the exact Track 02 ledger record.
- ✅ The authentic combined ISO/OGG RAR package is now supported as complete
  external original media for both US and JP; region selection and rebuilt
  Track 02 data are hash-locked.
- ✅ Authentic `TQJP02.bin` is now verified through the M11 startup path
  without a CUE file or US media in the same data directory. The regression
  requires JP hash `b7afb338ad31be1025b53f9aff12d73a`, real Soul Room records,
  and `theron-runtime` with a loaded 32 × 27 map; synthetic fallback data is
  forbidden.
- ✅ Raw-sector normalization now distinguishes JP BIN from ISO. JP data is
  extracted from its own MODE1/2352 sectors and passed to the existing
  variant-specific map loader. The US-specific spawn source remains gated
  for JP.
- ✅ The Japanese spawn source's region-specific block is now authenticated
  directly from `TQJP02.bin`: the pointer table is at UD `$273818`, the five
  zone records are at `$273858..$273950`, bank word `$2780` is used, and the
  Japanese roster prefix at `$2739EF` is required. No US offsets are borrowed.
- 🔒 Japanese spawn categories, graphics, and later object routes still
  require their own executed consumer evidence. The region-specific static
  spawn code is now source-bound at JP UD `$0868D2` (269 bytes, `7dc1e453`),
  alongside US code at `$0870E5` (`eb241d19`). JP records may be stored with
  source provenance, but their runtime category remains `$FF` until the
  Japanese code path is observed in an authentic run. A static comparison on
  2026-09-24 found the same 122-instruction sequence in both hash-verified
  regions, with helper calls moved and JP accumulator fields one byte
  earlier. This does not prove a live JP caller, RNG return, or spawn record.
  See `docs/source-lock/theron-jp-us-spawn-consumer-static-comparison-2026-09-24.md`.
- ✅ The scanner now reports both verified JP and US BIN files when both are
  present in the real Theron data directory. `--theron-native us|jp` selects
  exactly the requested region's canonical BIN from that directory and
  subjects it to the existing hash gate. Real-data tests start both regions
  from the shared directory; a US-only test requires JP selection to be
  rejected without regional fallback.
- ✅ The authenticated VDC/VCE capture can now be selected directly with
  `--theron-vram-snapshot` and `--theron-vce-snapshot`. The production
  viewport then uses the existing hash- and size-verified screen-space
  consumer; no synthetic tile or palette route is enabled.
- ✅ A complete original runtime can now be started explicitly in Mednafen
  from a complete 19-track CUE. The gate requires 17 readable audio tracks,
  19 BIN files, a known JP/US Track 02 hash, and the System Card 3.0 hash. A
  lone Track 02 file therefore cannot be incorrectly marked as a complete
  external runtime.
- ✅ The three authentic 19-track archives in `.firestaff/data/theron` can
  now be used directly as `--theron-disc`. Firestaff accepts only their
  three exact archive hashes, materializes the selected edition in a private
  hash-keyed cache, and then verifies all CUE members and Track 02 again. The
  US archive has been verified both on initial extraction and on cache reuse.
  The Japanese archive has also been materialized and accepted with JP hash
  `b7afb338ad31be1025b53f9aff12d73a` and 17 authentic audio tracks.
- ✅ `--theron-original us|jp` makes the original runtime selectable without
  four manual paths. It follows `--data-dir`, `FIRESTAFF_DATA`, then
  `~/.firestaff/data`, selects only the requested region's hash-verified
  archive, and locates Mednafen and System Card 3.0 in standard locations.
  Explicit overrides pass through the same region and hash gates.
- ✅ Raw US and JP BIN startup no longer jumps directly into a level with
  fixture-initialized champions. The authentic title/level-select/Soul Room
  sequence selects the party before the forcefield handoff. Both real-data
  tests now verify a two-member party and nonzero source objects from the
  corresponding Track 02.
- ✅ The native input facade now sends the explicit `pickup` command through
  the front-cell TAKE route. It reuses the real level's strict
  object-record/property/occurrence gate; generic item IDs cannot bypass it.
  `drop` and `use` remain rejected until original inventory-slot selection
  and the T900 consumer are verified.
- ✅ Tab now switches the active champion within the selected, live Soul Room
  party. The boot receipt reports `theronActiveChampion`; authentic US and JP
  startup require slot 1 after switching. Attack input remains gated because
  source-materialized monsters still lack a verified damage consumer.
- ✅ The automatic complete original runtime has been cold-tested from
  `~/.firestaff/data` for both US and JP, not only from an explicitly
  extracted CUE. Each region materialized its 20 original files and passed
  the 19-track, 17-audio-track, Track 02, and System Card gates.
- 🔒 Native runtime square-to-tile, perspective, HUD, object, combat, AI, and
  audio consumers remain open. The Mednafen path runs the original game and
  does not prove that Firestaff's built-in consumers are complete.

> **Latest capture boundary (2026-08-14):** The authenticated external-disk
> r25 capture contains a source-LBA→RAM→`$611D` record join and 7,100
> verified `$C3A0-$C429` register-sidecar rows in the same process. This is
> still provenance/execution evidence, not a semantic level/object claim.

> **Historical capture boundary (2026-08-13):** The earlier scripted replay
> was parser-ready but init-only. The current 2026-08-14 boundary is recorded
> above and includes the source-record join plus the `$C3A0` execution window.

> **Latest runtime rejection (2026-08-14):** The r30 state replay retained
> 256 `$B0E5` address hits and 65,756 register rows, but the observed A values
> remain `$2C/$85`; it has zero valid `A=0..3` entries, zero authenticated
> CD→RAM receipts and `transition=missing`. It is negative overlay evidence,
> not a spawn witness. Mednafen also reported a missing optional
> `palettes/pce.pal`; it fell back to its built-in palette, so this warning is
> not a Track 02 graphics proof.

> **Latest scripted replay boundary (2026-08-14):** The external-disk r31
> replay used the authenticated US CUE/System Card and the same late state
> with seven scripted PCE inputs. It retained 21,786 spawn-register rows and
> 12 physical `$B0E5` overlay hits, but every retained entry again had
> `A=$2C/$85`; there were zero valid `A=0..3` entries, zero `$4644/$4667`
> samples, zero authenticated CD→RAM receipts and `transition=missing`.
> The capture is negative runtime evidence only and remains outside GitHub.

> **JP screen-space replay check (2026-08-14):** The authenticated JP
> snapshot pair (`VRAM 8ae1e419`, `VCE 4e48c361`) is already allowlisted and
> passed `theron_v1_vram_trace_real_capture` with the production viewport.
> This verifies JP BAT/tile/palette screen replay only; it does not open JP
> level/object publication or the HuC6280 consumer gate.

> **JP runtime boundary (2026-08-14):** A fresh authenticated JP ISO replay
> reached 2 raw Track 02 sector spans and 25 CD IRQ callbacks, but produced
> zero authenticated CD→RAM receipts, zero `$E009` dispatches and
> `transition=missing`. It retained 65,536 main-RAM consumer rows and 4,096
> spawn-consumer rows, with no `$B0E5` entry. This is negative JP transport
> evidence; it does not alter JP screen-space admission or open JP semantics.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-14 — r26 state replay reaches the runtime `$2600` consumer window

- ✅ A current r26 replay against the hash-verified US Track 02/System Card
  accepts the local `main-ram-consumer` sidecar in parser-only mode:
  `reads=65,536`, `$2600–$27ff` `target_reads=311`, `target_nonzero=128`,
  `target_runtime=311`, `target_c3a0=47`, `target_c3a0_nonzero=13`, and six
  distinct `$C3A0–$C429` reader PCs. This verifies runtime address/execution
  provenance, not semantic level/object publication.
- ❌ The same sidecar does not verify the source-owned code window
  `$2c54–$2c69`, and the capture's transition receipt has zero CD-origin
  receipts, zero game-owned `$E009` dispatches, and `transition=missing`.
  The state replay therefore cannot open
  `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`, JP level data, VRAM/VCE semantics,
  or HuC6280 RAM publication.
- 🔒 Capture identity: main-RAM sidecar MD5
  `021efea135de2ac0b8ae241ffd63eaf6`, instrumented r26 binary MD5
  `ab6dbf674c68ee4891a185b83cff3149`, state MD5
  `82e151fa51aa3e7d578d0dfdb09eb55b`. Sidecars and the binary remain local
  on the external drive and must not be committed.

## 2026-08-14 — r26 cold start proves same-session CD→RAM transport

- ✅ A new authentic cold-start run with the same US Track 02/System Card
  produced `raw_sector_spans=161`, `cd_irq_callbacks=25`, two byte-exact,
  authenticated CD→RAM receipts, and 32 game-owned `$E009` dispatches.
  `transition=observed` and `main_ram_consumer_reads=65536` are verified.
- 🔒 The run still does not reach the dynamic `$2600` consumer:
  `target_reads=512`, all values are zero, and the reads are initialization
  from `$CB22`; `$C3A0` reads are absent. This therefore does not open
  `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`, JP level data, VRAM/VCE semantics,
  or HuC6280 RAM publication.
- 🔒 Capture identity: main-RAM sidecar MD5
  `21f771f92a35704cf0ea8be3a2adf199`, transition-sidecar MD5
  `c92f8d31269cdd1771464937f32d69bf`. Sidecars and the binary remain local
  on the external drive and must not be committed.

## 2026-08-14 — synthetic cross-route regression follows explicit door use

- ✅ The cross-route probe now uses the verified mechanics contract:
  `theron_v1_door_open()` is called explicitly before moving through a
  closed, unlocked fixture door. Movement does not implicitly consume a key
  or change door state.
- ✅ The full Theron regression suite is green after the correction: all 45
  Theron-labeled tests pass; six local capture tests correctly skip when raw
  external sidecars are absent.
- 🔒 This does not change source-level gates or enable any unidentified
  T500/T600/T700/T900 consumer.

## 2026-08-14 — state replay rejects the `$B0E5` overlay

- ✅ An r26-autoloaded Mednafen state produced 65,756 register samples and
  256 logical `$B0E5` hits, but all contained overlay values `A=$2C/$85` at
  physical PC `$000E10E5`. This is not the source-locked regular-spawn entry
  and yields zero valid categories, zero CD-origin records, and no target
  join.
- 🔒 The next positive witness must still bind the source bank's real `$B0E5`
  with category `0..3`, the preceding `$4644/$4667` chain, return ownership,
  and the same-session live creature record. The overlay hit must not enable
  RNG, spawning, AI, combat, loot, generators, T700, or T900.

## 2026-08-14 — Broader provenance capture: record-table join verified

- ✅ Corrected Mednafen instrumentation now separates the bounded receipt
  counter from provenance seeding and watches `$6000`, `$611D–$6126`, and
  `$2935–$293E` in the same authentic run.
- ✅ The verified 120-second r25 capture produced 238 authenticated CD→RAM
  receipts, four complete ten-byte records bound to source LBAs, and 7,100
  `$C3A0–$C429` register rows in the same process.
- 🔒 The provenance/execution chain is now closed, but the record's
  level/object/gameplay role remains unidentified. Capture sidecars and the
  instrumented binary remain local on the external drive.
- ✅ With authentic US CUE/BIN, converted US ISO, JP CUE, the r25 VDC/VCE
  snapshot, and CD sidecar, the Theron suite is green at 45/45. Without these
  local external inputs, five of the same tests intentionally `SKIP` rather
  than fail.

## 2026-08-13 — scripted replay reaches authenticated transport, not gameplay

- ✅ A run from the external drive with hash-verified US Track 02 and System
  Card accepts the scripted PCE sequence `run@1:1,run@480:30,i@900:30` and
  produces a parser-accepted transition receipt: 240 raw-sector spans, 25 CD
  IRQ callbacks, 256 authenticated CD-to-RAM receipts, and 32 game-owned
  `$E009` dispatches.
- ✅ The main-RAM sidecar is verified with `reads=65536`, `target_reads=512`,
  `target_nonzero=0`, `target_init=512`, and `target_runtime=0`. The separate
  transition test also passes.
- 🔒 The replay still shows no dynamic level/object consumer: there is no
  `$C3A0` reader or source-owned publication, and no square/tile/HUD/T700/T900
  semantics may be enabled. Sidecars and the instrumented binary remain
  local on the external drive and must not be committed.
- ✅ Reproducibility is documented in
  `docs/source-lock/theron-disassembly/theron-scripted-replay-transport-boundary-20260813.md`.

## 2026-08-13 — Door movement remains fail-closed

- ✅ Movement onto a closed or locked door blocks; the explicit door-use route
  owns opening and key validation, so a key is never consumed implicitly by a
  movement step.
- ✅ The first-room synthetic fixture marks the destination level resident
  before expecting a stair transition.
- ✅ The full Theron suite is 45/45 green when the authenticated external
  media and captures are supplied; five media-dependent tests are expected
  `SKIP` only when those local inputs are absent.

## 2026-08-13 — Track 02 map-directory envelope is fail-closed

- ✅ `theron_v1_world_load_track02_dungeon()` validates the complete map
  directory before clearing or replacing a dungeon bank. Zero/oversized map
  counts and `x_dim/y_dim + 1` values beyond the fixed world grid are rejected
  without changing the previously loaded bank.
- ✅ A regression covers both invalid dimensions and an oversized directory;
  the real US/JP Track 02 loader suite remains green.
- 🔒 This is an intake/state-safety invariant only. It does not promote the
  missing post-CD level/object consumer or any square/tile/HUD/T700/T900
  semantics.

## 2026-08-13 — Split-CUE normalization strengthens transport evidence

- ✅ Private split-ISO normalization (`TQUS19.iso + TQUS02End.iso`) produced
  a parser-accepted same-session capture with 161 raw sectors, 32 game-owned
  `$E009` dispatches, and 65,536 main-RAM reads.
- 🔒 `$2600–$27FF` still contains only 512 zero-valued reads from the
  initialization reader `$CB22`; runtime reads in `$C3A0–$C429` are absent.
  The spawn sidecar has no valid `$B0E5`, source-owned target publication,
  or live creature record. No level/object/square/HUD/T700/T900 semantics
  are enabled.
- ✅ Provenance and sidecar hashes are documented in
  `docs/source-lock/theron-disassembly/theron-split-cue-consumer-capture-20260813.md`.

## 2026-08-13 — Source-bound pit blocks without fixture damage

- ✅ Query, movement, and the public pit handler now recognize that the T700
  consumer is absent for source levels. A pit tile is reported as blocked
  instead of applying host HP/stamina damage or falling through as floor.
- ✅ The hardening probe verifies that position, HP, and stamina are unchanged.
- 🔒 This does not enable the original pit/fall/levitation consumer; fixture
  levels retain the previous ReDMCSB-based probe semantics.

## 2026-08-13 — Incomplete dungeon exits fail closed

- ✅ Exit queries and movement now use the same `dungeon_complete` gate and
  require successful transition execution. An incomplete or unresolvable
  exit is reported as `THERON_MOVE_BLOCKED` with no movement effects.
- ✅ The hardening probe verifies that party position, transition state, and
  stamina remain unchanged at an incomplete exit.
- 🔒 This does not enable quest completion or the next dungeon; the
  source-owned completion consumer remains separate from the movement
  invariant.

## 2026-08-13 — Unloaded stairs fail closed

- ✅ Movement query and mutation now require the stair destination level to
  be loaded. A failed `transition_execute()` is no longer reported as a
  successful `THERON_MOVE_STAIRS`, and no movement effects run.
- ✅ The hardening probe verifies that moving down unloaded stairs leaves
  party position, level, transition state, and stamina unchanged.
- 🔒 This is a state invariant; dynamic source-owned level loading and the
  original stairs consumer remain separate capture-gated questions.

## 2026-08-13 — Unresolvable teleporters fail closed

- ✅ The movement path now respects the return value of
  `theron_v1_teleporter_resolve()`: a missing endpoint, cycle, or other
  unresolvable source data is reported as `THERON_MOVE_BLOCKED`, without
  advancing the party, transition, or movement effects.
- ✅ The hardening probe covers a teleporter without an endpoint and verifies
  that position, transition state, and stamina remain unchanged.
- 🔒 This does not enable new teleporter semantics. Only already verified
  object-ID and Track 02 coordinate links may pass the resolver gate.

## 2026-08-13 — Locked door restored as a blocking sentinel

- ✅ The door state machine now distinguishes `LOCKED=6` from the opening
  frames `QUARTER_OPEN..DESTROYED`. Query, movement, and `door_open()` can no
  longer accidentally treat the numerically higher locked sentinel state as
  passable.
- ✅ The mechanics hardening probe covers the locked sentinel in both query
  and mutating open paths. This is a local state-machine correction; it does
  not enable the still capture-gated T900 key/object consumer for source
  levels.

## 2026-08-13 — New instrumented cold start reaches CD transport only

- ✅ A fresh run from the external drive against hash-verified US Track
  02/System Card reads 256 raw 2,352-byte sectors from LBA 3234. It also
  shows 3,584 target writes and 4,096 spawn-consumer reads as raw provenance.
- 🔒 The run loops in the BIOS/CD reader: it produces zero game-owned `$E009`
  dispatches, zero CD/FIFO-to-RAM origin receipts, zero RNG windows, and zero
  authenticated level/object consumers. All 512 reads in `$2600–$27FF` are
  zero-valued reads from `$CB22`; they must not enable level, object, square,
  HUD, creature, combat, T700, or T900 behavior.
- 🔒 The spawn sidecar reads only initialization range `$20EC–$20EE` and has
  no `$B0E5` category, return ownership, or source-owned target publication.
  The capture is therefore reproducible negative evidence, not new gameplay
  semantics. Raw sidecars and the instrumented build remain on the external
  drive.

## 2026-08-13 — source-gated object handoff is transactional

- ✅ The existing source-gated object/gameplay handoff now validates the
  entire selected level before mutation and restores the object pool, current
  level, `thing_count`, and runtime media if a later placement step fails.
  The regression forces an `INT_MAX` ID on a remaining object and verifies
  that the world hash and object pool are unchanged after rejection.
- 🔒 This does not change the semantic gate: a same-session authenticated
  object consumer is still required before the handoff can enable dungeon
  rendering, square-to-tile mapping, or gameplay without fallbacks.

## 2026-08-13 — source-runtime state invariants are no longer no-op

- ✅ The production adapter now clamps champion HP, stamina, and mana to
  their respective maxima and zero lower bounds. Champion death also clears
  health and `alive` through the same clean state lifecycle as the already
  source-bound creature-retire routine.
- 🔒 This does not enable attack, spells, AI, RNG, loot, sound, or T700/T900;
  their original consumers remain fail-closed.

## 2026-08-13 — Longer replay reaches source-owned spawn pre-consumer

- ✅ The authenticated replay now parses as a positive execution window:
  the `$CC4C` consumer, 48 `$4644` pre-consumer samples, and 160 `$4667`
  helper samples are present in the same sidecar. The test requires these
  edges while also ensuring that no valid `$B0E5` category or RAM-loaded
  helper branch was observed.
- 🔒 `$B3=$FF` in every `$4667` sample means the special `$B3 & 7 == 4`
  branch is not reached. Without `$B0E5` with A=`0..3`, return ownership,
  source-owned target write, and a live creature record, spawning, RNG, AI,
  combat, loot, generators, T700, and T900 remain disabled.

## 2026-08-13 — Authenticated CD→RAM transport from replay verified

- ✅ The replay transition receipt (`theron-capture-20260813/replay`) now
  parses as `observed`: 161 raw sectors, 47 byte-exact CD→RAM origin
  receipts, and 32 game-owned `$E009` dispatches, with hash-verified US
  Track 02 and System Card.
- ✅ The regression now requires verified minimums instead of the previously
  incorrect exact campaign length (`2` CD receipts/`3584` RNG samples), so
  new authenticated replay lengths are not rejected arbitrarily.
- 🔒 The same replay still has 512 `$2600` reads, all from `$CB22`, with no
  nonzero values. Transport therefore does not enable level/object,
  square-to-tile, HUD, creature, combat, T700, or T900 semantics.

## 2026-08-13 — Consumer receipt separates initialization from source caller

- ✅ The receipt now separately counts `$CB22` initialization reads, other
  runtime reads, and reads from the byte-locked `$C3A0–$C429` window. For the
  C3A0 window it also retains nonzero counts and distinct reader PCs.
- ✅ The external VDC replay's main-RAM sidecar (MD5
  `c6f8f3bc32ce4b29ac32b376096756d1`) passes parser-only with 311
  target reads, 128 nonzero values, and `semantic_publication=blocked`.
  These fields preserve caller provenance but do not classify level, square,
  object, HUD, creature, T700, or T900 semantics.
- 🔒 The replay still lacks authenticated CD/FIFO-to-RAM origin in the same
  session. The new shape receipt enables no gameplay semantics.

## 2026-08-13 — Authenticated VDC/VCE pair from RAM replay admitted for screen space

- ✅ `theron_v1_vram_trace_load_known_capture_files()` now accepts the
  external, hash-verified pair `theron-vdc-ram.exXuQu`:
  VRAM FNV-1a `087da136`, VCE FNV-1a `5376a91b`.
- ✅ This pair can be used by the production viewport's authenticated
  screen-space renderer; the raw files remain local on the external drive
  and are not copied to GitHub.
- 🔒 The capture's `$2600–$27FF` reads are preceded by the same `$CB22`
  routine that writes zeros to the RAM window. This is therefore not evidence
  for a level, object, square, HUD, T700, or T900 consumer. The semantic gates
  remain closed.

## 2026-08-13 — Consumer receipt distinguishes initialization from runtime reads

- ✅ The receipt now retains the number of `$2600–$27FF` reads, the number of
  nonzero values, and the number of distinct reader PCs.
- ✅ The external combat replay (`live.trace.main-ram-consumer`, MD5
  `4d9da34dd8a0042dc302449af78c54cc`) shows 19 target reads, 3 nonzero
  values, and 19 reader PCs. This is stronger runtime provenance than
  `$CB22` initialization, but the replay lacks a CD/FIFO join and must not
  enable level/object, creature, combat, T700, or T900 semantics.

## 2026-08-13 — Game-owned `$2600` window retained as provenance

- ✅ `theron_v1_mednafen_main_ram_consumer_trace_parse_file()` now preserves
  `target_2600_bytes_present` when a verified `main_ram_consumer_read` is
  actually within `$2600–$27FF`. Previously, final initialization cleared
  the flag and discarded the observation.
- ✅ A new parser test covers a read across the window boundary. The local
  MPR capture from the external drive (`mpr.trace.main-ram-consumer`, MD5
  `12f470ef2c38febd9b2c9699dad3b4cb`) passes parser-only and reports
  `target_2600=present`.
- 🔒 This classifies address provenance only. It does not identify the bytes
  as level, object, T700, or T900 data and enables no gameplay semantics.

## 2026-08-13 — Text codon positional provenance is now preserved

- ✅ The Track 02 text decoder now retains each packed 5-bit value together
  with its source word and slot (`word_index`/`packed_slot`) in a token view.
- ✅ The codec layer now distinguishes raw characters, known codec markers,
  and the terminator without claiming to know the original HuC6280 control
  codes' meanings.
- ✅ Live world state now retains both raw text words and their
  position-bound token view through the dungeon loader; a later consumer
  binding does not need to reconstruct token positions from media.
- 🔒 This is lossless positional provenance, not enablement of text, menu, or
  HUD semantics. The game-owned text consumer and its VDC target must still
  be bound in the same run before world/UI publication is allowed.

## 2026-08-13 — Save-state replay remains negative for game-owned CD

- ✅ A new local replay from the authenticated dungeon save state produced
  65,756 register samples, 256 `$B0E5` address overlays, 4,096
  `spawn_consumer_read` rows, and 2,213 RNG samples. US Track 02 and the
  System Card were hash-verified.
- 🔒 The replay produced only one CD IRQ after autoload: zero raw sectors,
  zero source-backed CD→RAM receipts, zero valid `$B0E5` categories, and zero
  `$4644/$4667` samples. It therefore must not enable spawning, RNG, AI,
  combat, loot, T700, or T900.
- ✅ The same external US/JP mechanics-playability probe passes 79/79 and
  continues to cover the source-bound grid/loader while dynamic original
  consumers remain fail-closed.

## 2026-08-13 — Capture-based Theron regression verified

- ✅ The full Theron regression suite passes with an external `TMPDIR`: 253
  selected tests, 247 executed, and 6 expected capture skips without local
  fixtures.
- ✅ With authenticated local fixtures, VRAM/VCE readiness, the main-RAM
  consumer, and the CD-state sidecar also pass. The fresh replay produces
  161 raw sectors, 51 SCSI reads, 25 CD IRQs, 47 FIFO→RAM receipts, and
  65,536 VDC writes.
- 🔒 This resolves the test-environment and transport blockers. Gameplay
  semantics are still not enabled: the session lacks a game-owned FIFO→RAM
  receipt, spawn consumer, and RNG window.

## 2026-08-13 — VDC I/O provenance now included in transition admission

- ✅ The VDC I/O parser now accepts the real 65,536-entry file. Mednafen's
  raw bus address may use bit 31 only as a marker and is normalized
  separately (`$801FE000` → `$001FE000`); other high bits are rejected. Its
  `HuCPU.Timestamp()` is verified as 24 monotonic epochs with exactly 23
  observed counter resets, rather than being incorrectly required to be
  globally monotonic.
- ✅ The same parser can now retain all verified write entries in an
  explicitly freeable replay structure capped at 65,536 entries. The entire
  file must pass before the array is published; every entry preserves
  sequence, timestamp, logical address, raw and normalized physical address,
  value, writer PC, and A/X/Y.
- ✅ The capture script counts authentic `vdc_io_write` rows and writes
  `vdc_io_writes` to the transition receipt. The receipt parser requires a
  positive count together with 64 KiB of VDC VRAM and 1 KiB of VCE data.
- 🔒 This binds transport provenance, not text, BAT, square, HUD, or gameplay
  semantics. A preliminary exact HuC6270 replay produces 26,048 VWR commits
  and matches 6,898 of 7,328 written snapshot addresses; 430 mismatches show
  that write and snapshot boundaries must be correlated in time before
  viewport VRAM can be mutated.
- ✅ The instrumentation patch now captures the VRAM/VCE/VDC/SAT snapshot
  immediately after write 65,536 passes through the real VDC, writes
  `vdc_snapshot_boundary sequence=65536`, and prevents CloseGame from later
  overwriting the bundle. The capture script rejects new runs without this
  footer. 🔒 The existing real file lacks the footer and may therefore only
  be used as older transport evidence until a new authentic run is made.

## 2026-08-13 — TQTR verification can run on an external temporary drive

- ✅ `test_theron_v1_vram_trace_loader` now uses `TMPDIR` for its expanded
  TQTR fixture. It can therefore run when the macOS system volume's `/tmp` is
  full without writing to or requiring space there.
- ✅ The authenticated US screen-space capture passes separately with
  `vram_nonzero=24336`, `bat_tiles=1057`, and `presented_nonzero=44947`.
- 🔒 This still does not enable square-to-tile, text, HUD, or gameplay
  semantics.

## 2026-08-11 — RNG edge capture is still not a spawn handoff

- 🔒 An external authenticated US save replay observes `$4644`/`$4667` and
  RNG windows, but no valid `$B0E5` category or target publication. It must
  not enable RNG return, monster stats, AI, combat, loot, generators, T700,
  or T900.
- ✅ The older 18-field sidecar's 192-step format can now be read without
  inventing modern return-boundary fields or semantics.

## 2026-08-12 — Raw A value at RNG return boundary retained; semantics remain closed

- ✅ The RNG parser now retains the A register and observation count at the
  instrumented stack-based return boundary.
- 🔒 This field is provenance only. It does not enable RNG return, spawn
  stats, or AI without a source-bound caller and same-session target consumer.

## 2026-08-12 — C96B-only combat capture registered as a negative test case

- ✅ The authenticated external combat capture can now run as an explicit
  negative parser case: `$C96B` reads and `$B0E5` address overlays are
  preserved, while the absence of `$CC4C` and a valid category continues to
  reject runtime semantic publication.
- 🔒 This is capture classification, not recovered RNG, AI, combat, T700,
  generator, or T900 semantics.

## 2026-08-12 — New Stage-2 session still lacks a gameplay consumer

- ✅ A new isolated session with a verified direct-SDL2 binary reached real
  Stage-2/System Card code and produced 2,048 register samples.
- 🔒 The session lacked `$CC4C`, `$B0E5`, and subsequent dungeon/object
  targets; it therefore must not enable creature, RNG, T700, or T900 semantics.

## 2026-08-12 — JP portraits and original mechanics remain open

- 🔒 JP Track 02 roster records are authenticated, but no source-bound
  portrait-pixel consumer or portrait-ID binding has been captured.
  `portrait_index` must therefore remain `THERON_PORTRAIT_UNAVAILABLE`.
- 🔒 The parity matrix now counts fixture/numeric-record evidence as `PARTIAL`
  for combat and the champion system; T500/T600/T900 consumers must still be
  bound to same-session runtime data before production is enabled.
- 🔒 The new external combat capture is verified as autoload/C96B-only: no
  `$CC4C`, valid `$B0E5` category, or CD→RAM loader transition. It must not be
  used to invent synthetic AI, RNG, T700, or T900 semantics.
- 🔒 The same capture has a new VDC/VCE pair that can now be replayed in
  screen space; square-to-tile, HUD, and gameplay ownership remain separate
  gates.

## 2026-08-11 — Audio consumer remains capture-gated

- 🔒 The static System Card catalog classifies real CD/ADPCM vector calls,
  and the authenticated capture path binds CD/FIFO→ADPCM RAM.
- 🔒 No same-session CPU read, sample start, or game-event ownership has yet
  been verified. `theron_v1_play_sound()` must therefore remain fail-closed;
  creature, actuator, and menu events must not trigger synthetic sound. The
  parity matrix's previous `PROVEN` row has been corrected to
  `PARTIAL`.

## 2026-08-11 — Register trace can now bind the `$C3A0` caller window

- ✅ Mednafen instrumentation now writes optional `record_c3a0_window=1` in
  the same register trace as `$C96B/$CC4C`; the parser counts the window
  without breaking older v3 traces.
- 🔒 The flag is capture provenance, not semantics. `$C3A0` must still be
  captured in the same run as its `$C96B/$CC4C` calls and target writes before
  creature, object, generator, T700, or T900 rules can be enabled.

## 2026-08-11 — New authenticated `$C3A0` caller is source-locked

- ✅ A new 150-byte US Track 02 fragment from raw offset `$9C450` / HuC6280
  `$C3A0` matches `TQUS02.bin` byte for byte and has FNV-1a `$666DED61`.
- ✅ Disassembly admission now verifies the fragment together with the
  existing `$4667`, `$C96B`, and `$CC4C` windows.
- 🔒 The fragment shows the source code's caller/table flow but does not
  identify
  whether `$2998/$299C` are creature, generator, T700, or T900 records. No
  game semantics may be enabled without same-session runtime evidence.

## 2026-08-11 — live source creatures no longer receive synthetic PASSIVE AI

- ✅ Category-4 creatures admitted from authentic US/JP Track 02 records now
  carry `THERON_AI_UNAVAILABLE` until the original T500/T600 AI consumer is
  authenticated. The AI tick ignores that explicit unavailable state.
- 🔒 This is a correctness boundary, not recovered AI: RNG-spawn, creature
  AI, attacks, damage, loot, generator timing, T700 and T900 remain closed
  until the disassembly consumer and a same-session runtime capture agree.

## 2026-08-11 — Multi-window RNG captures validate correctly

- ✅ The RNG consumer parser now counts complete 512-step windows in a longer
  same-session trace; a valid trace is no longer rejected merely because it
  contains multiple windows.
- ✅ An external US Track 02 capture has 22 complete `$5D64` windows and a
  source-byte-matched `$5D64` code window.
- 🔒 This proves source-consumer execution and code provenance, but not yet
  which return value belongs to spawn stats or later creature semantics.

## 2026-08-11 — inventory transitions now require the authenticated property table

- ✅ The loaded level now retains whether the complete source-owned 66-row
  Track 02 item-property table matched the selected US/JP bank.
- ✅ Source inventory swap/drop transitions require both the object-record
  header and that table-authentication bit; a map header alone is no longer
  sufficient.
- 🔒 This remains provenance validation. T900 equip/use/stack semantics are
  still not implemented without the original consumer capture.

## 2026-08-11 — authenticated BAT preview now decodes real PCE tiles

- ✅ The source-bound VRAM/VCE presentation route now runs every admitted BAT
  tile through the real PCE planar 2/4bpp decoder before applying its BAT/VCE
  palette group. It no longer treats raw 32-byte 4bpp planes as indexed
  pixels.
- ✅ The real external US dungeon pair (`VRAM=5d20ebc7`, `VCE=ea83f117`)
  passes the production capture test with 1,057 atlas tiles, 896 screen cells
  and a non-empty authenticated frame.
- 🔒 This fixes bitmap decoding only. Square-to-tile, depth/perspective and
  creature/object atlas ownership remain separate source-consumer gates.

## 2026-08-11 — disassembly-visible spawn arithmetic is receipt-only

- ✅ `theron_v1_track02_apply_spawn_consumer_witness()` now reproduces the
  instruction-visible arithmetic in `$B0E5-$B1EB` from a same-session witness:
  category branches, `$B8` scaling, `$B4/$B5` divide, bounded `$4667` values,
  HP cap `#$0384` and the `$2980/$2990` caps.
- 🔒 This API does not generate RNG values, does not publish `Theron_SpawnStats`
  and is not wired into creatures. `$5A76`, `$5B8F`, `$D23A`, `$4667`, the
  `$2A10/$D0FE` writes and the later stat/AI/combat owners still need one
  authenticated runtime execution window before gameplay semantics can open.

## 2026-08-11 — M11 handoff regression test is headless-safe

- ✅ The boundary test uses SDL dummy audio by default, preventing a local
  CoreAudio wait from being mistaken for a Theron runtime hang.
- 🔒 This does not alter production audio-device selection.

## 2026-08-11 — JP roster text now copies verified raw bytes

- ✅ JP startup names and titles are emitted from the authenticated raw
  offsets after matching, rather than from the expected search literals.
- 🔒 This proves payload provenance only; the original JP portrait/font/VDC
  consumer remains unresolved.

## 2026-08-11 — authenticated manual VRAM/VCE capture is admitted

- ✅ The production viewport now accepts the externally captured US Track 02
  screen pair `VRAM=5d20ebc7`, `VCE=ea83f117` after exact-size/hash checks.
- 🔒 This is screen-space bitmap/palette ownership only; square-to-tile,
  perspective, HUD and gameplay consumers remain separately gated.

## 2026-08-11 — inventory property category is source-checked

- ✅ Pickup, source-slot movement and drop now reject a carried record when
  its property-category byte no longer agrees with the source object class.
- 🔒 This hardens provenance only; property-byte meaning and T900 equip/use/
  stack rules remain unpromoted.

## 2026-08-11 — unbound spawn categories are now fail-closed

- ✅ Direct level loads no longer copy a reconstructed static spawn-zone
  category into live creature provenance.  The field is published only after
  an authenticated US Track 02 spawn source is bound.
- 🔒 This does not enable random spawning, AI, combat, generators, T700 or
  T900 semantics; those still require their original runtime consumers.

## 2026-08-11 — source creature IDs now survive pool rebuilds

- ✅ Both authenticated category-4 level materialization and explicit source
  admission derive IDs from `source_ref` plus member slot. Removing or
  reloading a pool no longer renames a source creature by its array position.
- 🔒 This is provenance-only; no unproven RNG, AI, combat, generator, T700,
  T900, loot, presentation or event-audio semantics were enabled.

## 2026-08-11 — unbound source creatures cannot enter fixture combat

- ✅ Source-backed members with authentic HP remain visible/collidable, but
  champion damage, creature attacks and spell damage now reject them while
  their original attack consumer is unknown.
- 🔒 This is a safety boundary, not completed combat parity; the real attack,
  damage, AI and event-sound owners still require the authenticated runtime
  capture described below.

## 2026-08-11 — category-4 members now materialize from real HP records

- ✅ Live static creatures are now admitted one-for-one from authenticated
  Track 02 category-4 group members. Each member copies its real HP word,
  packed cell ordinal, group count and source identity into the runtime pool;
  the previous fixture-stat path is no longer used for this source route.
- 🔒 Attack, defense, speed, AI, loot and generator behavior remain explicitly
  unpopulated until their original consumers are bound by the HuC6280
  disassembly and a same-session authenticated runtime capture.

## 2026-08-11 — production replay now uses the authenticated native screen consumer

- ✅ When a hash-verified VRAM/VCE pair is explicitly mounted, Theron's
  production viewport now uses the explicit 256×224 native screen consumer.
  The focused real-capture test compares the production framebuffer byte for
  byte with the authenticated screen route.
- 🔒 This is still screen-space BAT/tile/VCE binding. No cell is assigned to
  square-to-tile, perspective, HUD, object, or creature semantics without the
  corresponding original consumer.

## 2026-08-11 — save-state `$B0E5` hits are not the regular-spawn caller

- ✅ The rebuilt external Mednafen capture against the authentic US CUE now
  also logged the HuC6280 stack return word at every `$B0E5` hit. The
  authenticated Track 02 hash remains `f23601102138f87c33025877767ebf76`,
  and the capture produced 50 `$B0E5` hits.
- 🔒 All 50 hits had A=`$2C` or A=`$85`, not disassembly's spawn categories
  0–3, and no hit was followed by `$4667`, `$5D64`, or `$5D6A`. The stack
  words were also `return_pc=$0002`/`$3F3F`, which is not a verified game-code
  caller. This is therefore a rejected overlay/state witness, not RNG or
  spawn evidence. RNG, AI, generators, T700, T900, loot, and combat must not
  be enabled from this session.
- 🔧 The next capture must reach an actual dungeon tick or object action and
  show a valid caller, category argument, RNG return, and the consumer's
  target write in the same authenticated session.

## 2026-08-11 — `$B07D` caller window is source-locked

- ✅ The static US disassembly now has a separate, hash-verified caller
  window for `$B07D-$B1EB`. It shows four `$4644` calls before `$B0E5` and
  which register/RAM fields are passed into the dispatch.
- 🔒 The window does not yet prove that `$2980/$2990/$29A0` or `$2A20/$2A28`
  are creature stats. The next positive capture must bind the same caller,
  valid category 0–3, RNG return, and subsequent writes to a real Track 02
  record before any gameplay semantics are enabled.
- ✅ The register sidecar can now mark the static caller window as
  `caller_b07d_window=1`; older v3 sidecars continue to be read as provenance
  without the new flag.

## 2026-08-10 — README capture is reference-only

- 🔒 The published screenshot documents the original US presentation only.
  A Firestaff-native capture with authenticated rendering/gameplay parity is
  still required before claiming Theron is complete.

## 2026-08-10 — BAT→VCE relation is bound; world mapping remains open

- ✅ The authenticated VRAM/VCE loader now verifies BAT palette-group bits
  against the exact VCE snapshot and exposes the relation receipt.
- 🔒 The same evidence still does not identify which decoded screen-space BAT
  cells belong to a dungeon square, depth/perspective slot, HUD element,
  object, or creature. Those consumers remain source-capture gated.

## 2026-08-10 — File-select replay still lacks regular-spawn handoff

- ✅ The complete `Run → Button I → movement` replay against authentic US
  Track 02 produced 28 authenticated CD→RAM origin receipts and 32 `$E009`
  dispatches.
- 🔒 The same session produced zero `$B0E5` hits, RNG returns, spawn-consumer
  reads, or target writes. The next capture must reach a verified dungeon
  tick before RNG/AI/generator/T700/T900 or loot can be implemented.

## 2026-08-10 — save-state replay reaches only a rejected `$B0E5` overlay

- ✅ An authentic Mednafen save state was run against the complete raw
  MODE1/2352 US CUE on the external drive. The capture verified Track 02 hash
  `f23601102138f87c33025877767ebf76` and observed 30 `$B0E5` hits.
- 🔒 Every hit had A=`$2C` or A=`$85`, not disassembly's valid regular-spawn
  categories 0–3. The parser therefore correctly rejects these hits as
  same-address overlays; no RNG return, spawn record, AI, loot, T700, or T900
  semantics are enabled. The earlier 2,048-byte CUE run was also rejected
  because it lacked authenticated CD→RAM origin.

## 2026-08-10 — complete US CUE capture remains transport-only

- ✅ Complete `TQUS.cue` with 19 tracks was run from the external drive.
  Track 02 was reconstructed from the archive's authentic
  `TQUS19.iso + TQUS02End.iso`.
- 🔒 The session produced 159 raw sectors, 88 spawn-register samples, 17
  `$4644` and 64 `$4667` samples, but zero valid `$B0E5` hits, RNG windows,
  spawn-consumer reads, or target writes. Semantic consumers remain closed.

## 2026-08-10 — all decoded Track 02 occurrences retained; consumers gated

- ✅ The real US campaign now retains all 2,266 authentic ground-reference
  occurrences in the world source ledger, including control records and
  carried objects. This is lossless provenance from Track 02, not synthetic
  data.
- 🔒 Original RNG, spawn timing, creature AI, attack/damage/loot, T700/T900,
  item semantics, and source-bound presentation/audio remain gated until
  their real consumers are bound by disassembly and a same-run capture.

## 2026-08-10 — Input fix complete; semantic gates remain

- ✅ Held WASD/arrow-key input is now connected to Theron's own tick cadence.
  The ordinary mouse moves the pointer freely without object jumping; Button
  I/II and touch are unchanged.
- 🔒 This does not change the separate gate on original RNG, creature AI,
  T700/T900, object records, or source-bound audio/presentation.

## 2026-08-10 — remaining creature semantics are source-capture gated

- ✅ Removed the unauthenticated DMWeb/DM1 creature-generator fallback; real
  Track 02 category-4 records are the only source for live creature creation.
- 🔒 Do not add replacement tables. The next implementation witness must bind
  the original RNG return, generator reactivation/timing, AI/attack/damage/loot
  consumers and T700/T900 state writes in one authenticated runtime.

## 2026-08-10 — cold-start transport witness is still semantically negative

- ✅ An external cold start against US Track 02 verified 159 raw sectors, 32
  `$E009` dispatches, two CD→RAM origin receipts, 17 `$4644` and 64 `$4667`
  observations, and VDC/VCE snapshots in the same authenticated session.
- 🔒 The same run produced zero `$B0E5` hits, special branches, RNG windows,
  or target writes. Do not implement RNG, spawning, AI, combat, loot,
  generators, T700, or T900 from this; the next witness must capture an
  actual dungeon/spawn or object consumer.
- ✅ The 2026-08-20 follow-up byte-exactly bound the completed `$2800` block
  from the first game-owned `$3840` call to US Track 02 record `$4E0` at the
  next `$3840` dispatch. The transport and destination chain are therefore
  proven.
- 🔒 The payload's internal grammar and the subsequent VDC/dungeon consumer
  remain unbound. Do not use the sector binding to enable RNG, spawning, AI,
  combat, loot, T700/T900, or native dungeon rendering.

## 2026-08-10 — VDC/VCE screen-space capture admission

- ✅ Production intake now has a closed allowlist of five verified complete
  VRAM/VCE hash pairs. US dungeon, US interactive, JP startup, and US cold
  start pass the authentic BAT/tile/palette binding and M11 presentation
  from the external drive.
- 🔒 This remains a screen receipt. A screen-space snapshot does not enable
  square-to-tile, perspective, HUD/object consumer, monster, RNG, T700, or
  T900 behavior.

## 2026-08-09 — Current cold capture has transport evidence only

- ✅ The authentic US run reaches `transition=observed` and produces four
  byte-identical source-backed CD→RAM receipts that can now be verified in
  both receipt formats.
- 🔒 The same run has no `pce_cd_fifo_origin_main_ram_consumer` rows and no
  RNG return/spawn entry. Original creature, T700, T900, item, graphics, and
  audio semantics therefore still must not be implemented from this
  transport-only evidence.

## 2026-08-09 — Next capture requires an active dungeon

- 🔧 The capture script's macOS input grab is now retry-safe and waits for
  both a Quartz receipt and Mednafen-reported `InputGrab=1` before sending
  the sequence.
- 🔒 The next authenticated run must use the verified startup sequence to
  Akutuba and then reach an active dungeon; the previous bounded run stopped
  before the game-owned CD→RAM consumer. RNG, spawning, AI, T700, T900, and
  presentation remain gated until one run binds those consumers.

## 2026-08-09 — Summary-only original-consumer admission closed

- ✅ Runtime admission now requires raw, exactly joined
  `pce_cd_fifo_origin_main_ram_receipt`/`...consumer` rows in the same
  capture for palette, non-startup, and object-table offsets. A summary
  receipt without these rows no longer enables original-consumer semantics.
- 🔒 The authentic external US session remains correctly blocked: it has two
  CD→RAM origin receipts but no game-owned FIFO consumer. The next step is a
  new authenticated session that actually produces these rows; RNG,
  spawning, AI, T700, T900, rendering, and save remain closed until then.

## 2026-08-09 — Combined cold start still has no spawn return

- 🔒 A new bounded cold start on authentic US Track 02 produced 256 verified
  CD→RAM origin receipts, 26 `$E009`, 33 `$4644`, and 96 `$4667` events in
  the same session, but zero `$B0E5` events, RNG samples, or `.rng-code`
  windows. These pre-consumer events therefore do not constitute an RNG
  return or spawn event.
- 🔧 The capture script's `pce_fast` gate now rejects builds that contain
  `pce_fast` strings but do not advertise the module in Mednafen's own module
  list.

## 2026-08-09 — Raw-code source-byte join verified

- ✅ The `.rng-code` parser now requires a sidecar header, correct
  `$5D64/$5D6A`, 256 bytes of hex code, a valid HuC6280 address range, and
  the authentic 8,104,992-byte US Track 02 file. It compares the complete
  window against the seven observed offsets `0x975c4 + n*0x49800` and runs
  against the authentic external capture receipt.
- 🔒 This proves byte provenance, but not the mapped bank, RNG return value,
  caller, spawn category, or gameplay semantics. The next witness must still
  bind the same run to the real return and spawn consumers.

## 2026-08-09 — Raw RNG code captured; semantics remain gated

- 🔧 The capture script and reproducible Mednafen patch chain now write
  `.rng-code` with 256 actual bytes at `$5D64/$5D6A`, logical PC, and physical
  HuC6280 address. An authenticated `.mc0` run produced `$5D64`, 50 `$B0E5`
  entries, and 512 instruction samples.
- 🔒 The run lacked CD→RAM origin receipts and showed no verified RNG return
  owner. The raw-code sidecar must therefore not be used to invent RNG
  values, monster stats, AI, loot, T700, or T900 behavior.

## 2026-08-09 — Remaining Theron semantics after teleporter fix

- 🔧 The external Mednafen capture now has an explicitly extended, bounded
  register limit. An authenticated `.mc0` run reached `$B0E5` and `$5D64`,
  while a separate cold start proved CD→RAM transport and `$4644`/`$4667`.
  The sessions remain separate; neither publishes RNG, spawn, AI, T700, or
  T900 results.

- 🔒 RNG return, live creature AI, attacks/damage/loot, generator timing,
  T700 stats, and T900 rules remain gated until the same authenticated
  runtime capture binds their real consumers.
- 🔒 Dungeon material bank, perspective/square-to-tile, VCE palette ownership,
  bitmap decompression, US text consumer, JP portraits, and audio/ADPCM/SFX
  consumer remain separate source-join gates.

## 2026-08-09 — Native SDL capture verified but semantics remain gated

- ✅ The capture script now accepts an authenticated instrumented Mednafen
  PCE binary even when its `-help` output lacks the module list. The fallback
  requires the PCE CD-core binary signatures and leaves media, runtime, and
  semantic gates unchanged.
- 🔒 A real run with native SDL 2.32.10, US Track 02, System Card, and a save
  state produced VDC/VCE snapshots and authenticated input/CD-start receipts,
  but did not reach the game-owned CD→RAM consumer: `host_keys=0`,
  `authenticated_cd_ram=0`, and no dynamic RNG/creature/AI/T700/T900 receipts.
  No semantics may therefore be enabled from this run.

## 2026-08-09 — Verified gate on later-level frame chain

- 🔒 Authentic US/JP later-level blocks and their six-byte framing are
  hash-verified, but a direct attempt to run US level 1 through the flat host
  lift stops at `DECODE_POINTER_TABLE`, even when the shared `$E8` prologue
  is used only as a diagnostic seed. This is negative evidence, not a reason
  to create a table.
- 🔧 The next capture must bind the `$23DC -> $23AD` recursion, end of the
  frame chain, destination pointer, MPR table `$3B7E-$3B85`, and the following
  `$2600` consumer in the same authenticated run. Until then, bitmap/tile
  atlas, square-to-tile, perspective, VCE palette, and object semantics
  remain closed.

## 2026-08-09 — Remaining source semantics after dungeon lookup fix

- 🔒 Dungeon-aware source-creature lookup is verified. The major gate remains:
  the original `$B0E5`/`$4644`/`$4667` RNG/spawn return, T500/T600 AI and
  attack/damage/loot, T700 stat consumer, and T900 object/inventory ownership
  still lack complete authenticated runtime evidence and must not be replaced
  with host data.

## 2026-08-09 — Remaining startup gate after InputGrab evidence

- 🔒 The new v15 capture binary confirms Mednafen-owned
  `input_grab_state enabled=1` after the real macOS `Ctrl+Shift+G` chord.
  `Z`/`X` are delivered as SDL scancodes 29/27 and Run as 40, but the
  authentic US Track 02 run remains in the System Card/BIOS: 47 PCE input
  transactions, 2 IRQ2 callbacks, 0 raw sectors, and every PCE read returns
  `0x3f`. The next step is therefore startup/CD frame progression with the
  authentic runtime, not more host key bindings. No RNG, creature, AI, T700,
  or T900 semantics may be enabled from this negative capture.

## 2026-08-09 — loader-write instrumentation

- 🔧 The capture build now applies a post-patch `v3`
  `main_ram_loader_write` hook. A new real Mednafen run with a native SDL2
  runtime is still needed; the compiled local binary is therefore not yet
  runtime evidence.

## 2026-08-09 — After the byte-decompression lift

- 🔒 The complete retail routine `$23AD–$252A` has now been lifted at the
  byte level and tested with safe bounds. The next required evidence is the
  MPR table, destination, and pointer-table state from stage 2 in the same
  capture for a real later level. Without it, the authentic decoded bytes
  must not be called a tile atlas, bitmap, dungeon map, or object record.
- 🔧 Bind `theron_v1_huc6280_decode_resource()` to such an authenticated
  runtime window and check the result length/hash against the game consumer's
  CD sector and `$2600` RAM. Atlas binding and square-to-tile mapping can
  then proceed; RNG/AI/T700/T900 remain separate capture gates.

## 2026-08-09 — Continued authentic runtime capture

- 🔒 The latest clean v3 capture used replay
  `run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30` on authentic US Track
  02. All five scripted events were verified on the PCE bus: Run=`0x0008`
  and Button I=`0x0001`. The capture produced 5,943 input samples, 161 raw
  sectors, and 87 MPR-bound spawn-register samples. It still did not reach
  `$B0E5`, a game-owned dynamic CD read, or a dynamic return contract; RNG,
  creatures, AI, loot, T700, and T900 therefore remain gated.

- 🔒 A new 120-second v3 capture with replay
  `run@8:60,i@480:30,i@900:30` now uses the correct startup sequence of Run
  followed by Button I on authentic US Track 02 media. The verified PCE
  input receipt contains 10,145 input samples, `I=0x0001`, `Run=0x0008`,
  161 raw sectors, and 215 spawn-register samples. The capture still lacks
  `$B0E5`, a game-owned dynamic CD read, and `$C96B/$CC4C` consumer return; it
  therefore remains rejected by the strict gate and must not drive T900,
  RNG, AI, loot, or T700.
- 🔒 The new v3 sidecar is now strict: a semantic spawn correlation must
  observe `LB0E5` (`$B0E5`) in the same run as `$4644`/`$4667`, the consumer
  windows, and the dynamic return contract. A v3 capture reached 161
  authentic Track 02 sectors and 87 register samples but lacked `$B0E5`; it
  is therefore correctly rejected and must not drive T900, RNG, AI, loot, or
  T700.

- 🔒 If comma/period do not respond in a native Mac run, the capture must
  first have an approved Quartz helper build and Mednafen must have input grab
  active. `Z`/`X` are the layout-stable Button I/II fallback. This does not
  affect the gate on game-owned CD reads or later RNG/AI/T700/T900 semantics.
- 🔧 The macOS global-HID helper now reports the observed frontmost PID and
  uses it as focus evidence. A local run was still stopped when macOS kept
  another window frontmost; this is not yet evidence of game-owned input or
  CD handoff.
 - ✅ A separate execution-window parser now accepts the real state capture's
  2,048 register samples in `$C96B–$CA69`/`$CC4C–$CD13` even when `$4644` and
  `$4667` are absent. Register PCs are validated against the HuC6280's full
  21-bit bank address space rather than incorrectly requiring only `$1fxxxx`.
- 🔒 The strict semantic gate still requires the `$B0E5` spawn entry,
  `$4644` pre-consumer, `$4667` helper, and dynamic return contract. The new
  receipt path publishes no RNG, creature, AI, loot, T700, or T900 rules.
- 🔧 Combine `$4644`/`$4667`, the complete `$C96B–$CA69` consumer window, and
  the RAM reads instrumented as `spawn_consumer_read` in one authenticated
  run. The new-game replay now proves the pre-consumer/helper, while the
  state autoload proves the `$C96B` window; two separate runs must not be
  combined into a synthetic spawn record.
- ✅ The Mac Mednafen profile now has a working input-grab shortcut at
  `Ctrl+Shift+G`; the default `Ctrl+Shift+Menu` does not work on keyboards
  without a Menu key. Explicitly configured comma/period bindings can thus
  reach the emulated PCE controller for Button I/II.
- 🔧 The capture script now verifies the actual PCE wire masks in each
  scripted input receipt: Button I `0x0001`, Button II `0x0002`, Select
  `0x0004`, Run `0x0008`, and direction bits `0x0010..0x0080`. An old or
  incorrectly built Mednafen binary is stopped rather than allowed to
  produce false-positive input evidence. The new clean instrumentation
  passes the mask check with authentic US Track 02 media; game-owned CD
  reading remains the next gate.

## 2026-08-08 — Next T900 evidence

- 🔧 Input transport is now verified with real PCE wire masks and the local
  macOS profile: Button I `Z`, Button II `X` (layout-stable SDL bindings).
  Comma/period may only be used when explicitly present in `mednafen.cfg`.
  The IRQ2 trace now has correct MPR-based physical-PC provenance. Continue
  capturing the missing game-owned CD read after the 161 authentic Track 02
  sectors before enabling any RNG, AI, T700, or T900 semantics.

- 🔧 Use the saved source spawn category when the authenticated RNG consumer
  is captured. The category is now provenance in the live pool, but must not
  drive HP, AI, attacks, or generators until `$4667`/`$5D64`/`$5D6A` are
  bound at runtime.

- 🔧 Raw item records now follow inventory through pickup, drop, and save/load.
  Bind the original T900 consumer for equip/use/stack and validate its state
  writes against the same bytes before enabling any rules.

- 🔧 Run the existing Mednafen/System Card capture path with original media
  to replace `ram_consumer_2600=not_present`; without that capture, T700/T900
  stats, loot, AI, and generator logic must continue to reject mutation.

- 🔧 Bind the preserved US text codon stream to the original HuC6280 text
  consumer and control-code table. The loader must not turn `{...}` values
  into a host string before that chain is captured.

- 🔧 CDDA intake and stream handoff are verified against the local original
  RAR corpus. Still bind original game events to the correct CDDA/ADPCM or
  SFX consumer before triggering audio from creature, actuator, or menu logic.

## Theron Authentic CD Trace Follow-up (2026-07-12)

2026-07-13 live stage-two correction: the authentic US-CUE/System Card capture
2026-07-15 post-`$3800` order gate: a future positive transcript must now
record the original Stage 3 `BRK $ff` IRQ2 return from `$3800` to `$3802`
before its later `$e009` dispatch. This proves ordering through the original
loader entry only. It does not classify the later sector, promote a grid,
or establish level, object, bitmap, palette, or transition semantics.

## Theron CUE IPL/Stage-Two Follow-up (2026-07-12)

The documented converted CUE layout now resolves only its explicit
that other selectors are CD commands or bind any later record to an object or
level; later loader execution evidence is still required.
physical MODE1 sectors are validated in JP/US media. Its payload role remains
218-unit manifest envelope, but its entries remain unclassified; do not treat

## Theron Track 02 Semantic Binding Follow-up (2026-07-11)

## Theron Original Backup RAM and Save Disk Follow-up (2026-07-11)

Historical capture status (2026-08-20; superseded by the authenticated source
lock linked below): at that point, the campaign byte and selected-slot route
were mapped, while the two slot-tail bytes and the remaining body fields had
not yet been correlated. The note correctly withheld gameplay semantics for
those then-unidentified bytes.

The 2026-09-23 source lock subsequently established the complete
`DMS-SG.001` record boundary and all 134 writer-owned body bytes. The final two
bytes in each 136-byte slot are transport padding. The original restore
consumer now binds the campaign byte, Theron's maximum vitals, seven maximum
attributes and 20 temporary plus 20 persistent skill-experience values. The
native Backup RAM restore/encode path uses those authenticated fields only;
it leaves companions, inventory, equipment, position and loaded dungeon media
unchanged. Later completion values rejected by the original Continue branch
remain a documented compatibility gap. See
[`docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md`](docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md).

The community gzip `.srm` Save Disk body is a separate format and remains
opaque and non-launchable. Firestaff's `.tqsv` and `FSTQPTY1` serializers are
host/interchange formats, not substitutes for either original save path.
SRM import/export and unknown-body rejection are tracked separately from the
authenticated PC Engine Backup RAM restore.

2026-08-21: The production viewport can now find an authenticated atomic
VDC/VCE capture directly in `<theron-data>/capture/trace.*` or
`<theron-data>/trace.*`. The local original capture `work/theron-atomic`
passes the closed file-hash check and proves that the VDC write stream
reconstructs the same VRAM snapshot: 1,650 BAT cells, 63,923 nonzero pixels,
and 85 sprite pixels are presented through the normal startup/rendering path.
This remains an authentic screenshot, not a world-driven dungeon renderer.
The mapping from level cell, wall, object, and HUD state to the original
graphics selection remains open.

2026-08-21: The same authentic US media, System Card, and active-dungeon
save state were run with a scripted `RIGHT` command at emulator frame 1,
held for 10 frames. The original polling read raw mask `$0020` with result
`$3D`. The subsequent atomic capture differs in VRAM, SAT, and VDC write
stream but retains the same VCE palette and geometry. Replay verification
checks 25,890 writes and all 8,784 affected VRAM words with no mismatch. The
production viewport can now bind this specific input-to-screen relation when
the complete `trace.input` and `trace.transition` files also match. It must
not yet be interpreted as a change in party direction or position; the RAM
join remains outstanding.

An independent `LEFT` run from the same save state produces raw mask `$0080`,
polling result `$37`, and a third atomic VRAM/SAT/VDC image. It also verifies
25,890 writes and all 8,784 affected VRAM words. Memory reads show the expected
button buffer at `$28B8`: baseline `$F0/$00`, right `$D0/$20`, and left
`$70/$80`. This proves the original input chain but not yet a direction
state. The next correlation should look for a separate stable RAM field that
distinguishes right and left after `$28B8` returns to zero.

## Theron's Quest

### Theron V1

- 🔧 2026-07-15 Track 02 post-Stage-2 `$e00f` service boundary: the same
  authentic 45-second boot receipt now covers direct non-System-Card calls to
  both System Card loader entries. Across two Stage-2 returns and 52 observed
  post-stage physical code pages, the only `$e00f` call is the already-known
  Stage-2 `$40a4 -> $e00f` setup, with `ff0000`/`ffff`/`ff` sentinel fields;
  the only `$e009` call remains `$3840` with the same invalid fields. No later
  direct game loader call to either entry and no game-owned `$1801` writer is
  observed. Indirect, block-transfer, or unobserved-route calls remain
  unclassified, so this is a boot-path boundary, not a universal absence
  claim. The next route still requires a non-sentinel caller correlated with
  a raw-sector receipt and verified return destination.

- 🔧 2026-07-15 Track 02 post-Stage-2 game-call boundary: an authentic
  45-second US CUE + System Card 3.0 capture accepts two real host RUN
  transitions, reaches two Stage-2 returns, and observes 61 physical code
  pages afterwards. It contains exactly one direct non-System-Card
  `$3840 -> $e009` call, but its record (`ff0000`), destination (`ffff`), and
  mode (`ff`) are all sentinel values; it is not followed by a game-owned
  `$1801` writer (only System Card `$e90d/$e92d/$e981` are observed). The
  candidate therefore remains rejected and cannot be treated as a later
  record or dungeon handoff. Next evidence must be a non-sentinel game call
  correlated with a subsequent raw-sector/SCSI receipt and a verified return
  destination.

- 🔧 2026-07-15 Track 02 live SCSI caller/destination boundary: a fresh
  authentic US CUE + System Card 3.0 capture records every `$1801` SCSI CDB
  byte with its HuC6280 caller, alongside each decoded READ(6) packet and raw
  sector binding. All 48 observed READ(6) packets, including later reads
  through generation 48 / LBA 4265, were issued by System Card `$e981`
  (command bytes) after `$e90d` selection; FIFO bytes were copied only by
  `$ea50` into System Card RAM `$1f:2256+`. No non-System-Card CD caller,
  dynamic `$e009`, or game-owned destination was observed, so none of those
  later records may enter the dungeon handoff. Next admissible evidence is a
  real game-code caller and destination after the System Card returns, tied
  to a hash-verified Track 02 sector and an original level/object consumer.

- 🔧 2026-07-14 Track 02 initial-level payload handoff: the one complete,
  trace-witnessed 2048-byte `$e009` payload is now copied atomically from the
  rehashed original MODE1 user-data sector into the runtime boot receipt.
  Record `0x0b52`, source coordinate `0x114`, destination `$3800`, byte
  count, and FNV-1a checksum must all agree; any change rejects the Soul Room
  route and cannot select a generated fallback. The payload remains opaque:
  its dungeon/object/tile/bitmap/palette grammar and a positive level
  transition still need original execution evidence.

- 🔧 2026-07-15 Track 02 level/object boundary: the authenticated original
  evidence is a game-owned post-`$3800` consumer that reads a separately
  hash-bound level/object record and proves its grammar.
  boundary: level envelope `[0x114,0x480)` and the remaining opaque bytes
  remains blocked. This proves media identity and record coordinates only;

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop on original media, and evidence of where the
    loop terminates or dispatches into a record consumer.
    provenance only. Remaining: an authentic capture of the repeated

- 🔧 2026-07-11 Theron paired-CUE real-media follow-up: the hash scanner now
  accepts a CUE only when its one readable Track 01 AUDIO plus Track 02
  MODE1/2352 declaration canonically resolves to the independently
  hash-verified Track 02 payload. M12 passes that original CUE path to the
  launch profile, while an absent, malformed, renamed, or mismatched pair
  stays Track-02-only. No media is copied or synthesized. The bounded Track 01
  consumer now accepts only the CUE-declared WAV stem's local OGG counterpart
  and decodes it through optional Vorbis support; platforms without that
  decoder remain silent. Remaining work is user-staged JP/US title
  playback/capture evidence, not broader filename pairing or invented audio.

- 2026-07-27 Theron raw-CUE runtime launch regression: the current M11 path
  reaches the real startup route from the authentic USA MODE1/2352 CUE/BIN
  set (`f23601102138f87c33025877767ebf76`) and no longer relies on a direct
  Track-02-only probe. The focused runtime CTest advances title, stage, and
  Soul Room inputs under the dummy SDL driver, then requires
  `phase=theron-startup-2` and the original US asset identity. This proves
  startup admission and flow only; it does not promote unbound Track 02
  graphics, later dungeon records, or save semantics.

- 🔧 Track 02 graphics-format follow-up: the real hash-verified JP/US raw-BIN
  catalog found 1,522 strict HuC6260-shaped windows and 78 strict LE16
  stride-shaped windows across 2,022 exact matching nonzero MODE1 sectors.
  Its bounded detail list retained 64 records and overflowed 1,536; these are
  overlapping syntax matches, not independently proven palettes/tables. The
  catalog authorizes no decoder or runtime route. Exact media receipt:
  `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
  Next evidence must trace one catalogued user-data offset through HuC6280 CD
  loader code to a VCE palette write or VDC VRAM destination, including the
  loaded byte count; only that can bind a candidate to graphics, a palette,
  or a compression routine.

- 🔧 2026-08-06 JP Stage-2 disassembly follow-up: the authentic JP Track 02
  BIN is now materialised as `~/.firestaff/data/theron/TQJP02.bin` and its
  IPL loader plus dynamic `$3800` payload receipt pass against record `0x4df`.
  The later static Stage-2 byte windows remain US-only because the JP image
  has region-specific bytes; do not widen those verifier gates until a JP
  disassembly identifies equivalent instruction/data spans and their callers.

- 🔧 2026-07-11 IPL-loader provenance update: original CUE sheets prove Track
  01 is CD-DA narration, while Track 02 is the MODE1 code track. The
  hash-gated JP/US Track 02 IPL information block at logical sector 1 selects
  record `0x0003a3`, load/entry `$4000`, and a 3-sector JP or 4-sector US
  executable. Both actual executables contain `JSR $e009` (System Card
  `CD_READ`) at CPU `$40cd`; the immediately verified setup selects local RAM
  `$3000` (`DH=$01`), not VRAM (`DH=$fe/$ff`). This is the first genuine
  loader/media linkage, but it does not bind the selected record, count,
  decompressor, palette, or graphics candidate. The next admissible step is
  bounded dataflow from this loader's record table through one complete read
  setup to a verified VDC/VCE destination; generated rendering remains
  fail-closed meanwhile.

  - Update 2026-07-21: L424B's callees and the $45A6 TII gap stream
    far-call targets, and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
    streams. Remaining: JP verification awaits staged JP media; the

- 🔧 2026-07-13 dynamic Track 02 RAM receipt: the instrumented original
  Mednafen route now requires a 32-byte FNV-1a receipt from System Card
  destination `$3800` immediately after the authenticated dynamic `CD_READ`
  returns. This proves record-to-RAM transfer but does not identify a Track 02
  source byte, decompressor, palette, VCE word, VDC transfer, level, or object
  family. Next evidence must tie that exact destination span to a hash-verified
  source sector and follow its bytes through one original VCE/VDC operation.

- 2026-07-16 update: the Track02 loader-intake chain now has a
  post-predecode-to-dungeon-level gate that preserves object/dungeon
  read-window topology only when it can also consume the source-locked initial
  level handoff for the same JP/US Track02 media. Missing raw media produces
  an explicit no-fallback blocker, and the positive branch remains conditional
  on `FIRESTAFF_THERON_TRACK02_RAW`. Remaining work is still real original
  loader/CD-read evidence that assigns a verified object-table or
  dungeon-record grammar before runtime/render admission.

- 2026-07-16 update: a grammar-admission barrier now consumes that
  dungeon-level topology receipt and preserves the original CD-read record,
  byte-window, hash, and topology evidence while explicitly requiring a future
  original object-table/dungeon-record grammar witness. It admits no grammar,
  decoder, runtime, rendering, fallback visual, or synthetic byte path.
  Remaining work is a real HuC6280/System Card trace that follows one of these
  exact windows into the original object or dungeon parser.

- 2026-07-16 update: the grammar boundary now also binds back to the
  read-table/layout-binding receipt, so a positive real-media path must
  preserve the exact original CD-read records, MODE1 user-data offsets,
  destinations, byte windows, copied-byte hashes, and topology hash before it
  can reach the grammar-witness-required blocker. Remaining work is still the
  original parser trace itself; this gate deliberately admits no object-table
  fields, dungeon-record grammar, runtime handoff, rendering, fallback visuals,
  or synthetic bytes.

- 2026-07-16 update: a parser-witness gate now admits object-table and
  dungeon-record grammar provenance only when supplied original trace facts
  prove that the original loader/parser consumed those exact preserved
  CD-read windows. Even that positive receipt keeps object fields, dungeon
  record fields, decoder semantics, runtime handoff, rendering, fallback
  visuals, and synthetic bytes blocked. Remaining work is to source such
  witness facts from a real HuC6280/System Card trace instead of a caller
  supplied receipt.

- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds. **2026-07-23 update (Lane E, cycle 11):** new `firestaff_theron_v1_mechanics_playability_probe` loads the authentic JP/US Track 02 Hall-of-Records level-0 grid and verifies movement, turning, wall blocking, and floor movement on the real 32×27 loader-accepted grid (36/36 PASS on staged TQUS02.bin + TQJP02.bin). **2026-08-06 update:** the real-data thing-data regression now discovers the supplied standard `~/.firestaff/data/theron/TQUS02.bin` path (or `FIRESTAFF_THERON_TRACK02_RAW`) before the legacy fixture path and verifies all seven dungeon object/text regions: AKUTUBA 228 ground refs/1021 items, DRATOR 249/969, FORMICIA 224/871, SARMON 226/1132, SHADODAN 264/980, THIEVES 255/988, DEMON 190/881. The loader also rejects non-sector-aligned raw input. Remaining work is broader real-asset gameplay traces for doors, pits, teleporters, altar, combat, drops, and sounds once those object semantics are source-locked.

- 2026-08-06 update, reverified 2026-09-25: the real-data map, ground-reference and door/teleporter regressions now discover `FIRESTAFF_THERON_TRACK02_RAW` or standard `~/.firestaff/data/theron/TQUS02.bin` before the legacy fixture path. Against the supplied US BIN they verify all seven map groups, 4, 8, 5, 6, 3, 4 and 4 maps respectively; all seven ground-reference chains; and all seven door/teleporter tables. The same `theron_v1_track02_dungeon_map` CTest also loads the authenticated `~/.firestaff/data/theron/TQJP02.bin` and passes all seven JP map groups using the region-specific offsets in `src/theron/theron_v1_track02_dungeon_map.c`; JP map offsets are therefore no longer an open source-format gap. JP runtime publication and semantic consumer binding remain open under `THERON-V1-TRACK02-JP-LEVEL-DATA`.

- 2026-08-06 update: Track 02 raw-media intake now parses `FILE`, `TRACK`, and
  related CUE directives case-insensitively, matching the CUE format instead of
  depending on one editor's capitalization. A real-data regression builds a
  temporary CUE around the supplied `TQUS02.bin`, verifies the US pregap/index
  at raw sector 225, the authenticated BIN MD5, and trace preparation. The
  remaining intake gap is broader real CUE/BIN/ISO corpus coverage, not a
  generated fixture.

- 🔧 2026-08-06 Theron drop-placeholder removal: the old category-to-item
  resolver accepted synthetic item IDs and a host seed, then presented a
  guessed weapon, armour, consumable, scroll, or key as a real drop. The
  category table remains a verified item-name/category receipt, but no drop
  can be admitted until the original T900 consumer and selection record are
  decoded from Track 02. `theron_v1_drop_loot()` already fails closed at that
  boundary; the obsolete resolver and its positive fixture assertions are
  removed. Next evidence is a real T900 drop record plus its consumer.

- 🔧 2026-08-05 Theron production combat boundary: removed the inferred
  `theron_v1_compat.c` implementation from the `firestaff_theron` library.
  Production now uses the existing fail-closed adapter, so creature speed,
  AI, attack/defense formulas, spell combat, drops and sound IDs cannot be
  published from guessed records. Compatibility mechanics remain explicit in
  fixture/probe targets. The next replacement is still the authenticated
  Track 02 T500/T600/T900 consumer, not a new host-side table.

- 🔧 2026-08-05 Theron static consumer receipt: the authenticated US Track 19
  image now has a byte/MD5-locked regression for bank `$1f` `$243e–$24c3`.
  It proves the existing HuC6280 bitstream/register-map fragment against the
  real `TQUS19.iso` and explicitly records that the `$2600` consumer is absent
  from static ROM. The next step remains a real post-CD RAM capture with PC
  and source-LBA provenance; no RAM bytes or level/object semantics are
  inferred from this receipt.

- 🔧 Startup presentation hardening: stage/Soul Room render rows, enriched startup layout labels, and Track 02 descriptor-role receipt summaries are now test-visible; remaining work is real Track 02 startup art/audio decoding and pixel evidence instead of fallback text presentation.

  - 2026-07-08 update: Theron boot now owns the runtime dungeon/UI/V2-HUD/present render frame facade. M11 no longer calls `theron_vp_render_dungeon`, `theron_vp_render_ui`, V2 HUD render, or `theron_vp_present` directly in the Track 02 runtime path.

  - 2026-07-08 update: Theron boot now owns runtime ownership release for profile/world/viewport/assets, and M11 shutdown no longer frees those Track 02 objects directly.

- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, shop price-table regressions, and data-free cross-slot export/import are green. An authentic US Track 02 Backup RAM Continue import now passes through M11 into source-backed dungeon 2 (Soul Room), then admits the authentic Drator forcefield handoff and three movement steps on that loaded map. This is a bounded native route, not a completed original dungeon transition. The authentic US BRAM container and all 134 writer-body bytes have source-bound layout and field semantics. Update 2026-09-27: a production original-format BRAM encoder and explicit-path atomic writer now exist; against the authentic unchanged Akutuba-complete artifact, they reproduce the full 2 KiB image byte-for-byte, and Continue resolves the user's original-format save before the bundled baseline. This still does not prove persistence of changed gameplay progress or an in-game between-dungeon save workflow; JP progressed Continue remains unverified because its available authentic SRAM is empty. In-dungeon Save Game correctly fails closed because original Theron has no such transaction. Do not synthesize save fields.

### Theron V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`theron_v2_texture_upscale_pc34.c` provides `theron_v2_epx_upscale` indexed→RGBA via PCE palette). The Theron V2.2 manifest parser remains available for fixture inspection, but production now requires `source_provenance="authenticated_track02"`; the existing procedural/gpt-image-2 pack is explicitly rejected as real data. Remaining: obtain source-owned Track 02 bitmap/material records and bind them before enabling V2.2 art.

- 🔧 **2026-06-27 Theron V2 Phase 3 initial seed landed (presentation-only, data-free):** `theron_v2_hud_overlay_pc34.c/.h` is the Theron-specific sibling of `csb_v2_hud_overlay_pc34.c` + `dm2_v2_hud_overlay.c`. New CTest `theron_v2_phase3_hud_overlay_probe` (40/40 PASS, labels `tier2;theron;v2;phase3;hud;presentation-only`) covers the phase-gate + presentation-mode selector contract (V1_FAITHFUL → no HUD overlay, V20_FILTERED / V21_UPSCALED / V22_MODERN → HUD active), all 6 setters (direction, quest items, dungeon progress 1/7, relic counter 0/7, spell-rune ready indicator, 4-champion bars), render into a 256×224 indexed framebuffer, V1 chrome preservation when V2 inactive, source evidence citations (THQUEST.ASM T520/T560/T600/T700/T800/T900 + HuC6260/HuC6270 + dmweb Theron 7 dungeons + 7 relic goals + sibling csb/dm2 modules), and null safety. Companion smoke test `theron_v2_hud_overlay_pc34` (58/58 PASS, CTest `theron_v2_hud_overlay_pc34`) covers init/reset, hit-flash decay, low-HP pulse trigger, top-bar / stats-bar / action-strip visibility toggles, and per-region pixel-write assertions (compass / quest / dungeon / relic / champion bars / action strip all paint when active, and `visible=0` or `opacity=0` writes zero pixels). HUD surface: top-bar (compass + quest items + dungeon progress 1/7 + relic counter 0/7 + spell-rune ready indicator), bottom-panel (4 champion mini-bars HP/Stamina/Mana with Theron-as-leader at slot 0), and bottom action strip (ATK/CST/USE/DRP/MOV with active underline and hit-flash). Theron-specific surfaces (PC Engine 256×224 indexed fb, HuC6260 VDC layout, 7 dungeons + 7 relic goals, rune magic ready indicator) are mirrored from `dm2_v2_hud_overlay.c` + `csb_v2_hud_overlay_pc34.c`. **2026-06-27 Phase 3 placeholder-vs-real asset gate landed:** `theron_v2_hud_widget_assets_pc34.c/.h` is the Theron-specific sibling of `dm2_v2_hud_widget_assets` (the original Phase 3 gate pattern). New CTest `theron_v2_hud_widget_assets_pc34` (105/105 PASS) and headless probe `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS, labels `tier2;theron;v2;phase3;hud;widget-assets;presentation-only`) cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with the NO_MANIFEST-by-default baseline matching the current runtime. Slot table (7 slots, stable order, ordinals = indices): 5 Phase 3 primary (`compass_rose`, `quest_items`, `dungeon_progress`, `relic_counter`, `rune_indicator`, category `hud_widgets`) + 2 chrome supporting (`champion_bars`, `action_strip`, category `hud_chrome`). Manifest schema `{ id, generator, source_file, width, height }` aligned with sibling `theron_v22_modern_assets_pc34` and `dm2_v2_hud_widget_assets` shapes; manifest path `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`. Companion source-lock doc `docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md` documents the slot table, schema, gate state machine, M12/Phase 7 integration points, and honest boundary. Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview, `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`, sibling `dm2_v2_hud_widget_assets.h`. **2026-06-28 runtime handoff landed:** M11 now calls `theron_v2_hud_render()` in the live Theron Track 02 render path after `theron_vp_render_ui()` and before `theron_vp_present()`, gated by non-V1 presentation mode. **2026-06-29 overlay seed gate landed:** `theron_v2_hud_seed_from_v1_world()` now owns the V1-world snapshot mapping and returns explicit `V1_SKIPPED` / `V2_READY` states; `firestaff_theron_v2_overlay_seed_gate_probe` covers V1 hidden/no-paint behavior, V2 field mapping, byte-identical synthetic V1 world state before/after seeding and rendering, deterministic framebuffer output, gate-name stability, and NULL safety. **Remaining Phase 3 work:** (a) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`, (b) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator ≠ "placeholder"` so the gate can promote to `PARTIAL`/`COMPLETE`, and (c) real-art visual verification + per-region pixel gates against real Track 02 captures.

- ❌ Phase 4 - Enhanced lighting/effects.

- ❌ Phase 5 - Smooth movement and viewport interpolation.

- 🔧 Phase 6 - Touch/controller ergonomics: **2026-06-29 initial Theron-specific input seed landed (presentation-only, data-free):** `theron_v2_touch_controller_affordance.c/.h` maps Theron V2 touch swipes, edge-strafe, D-pad, left-stick, and right-stick affordances onto the shared DM1-family C001-C006 command ids while rejecting every affordance when V2 presentation is off; `theron_v2_touch_runtime.c/.h` translates accepted affordances into `Dm1V1QueuedCommandPc34Compat` entries and adds a Theron 256x224 HUD-chrome exclusion gate for touch starts on the V2 top bar, champion mini-bars, and action strip while controller inputs bypass the framebuffer coordinate gate. New CTest `theron_v2_touch_controller_affordance` (267/267 PASS) and probe `theron_v2_touch_runtime_probe` (138/138 PASS) are data-free and source-locked against THQUEST.ASM T520/T560/T600 plus ReDMCSB DEFS.H:238-243, COMMAND.C:2045-2155, CLIKMENU.C:142/180, and GAMELOOP.C:164-219. **2026-08-09:** live M11 Theron binds W/S/A/D to the four-way PCE pad, mouse 1/2 to Button I/II, and short/long touch to Button I/II; held motion is gated to the loaded dungeon phase. Shared M11 SDL gamepad routing now exists; remaining Theron-specific work is a real touch-layout target-size audit across launcher/game views and real Track 02 runtime proof.

- ❌ Phase 7 - V2 verification suite.

## Theron Track 02 remaining evidence

- 2026-09-25 authentic JP Track 02 regression: copied the supplied JP Rev. 1
  Track 02 BIN from the user's trv2 data directory into ignored local scratch
  and verified its MD5 against the locked identity
  (`b7afb338ad31be1025b53f9aff12d73a`). The JP later-dungeon runtime,
  champion roster, dungeon loader, door, dungeon map, level-data-block and
  thing-data tests all pass against that real BIN. This verifies source-data
  intake and the currently implemented source-only world routes; it does not
  establish JP text decoding, original graphics consumption, gameplay parity,
  or complete JP-disc availability. A JP startup-script attempt with only the
  CUE, Track 01 and Track 02 staged remained fail-closed because the JP Track
  19 bank was absent. Follow-up with the authentic Rev. 1 raw Track 19
  (`27d54f58154662885bb67d5967e5111e`) passes
  `test_theron_v1_jp_raw_bin_startup.sh`: the native JP route reaches the real
  runtime, reports all seven Track 02 item-name banks, binds the JP Track 19
  name bank and proves its Sarmon item mapping, and accepts the movement
  sequence. This closes the Track 19 staging gap for that route; the staged
  CUE still lacks its remaining tracks, so CD-DA readiness and complete-disc
  audio remain unverified. Original graphics consumption and broad gameplay
  parity remain open. The same native JP boot probe also passed directly
  against the user's `.firestaff/data/theron` files (`TQJP02.bin` and
  `TQJP19.iso`): it loaded four authenticated dungeon levels, 291 source
  objects and the two-champion party, with the JP Track 19 name bank and
  Sarmon mapping bound. Seven focused JP runtime/data CTests passed against
  those same local files. `theronTrack01CddaReady` remained zero because no
  complete JP CUE/audio set is currently available in that directory. A
  complete authentic Rev. 1 CUE and its nineteen original BIN tracks were
  then staged from the user's trv2 corpus in ignored scratch. The JP CUE
  runtime regression passes, and its native boot receipt reports
  `theronTrack01CddaReady=1`. The real-media Track 01 CDDA handoff regression
  also passes and starts/queues the original raw audio sectors through SDL's
  dummy output. This verifies JP Track 01 handoff, not later music-track
  transitions or audible hardware output.
  `theron_v1_jp_raw_bin_startup` now tests this supported raw-media route
  without requiring the unavailable full-disc CUE/archive; it requires the
  authentic Track 19 bank as well, and its real-media CTest passes against the
  local files.

- [ ] THERON-V1-TRACK02-LIVE-LOADER-CONSUMER: the latest US replay against the
  authenticated US Track 02 ISO now gives a real HuC6280 loader witness
  (`$2286` `TIA` followed by 13 block transfers, 24 RTS and 24 post-RTS rows)
  plus 4,096 static-bank consumer reads and an executed `$2c54–$2c69`
  code-window receipt. The parser now accepts this richer real trace. It still
  has no `$2600` dynamic consumer bytes, no VDC VRAM/VCE snapshot, and no
  source-owned level/object field decisions, so visual runtime drawing and
  source-consumer correlation remain blocked. The interactive forcefield route
  now admits a source-only map/thing handoff from authenticated raw BIN data;
  it does not promote VDC/VCE pixels, host item semantics, or guessed field
  meanings. Next evidence is a capture that reaches the game-owned post-CD
  consumer and closes the VDC snapshot on clean exit.

  - 2026-08-14 direct source-to-record capture verified locally on
    2026-09-25 with `scripts/verify_theron_record_table_provenance.py`:
    29,914 direct provenance rows, four complete ten-byte runtime records
    from authenticated Track 02 LBAs 4880, 4886, 4896 and 4901, 40 exact
    `theron_record_watch` write matches, and 7,100 executing `$C3A0–$C429`
    caller-window rows. This closes source-sector → game-owned RAM write →
    runtime-record mutation → executing-caller provenance. It still does not
    identify the records as levels, squares, objects, creatures, or gameplay
    transactions; level/object, rendering, AI, loot and gameplay gates remain
    closed pending an original semantic consumer and reproducible transaction.
    The large raw traces remain local on the external disk and are not added
    to the repository.

  - 2026-08-14 update: an authentic Mednafen savestate execution-window
    capture now identifies a mutable 10-byte runtime record-table chain:
    `$C9BD` derives a base from `$6000,X`, `$CB89` scans `$611D` records, and
    `$CBCC` copies `$2935–$293E` into `$611D–$6126`. This is useful runtime
    consumer evidence, but the savestate has no same-session CD-origin
    receipt, source LBA, or proven level/object role. The production gate
    remains closed; details are in
    `docs/source-lock/theron-disassembly/theron-runtime-record-table-consumer-20260814.md`.

  - 2026-08-14 bounded-receipt replay: the checked-in Mednafen receipt now
    emits 4,096 runtime-table rows. The recurring raw row
    `4080007098a8c8700020` also occurs byte-exactly at seven authenticated US
    `TQUS02.bin` offsets. This strengthens raw source overlap only; the
    savestate run has no same-session CD-origin receipt, so level/object and
    gameplay semantics remain blocked.

  - 2026-08-14 same-process continuity replay: a fresh real-SDL instrumented
    process produced 256 authenticated CD→RAM receipts, 32 game-owned
    `$E009` dispatches and 4,096 runtime-table rows after selecting the
    verified Theron state slot 6 and issuing Load State in that same process.
    This closes the earlier process-continuity gap, but the runtime rows still
    begin after explicit state injection; there is no source-LBA → RAM-write
    → record-mutation join. The result therefore remains a continuity witness,
    not a level/object or gameplay semantic promotion. Capture binary MD5:
    `2d84469309f81c582ed59160493fa170`.

  - 2026-08-14 natural interactive replay: a fresh real-SDL instrumented
    process booted the authenticated US CUE and reached the same runtime
    record-table consumer without savestate autoload. The capture contains
    256 `pce_cd_origin_ram_receipt` rows, 32 `$E009` dispatches and 4,096
    `theron_runtime_record_table` rows; the first runtime row is the known
    `$611D` value `4080007098a8c8700020`. This confirms live execution of the
    consumer after authentic CD activity, but the capture has no
    `fifo_origin_main_ram_consumer` row and does not bind a Track 02 source
    LBA to the later `$6000`/`$611D` mutation. It therefore strengthens the
    runtime witness only and does not open the production level/object or
    gameplay gates.

  - 2026-08-14 direct source-to-record join: an external-disk r25 capture
    against the authenticated US CUE produced 238 authenticated CD→RAM
    receipts and four complete ten-byte records bound to source LBAs 4880,
    4886, 4896 and 4901 (source offsets 301–310). The destinations are
    `$0d/$0f` banked RAM at logical `$611D–$6126`; reader `$F406` and writer
    `$F427` were observed, and all 40 bytes matched exactly one
    `theron_record_watch` write. The join is reproducibly checked by
    `scripts/verify_theron_record_table_provenance.py`. This closes the
    source-LBA → RAM-write → record-table mutation instrumentation gap, but
    not the original level/object/gameplay semantic consumer; those gates
    remain fail-closed.

  - 2026-08-14 consumer-trace expansion: the capture-only HuC6280 hook now
    retains reads from the wider disassembly-bound `$C600–$CD13` consumer
    window, including banked runtime addresses rather than only physical
    bank `$1f`. A fresh bounded replay still reaches only the `$20F8/$20F9`
    record-base pointer reads and has no authenticated CD-origin receipt or
    `$611D` mutation in that session. This improves the next capture's
    observability but does not open the level/object or gameplay gates. The
    savestate replay separately confirms a `$C68C` read of `$611D` through
    `record-watch`, but has no same-session authenticated CD-origin receipt.
    With a bounded 1,048,576-read state trace, parser-only admission reports
    427 runtime reads in `$2600`, 147 non-zero values, and 84 `$C3A0` reads;
    the full `$2C54` code-window check does not pass on this segmented state
    trace, so these counts remain diagnostic evidence only.

  - 2026-08-14 same-session `$C3A0` consumer: the same direct capture contains
    7,100 `record_c3a0_window=1` register rows in `$C3A0–$C429`. Each row's
    physical PC matches the captured MPR-derived bank coordinate. The
    provenance verifier accepts `--spawn-registers` and fails unless this
    execution witness is present. This proves the original runtime caller
    executes after the source-bound record writes; it still does not identify
    the record's semantic role or open level/object/gameplay publication.

  - 2026-08-14 extended Cocoa gameplay attempt: a 120-second authenticated
    r25 run delivered 19 scheduled host inputs, 238 authenticated CD→RAM
    receipts, 29,913 direct provenance rows and 7,100 `$C3A0` rows, but zero
    valid `$B0E5` category entries (`A=0..3`). The 256 `$B0E5` address hits
    were bank overlays only. This is stronger same-session negative evidence;
    it must not be merged with the separate `.mc0` run that reached a valid
    entry without CD provenance.

- 2026-08-06 update: the Track 02 thing-category enum is now source-bound to
  the retail order used by DMBUILDER6 (`4=monster`, `5=weapon`, `6=clothing`,
  `7=scroll`, `8=potion`, `9=chest`, `10=misc`, `14=missile`, `15=cloud`).
  A real US Track 02 regression now checks all seven dungeon object-count
  tables and requires nonzero copied payload for every populated category.
  This is raw record provenance only; runtime item/monster publication and
  combat/render semantics remain closed until their consumers are bound.

- 2026-08-06 update: categories 4–10 now have a portable little-endian raw
  record decoder. It binds the two-byte next-reference prefix and the
  DMBUILDER field layouts for monsters, weapons, clothing, scrolls, potions,
  chests, and misc across every populated record in the real US corpus.
  Categories 14/15 now use the same source decoder for their six-/two-byte
  payloads; no item is published into the runtime object model yet.

- 2026-08-06 update: the full Track 02 dungeon loader now consumes those
  source-bound records and follows their authentic next-reference chains on
  all seven US dungeons. It reports decoded/unbound records separately and
  leaves `Theron_V1_Object` untouched for categories whose host owner is not
  proven. The remaining handoff is the original object-kind/item-index
  consumer, not raw media intake or chain traversal.

- 2026-08-06 update: each real Track 02 map header now survives the world
  handoff as an exact verified receipt (`x/y` offsets, opaque bytes, XP and
  door bytes, map id and creature count). These fields remain semantic
  read-only evidence; seed, spawn direction and object-kind publication stay
  closed pending the original consumers.

- 2026-08-06 update: the same world handoff now retains each real map's
  `creature_gfx_bank` and cumulative column thing-count from the Track 02 map
  directory. They remain raw level-record evidence; no creature graphics or
  object semantics are inferred from either field.

- 2026-08-06 update: every real category 4–10, 14 and 15 occurrence now carries
  both its exact raw bytes and the decoded source record (including missile and
  cloud payload fields) through the full-dungeon handoff. Host object-kind,
  inventory and projectile/cloud ownership remain deliberately unbound; no
  synthetic object is created.

- 2026-08-06 update: the real-data thing-record regression now covers both
  authenticated `TQUS02.bin` and `TQJP02.bin`. All seven Japanese dungeon
  blocks use their source-bound map/item offsets, retain 871–1 132 records per
  dungeon and decode every populated category without publishing a host
  object. Japanese text remains at zero until its codon consumer is proven;
  no translated or synthetic text is inserted.

## Theron Track 19 remaining evidence

- 2026-08-06 update: the authenticated 32x27 Track 19 startup-level record now
  survives the file-inventory handoff with its six raw header words, payload
  size/nonzero count and payload FNV-1a. This remains a source receipt only;
  tile, object and later-level semantics still require the original consumer.

- 2026-08-06 update: the real US and JP Track 19 startup envelope now has a
  bounded structural reader: big-endian 32×27 dimensions, six retained raw
  header words, and an 864-byte borrowed payload span are checked against the
  authenticated envelope hash. The payload remains opaque; tile/object
  ownership and later-level consumer semantics still require disassembly.

- 2026-07-15: Runtime level-bank selection now retains the authenticated
  startup bitmap's Track 02 MD5 and raw/user-data sector envelope. Remaining:
  obtain original loader/CD-read evidence that binds a post-startup bitmap or
  object-table record to a concrete runtime consumer. Do not infer palette,
  layout, object fields, or draw behavior from the retained startup envelope.

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    loop on original media, and evidence of where the loop terminates or
    dispatches into a record consumer.
    Remaining: an authentic capture of the repeated consume/dispatch

  - Update: the render-asset admission receipt can now feed a dungeon-facing
    real-data handoff receipt only when the same admitted US raw Track 02
    session carries matching route hashes, payload/envelope/consumer checksums,
    decoded level/object-table/bitmap/palette hashes, source-byte binding,
    object-table layout proof, and bitmap/palette decode proof. The handoff
    explicitly keeps dungeon drawing and fallback visuals closed and rejects
    synthetic dungeon state, synthetic object layout, synthetic bitmap/palette
    decode, hash drift, and fallback observation. Remaining: the positive
    original capture/decoder producer that supplies these real proofs from
    Track 02 without sidecar or generated visual data.

  - Update: the multilevel Track 02 runtime path can now retain a same-capture
    bitmap/palette source-window receipt after a real level transition. The
    receipt binds the selected record, source/target levels, palette raw and
    MODE1 user-data offsets, palette payload/decode checksums, bitmap atlas
    route facts, and a combined source hash while explicitly requiring
    palette decode, bitmap decode, pixel output, M11 render admission, dungeon
    draw, and fallback visuals to remain closed. Remaining: acquire a positive
    original loader/decoder trace that proves palette words and bitmap pixels
    before connecting this source receipt to render-asset or M11 admission.

  - Update: a positive decode-vector receipt now consumes that source-window
    receipt plus the real US Track 02 bytes and verifies the HuC6260 palette
    words, the indexed bitmap atlas, route/tile/nonzero-pixel counts, first
    pixel row hash, and source checksum agreement. This proves a real
    palette/indexed-pixel vector on the multilevel route, but it deliberately
    still blocks M11 runtime consumption, M11 rendering, dungeon draw, and
    fallback visuals. Remaining: capture the original nonstartup dungeon
    graphics consumer that binds these decoded vectors, or another real
    Track 02 bitmap/palette window, to the active dungeon level before any
    render-asset admission or host-surface upload.

  - Update: the positive decode vector can now feed a production M11
    Soul Room runtime-consumption receipt. The receipt selects Track 02 level
    0 through the live `Theron_RuntimeLevelMedia` Soul Room surface, verifies
    exact indexed-atlas route checksum/nonzero pixels/offsets against the
    decode vector, verifies 1:1 host placement and clipping, and permits M11
    host presentation only for that source-owned Soul Room surface. Generic
    dungeon draw, fallback visuals, scale changes, checksum drift, later-level
    graphics, and non-Soul Room routes remain blocked. Remaining: prove the
    original nonstartup dungeon graphics consumer and per-level render layout
    before promoting broader dungeon rendering or host uploads.

- Nexus Saturn memory-card intake remains opaque: the verified boundary accepts
  only an authenticated, hash-bound 8 KiB image with 16 x 512-byte blocks on
  an active title/champion route. Remaining work is an original-card corpus
  and capture proving the proprietary header, slot layout, checksums, and any
  state semantics; FNXS/native-save fallback remains prohibited.

- Nexus Mednafen capture remains operator-only: a dry-run manifest now binds
  VDP1 word layout, decoder, palette, pixel, or render admission from the
  current files.
  exact byte count, FNV, and SHA-256. Both byte streams remain uninterpreted;
  therefore remain capture-required: do not infer a source-to-command parser,

  - 2026-07-17 M11 presentation audit: the full-output admission is still an
    opaque evidence receipt. It authenticates one complete output byte range,
    its SHA-256/FNV, and later VDP1 command order, but deliberately publishes
    no indexed-pixel declaration, width, height, stride, CLUT/palette span,
    BGR/RGB ordering, transparency rule, or host placement. Both
    `graphics_permitted` and `decoder_promoted` remain zero. Do not connect
    this output to M11's indexed/palette surface, reuse WARNING.BIN's PP
    contract, or synthesize a title/menu image. A future original trace must
    attest all of those output-format facts before a byte-exact M11 consumer
    can be added.

- Nexus Structure1F multi-level capture remains no-draw: LEV00--LEV15 now
  original Saturn trace observations; mesh/face geometry and all
  pixel/palette semantics stay uninterpreted.
  remain missing.
  Remaining work is direct,

- The direct SLEV/SAL/MAP/SDDRVS discovery route now has the materialized
  English retail auxiliary corpus with positive hash/identity and bounded
  parser receipts. Retail-positive script/audio trace evidence, dispatch,
  decoding, and playback remain blocked. The direct SDDRVS dungeon
  admission also revalidates its direct file at consumption, but it still
  awaits authentic package/level/trace evidence before any script claim. The
  matching direct SAL/SLEV/MAP dungeon route now has the same identity-only
  rehash-on-consume guard; it does not establish a codec, event meaning,
  playback, or script semantics. The verified SAL `dsp01.EX` container
  preamble and bounded opaque payload interval are now retained only as
  provenance; descriptor/sample grammar and codec evidence remain open.
  Direct SNDLEV MAP provenance now also retains only its 24-byte header,
  bounded 8-byte rows, and terminator. M11 can bind one rehashed row to the
  active level/package/card/epoch, but selector/event semantics, codec proof,
  and playback remain unproven and blocked.

- Nexus SLEV task-body capture remains no-dispatch: every SLEV00--15 target
  requires matching admitted header/literal, raw-trace, and source-order
  receipts plus opaque external opcode and callback-owner labels. Remaining
  work is reviewed original-Saturn task-body grammar and callback ABI proof;
  no task opcode executes and no fallback script is admitted.
  The selected target can now enter M11 startup only through the matching
  direct SLEV/SAL/card/package/epoch receipt and exact SLEV FNV. That is an
  opaque source-order/trace admission only; authentic retail task-body and
  callback evidence is still required before any dispatch claim.

- Nexus SNDLEV/SAL capture planning remains playback-blocked: each unique
  audio, play sound, or draw. A real retail `NXSLSC01` capture and original
  command/driver semantics remain required.
  The payload remains opaque and non-retained; a real command grammar and

- Nexus PRS3 original-execution intake remains evidence-only: one independently
  authenticated V10 export must bind one MENU.BPK stream's complete SH-2 input
  reads, output fingerprint/range, and later VDP1 source command. Remaining
  work is reviewed opcode, pixel, and palette semantics; no decoder or graphics
  route is admitted.

  - 2026-07-22 capture-admission update: the final byte-admission stage now
    rehashes the supplied full MENU.BPK and DM.BIN bytes, derives the exact
    bounded MENU.BPK stream by the V10 offset/length, and requires FNV-1a plus
    SHA-256 agreement for those three source lanes before it accepts opaque
    output and VDP1 capture bytes. It also repeats the trace's strict final
    output-write -> VDP1-command ordering. This is not a PRS3 decoder, VDP1
    command parser, palette interpretation, pixel path, or draw permission.
    The remaining blocker is still an independently authenticated retail
    Mednafen/Saturn V10 export and its four real byte artifacts.

- Nexus PRS3 multi-capture review remains non-promoting: representative,
  independently authenticated MENU.BPK modes must agree on opaque bit-order
  and termination observations before a decoder candidate may be reviewed.
  Decoder, palette/pixel meaning, rendering, and fallback visuals remain off.

- Nexus Structure3 face/texturing capture remains capture-only: DGN face and
  Structure1F/2 provenance must agree with opaque material candidates and VDP1
  evidence. Pixel and mesh semantics remain unproven and no draw route opens.

- Nexus multi-level DGN capture remains opaque: LEV00--15 needs matched
  Structure1F, Structure2 placement, Structure3 face targets and ordered
  command/frame receipts. No decoder, mesh inference, or rendering is admitted.

- Nexus active dungeon route may report only capture-ready coverage when its
  loaded DGN identity matches the full multi-level adjudication receipt. Level,
  package, PRS3 trace FNV, or trace-size drift clears it. Decoder,
  mesh/texturing, and rendering remain unavailable.

- Nexus multi-level capture jobs remain operator-only planning data. A future
  Mednafen invocation must independently re-hash every staged retail asset and
  preserve the emitted job order; this planner never launches, captures, or
  interprets a trace.

- Nexus campaign asset intake is read-only and hash-first for explicitly staged
  loose files, ZIP members, and ISO/BIN/CUE members. Virtual container entries
  are never extracted or copied; unsupported containers remain blocked.

- Nexus Saturn memory-card startup intake remains opaque: authenticated 8 KiB
  card identity and selected route epoch may gate champion startup only. Save
  layout, FNXS fallback, and native-save semantics remain blocked.

- Nexus M12 card-startup selection consumes only exact opaque card/epoch
  readiness; native FNXS resume remains a separate route.

- Nexus Saturn-card discovery currently admits only one direct 8 KiB file;
  virtual ZIP/ISO/BIN/CUE identities are diagnostic-only and contents stay
  opaque; container launch remains blocked.

- Nexus champion startup accepts only an atomically bound direct card, package
  identity and current M11 route epoch; when the M11 PRS3 presentation receipt
  is present, it must share that exact package and epoch. Card bytes remain
  opaque and PRS3 remains no-draw.

- Nexus Structure1F records now retain parser-observed raw spans only; face,
  mesh, palette and texture semantics remain unproven and no-draw.

- Nexus Structure2 descriptor spans are source provenance only; codec, pixel
  and palette meaning remain blocked pending original evidence.

- Nexus Structure3 face spans are raw package provenance only; PRS3, palette,
  pixel and texture semantics remain blocked. The direct-source admission now
  also retains one hash-bound 40-byte entry header, its raw tag/count fields,
  and the three count-bounded 12-byte intervals only when the already admitted
  Structure3 target and ordinary source file still agree. This is framing, not
  a geometry, normal, material, texture, transform, or draw claim. The local
  retail LEV corpus is still absent, so positive corpus confirmation remains
  pending.

- Nexus Structure3 image/palette references are bounded source intervals only;
  codec and decoded surface admission remain blocked.

- Nexus MENU.BPK startup provenance now binds a selected PRS3 entry's bounded
  payload offset/length/FNV and header facts through an epoch- and
  package-bound M11 no-draw host receipt. Any engine-owned verified row may be
  selected, but its recognized mode byte, bounded opaque compressed body,
  declared output size, and body FNV must exactly match; unknown modes and
  declaration/span/FNV drift reject, including across launcher/card epoch
  transitions. PRS3 pixels, opcode grammar, and decoder promotion remain
  unavailable pending independent original-Saturn codec evidence.

- The legacy `nexus_v1_bpk_surface_class` synthetic fixture still asserts a
  synthetic PRS3 literal decoder and decoded material import. Its stored
  payload receipt now keeps the fallback-provenance bit closed, but it is
  incompatible with the current retail fail-closed PRS3 route and is not
  evidence for a Saturn codec; replace it with authenticated capture-backed
  expectations before treating it as a promotion test.

- 2026-07-17 DM1 original-save C-event package completed: F0435 now retains
  C2 `ActionIndex` and `PoisonEventCount`; F0802/F0796 preserve their bounded
  PC34 bytes. C25 and C29 exports require authenticated F0435 provenance,
  while C3/C4 snapshot drift, malformed poison width, synthetic C25/C29, and
  invalid source squares reject. The targeted original-save handoff suite is
  green; remaining work is external original-save corpus evidence.

- 2026-07-17 DM1 C2 PARTY_INFO follow-up completed: source byte 86
  `Event71Count_Invisibility` now materializes into both M10 invisibility
  owners and F0802 writes it back only as a bounded PC34 byte. The focused
  C71 path and full original-save handoff suite are green.

- 2026-07-17 DM2 DB14: the normal `QUERY_PICST_IT` `0x40`/neutral-mode branch
  now copies only authenticated native-size indexed IMG3 pixels under matching
  RAW4 clip and palette receipts. Flip, crop, nonzero offset, scaling, and
  every other blitmode remain fail-closed. Remaining: source-proven non-normal
  transform branches and live frame ordering.

- 2026-07-17 DM2 HUD SUMMARY_IMAGE: `c_gui_draw.cpp:926-942` now has a
  no-draw M11 receipt for exact `(1,vb_144,field)` HUD commands. It requires
  the source plan's decoded GDAT pixels, local palette, and RAW4 destination
  identity; tuple mismatch, absent palette, and stale destination reject.
  Remaining: source-proven HUD transform admission before any new draw path.

- 2026-07-17 DM2 HUD PICST transform: the exact `c_gui_draw.cpp:926-942`
  branch admits only source values `0..0x28`, retaining X scale `0x1f` for
  `0..0x0f` or `0x2f` otherwise and Y scale `0x35`. Out-of-range values,
  missing SUMMARY_IMAGE material, or stale destination reject; it remains
  source-gated for draw only where the resolved destination is the complete,
  exact scaled rect. Partial/unknown `QUERY_BLIT_RECT` clipping, flips, and
  every other HUD transform remain no-draw.

- 2026-07-17 DM2 pit viewport admission: `c_gui_vp.cpp:234-292`
  `DM2_DRAW_PIT_TILE` now has a bounded source receipt for cells 1..15. It
  binds `table1d6c70/90/a0/b0` selection, the live cell's `+8` state word,
  `DRAW_DUNGEON_GRAPHIC` light parameter, exact `(GRAPHICSSET,field)`
  SUMMARY_IMAGE, GFX256 raw material, decoded U4 bytes, and local palette.
  It remains `no_draw`: cell 0's `SET_GRAPHICS_FLIP_FROM_POSITION` and the
  selected `QUERY_BLIT_RECT` placement/clip chain are not yet proven.

  - 2026-07-17 composition update: cells 1..15 now bind their accepted
    SUMMARY_IMAGE/GFX256 material identity into the current DM2 viewport
    composition session/data epoch and parent ordering receipt. The receipt
    explicitly records that PIT_TILE's own draw slot is unresolved, so it
    cannot consume pixels. The sole remaining promotion precondition is the
    source's per-cell `QUERY_BLIT_RECT` destination/clip transaction.

  - 2026-07-17 RAW4 placement update: `table1d6c70[cell]` now binds through
    `DRAW_DUNGEON_GRAPHIC`/`QUERY_PICST_IT` to the exact
    INTERFACE_GENERAL/0/RAW4 root row, with destination, full material extent
    and table/row hashes retained in the PIT composition receipt. Chained
    rectangles, crop and clip grammar remain rejected. It stays no-draw until
    a PIT-owned ordered composition slot and authenticated buffer handoff are
    proven together.

  - 2026-07-17 buffer/slot update: PIT_TILE now retains its own authenticated
    decoded U4 buffer handoff and binds it to the generic DM2 viewport
    before/after surface snapshot and composition identity. Pointer, extent,
    stride, palette, material and surface-generation drift reject with no
    write. The slot deliberately remains no-draw: source proof is still
    missing for PIT_TILE's normal-branch `DRAW_PICST` row ordering.

  - 2026-07-17 normal-row update: cell 1's `blitmode=0` branch is now bound
    to `DRAW_PICST`'s top-to-bottom/left-to-right U4 row order and exact RAW4
    placement identity. It remains no-draw because `DRAW_DUNGEON_GRAPHIC`
    applies `DM2_query_B073` before that row loop; PIT still lacks its own
    authenticated transformed palette transaction.

  - 2026-07-17 B073 update: cell 1 now binds `DM2_query_B073`'s RAW7
    count/left/right/lookup palette program to its material, RAW4 placement
    and normal-row receipt. RAW7, placement or palette drift rejects. The
    transformed palette remains no-draw until alpha ownership and the final
    ordered handoff consumer are jointly admitted.

  - 2026-07-17 cell-1 consume update: only cell 1's normal (`blitmode=0`)
    path now consumes the authenticated U4 handoff through B073's transformed
    palette and low-nibble alpha into the current ordered owner surface. All
    other PIT cells, mirrors, crops and chained clips remain fail-closed.

  - 2026-07-17 cell-3 consume update: cell 3's independent normal
    (`blitmode=0`) route now admits only its exact GRAPHICSSET field `0x6e`,
    RAW4 rect `0x35b`, B073 transaction and ordered U4 handoff. Cell 2 and all
    other mirrored or unproven normal forms remain fail-closed.

  - 2026-07-17 cell-4 consume update: cell 4's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x6f`, RAW4 rect
    `0x35a`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Cell 2 and every other mirrored or unproven normal form remain
    fail-closed.

  - 2026-07-17 cell-6 consume update: cell 6's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x71`, RAW4 rect
    `0x358`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-7 consume update: cell 7's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x72`, RAW4 rect
    `0x357`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-11 consume update: cell 11's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x76`, RAW4 rect
    `0x355`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-12 consume update: cell 12's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x77`, RAW4 rect
    `0x354`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-14 consume update: cell 14's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x79`, RAW4 rect
    `0x352`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-2 HFLIP consume update: cell 2 admits only GRAPHICSSET
    field `0x6c`, RAW4 rect `0x35f`, B073/RAW7, and its own source-locked
    reverse-X U4 row walk. Crop, chained clips, vertical flip and all other
    mirror cells remain fail-closed.

  - 2026-07-17 cell-5 HFLIP consume update: cell 5 admits only GRAPHICSSET
    field `0x6f`, RAW4 rect `0x35c`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-8 HFLIP consume update: cell 8 admits only GRAPHICSSET
    field `0x72`, RAW4 rect `0x359`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-13 HFLIP consume update: cell 13 admits only GRAPHICSSET
    field `0x77`, RAW4 rect `0x356`, B073/RAW7 and its source-locked reverse-X
    U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-15 HFLIP consume update: cell 15 admits only GRAPHICSSET field `0x79`, RAW4 rect `0x353`, B073/RAW7 and its source-locked reverse-X U4 row walk.

  - 2026-07-17 crop/chained-clip update: `QUERY_BLIT_RECT` source-coordinate mutation remains no-draw behind a source-locked PIT provenance receipt; root RAW4 does not prove crop or chaining.

- 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT material admission:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now bind the
  successful `QUERY_GDAT_ENTRY_IF_LOADABLE` branch only: exact state-table
  lane, GRAPHICSSET SUMMARY_IMAGE/GFX256 raw bytes, decoded U4 indices, local
  palette, root RAW4 placement and the live DM2 composition/surface snapshot.
  It remains no-draw. The `QUERY_TEMP_PICST` fallback and the downstream
  B073/`QUERY_PICST_IT`/`DRAW_PICST` transform must be proven separately.

  - 2026-07-17 fallback update: the exact non-loadable `table1d6f7c` path at
    `c_gui_vp.cpp:514-527` now admits its own SUMMARY_IMAGE/GFX256 U4 and
    RAW4/M11 receipt plus `QUERY_TEMP_PICST(1,0x40,0x40,0,0,0,rect,-1,light,
    -1,8,graphicsset,field)` provenance. It remains no-draw because
    `query_32cb_0804` selects a live B073/field-7 palette transaction from
    `c_querydb.cpp:2415-2465`, which is not yet authenticated.

- 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary material admission:
  `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287` bind only cells
  1..8 with a defined `table1d6fdc/table1d6fee` state lane to authentic
  GRAPHICSSET SUMMARY_IMAGE/GFX256 U4 bytes, local palette, root RAW4 and M11
  owner surface. B073/`DRAW_PICST` remains no-draw pending a live palette and
  transform receipt.

  - 2026-07-17 transform provenance update: `SKULLWIN/c_image.cpp:450-475`
    now binds the side-stairs `DRAW_DUNGEON_GRAPHIC` delegation to blit mode 0,
    default normal scale and zero source offset; its source rects explicitly
    exclude the `0x2bc/0x2bd` offset special case. Material, RAW4 and M11
    identities must agree. The live `DM2_query_B073(image.palette,
    ddat.v1e12d2, alpha, -1, ...)` transaction remains unauthenticated, so
    the complete branch is intentionally no-draw.

  - 2026-07-17 live `DRAW_WALL` update: the receipt now binds one existing
    `QUERY_TEMP_PICST` wall command to the same recomputed material hash,
    M11 wall-composition identity and atomically identical owner snapshots.
    Only the source's `0x40` normal scale, RAW4 `0x2be + cell`, movement
    offset and source flip are recorded. This gate remains no-draw; it does
    not introduce a second wall renderer.

- 2026-07-17 DM2 `DRAW_WALL_TILE` admission: `SKULLWIN/c_gui_vp.cpp:6703-6741`
  and `dm2data.cpp:266-273,602-605` now bind every `table1d7012` cell branch
  to the existing authenticated wall/M11 identity. The receipt records the
  exact 0/1/2 delegated-call count and `table1d6afe` orientation; it remains
  no-draw because `DM2_guivp_32cb_15b8` has separate unbound GDAT transforms.

  - 2026-07-17 `32cb_15b8` input update: the first simple `QUERY_TEMP_PICST`
    call at `c_gui_vp.cpp:6618-6628` now has a source-owned no-draw input
    receipt for category 9 selector/image field, exact `0x40` scales, flip,
    query parameters and RG71l alpha. Record layout and destination remain
    explicitly unavailable.

  - 2026-07-17 loadable `0x0f` update: the distinct `c_gui_vp.cpp:6651-6692`
    category-9 `QUERY_GDAT_ENTRY_IF_LOADABLE` branch now binds its successful
    `0x0f` selector, normal scales, transform inputs and RG71l alpha as a
    no-draw receipt. Its destination is still not inferred.

  - 2026-07-17 category-8 overlay update: `c_gui_vp.cpp:6322-6329` now has
    its own no-draw QUERY_TEMP_PICST input receipt for selector/image field,
    normal scales, flip, transform parameters and RG71l alpha.

  - 2026-07-17 branch-set update: the three authenticated category-8/9
    `32cb_15b8` input receipts now combine only when their independent
    identities and the loadable `0x0f` field agree; the aggregate stays
    no-draw and has no placement contract.

  - 2026-07-17 DRAW_TEMP_PICST admission update: the aggregate now has a
    no-draw consumption gate that rechecks every branch-set identity,
    category, `0x0f` field and normal-scale transform before admitting the
    source call. It carries no destination or pixel information.

- 2026-07-17 DM2 `query_B073` input admission: `c_querydb.cpp:2506-2545`
  now requires authentic palette, live light, alpha/mask, colors/cache,
  RAW7, lookup and traversal identities in one no-draw receipt. No palette
  buffer or pixel result is produced.

  - 2026-07-17 B073/DRAW_TEMP_PICST surface update: authenticated B073 and
    DRAW_TEMP_PICST receipts now bind to an owned viewport-surface snapshot
    in one no-draw palette/surface receipt. No buffer is borrowed or written.

  - 2026-07-17 original palette update: the next consumer may borrow only
    original 16/256-byte palette storage when its bytes hash matches the
    caller's authenticated identity and the B073/surface receipt remains
    current. No transformed palette or pixel buffer is created.

  - 2026-07-17 M11 palette-consumer update: borrowed original palette bytes
    now bind to a current owner-surface generation in a no-draw M11 receipt,
    with no transform, destination or pixel material.

  - 2026-07-17 original material update: a later consumer may borrow only
    original decoded GDAT storage with proven dimensions, stride, byte count
    and byte hash paired to the current M11 palette consumer. No decoder or
    render path is admitted.

  - 2026-07-17 M11 material/palette pair update: original material and
    original palette now admit only as a matching no-draw pair with current
    owner generation and verified dimensions/stride. No render contract.

  - 2026-07-17 live materialization update: the validated pair now has a
    no-draw M11 handoff guarded by the same live owner generation. It carries
    only borrowed bytes/layout, never a blit or destination.

  - 2026-07-17 DRAW_PICST trace update: source handoff now reaches an exact
    `QUERY_PICST_IT`/`DRAW_PICST` trace receipt, but missing source and
    destination rectangles remain an explicit no-draw blocker.

  - 2026-07-17 DRAW_PICST rect update: `query1 == -1` now admits only the
    exact direct `srcx/srcy + imgdesc.x/y` source rectangle branch from
    `c_image.cpp:240-296`; all QUERY_BLIT_RECT, flip and destination paths
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT trace update: `c_xrect.cpp:217-280` now
    admits only an authenticated unsigned root rectangle node with
    `query2 == -1`, `mode1 <= 8`, `mode2 == 0`, a present bitmap and its
    captured source-rectangle identity. Signed, overridden, mode-9 and
    chained nodes remain no-draw until their clip/destination semantics are
    separately evidenced.

  - 2026-07-17 QUERY_BLIT_RECT signed-root update: `c_xrect.cpp:228-276`
    now records the exact signed-node `datax/datay + input-x/input-y`
    transform for an authenticated unchained root. `crdecode`, final clip,
    destination, overrides and every chained node remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT mode-1 update: the signed-root receipt now
    reaches the exact `crdecode(1, ...)` origin assignment in
    `c_xrect.cpp:162-211,426-436`, guarded by current authenticated material
    dimensions and surface generation. All other modes, clipping and final
    destination bounds remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT default-clip update: `c_xrect.cpp:239,438-470`
    now admits the untouched `rc=[-10000,10000)` range only for a current
    mode-1 receipt whose full material rectangle lies inside it. Global clip
    override, chained terminal nodes and all surface-specific destinations
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-clip update: the explicit
    `c_gui_vp.cpp:570-573` `TRIM_BLIT_RECT` transaction now provides the only
    admitted `dm2rect1` override input for `c_xrect.cpp:438-439`, with active
    flag, trim-call, material and surface identities. Intersecting that clip
    with the destination rect and every final blit remains no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-intersection update:
    `c_xrect.cpp:446-470` now admits the exact `dx/dy` source-offset and
    clipped destination-rectangle calculation for the authenticated mode-1
    global-clip path. Missing overlap or any clip/material/surface identity
    drift rejects; no blit is admitted.

  - 2026-07-17 DRAW_PICST surface-address update: `c_image.cpp:293-335` and
    `c_gfx_blit.cpp:604-656` now admit only the native 8-bit `gfxsys.dm2screen`
    row-address path with packed original source stride, exact source/dest
    offsets, no palette translation and no alpha mask. The receipt borrows
    addresses only; all pixel writes and other surface formats remain no-draw.

  - 2026-07-17 DRAW_PICST row-traversal update: the original material bytecount
    now remains attached through the M11 handoff. `c_gfx_blit.cpp:604-656`
    default `BLITMODE0` admits only forward rows with authenticated first/last
    row offsets and exclusive source/destination bounds. Other modes and every
    pixel operation remain no-draw.

  - 2026-07-17 DRAW_PICST mask/palette update: `c_image.h:45-70` and
    `c_gfx_blit.cpp:655-760` now admit only the masked translated `BLITMODE0`
    input transaction with 256 authenticated palette bytes, exact alpha index,
    original material bytecount and forward row bounds. Palette translation
    and every pixel write remain no-draw.

  - 2026-07-17 DRAW_PICST palette-index update: `c_gfx_blit.cpp:39-42,675-682`
    now has a source-locked trace that records the exact ordering: compare raw
    8-bit source index to alpha first, then use that same index in PAL256.
    It does not dereference source/palette bytes or write pixels.

  - 2026-07-17 DRAW_PICST palette-write update: source proof now fixes each
    `t_palette` entry to one `c_pixel256` byte and PAL256 to 256 bytes. The
    masked destination write order is carried as no-draw row metadata with
    current surface identity; no conditional pixel write is executed.

  - 2026-07-17 DRAW_PICST native execution update: the fully authenticated
    8-bit BLITMODE0/PAL256/mask branch now has its first source-backed pixel
    consumer. It revalidates all receipts and owner generation before the
    exact forward masked writes; every mismatch is no-write.

  - 2026-07-17 DRAW_PICST M11 update: the native executor now enters only
    through a DM2-owned M11 consumer that requires the exact live material
    handoff buffer/palette and owner generation. No legacy renderer or
    fallback path can reach this consumer.

  - 2026-07-17 DRAW_WALL admission update: authentic GDAT wall commands now
    enter a strict DRAW_PICST admission with their raw/decoded/palette/geometry
    receipts, but remain no-draw because the source route owns PAL16 rather
    than the proven native PAL256 executor contract.

  - 2026-07-17 DRAW_WALL B073 update: PAL16 now binds to a strict PAL256 cache
    output receipt only with complete RAW7, lookup, traversal and allocation
    identities from `c_querydb.cpp:2506-2668`; no expansion or write occurs.

  - 2026-07-17 DRAW_WALL B073 contiguous RAW7 loader update: only the
    original `INTERFACE_GENERAL/0/RAW7/2` record admitted by
    `dm2_v1_asset_load_typed_sized()` may bind its contiguous bytes, exact
    length and FNV identity to the wall PAL16/B073 cache allocation.

  - 2026-07-17 DRAW_WALL B073 interpreter update: `c_gdatfile.cpp:1919-2003`
    and `c_querydb.cpp:2506-2668` now source-bind RAW7's descriptor, interval,
    output and lookup regions to a supplied owned PAL256 cache. The resulting
    cache is attached to the wall `DRAW_PICST` output receipt, but remains
    no-draw until the authentic U4-to-PAL256 blit consumer is proven.

  - 2026-07-17 DRAW_WALL native M11 update: the proven normal, unflipped,
    unmoved 0x40 U4-to-8 branch now consumes the authenticated B073 cache
    using `c_gfx_blit.cpp:495-548` source order. Scaling, flip, movement,
    clip, cache, surface and composition drift remain fail-closed.

  - 2026-07-17 DRAW_WALL HFLIP M11 update: the separately proven BLITMODE1
    branch now follows `blitline_48_mi/mima` reverse-X destination order.
    Vertical/chained flips, movement and scale changes remain fail-closed.

  - 2026-07-17 DRAW_DOOR panel M11 update: the stationary, closed, unflipped
    and unscaled DOORS panel now consumes its exact IMG3 U4 bytes, local PAL16,
    colour key, RAW4 rectangle and composition-owned surface. Opening,
    movement and light-remap branches remain fail-closed.

  - 2026-07-17 DRAW_DOOR split M11 update: only the source-proven horizontal
    opening states 1..3 now consume the paired halves in the `DRAW_DOOR`
    table order: right half (`base + state + 6`), then left half
    (`base + state + 3`). Both halves require the same authenticated DOORS
    material receipt and distinct RAW4 geometry rows, plus current composition
    and owner surface identities. Vertical opening, movement, flip and every
    incomplete table/material chain remain fail-closed.

  - 2026-07-17 DRAW_DOOR vertical M11 update: the source-proven vertical
    intermediate states 1..3 now retain the whole original DOORS image and
    select exactly `tlbRectnoDoorPosition[cell] + state` before one forward
    palette-mapped consume. The raw material, RAW4 table row, composition and
    live surface must all still match; horizontal split, movement and flip
    remain separate fail-closed routes.

  - 2026-07-17 DRAW_DOOR_FRAMES right-jamb M11 update: the stationary
    `QUERY_TEMP_PICST(1, 0x40, 0x40, ..., rect, 3)` route now admits the
    authenticated GRAPHICSSET U4/PAL16 side-frame with reverse-X writes. Its
    scene-owned colour key and current scene hash are required alongside RAW4
    geometry, composition and surface identity. Left jamb, panel flips,
    frame motion and every other transform remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES left-jamb M11 update: the matching stationary
    `QUERY_TEMP_PICST(0, 0x40, 0x40, ..., rect, 4)` branch now consumes its
    authenticated GRAPHICSSET U4/PAL16 material in forward-X order. Receipt
    identity locks the jamb kind, RAW4 row, scene colour key/hash, composition
    and live surface, so it cannot be used as the mirrored right route.
    Frame motion, scaling and panel flips remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES movement M11 update: only the source's
    `v1e12d0` branch may select `table1d6b2c[cell]` and its swapped
    `table1d6ee1` jamb column, while preserving the original cell's RAW4
    rectangle and normal jamb direction. The movement owner bit, selected
    field, scene, composition and surface must all match; panel motion,
    scaling and every unrelated transform remain fail-closed.

- 2026-07-17 DM2 pit-roof viewport admission: `c_gui_vp.cpp:118-206` now
  source-gates cells 1..8 on the exact roof flag, `LOCATE_OTHER_LEVEL`
  success, remote tile type 2, and remote bit 0x08 before applying
  `table1d6c4c/5e/67`. The resulting GRAPHICSSET SUMMARY_IMAGE, GFX256 raw
  receipt, decoded U4 bytes and local palette remain `no_draw`; cell 0's
  position flip, the actual remote-map address walk, and `QUERY_BLIT_RECT`
  placement/clip still require separate evidence.

  - 2026-07-17 prerequisite update: the admitted PIT_ROOF receipt now also
    binds `DRAW_DUNGEON_GRAPHIC`'s `DM2_query_B073` c_light transaction and
    the authentic INTERFACE_GENERAL/0/RAW4 row for rects `0x360..0x368`.
    Only the exact root `mode1=1/mode2=0` `QUERY_BLIT_RECT` form is admitted;
    changed c_light identity, palette, RAW4 row/table, clip chain, cell 0,
    and every richer rectangle branch reject. It remains no-draw until the
    full B073 palette expansion and a pixel consumer are separately proven.

  - 2026-07-17 alpha/blend update: `SKULLWIN/c_image.cpp:450-475` and
    `c_gfx_blit.cpp:370-549` now bind the exact U4 alpha transaction to the
    B073 and RAW4 identities. The source alpha mask is retained in full and
    its low nibble is the only admitted transparent source index; only the
    proven normal and horizontal-mirror modes enter the no-draw receipt.
    Mask drift, vertical/combined modes, palette drift, and destination
    identity drift reject. B073's transformed palette and final destination
    composition still need independent source proof before any blit.

  - 2026-07-17 B073 table update: `SKULLWIN/c_gdatfile.cpp:1919-2003` now
    binds the exact `INTERFACE_GENERAL/0/dt07/2` RAW7 program that initializes
    `v1e020c` and `v1e0210` for `DM2_query_B073`. The count/length layout,
    both packed table regions, trailing color lookup region, raw hash, and
    B073/material identities are retained as no-draw evidence. Missing dt07/2,
    malformed lengths, and any valid raw-data drift reject.

  - 2026-07-17 B073 traversal update: `SKULLWIN/c_querydb.cpp:2506-2668`
    now admits only the cache-free per-color traversal for the authenticated
    U4 palette. Every palette byte must have an in-range two-byte RAW7 lookup,
    group, subindex, interval and alternate alpha neighbour; index drift and
    alpha-branch ownership drift reject. The transformed palette is still a
    no-draw receipt pending the exact QUERY_PICST_IT destination composition.

  - 2026-07-17 destination update: `SKULLWIN/c_image.cpp:98-410` now binds
    the normal-scale (`0x40/0x40`), zero-crop PIT_ROOF `QUERY_PICST_IT` path
    to its root RAW4 `QUERY_BLIT_RECT`, B073 palette traversal, alpha mask and
    source-proven horizontal flip. Clip receipt drift and every scale/crop or
    unsupported flip reject. It remains no-draw: the destination bitmap's
    live ownership, dimensions/resolution and final viewport clip are inputs
    to `DRAW_PICST` that are not yet retained by this DM2 receipt chain.

  - 2026-07-17 surface-owner update: DM2 viewport ownership now publishes an
    atomic framebuffer snapshot with pointer, dimensions, stride, resolution
    and monotonically advanced generation. PIT_ROOF binds only the exact
    current generation and remains no-draw on stale or rebound surfaces.

  - 2026-07-17 composition-slot update: PIT_ROOF additionally requires the
    DM2 composition slot's before/after owner surface pointer and generation,
    session identity, data epoch and ordered-member identity. Every mismatch
    remains no-draw; native blit still lacks a source-owned M11 consume hook.

  - 2026-07-17 material-buffer handoff update: PIT_ROOF now retains a borrowed
    identity receipt for the already authenticated decoded U4 buffer. Its
    pointer, width, height, stride, pixel count, palette hash and material
    identity must equal the composition candidate; every buffer or receipt
    drift remains no-draw.

  - 2026-07-17 ordered-consume update: the source-owned PIT_ROOF hook now
    executes only `DRAW_PICST`'s authenticated normal-scale U4-to-8bpp masked
    rows, including the proven horizontal mirror. It consumes the borrowed
    handoff buffer directly after the composition-order and before/after
    surface checks; there is no reload or re-decode. Every crop, scale,
    vertical/combined flip, changed source index, composition/surface drift,
    or incomplete receipt remains no-write.

# Theron V2 HUD widget pixels remain blocked in production: the manifest parser is fixture-only and the runtime now fails closed until all seven slots resolve to decoded Track 02 source assets.

- 🔧 CSB V2.2 artpack follow-up: the hand-authored per-cell asset-id catalog is
  contract-test-only; production retains just the F0128 source-provenance
  admissions. A reviewed PC 3.4 GRAPHICS.DAT pixel binding is still required
  before any modern art is admitted.

- 🔧 CSB V2.2 artpack follow-up: both mode selection and F0128 cache blits now
  reject a launcher flag or readable RGBA cache until the complete
  PC 3.4 source-material/provenance gate passes. The remaining work is a
  reviewed original GRAPHICS.DAT extraction and pixel binding; no generated
  cache or PBR substitute may be admitted.

- 🔧 CSB Utility Disk CMP follow-up: production accepts CMP bytes only as a
  portrait/name/title overlay for an already authenticated champion. A
  positive original CMP-plus-save corpus is still needed before exposing that
  combined import route in the launcher.

- 🔧 CSB creature-drop follow-up: the old no-op fixed-possession API and
  no-context DSA stubs are contract-only. Bind original dungeon placement and
  the imported DSA interpreter before enabling either live creature drops or
  DSA filters.

- 🔧 CSB hidden-graphics follow-up: only the real source-loader is available
  in production. Bind a verified original GRAPHICS.DAT hidden-item corpus to
  a visible owner before promoting those records into a runtime presentation.

- 🔧 CSB Atari ST graphics follow-up: the production DMCSB1 reader accepts
  only user-supplied Atari ST data. Bind verified original animation/image
  records to the startup presentation before promoting this container reader
  beyond its current source-data loading role.

- 🔧 CSB Mac app-capture follow-up: an interactive capture of the installed
  opening-door capture; compare a rebuilt installed app against v3.0.197
  before diagnosing or masking the old red-strip report.
  an invalid step-zero gap and retained the closed C004/C002/C003 page. The

- [ ] DM1-HOC-OBJECTS-001 Capture the corrected live PC34 HoC wall-torch
  material and holder composition against the original GRAPHICS.DAT. The
  source mapping is now corrected to ReDMCSB I34E `G0194` (DUNVIEW.C:932-1007)
  and the exact `G0198`/`G0199` palette/depth route remains source-bound; close
  only after a real app capture proves the torch and holder pixels at each
  visible depth. No synthetic black ornament is admitted. Invalid global
  ornament indices outside the 60-entry G0194 table now fail closed; the
  real capture is still required. Runtime now distinguishes the synthetic
  final local inscription slot from real global ornament 0, so a real
  ornament-0 torch/holder cannot enter the inscription path.

  - 2026-08-06 fallback audit: the remaining legacy wall/door/floor helper
    paths now fail closed unless the authentic per-map ornament table and
    decoded pixel buffer are present. They cannot manufacture a global
    ornament index or draw a dimension-only slot. This is code-side cleanup;
    the real Mac/window torch-and-holder capture is still open.

  - 2026-08-06 viewport-coordinate audit: the live M11 F0128 iterator uses
    normalized D3 outer-wall offsets `-1/+1`, while the raw F0115 D3L2/D3R2
    source contract also exposes `-2/+2` aliases. The C127 mirror admission
    now accepts both representations and keeps the real C346 backing material
    for `viewWallIndex` 0/1. Real PC34 all-cell coverage passes; Mac/window
    pixel capture remains open.

- [ ] DM1-HOC-OBJECTS-002 Capture a real PC34 HoC pickup/placement round trip
  The manual does not replace the required original PC34 runtime capture or
  the M564 name/slot evidence.
  C00/C01 hand masks and backpack ownership remain source-backed. The F0033

- 2026-08-06 source-runtime verification: the real PC3.4 alcove test now
  completes pickup-to-placement for Thing 5196 (graphic 511), preserving the
  source `AllowedSlots=0x40` mask and placing it in legal quiver slot C519.
  M564 name-table validity remains intact after placement. Remaining scope is
  real macOS/window capture plus the requested weapon, potion, scroll,
  container and junk corpus; do not reopen the source route without a failing
  real-data case.

  - 2026-08-06 source-identity hardening: the live DM1 F0115 floor and
    F0121/F0124 alcove consumers now require the raw PC34 `THING` record before
    resolving subtype or drawing an icon. Candidate viewport metadata can no
    longer manufacture a plausible but incorrect object when the source chain
    is incomplete; the real floor-item and alcove pickup/place tests still pass.

- 2026-08-06 update: the active legacy stairs helper now rejects dimension-only
  cache entries unless the authentic GRAPHICS.DAT surface is decoded
  (`loaded` and `pixels` are both present). This prevents an invalid stair
  cache record from reporting a successful draw and covering the source wall
  or floor. Real Mac capture of each visible stair depth is still required.

- 2026-08-06 update: the active DM1 zone-blit, door-ornament, destroyed-door,
  Thieves' Eye, and door-button consumers now use the same decoded-surface
  gate. Dimension-only cache records cannot reach `BlitRegion`/`BlitScaled`
  in those F0102/F0110/F0111/F0113 routes. The real PC34 sweep remains the
  authoritative data check; packaged Mac capture is still required.

- 2026-08-06 update: the DM1 action/spell utility-panel admission now also
  requires decoded C010/C009 pixel payloads, not only loaded flags and native
  dimensions. A dimension-only cache record can no longer suppress the real
  source-owned panel route while leaving the action/spell strip empty.

- [ ] DM1-HOC-OBJECTS-003 Capture the live held-object cursor on the host window
  after pickup and during movement. The source framebuffer now invalidates on
  pointer motion and hides the host arrow while G4055 is occupied; close only
  after a real Mac capture proves the object-shaped pointer remains visible at
  the mapped pointer position. 2026-08-06 source-side proof: the real-data
  `test_m11_dm1_real_object_names` now verifies 169 non-zero F0702 pixels for
  `EYE OF TIME`; only the packaged macOS/window capture remains.

- 2026-08-06 source-runtime hardening: authenticated DM1 V1 F0287 bar graphs
  now ignore `FIRESTAFF_V1_BAR_GRAPHS=0` and never re-enable the retired
  horizontal host bars. The switch remains available for non-source/debug and
  V2 compatibility sessions. Real object-corpus and held-cursor tests pass;
  packaged Mac capture remains governed by the open capture items above.

- 2026-08-06 CI follow-up: the CSB V2 touch/controller test now has its
  source-required PC34 VGA palette module. Continue watching the main build
  matrix; this closes only the missing-link regression, not a presentation
  parity claim.

- Theron teleporter resolution now rejects unresolved object-ID links and
  cycles; restore positive legacy links only when backed by an authenticated
  Track 02/T900 record corpus.
  The 2026-09-26 real-data mechanics census now exercises 335 coordinate-linked
  records in each region. Of 170 enabled pads, 72 resolve, 89 target wall
  squares, and nine continue to another enabled pad; all level references and
  teleporter endpoints are present. Keep wall-target and chained routes
  fail-closed until a source consumer or authenticated movement capture binds
  their exact outcomes.

  2026-09-30 update: the real-media movement regression now exercises four
  US and four JP active-to-active chains whose final decoded destination is a
  wall. Original movement blocks each attempted route without changing party
  position or transition fields. This is bounded parity for those records;
  the other active-link chains and direct wall-target paths remain open.

- [ ] THERON-V1-TRACK02-JP-LEVEL-DATA: the authenticated Japanese Track 02
  framing and bounded HuC6280 resource admission are verified for all seven
  level blocks, and the source-bound JP creature/object corpus is exercised
  by the real-data level-loader tests. This does not promote the bytes to
  original tile/map/object semantics: the stage-2 MPR destination, complete
  decompression consumer, and the later runtime publication path remain
  unbound. Keep JP tile/map/object publication fail-closed until an
  authenticated JP runtime consumer capture identifies those destinations.

- [ ] THERON-V1-TRACK02-VRAM-CONSUMER: bind the real VDC BAT/tile and VCE
  palette snapshot to the source-owned square/material/UI consumer. An
  instrumented Mednafen replay now emits exact 64 KiB VRAM and 1 KiB VCE
  snapshots; the production viewport can explicitly mount that pair through
  `FIRESTAFF_THERON_VRAM_SNAPSHOT` and `FIRESTAFF_THERON_VCE_SNAPSHOT`, and
  the real-capture regression verifies non-zero BAT/tile data, 154 tile/palette
  pairs and 512 palette entries. This remains a screen-space capture binding:
  `$2600` source-LBA joins, object/level records, square-to-tile semantics,
  and production dungeon/UI admission remain blocked until the HuC6280
  consumer is disassembled and tied to Track 02. On 2026-09-25, an isolated
  Mednafen build against official SDL 2.32.10 headers/runtime passed a startup
  version check and produced authentic US CUE/state VDC traces locally. The
  corresponding VDC replay matched its captured VRAM writes but still emitted
  no authenticated CD-to-RAM consumer receipt or dungeon transition; it does
  not open the source-semantic gate. The exact VRAM/VCE/VDC-state/SAT/VDC-I/O
  bundle is now admitted by production's atomic screen-capture allowlist: the
  receipt's final sequence-65,536 bus marker matches the snapshot, replay
  matches all 9,360 written words with no mismatches, and the source-only
  renderer presents the authentic 320x200 frame through the boot facade,
  which now preserves the capture byte-for-byte instead of overlaying the
  unauthenticated legacy UI compositor. The capture regression now links the
  production viewport implementation directly (rather than satisfying those
  calls with weak no-op symbols); with the authentic capture root it passes,
  and the full Theron suite remains 68/68 with six fixture-dependent skips.
  The capture's 27,556 VDC commits, 40,980 input polls, unchanged authentic
  BRAM/code page, zero non-System-Card reads and zero game-owned `$E009`
  dispatches keep the admission strictly screen-space; no room, transition,
  UI-widget or gameplay semantics are claimed. Raw bytes and rendered
  screenshot remain local under ignored `.codex-scratch/`. Its main-RAM read
  trace also records 18 reads in `$271b..$2724` from 18 executing PCs in
  `$c2d8..$c450`, plus two reads at `$278c` and `$279f`; without the matching
  executed-code/source-sector join these remain address observations only,
  not field semantics or a loader-consumer witness.

- [ ] THERON-V1-HUC6280-RAM-CONSUMER: the real US/JP bank-$1f static support
  fragment at `$243e` is now byte-verified in both retail ISO projections.
  It proves the bounded bit/byte helper, bank-switch table and forward/reverse
  byte paths, but it is not the post-CD `$2600` RAM-loaded consumer. Capture a
  source-owned RAM instruction window around `$2400–$2800` with executing PCs
  before promoting decompression, tiles, maps, objects or HUD pixels.

- 🔧 DM2 HUD follow-up: M11 now leaves the accepted V1 runtime frame as the
  sole production HUD owner. The retired V2 compatibility blit used a static
  GDAT plan without SKProject's live GUI/session inputs, so it cannot return
  until complete per-command, party and champion-state receipts drive the
  original UI route. Diagnostic V2 HUD modules remain non-production only.

- [ ] DM2 SKSAVE runtime restoration: the corpus reader now follows the
  **2026-08-07 real possession-continuation gate:** the corpus regression now
  passes every genuinely decoded direct-root link, in source order, into the
  bounded `DM2_2066_062b` 10-bit continuation reader. The 135/135 real
  PC-DOS checks therefore cover both record-body consumption and the
  subsequent type-9/type-0xE continuation boundary. The receipt remains
  read-only; live record-pool, possession-index, timer and GAME_LOAD owners
  are still not connected.

- [ ] DM2 champion-mirror activation: the canonical PC G1 dungeon has 16
  **2026-08-13 source-bound transaction progress:** the lifecycle seam now
  exposes a source-bound `SELECT_CHAMPION` transaction that requires the
  authenticated marker identity and every live mutation owner before it can
  commit. Its callback order follows `c_hero.cpp:1052-1200` (creation-map
  switch, signed `REVIVE_PLAYER`, first-party leader, tile possession
  transfer, champion-strip refresh, map restore, weight recompute). The
  mounted PC mirror receipt now drives a positive callback-order regression;
  production GAME_LOAD/session wiring and source hero-stat ownership remain
  open, so this does not yet claim playable champion selection.

- [ ] DM2 delayed movement ownership: `PERFORM_MOVE`'s real
  **2026-08-13 delayed-owner audit:** when the exact half-step gate admits,
  the execution receipt now exposes six missing live-owner bits (hero load,
  wounds, walk speed, Aura-of-Speed, current pose and tick/countdown). The
  proven mask remains zero for caller-supplied compatibility snapshots; no
  interpolation or viewport offset is enabled.

- [ ] DM2 creature animation-frame ownership: `DM2_1c9a_0958` now carries
  **2026-08-13 0958-owner progress:** the exact DB4 cursor now also performs
  the source `DM2_query_1c9a_02c3`/`DM2_query_4E26` 0xfc read during boot
  materialization. Static AI rows retain the real `frame_bit14`, query index
  and blended value through the viewport/runtime receipts; dynamic rows retain
  an explicit CAII block. No command-0 or `0xffff` frame is promoted.

- 2026-08-06: PC-DOS startup's decoded `TITLE/0/4` surface is now named and
  receipted as an original GDAT image route, not a fallback. It remains the
  verified alternative only when `SHOW_MENU_SCREEN` has no source raw-screen
  record; generated menu text or rectangles remain forbidden.

- 2026-08-06: DM2's cross-platform CMake build now has its immediate Windows
  and macOS linkage faults corrected. Re-run the GitHub build matrix after the
  verified main push; retain the usual platform-specific test coverage.

- [ ] DM2 startup status-panel ownership: host-authored English status,
  **2026-08-13 empty-panel removal:** successful DM2 launch/resume and the
  generic DM2 launch-failure callback now return M12 to its ordinary main
  view instead of displaying a blank host message panel. The launch intent
  and structured failure receipt remain intact; M11 can therefore hand the
  next visible frame directly to the source-owned `SHOW_MENU_SCREEN` or
  dialogue path. The actual source failure dialogue producer is still open.

- [ ] DM2 runtime action/save text ownership: action, shop, movement and save
  **2026-08-13 pre-resolver correction:** DM2 quick-save and quick-load now
  enter the source-owned silent boundary before shared path resolution. This
  prevents path-length, directory and other generic host errors from leaking
  into the DM2 status channel. The original `DM2_GAME_SAVE_MENU`/GAME_LOAD
  producer is still not connected, so the item remains open.

- [ ] DM2 GDAT structure loader: `DM2_READ_GRAPHICS_STRUCTURE` remains
  **2026-08-07 underlay progress:** a source-owned materializer now resolves
  the exact `dtRaw8/0/0` ENT1 row, reads its real four-byte image-to-underlay
  table through the ULP raw-entry reader, validates source raw-index bounds
  and sorted order, and returns payload/pair hashes. The mounted PC-DOS v5
  corpus has no such source row, so its regression stays fail-closed; no
  empty or synthetic underlay table is admitted. Positive underlay-corpus
  wiring and decoded overlay/cache ownership remain gated.

- **2026-08-07 save-dungeon parity correction:** the isolated
  `DM2_STORE_EXTRA_DUNGEON_DATA` teleporter gate now matches SKProject's
  `current_map > target_map` backward-reference skip; the complete raw-dungeon
  record allocator and runtime restore owner remain gated.

- [ ] DM2 combat source contract: a creature Defense GDAT row alone cannot
  **2026-08-07 party-wound correction:** the diagnostic `DM2_ATTACK_PARTY`
  seam now applies the source `DM2_MAX(1, per_hero_damage)` clamp before
  `WOUND_PLAYER`, matching `skhero.cpp:3365-3392`; a `base_damage=1` regression
  is green. The live champion/target/RNG/writeback chain remains absent.

- [ ] DM2 FM Towns English text consumption: a selected FM Towns Japanese CD
  **2026-08-13 direct-launch parity:** `firestaff --game dm2 --fm-towns`
  now accepts `--dm2-english-companion <PC-English GRAPHICS.DAT>`, forwarding
  that explicit path through the same M12→M11 launch receipt as the menu.
  The boot layer still verifies its canonical hash and keeps it in RAM; the
  option does not broaden text-consumer admission or unpack game data.

- 2026-08-06: the full 30-file retail MNS corpus now decodes without silent
  texture/MOTN truncation (VEXIRK=64 TEXT descriptors, D_GOLD=11 MOTN
  tables). Remaining work is original Saturn/VDP1 capture and source-locked
  face/mesh texture placement; parser success is not viewport proof.

- 2026-08-06: the MNS pose/texture helper is now excluded from the production
  Nexus library because its fixed-point Taylor trig and BGR555 conversion
  have no Saturn execution/capture receipt and no production caller. The
  real-data decoder test still compiles it explicitly; restore a production
  mesh route only after VDP1/VDP2 capture proves rotation, CLUT and draw order.

- 2026-08-06: DGN Structure2 texture decode now resolves DMWeb's real
  `Palette offset = 0` reuse rule by prior Palette ID association. The
  hash-verified LEV00-LEV15 corpus decodes 1,678 descriptors (1,553 indexed4,
  125 direct555). Remaining gap is Saturn VDP1 upload/CLUT and Structure3
  face-to-texture/draw-order capture; do not promote this byte proof to pixels.

- 2026-08-06: Nexus spell lookup remains available from the real DM.BIN table,
  but `nexus_v1_cast_spell()` is now side-effect free and returns `-1` until a
  Saturn dispatcher capture binds mana commit, effect/target routing, RNG and
  SLEV/SFX publication. The previous host mana/damage mutation was synthetic.

- **NEXUS-EVENT-DGN-OWNER-CAPTURE:** Real DGN Structure1F/Structure1B bytes
  remain retained as source evidence, but the runtime no longer promotes
  apparent door/teleporter/pit/stairs records into live registries. The
  verified corpus does not prove that low DGN bits select DM1-like events,
  nor that `SDDRVS.TSK` dispatches them. Original-Saturn capture must bind
  event owner, selector order, destination fields, and state transitions.

- **NEXUS-UI-EVENT-DISPATCH-CAPTURE:** Retail `nexus_mechanics_dispatch_event()`
  now rejects host UI events for ISO/extracted data until the Saturn SLEV/SDDRVS
  producer, queue and state-write contract is captured. The source-less fixture
  lane remains available for isolated tests. Bind the original event route before
  admitting automap, inventory, save, leader, throw or drop mutations.

- **NEXUS-LEVEL-TRANSITION-CAPTURE:** The public level-transition helper now
  rejects ISO/extracted transitions until the Saturn SLEV/SDDRVS owner is
  captured; the tick gate alone was insufficient because callers could invoke
  the helper directly. Bind the original transition producer, destination
  fields and level-load timing before enabling retail level changes.

- **NEXUS-BPK-NO-DRAW-REGRESSION:** The bounded PRS3 presentation receipt must
  continue to admit exact retail-shaped rows only as opaque no-draw evidence;
  decoder drift, payload/hash drift, unknown modes and malformed spans must
  remain rejected before M11. The previously inverted matching-row assertion
  is corrected and the focused BPK/M11/Saturn-card gates are green.

- **NEXUS-WORLD-SCRIPT-CLAIM-QUARANTINE:** The linked native world/save state
  now labels its event, timer, hash and provisional action vocabulary as
  Firestaff-native/test state rather than recovered SDDRVS/SLEV semantics.
  Keep the actual SLEV task body, callback owner, event selector and dispatch
  capture-gated; do not promote the compatibility enum into Saturn opcodes.

- **NEXUS-SAVE-ROUNDTRIP-STACK:** The manager-level native save round-trip is
  now verified with heap-owned test state; keep the serialized world contract
  unchanged while extending real Saturn-card save provenance separately.

-  - 2026-08-06 Nexus PRS3 capture-schema correction: the real retail
    `MENU.BPK` MD5 admission constant was stale (`c277...`) while the
    verified corpus and boot profile use `a6f2272a4f6cb3c6b3b33012bc5b15ed`.
    Update the capture-sidecar evidence only; Saturn authentication and
    runtime texture upload remain blocked until independent VDP1 capture.

-  - 2026-08-06 Nexus production-source boundary now has a CTest verifier.
  It keeps synthetic V2 HUD/renderer modules and unproven text/MNS
  presentation paths out of `firestaff_nexus` during future source-list edits.

-  - 2026-08-06: `.github/workflows/verify.yml` now hard-runs that data-free
  production-source boundary after the cross-platform Nexus library build.
  Real retail-media and Saturn-capture tests remain local by design.
2026-08-06 regional capture follow-up: the same private CUE normalization
now accepts the archive's Japanese `TQJP02.iso` alias and binds the complete
sibling `TQJP02End.iso` only after the authenticated JP ISO MD5 matches
`397039af02d50d15c70b74088eb8a1cb`. The new generic `THERON_CUE` variable
retains `THERON_US_CUE` compatibility. A fresh JP consumer capture remains
required before semantic promotion.

- **CSB-AMIGA-LIVE-AUDIO:** M11 now transports the selected authentic Amiga
  `GRAPHICS.DAT` sample bytes through the F0709 period calculation
  (`ioa_Period = 72800 / SOUND_DATA.Period`) rather than falling back to the
  PC3.4 PIT/marker route. The remaining Amiga work is source-captured
  audio.device voice allocation, left/right volume arbitration and overlap
  behavior; do not infer those from PC3.4's distance-volume model.

- 2026-08-07: An authentic European Mednafen capture now records a 48-word
  SH-2 code window around the VDP1 source writer at runtime PC `0x06013098`
  while it writes `0x47c00`. The routine contains a real branch to
  `0x06012f52`, but relocated/decompressed code is not yet joined to an
  authenticated DM.BIN/TM.BIN source span. Keep VDP1/VDP2 composition and
  production draw admission blocked until that identity and command/CLUT
  contract are proven.

- 2026-08-07: The authentic high-RAM load trace shows 3,080 writes into the
  `0x06013000..0x06013fff` code corridor from runtime loader PC `0x2368`.
  This is a BIOS/runtime-loader receipt only; the trace does not yet expose
  the CD source read or identify the retail member that supplied the bytes.
  Keep the VDP1 source join and production composition blocked.

- 2026-08-07: The Saturn-CDB hook now traces the real `cdb.cpp` data-sector
  path. The current bounded run reaches only BIOS LBA `0..16` (1,024 reads);
  no `DM.BIN`/`TM.BIN`/other retail member has been joined yet. Continue with
  a capture route that reaches the authenticated game startup window; do not
  promote the BIOS sector receipt to SLEV/SAL or VDP1 source evidence.

- 2026-08-07: The corrected input ordering now reaches the authenticated
  French Nexus startup window. A 50,000-read CDB trace joins `DM.BIN`,
  `TM.BIN`, `ITEM.IBS`, `MENU.BPK`, `SLEV00.BIN`, `SDDRVS.TSK`, DGN and SAL
  spans to the retail ISO; the new analyzer reports this as LBA provenance
  only. The same run records 3,080 runtime-loader writes and one raw frame,
  but no VDP1 writer trace. Keep PRS3 pixel consumers, VDP1/VDP2 composition,
  HUD/viewport, SLEV/SAL/SDDRVS semantics and SFX playback blocked pending a
  live producer/consumer join to those authenticated bytes.

- **DM2 SKSAVE direct-root pool ownership:** The raw DB baseline and DB4–DB15
  clear phase are now followed by source `READ_RECORD_CHECKCODE` allocation
  into the authenticated c_record pools, including source list links,
  child-owner fields, type-9/type-0xE continuation writes, and a hash/count
  receipt. Remaining work is attaching the returned roots to champion/hand,
  possession-index and tile-chain owners; failed decode restores the cleared
  baseline and never publishes a session. The mounted workspace has no raw
  SKSAVE corpus, so this positive path remains compile/test-gated until one is
  supplied.

- **THERON-RNG-RETURN-OWNER:** The external-disk `.mc0` replay now captures a
  declared 4,096-step `$5D64` execution window, but the state reaches no
  `$4667` helper or game-owned CD→RAM join and exposes `return_pc=0001` rather
  than an authenticated caller return. Keep RNG, spawn, creature AI, loot,
  generator, T700 and T900 admission closed. The next required witness is one
  same-session state or live replay that joins `$4667` → `$5D6A/$5D64` → return
  value to the authenticated Track 02 payload and consumer.
# 2026-08-10 — source roster/stat handoff is fixed

- Completed: optional US roster text no longer blocks the source-owned
  champion stats/skills handoff at forcefield entry.
- Remaining: authenticate the US text consumer and T900 equipment semantics.
# 2026-08-10 — source group bounds fixed

- Completed: category-4 live-creature admission now applies the same
  four-member source bound in its counting and materialization passes.
- Remaining: authenticate dynamic RNG, AI, T700 and T900 consumers.

# 2026-08-20 — Atomic VDC bundle exists; source join remains open

- Completed: an authentic US disc, System Card 3.0, dungeon save state, and a
  clean Mednafen 1.32.1 build against real SDL2 produced 65,536 sequenced
  VDC writes and same-instant VRAM, VCE, VDC-register, and SAT snapshots.
  The sidecar ends with the verifiable boundary
  `vdc_snapshot_boundary sequence=65536`.
- Remaining: the session was loaded from a save state and produced no
  authenticated game-owned CD→RAM receipt. Join the same VDC boundary to a
  real Track 02 consumer before replay can mutate the production viewport.
- Completed: a separate cold US start produced same-session Track 02
  transport, 32 main-RAM `$E009` dispatches, and an exact atomic 24,576-word
  VDC replay. The native viewport now requires five files and rejects the
  older four-file bundle.
- Remaining: the two byte receipts are still owned by the System Card
  routine at physical `$1F01E7` and have zero provenance copies. Bind an
  actual game-owned `$E009` destination block to its Track 02 sectors and
  later VDC consumer.

# 2026-08-13 — Theron-verifier tests respect external TMPDIR

- Completed: capture-manifest, HuC6280 event-log, SRM-classifier and rendering
  fixtures now place their temporary files below `TMPDIR` when it is set.
  This allows the focused Theron verification set to run on the external disk
  when the macOS system volume is full, without changing runtime paths or
  promoting synthetic rendering.
- Remaining: the full suite still requires complete authenticated runtime
  capture inputs beyond the available System Card and media, and the semantic
  text, square/material, RNG/AI/loot and T700/T900 consumers remain closed
  until their source/runtime joins are proven.

## First game-owned E009 consumer (2026-08-20)

The authentic cold US run now binds Track 02 record `$4E0` to main RAM
`$2800` and then to five ordered reads of block offsets `$513..$517` before
the next `$3840 → $E009` dispatch. This closes the previous gap between the
first completed payload and a real game-code consumer.

Code `$37C8..$383F` is now source-bound to Track 02 records `$4C4/$4C5`.
The proven address relation is `$2803 + 6 × $D8 = $2D13`; the routine then
reads five bytes, and the next `$3840 → $E009` dispatch is observed.

Storage for the five output values and the raw parameters of the subsequent
`$3840` call are now bound to the same code execution. Parameter block
`00 20 00 10 00 06 F8 FE` is also bound to READ(6) generation 6, LBA
5018–5021, and 8,192 byte-exact VDC port writes from Track 02 records
`$7D9..$7DC`. A new snapshot exactly at the end of the generation shows the
real VDC effect: each 16-bit source word is written twice, filling 8,192 VRAM
words `$1000..$2FFF`; the snapshot FNV is `9B9F7361`. An authentic negative
control shows that the first call grammar must not be reused: `$FA/$FB =
$1000` maps through MPR `$FF` to physical I/O space `$1FF000`, not main RAM,
so an 8 KiB RAM hash there is invalid as payload evidence. Generation 7's
BAT builder, VCE palette, and enabled background are also bound: all 960
active 32×30 cells use 60 unique source-bound tile indices, and the decoded
256×240 indexed image contains 2,848 nontransparent pixels. Palette group 0
is byte-verified against the same snapshot. The executed code is now joined
to statically byte-bound Stage 2 routine `$466B`: its self-modifying `TIA`
transfers the generated row at `$47E0` to the VDC VWR port `$0002`. The next
12-sector load is also byte-bound: generation 49 reads LBAs 4622–4633, Track
02 `$64D..$658`, and sends all 24,576 bytes to the VDC in order. Generation
51 is then a continuous `$50F1/$5111` drawing loop. An atomic snapshot at the
first complete 1,035-entry frame boundary still shows a transition image and
zero SAT sprite pixels. The next gate is therefore to bind the caller to
`$50F1/$5111`, its command table, and the frame phase where the loop changes
to the following menu or gameplay image before the parameters or payload are
given any gameplay meaning. The values `F9 02 04 00 20` must not be named as
coordinates, record fields, graphics, objects, or dungeon data until the
semantic chain is proven. Invalid provenance fields in the sidecar must not
be used as byte origins; the binding relies on the destination's already
hash-verified 2,048-byte block and byte comparison against the same
authenticated raw sector.

# 2026-08-13 — fresh System Card replay confirms transport-only boundary

- Completed: a new local replay with hash-verified US Track 02
  (`f23601102138f87c33025877767ebf76`), real System Card 3.0 and instrumented
  Mednafen ran from the external disk. It produced 161 raw sectors, 51 SCSI
  commands, 25 CD IRQ callbacks, 161 sector bindings, 47 byte-exact FIFO-to-RAM
  receipts and 65,536 VDC-I/O writes. `verify_theron_origin_ram_receipt.pl`
  passes all 47 receipts.
- Remaining: the same session has no game-owned FIFO-to-RAM receipt, spawn-
  consumer reads or RNG windows. It therefore does not open dungeon-consumer,
  square/material, RNG, AI, loot, T700 or T900 semantics. Raw output remains
  local at `/Volumes/Extern-disk/theron-capture-20260813/replay/` and is not
  pushed.

# 2026-08-14 — RAM provenance probe remains negative

- Completed: a capture-only provenance hook was built against the original
  Mednafen 1.32.1 source and linked against the real SDL2 runtime. The hook
  carries authenticated CD-origin bytes through CPU RAM reads/writes without
  changing emulator or game behavior. The patch dry-run, shell checks and
  runtime-linkage verifier pass.
- Verified: the 120-second authentic US replay
  (`run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30`) produced 166
  CD→RAM-origin seeds and 0 provenance copies. No copy reached `$2935`,
  `$293E` or `$611D`; the source→RAM→record mutation join is therefore still
  absent. Keep `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`, gameplay, square,
  material, RNG, AI, loot, T700 and T900 admission closed. The raw capture is
  local and is not pushed.
## Theron gameplay ADPCM event correlation (2026-08-20)

The capture now instruments the original `$180D` ADPCM control and `$180E`
playback rate with the logical and MPR-derived physical HuC6280 PC, as well as
address, length, frequency, and start transition. The next real-data run
should trigger one isolated, source-identifiable door, pickup, or attack event
in an authentic dungeon save state and bind the same event window to a real
playback start. No `Theron_SoundID` may be enabled until the event, PC,
ADPCM-RAM range, and sample byte are present in the same original session.

The first authentic run is now complete. Cold startup loaded 2,048 bytes from
Track 02 LBA 4719 and produced 140 `$180D/$180E` records, but all were System
Card bank configuration and none had `playback_start=1`. The authentic
save-state replay produced zero control records. The remaining work is
therefore a new original session that reaches an isolated gameplay event,
not further interpretation of bank loading. See
`docs/source-lock/theron-adpcm-playback-capture-2026-08-20.md`.

## Atomic dungeon input and RAM boundary (2026-08-21)

- ✅ VDC tracing can now be selected between 65,536 and 2,097,152 real CPU
  port writes. The reader accepts the old boundary and the new upper boundary
  without assigning semantics to the length. Baseline, RIGHT, and LEFT
  captures continue to replay 8,816 and 8,784 written VRAM words,
  respectively, with zero mismatches.
- ✅ The same snapshot can now also contain the original full 8 KiB main RAM.
  The capture script requires exactly 8,192 bytes when the producer is
  enabled; no synthetic RAM image or host fallback is created.
- ✅ An authentic control pair from save state
  `f17f377df210b4a3ae904a13fb85a7f0` reads RIGHT `$0020`, DOWN `$0040`, and I
  `$0001` through the original input port. The click run and control have
  identical VRAM, VCE, VDC, and SAT images at sequence 262,144 but different
  RAM images (`8a635d6d31631a2ac60778525b5ae603` vs.
  `bdfee2baab717b3e3f13ca290284c116`), with 14 differing bytes.
- ✅ A later click/control pair from the exact same original disc and save
  state differs by 27,430 pixels. Button I is observed as `$28B8=$01` from
  original PC `$44E5`. In the same run, global direction `$203F` changes
  from `1` to `2`; captured original routine `$D900..$D92E` computes the
  direction difference modulo four and updates party fields `$2944/$2948`
  from `1` to `2`. The event is therefore verified as a quarter-turn, not
  the previously assumed forward step.
- ✅ The opposite click capture queues command type `$01` at `$7B/$8F` and
  binds `$203F`, `$2944`, and `$2948` from `1` to `0`. A long click/control
  pair differs by 27,226 presented pixels. Type `$01` is therefore left and
  type `$02` right; Firestaff runtime now uses these original commands rather
  than standalone host deltas.
- 🔒 Position and movement bytes remain unpublished until their own original
  writers are captured.
- 🔒 A longer CPU-port replay crosses the VDC's internal DMA and therefore
  cannot by itself reproduce the full VRAM image. The longer capture is not
  added to the production list of atomically approved screens until the DMA
  itself is traced or an earlier clean boundary is verified. The visible
  2,097,152-entry image is therefore analysis evidence, not production-
  approved.

## 2026-08-20 — File-select text source bound; screen consumer still open

- ✅ All three real US Track 02 copies of the PLAY/LOAD prompt are now bound
  to exact MODE1/2352 coordinates and byte hashes; modified real media is
  rejected.
- 🔒 The visually observed file-selection screen is not sufficient evidence
  for a text consumer. The next focused capture should log CPU reads from the
  `$4EA/$4EC/$4EE` payload to the text/VDC routine in the same session.
  Generation 51's `$64D..$658` reads are graphics transport and must not be
  misattributed to prompt text.
- 🔒 A focused RAM scan after Button I confirms that plaintext is absent from
  runtime RAM. The next capture should sample real data operands per frame
  after the `$4698/$511B` bulk phase and bind the encoded glyph stream to its
  Track 02/Track 19 source and VDC writer. `$3C2A..$3D2B` is only an observed
  candidate loop until source bytes and presented glyphs are joined.
- 🔒 The stable file-selection phase and its VDC consumer are now located:
  `$5561..$55D0` reads physical bank `$0D1D58..`, and `$5110` writes the VDC
  port stream in the same `frame 8580`. The next gate is to instrument the
  write/load that first fills `$0D1D58..`, bind it to the exact real Track
  02/Track 19 record, and then verify that the resulting VDC/BAT reference
  presents the prompt glyphs. None of these bytes may be called text or
  glyphs until the media chain and a negative real-data test are available.
- ✅ Media→RAM→VDC transport for record `$67B` is single-write verified. The
  512 VDC port bytes are bound through MAWR `$0800` to VRAM staging
  `$0800..$08FF`, and an atomic image shows that the staging area is identical
  to the VDC's internal SAT. The area is not a BAT-referenced pattern block.
- 🔒 The meaning of the 195 source bytes and 18 SAT entries remains closed.
  The SAT entries are bound to sprite pattern `$108..$10F`, and the exact
  256×240 image is composed with the authentic VCE palette. The sprite plane
  yields 24,576 black source pixels in two bands, but geometry alone does not
  prove what those bands mean. Comparing frames 8579 and 8581 now shows that
  the bands remain completely still while game code `$4993/$4999/$499F`
  reduces BYR from `$00E9` to `$00E8`; 12,644 pixels change only in the
  unmasked center. The control chain is now bound: game code `$4993` reads BYR
  from `$2210/$2211`, the loop at `$4175` reduces `$2210` from `$F0` to `$60`
  in 144 steps, and `$47BC/$47BD` counts `$0090` down to zero. The next
  authentic control phase is also bounded: `$47BA/$47BB` counts
  `$0040→$0000` in 64 ten-frame steps at `$4B1B/$4B20` between frames 10632
  and 11274, before reset at 11578. The next gate is to capture the stable
  surface after each stop and bind its presented BAT/pattern content to the
  same run. The bands and 195 source bytes still must not be called a
  file-selection screen, prompt, or any other screen content without that
  link.
- ✅ Generations 6 and 7 have now been recaptured with the new single-write
  build chain and verified by separate negative real-data tests. Presentation
  is bound only after 2,187 generation-7 rows; 2,151 is only the BAT end and
  must not be used as the screen boundary.
- ✅ Generations 49 and 51 have been recaptured with the new single-write
  build chain. Their bindings and negative real-data tests now use 24,580
  and 54,842 rows, respectively. The old row counts 49,160 and 100,755 are
  revoked.
- 🔒 The next graphics gate is to bind the verified VRAM, VCE, and SAT
  contents of the generation-51 frame to concrete file-selection or dungeon
  objects. Transport and frame evidence do not independently enable screen,
  tile, or object semantics.

## Dungeon-specific object properties (2026-08-20)

- ✅ The live Track 02 world now uses all seven authentic regional name,
  type-code, and property banks and rejects global fallback.
- 🔒 The older compatibility combat's compact champion-slot ID lacks the
  object's origin dungeon. The entire compatibility combat path is therefore
  fixture-only; production runtime returns closed and does not link the static
  66-row catalog. Combat and equipment are enabled only after the original
  T600/T900 consumer and equipment ownership are source-bound.
- ✅ The production archive no longer contains the compatibility combat's
  static action costs or unused US plaintext catalogs. Their fixtures must
  not be used as runtime fallbacks; the next text or combat consumer must
  read authenticated regional media.
- ✅ The production archive has also been cleared of twelve additional
  static fixture catalogs with no runtime consumer. This enables no new
  semantics; if text, glyphs, class data, or action parameters are later
  needed, they must be obtained from verified regional media and bound to a
  real consumer.
- ✅ The champions' numeric roster is now source-bound regionally in both US
  and JP. US uses its authentic packed 5-bit bank and JP its authentic
  A–P bank; production startup no longer has a static roster fallback.
- 🔒 Starting equipment, portraits, and title control-code interpretation
  remain separate original consumers. DMWeb equipment remains fixture-only
  until Track 02's real object ownership and T900 handoff are decoded.
## Quest-artifact presentation after authentic name binding

- The seven regional raw names are now bound from real Track 02 media.
- ✅ Startup chapter inspection and layout now surface US bytes from the
  world-owned source bank rather than a compiled label table.
- ✅ The host chapter marker strictly converts hash-verified JP quest names
  from Shift-JIS/CP932 to UTF-8 using the shared rejecting decoder; all seven
  authentic JP Track 02 names are tested. This is a host text projection only,
  not evidence for the game's original VDC glyph/rendering behavior.
- ✅ The chapter-marker API accepts the live world and keeps the production
  `source name unavailable` result whenever the relevant bank or safe rendering
  path is absent.
- 🔒 Do not treat the seven name-table indices as unique gameplay identities.
  Real US/JP thing-table censuses show zero, one or many ordinary carryable
  records at those indices, depending on dungeon and region. Recover the
  original retrieval-event consumer before enabling native quest completion;
  the seven low bits at `$267C` are campaign/dungeon state and have not been
  proven to represent collected artifacts.
- ✅ The seven US and seven JP retrieval-message records are hash-verified,
  regionally framed raw sources. Their post-dungeon selector is now also
  authenticated through text group 2, its `$00CA` command and the regional
  message-list offsets `$013D/$016D`; the decoder may therefore publish the
  original retrieval-event relation. 🔒 Host-side text rendering remains off.
- ✅ The authentic US/JP post-dungeon chain is now traced through the shared
  17-sector program. Initialization calls `$8243`, which copies the original
  selection and dungeon ordinal from `$2700/$2701` into the parameter block
  at `$2780`; a later shared routine then copies `$2781..$2787` into its local
  seven-byte table. Both regional program hashes and both copy instructions
  are production-gated. This proves
  ordinal transport.
- ✅ The subsequent ordinal dispatch is now also authenticated separately
  for US and JP. Seven comparisons select seven 32-byte blocks; each block
  executes text command `2B 02 n`, selecting group `2` and using `n` to
  advance the corresponding number of entries in the text engine. The text
  group's internal pointer leads directly to the seven messages at `$40C/$40D`,
  closing the discovery relation.
- ✅ The same chain now also authenticates the original routine that sets
  the CD base to track 19 and the shared 2 KiB loader helper in both US and
  JP. The helper builds the sector number from the descriptor's first two
  bytes, reads target type, target address, and sector count from the next
  fields, then calls `$E009`. World binding requires both the program hash
  and the exact call sequences. 🔒 This shows how a resource is loaded, but
  not yet which descriptor is selected by the forwarded ordinal.
- ✅ The dungeon-local transition now also binds the complete original
  sector calculation: record `$3C7 + 4 × ordinal` is read as four sectors to
  `$4000`. Each such program then reads two support sectors from `$3E3` and
  replaces itself with the shared 17-sector program from `$3E7`. All tables,
  BIOS calls, and US/JP program hashes are included in the gate.
  🔒 The retrieval resource `$40C/$40D` occurs later in the flow and is
  the same record as the ordinal formula above.
- ✅ The verified regional message bank is bound to the live world on every
  real dungeon load. Separate raw-record access can read exactly records 0–6
  and now requires the proven original relation. 🔒 Host rendering of the
  regional control codes remains disabled.
- ✅ The original seven-bit campaign/dungeon status is now located at
  HuC6280 RAM `$267C`. The dungeon routine first takes an ordinal 0–6, looks
  up `01 02 04 08 10 20 40`, merges the bit with `ORA $267C`, then loads the
  next code resource before jumping to `$4000`. The byte-identical shared
  US/JP routine preserves bit 7 and serializes the same byte. All code
  windows are hash-bound, and negative real-data tests mutate both shared
  code and one dungeon-local bit. The live world now cross-binds this writer
  with the authenticated message selector before publishing the seven low
  bits as discovery status; bit 7 is not part of the discovery mask.
- ✅ The real PC Engine Backup RAM container is now bound to the same byte.
  The original routine reads `$86` bytes from `DMS-SG.001` into `$267C`; the
  HUBM record body starts at file offset `$20`, so offset `$20` is the
  serialized campaign byte. The live world can restore the discovery mask
  and completion status from this byte without replacing the current
  dungeon, level, game time, or seeds.
  disc layout. `$FC:$FD:$FE = $0003C7 + 4×ordinal`, `$F8=4`, `$FF=1`, and
  `$FA:$FB=$4000` load four sectors locally and start them at `$4000`.
  INDEX 01 follows 225 raw sectors in US and 224 in JP, binding the seven
  regional programs to raw record `$4A8/$4A7 + 4×ordinal`. Each program writes
  the ordinal to `$2701`; dungeons 1–6 write `$09` to `$2700`, and the final
  dungeon writes `$0F`. The subsequent consumer and correct regional
  discovery message are now cross-bound by the chain above.
- ✅ The original four movement commands `$03..$06` are byte-bound from the
  queue at `$D3B0..$D3CB`, through movement routine `$CD87`, to the position
  commit `TII $20B4,$2040,$0002` at `$C1FA`. Firestaff's up-input path now
  uses these original command types with the loaded Track 02 world's real
  tile and object data. `$05` is authenticated as backward and commits
  `$02/$03→$01/$03`. `$04/$06` are right/left, and both captures prove the
  blocking path without a write to permanent position. The A/D host path now
  reaches `$06/$04` instead of the older rotation tokens that made side steps
  inaccessible. The US/JP real-data test verifies all four relative commands
  against both real floor traversal and real wall blocking.
- ✅ The real-data test no longer creates a synthetic level 1, door, pit, or
  stair after the Hall-of-Records check. The same authentic raw BIN is
  normalized through the verified MODE1 bridge, all of AKUTUBA is loaded,
  and every real level header is verified. Original command `$03` then takes
  an actual Track 02 stair to the loaded destination level in both US and JP.
- ✅ The same real-data test no longer fills the world with four fabricated
  champions, arbitrary values of 50, or 1,000 gold. Movement, wall, and stair
  evidence uses only the authentic map and source starting pose; regional
  roster and save status remain separate real-data consumers.
- ✅ Dungeon-map source tests now verify US and JP Track 02 identity against
  the published raw-BIN hashes before counting stair-family bytes across all
  seven dungeons: 171 US and 170 JP. Candidate coordinates are printed as
  raw source information for future capture targets; the test does not
  attribute direction or destination-level semantics. Without US source
  data, it reports a CTest skip instead of a false pass.
- ✅ The separate door/teleporter test now binds both US and JP files to
  their published raw-BIN hashes before loading all seven dungeon tables.
  Both regions verify the same door/teleporter counts; missing US media
  results in a CTest skip, and `assert()` checks remain active in Release.
  This verifies source records and decoding, not uncaptured button, key, or
  teleporter events.
- ✅ Older Track 02 tests for items, dungeon maps/objects, text, actuators,
  and ground references now use CTest skip status when their required real BIN
  is missing or cannot be normalized. Their `assert()`-based source checks
  also remain enabled in Release/NDEBUG builds.

## 2026-09-25 — Cold start replayed with full controller trace

- ✅ The same authentic US CUE, Track 02, System Card 3.0, and unchanged BRAM
  were replayed with RUN at frame 9600 and a controller-trace limit of 262144
  reads plus 262144 writes. The RUN mask was applied, but no `$1000` CPU read
  results with `raw=0008` occurred in this late run. The BRAM hash is
  identical before and after.
- 🔒 The result remains negative: 25 raw sectors in four SCSI reads, zero
  game-owned CD→RAM receipts, zero authenticated CD→RAM destinations, zero
  `$E009` data reads, and `$20DB=00`. The raw cold start does not reach the
  previously signed Drator menu code. Increasing the log limit fixed
  truncation but not the transition. The earlier plan
  `run@1:1,run@480:30,i@900:30` does reach the BIOS: CPU reads observe
  RUN=`0008` and I=`0001`. However, both the raw MODE1/2352 run and a
  controlled MODE1/2048 run produced the same 25 sectors and no CD→RAM
  receipts. The next investigation should follow the BIOS/CD command path
  after input reads and compare it with a positive host-input session. No
  semantics may be enabled from these negative runs.

## 2026-09-25 — authentic empty JP Backup RAM stays out of Continue

- ✅ The user's real JP Mednafen SRAM (`MD5 dbdedb0ec809227b289c2bc5b18b9c9d`)
  is a structurally valid 2 KiB HUBM / `DMS-SG.001` record, but the selected
  slot's campaign byte is zero. The original US and JP restore routine returns
  immediately for zero, then rejects masked campaign values `>= 7`. The save
  classifier now reports this gameplay boundary separately from container
  layout. Production startup no longer advertises the empty slot, explicit
  invalid BRAM paths do not fall back to another save, and campaign/party
  restore rejects the empty slot without changing world state.
- ✅ `theron_v1_pce_bram_real_artifact` passes using both the authentic
  Akutuba-complete US BRAM and the hash-verified empty JP save. The test keeps
  the JP original outside Git and symlinks it into a temporary `.bram` path so
  the production Continue route is exercised against the exact source bytes.

## 2026-09-26 — capture-only CDDA command trace

- ✅ Added a Mednafen capture patch for accepted PC Engine CDDA play, MSF play,
  NEC pause and end-position commands. Each record keeps the
  original LBA interval, Mednafen clock timestamp, command state and mode; it
  does not choose tracks or change Firestaff playback behavior. The complete
  patch sequence applies to an isolated copy of the available Mednafen 1.32.1
  source in patch-only mode and the complete instrumented binary builds on
  trv2. A first real-media capture exposed that command-buffer position is
  reset before the status callback; the trace now reads the completed command
  at that status boundary instead of testing the already-reset position.
- 🔒 No positive Theron gameplay CDDA command has been captured yet. Existing
  authenticated traces show System Card loading/menu activity, not a
  game-owned track selection. A cold-start capture with the authentic JP
  MODE1/2352 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card
  3.0 (MD5 `ff1a674273fe3540ccef576376407d1d`) recorded 24 raw-sector spans and
  4 SCSI read commands, but no CD->RAM origin receipt; the production capture
  correctly remained blocked. After fixing the trace boundary, a fresh build
  and capture had valid provenance but reached only one CD IRQ and no raw
  sectors, CDDA commands, or game transition. Do not infer gameplay track
  selection from either run. Further capture needs a verified real resume
  state or source-supported input path; preserve fail-closed transition checks
  and bind any accepted LBA interval to the authenticated disc TOC before
  adding playback behavior. A 2026-09-26 authentic JP CUE replay then exercised
  the instrumented controller script (`run@1`, `run@500`, `ii@700`,
  `up@800`, `up@900`); all five input events were observed, but the run again
  stopped at 24 raw-sector spans, four SCSI reads, one `$E009` dispatch, zero
  `$E009` data reads, zero CD-to-RAM receipts and `transition=missing`. This
  confirms input delivery only, not an in-game response. The authentic CUE,
  Track 02 and System Card were unchanged; capture artifacts remain in the
  isolated trv2 work root and are not tracked. A second JP replay used the
  Mednafen title-wait input, which is gated on the original opcode signature;
  that signature never fired for this JP run, and the capture ended with the
  same negative transfer counts. The title-wait hook is therefore not yet a
  JP input route. Do not modify another agent's shared checkout.
- A longer authentic JP replay then scheduled RUN at frame 9600 and ran for
  230 seconds against the unchanged Rev. 1 CUE and System Card. Mednafen ended
  at its configured timeout (`exit=124`) after 524,288 controller transactions;
  the sidecar records one scripted event applied at frame 9600 and the PCE
  controller-port read `$1000=$0008`. This confirms hardware-level input
  delivery, not a game-owned response. The receipt also reports zero host-key
  events, 115 CD IRQs, 24 raw-sector spans, one game-owned `$E009`
  dispatch/entry, zero `$E009` data reads and zero authenticated CD-to-RAM
  receipts; no dungeon transition was published. The isolated output BRAM has
  MD5 `2c063b192787ac9e7528c0e2096fc034`, distinct from the untouched JP user
  save (`dbdedb0ec809227b289c2bc5b18b9c9d`). Raw capture artifacts remain
  outside Git. The run does not change the earlier negative gameplay result.
- ✅ A 2026-09-29 capture-helper audit found and fixed two launch-contract
  defects: the CUE path was not a separate Mednafen argument, and the locally
  supplied SysCard3 image included a 512-byte copier header. The capture now
  keeps the source MD5 `ff1a674273fe3540ccef576376407d1d`, strips the header
  only in its private capture home, requires runtime-image MD5
  `38179df8f4ac870017db21ebcbf53114`, and records both values in the receipt.
  The C helper parser and capture-script checks pass on trv2. A fresh authentic
  JP Rev. 1 capture sent RUN at 25 seconds and the input trace records PCE port
  value `$0008`; its receipt verifies both BIOS hashes and the JP Track 02 MD5.
  After 125 seconds the instrumented run still ended with only 24 raw sectors,
  four SCSI reads, 115 CD IRQs, one `$E009` dispatch/entry, zero `$E009` data
  reads and zero authenticated CD-to-RAM receipts. Screens at 60/90/110
  seconds remain on `SUPER SYSTEM CARD needed` / old-CD-ROM2 warnings, not game
  artwork. This proves input reached the controller port but does not establish
  a playable session, stair transaction, or Theron parity. The private trace
  remains on trv2 outside Git. A clean-home control using stock Mednafen 1.32.1,
  the same CUE and the same normalized BIOS displayed `SUPER CD-ROM SYSTEM VER.
  3.00`; with RUN at the observed 25-second prompt, it reached authentic
  opening artwork by 55 seconds. `strace` confirms the instrumented binary also
  opened the exact normalized BIOS path, so its startup divergence is not
  explained by a missing BIOS file. The runs are not frame-aligned, but the
  instrumented binary is not yet a trustworthy gameplay oracle. Do not use its
  negative CD-to-RAM result as parity evidence until this source/runtime
  discrepancy is resolved.
- ✅ The live capture now also admits the authentic US CloneCD single-BIN CUE
  without rewriting its media: it bounds Track 02 from CUE `INDEX 01` sector
  3234 to the next track at sector 6605, extracts only that range in its
  disposable capture home, and verifies MD5
  `168bd6a63784e91885df8c47be62ab5a` before Mednafen starts. A real US capture
  on trv2 recorded that Track 02 identity, authentic System Card MD5
  `ff1a674273fe3540ccef576376407d1d`, 25 raw-sector spans, four SCSI reads and
  115 CD IRQ callbacks. It still produced no authenticated CD-to-RAM receipt,
  no `$E009` data reads, and no dungeon transition; no gameplay CDDA command
  was observed. Preserve the fail-closed boundary; this capture proves the
  real CUE/BIN intake and sector provenance only, not gameplay progression.
  Trace artifacts remain private on trv2 under
  `firestaff-theron-evidence/capture/us-clonecd-cdda-command-range-20260926-0826.trace*`.
- 🔒 A follow-up loaded the authentic post-Akutuba Mednafen state
  (`f17f377df210b4a3ae904a13fb85a7f0`) with the same US CloneCD CUE and replayed
  `i, up, run, i, ii, left, right`. The state-load receipt matches its source
  hash, but the session produced only one CD IRQ and zero command-RAM or
  command-input-buffer writes. This replay did not deliver a gameplay command
  and therefore adds no negative evidence about gameplay CDDA. Its trace bundle
  remains private on trv2 under
  `firestaff-theron-evidence/capture/us-clonecd-authentic-state-cdda-20260926.trace*`.
- 🔒 2026-09-26 longer cold-start replay: the authentic US CloneCD CUE and
  System Card were replayed with six 60-frame PCE button holds over 65 seconds.
  Mednafen recorded all six scheduled inputs, 25 Track 02 raw-sector bindings,
  and one `$E009` entry/return, but zero reads from `$E009`'s data window, zero
  authenticated CD-to-RAM receipts, and zero CDDA commands. Only four input
  buffer writes were observed (two reset writes at `$CB22`, two writes at
  `$EA9E`); they do not establish gameplay commands or data provenance. The
  capture remains blocked and fail-closed; its trace bundle is isolated on
  trv2 under `firestaff-theron-evidence/capture/attempt-long-input-20260926/`.
- 🔒 Follow-up distinguishes host-input failure from game progression: a
  65-second replay scheduled authentic PCE `RUN` holds at polls 1, 600, and
  1200, while enabling the signature-gated Drator research route. The input
  trace confirms all three events were delivered, but no source-signature
  route event fired; the receipt still reports 25 raw-sector bindings, one
  `$E009` entry/return, zero `$E009` data-window reads, zero authenticated
  CD-to-RAM receipts, and zero CDDA commands. A separate host-key attempt was
  rejected before launch because host-key capture requires macOS accessibility
  input and an explicit PCE profile; it is not evidence about the game. Do not
  repeat the same cold-start route without new evidence about System Card
  startup or a source-proven in-game launch path. The replay trace bundle is
  isolated on trv2 under
  `firestaff-theron-evidence/capture/attempt-long-input-20260926/`.
- ✅ The existing real-media startup/runtime regressions also pass on trv2
  against the staged originals: US CloneCD ZIP (direct boot, original/modern
  cards, mouse-only cards and six movement inputs through native dungeon
  startup), US CloneCD raw CUE Track 02, Japanese Rev. 1 CUE, Japanese later
  dungeon runtime, and Japanese raw Track 02 startup. These prove the listed
  authentic intake/startup routes, not CD-origin parity for savestate gameplay
  or completion of every Theron mechanic.
- ✅ Full inventory scanning now also passes strict Track 02 media intake on
  CUE-backed packages after the generic filename/hash pass. It publishes the
  original payload path and edition hash into the corresponding Theron row
  without replacing another edition or unpacking the source. The collector
  now checks every bounded CUE candidate below each scan root, rather than
  letting one earlier edition hide another in the same directory tree. A
  real-media regression checks an authentic CUE against the staged source when
  `FIRESTAFF_THERON_CUE` is configured; the ordinary local run skips this
  assertion when no authentic CUE is staged.
- ✅ Ran that regression with the original Japanese Rev. 1 cue and its Track 01
  and Track 02 extracted unchanged from the supplied 7z into a temporary test
  directory. The real Track 02 MD5 is `b7afb338ad31be1025b53f9aff12d73a`, and
  the full asset-status scan passed; the source archive and user data tree were
  left untouched.
- ✅ Independently validated the existing Track 01 handoff against the
  authentic full Japanese 7z disc: its CUE declares 19 tracks, Track 01 is
  7,916,832 bytes of AUDIO, and Track 02 is MODE1/2352 at 8,102,640 bytes.
  The archive's Track 02 SHA-256 equals the supplied `TQJP02.bin`
  (`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`).
  `FIRESTAFF_THERON_CUE=<authentic Japanese CUE> ctest --test-dir build -R
  '^theron_v1_track01_cdda_handoff$' --output-on-failure` passes, including
  actual CDDA stream startup. This proves the authentic JP Track 01 handoff,
  not which gameplay tracks the original executes.
- ✅ The equivalent US original 7z disc also passes the same real-media test.
  Its CUE declares the authentic Track 01 AUDIO and Track 02 MODE1/2352 pair;
  the archive Track 02 is 8,104,992 bytes and its SHA-256 matches the supplied
  `TQUS02.bin` (`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`).
  This proves Track 01 startup for both supplied editions,
  but still does not establish any gameplay CDDA selection.
- ✅ Added an archive-backed CTest that extracts the authentic US and JP CUE,
  Track 01, and Track 02 members from their supplied 7z discs into a temporary
  build-local directory. It verifies each original Track 02 BIN against its
  edition hash, calls the production verified-media CDDA handoff, and starts
  the file-backed production audio lifecycle. On trv2,
  `ctest --test-dir build -R '^theron_v1_track01_cdda_authentic_archive$'
  --output-on-failure` passed for both editions. This verifies title Track 01
  raw CDDA startup only; gameplay music routing and archive-native runtime
  reads remain separate parity requirements.
- 🔒 Runtime archive gap confirmed for both JP and US: booting directly from
  either supplied `.7z` with external archive tools enabled accepts its
  authentic Track 02, but reports `theronTrack01CddaReady=0`; the user's
  ordinary data directory has the same result. Booting from the authentic
  full-disc files in an isolated extracted copy reports
  `theronTrack01CddaReady=1`. The bounded archive bridge below resolves this
  gap for the supplied combined RAR's title Track 01 audio only; this does not
  establish gameplay track selection or support for other archive formats.
- 🔒 The local combined US/JP RAR listing contains source-named `TQUS.cue` and
  `TQJP.cue`, US/JP Track 01 OGG members, and OGG members for tracks 03–18.
  This establishes that the archive advertises original per-track audio, not
  that every title/gameplay command selects those tracks. The installed 7zz
  can list the RAR but reports `Unsupported Method` for every listed member,
  including both CUE files; `unrar` is also installed and its real-media
  `theron_v1_combined_rar_cue_handoff` test passes. The no-extraction
  `theron_v1_combined_rar_direct_boot` test passes for both authentic regions,
  verifies the selected Track 02 digests and reports
  `theronTrack01CddaReady=1`. The production bridge reads only the selected
  regional CUE and its exact same-stem OGG member into bounded memory,
  authenticates the OGG SHA-256 and decodes it through the existing Vorbis
  stream. The real-media CDDA regression independently verifies both archive
  OGG hashes and requires decoded sectors to be queued. This proves the title
  Track 01 stream, not any gameplay selection among the archive's other audio
  tracks. Runtime does not extract or materialize game media.
- ✅ The archive's actual US/JP CUE bytes were streamed from `unrar` to stdout.
  Each declares Track 01 as its exact original-stem WAVE (`TQUS01.wav` or
  `TQJP01.wav`); the archive instead carries the corresponding OGG transcode.
  Streaming those authentic OGG members directly through SHA-256 produced US
  `c2b296a82898a749503b10edab2523cbb5e7e165ef8c95abafe348fe36bc9c3e` and JP
  `bfac627f0e1ee7debd5bb356065d11f1b3542402e8831b1634d1eab3e119a619`.
  Preserve only the exact same-stem `.ogg` fallback already used for loose
  media, and bind it to the selected regional CUE and verified Track 02; do
  not infer a track from the archive's broad member list. Both authentic OGG
  members now pass SHA-256 admission and decode through the bounded in-memory
  stream.
- ✅ Authentic JP CUE-projected ISO startup now normalizes its verified source
  bytes at the 224-sector INDEX 01 offset and catalogs three source-backed
  startup bitmap anchors. Local real-media tests compare this JP projection
  byte-for-byte with raw Track 02 and validate all seven dungeon banks plus
  title/stage/soul-room/forcefield routes. The old hash-listed zero-filled JP
  ISO stub remains rejected. The ISO projection is not staged on trv2, so its
  host build did not run this specific split-ISO test.

2026-09-25 trv2 Linux Release verification: the complete Theron CTest label
suite passed all 68 tests with no failures. Twenty-one tests returned their
configured skip status because required RAR/ISO projections, raw-file names,
BRAM or capture sidecars are not staged on that host; real-media JP CUE runtime,
USA CloneCD startup and the remaining available probes passed. The authentic
combined RAR and JP CUE-projected ISO paths were instead tested locally against
the supplied original archive, with all six focused real-media regressions
passing after integration with current `main`.

2026-09-25 direct real-media follow-up: the current Release executable also
boots directly from the supplied authentic US and JP Track 02 BIN files, reaches
`theron-runtime`, and accepts the scripted native movement input for both
regions. Each boot receipt binds its matching regional roster/name banks and
Track 19 name bank. The focused CTest selection including both RAR regions,
JP CUE runtime, the seven-dungeon source loader and CDDA availability/handoff
passed 7/7 locally. Standalone BIN paths correctly report no Track 01 CDDA
because the matching full-disc cue/audio set is absent beside these two files;
this is distinct from the authenticated combined-RAR title-audio path above.

## 2026-09-26 authentic full-disc US CloneCD CUE admission

- ✅ The authentic full-disc US CloneCD CUE stores all 19 tracks in one
  MODE1/2352 BIN. Strict intake now bounds Track 02 between its CUE `INDEX 01`
  and the immediately following track's `INDEX 01` in that same member, then
  exposes only that range as a virtual `BIN::slice@offset:length` locator.
  Admission still requires the exact CloneCD Track 02 digest
  `168bd6a63784e91885df8c47be62ab5a`; the source BIN is read in place, not
  unpacked, copied, or rewritten. Full asset scans retain the original CUE as
  immutable `sourcePath`, restore its paired-media metadata only after
  revalidating the slice hash, and return the original CUE as the launch path.
- ✅ Verification used the originals on trv2 in an isolated build directory.
  CUE MD5 `46bebca37c7c1a18375e6e1ca32c3090`, full-disc BIN MD5
  `45d0593e3574ac92bd5a2d0170eb5383`, and the virtual Track 02 slice MD5
  `168bd6a63784e91885df8c47be62ab5a` were checked. The pre/post CUE and BIN
  hashes are identical. The actual-media asset-inventory test and native
  `test_theron_v1_us_clonecd_raw_cue_runtime_boot.sh` both pass on trv2; local
  unit/intake and authentic JP Rev. 1 CUE inventory tests also pass.
- 🔒 This proves authentic Track 02 discovery, provenance, and native startup,
  not full campaign parity. The raw US whole-disc CUE boot still reports
  `theronTrack01CddaReady=0`; authenticated CDDA selection and gameplay
  commands, dungeon transitions, and the remaining gameplay mechanics are
  still open. Do not substitute generated audio, graphics, or world data.

## 2026-09-26 trv2 full Theron suite and RAR extractor availability

- ✅ Rebuilt the Theron-named targets and `firestaff` in the isolated trv2
  build directory. The complete Theron CTest label selection then ran all 68
  registered tests: 60 passed, seven capture/media-dependent tests skipped,
  and the authentic combined-RAR direct-boot test failed before game startup.
  The authentic JP CUE/raw-BIN runtime, seven-dungeon source loader, real BRAM
  Continue, US raw CUE and ZIP startup, and source-data/mechanics tests passed.
- 🔒 The remaining RAR test is currently an environment limitation on trv2:
  its original archive is present (MD5
  `ac34e0f1482416e9728255dcb25d8234`), but `unrar` and `bsdtar` are absent.
  The installed `7zz`/`7z` can list the archive yet report `Unsupported Method`
  for its RAR members. Firestaff consequently cannot authenticate the selected
  Track 02 member on this host, so this does not prove the combined-RAR route.
  No archive was extracted or modified. Keep this runtime route open until a
  supported extractor is available and the real-media direct-boot and Track 01
  handoff tests pass again.
- ✅ 2026-09-26 local real-media follow-up: the existing configured build had
  the application target up to date but lacked the small launcher-handoff test
  helper. After building that helper, both registered combined-RAR tests pass
  against the authentic archive in `~/.firestaff/data/theron`: US and JP
  direct boot authenticate the original concatenated Track 02 members without
  extraction, and the Track 01 CDDA handoff test passes. This confirms both
  routes on this host; it does not remove the separate trv2 extractor gap or
  establish complete gameplay/audio parity.
- ✅ 2026-09-26 local full Theron regression: built all Theron/test targets
  (828 Ninja steps) from the same source tree as `bb0030158`, then ran all 68
  registered `theron` CTests against the installed authentic Theron data. Result:
  62 passed, six skipped, zero failed. Both combined-RAR routes, raw US/JP
  startup, JP CUE and later-dungeon runtime, seven-dungeon data/mechanics,
  BRAM Continue, archive/source boundaries, and V2 gate probes passed. The
  skips are the missing real VDC capture, US CloneCD ZIP/CUE media, authenticated
  CLI/original-command capture corpus, and Mednafen CD-state capture. No
  synthetic game media was introduced. This local run supersedes the earlier
  local partial-build result; the trv2 RAR-tool limitation remains host-specific.
- ✅ The combined-RAR direct-boot test now checks whether the host's first
  available RAR extractor can actually decode the archive's authentic US CUE
  before launching Firestaff. Local US/JP direct boot still passes against the
  original archive; trv2 now reports a clear CTest skip because its available
  7z cannot decode this RAR compression method, rather than misreporting a
  Firestaff startup regression. The real-media route remains unverified on
  trv2 and no game data is extracted or changed.

## 2026-09-26 trv2 rebuilt full-suite result

- ✅ After rebuilding the Theron test targets on trv2, the complete
  `ctest -L theron -j2 --output-on-failure` run against
  `/home/trv2/.firestaff/data/theron` completed all 68 registered tests:
  60 passed, eight returned their configured CTest skip status, and none
  failed. Authentic US CloneCD ZIP/CUE and raw-CUE startup, JP 7z/CUE/raw-BIN
  startup, the seven-dungeon JP runtime, real BRAM Continue, and the available
  source/mechanics probes passed.
- 🔒 The eight skips are evidence gaps or host-tool limitations, not passed
  gameplay routes: authentic RAR CUE handoff and direct boot (no `unrar`, and
  installed `7z` cannot decode the archive's CUE member), US converted ISO
  startup (ISO not staged), authenticated CLI capture (the required atomic VDC
  bundle is not staged), original-command capture (its capture-directory
  variable is unset), real VRAM capture (atomic VDC/VCE/SAT bundle unset),
  Main-RAM loader capture (no capture supplied), and Mednafen CD-state trace
  (trace environment variable unset). Within the passing JP later-dungeon
  test, its optional CUE-projected ISO subcheck also skipped because neither
  the legacy stub nor authentic projection is staged on trv2. The complete
  campaign, original gameplay consumers and presentation remain open as
  described above; this regression run does not establish them.
- ✅ Before that full run, synced only the save-progress test file whose local
  fix keeps its buffers alive through the malformed-save rejection check,
  confirmed the remote SHA-256 matched the current branch, rebuilt the test,
  and passed its focused CTest. The subsequent 68-test run therefore includes
  that verified test fix. No game-data files were copied or changed.

## 2026-09-26 — authentic original Backup RAM export encoding

- ✅ Added a source-gated encoder from the live Theron champion and campaign
  state to the original DMS-SG.001 selected slot. It follows the authenticated
  writer's exact 134-byte layout, requires the bound Track 02 campaign source
  and Theron in party slot zero, preserves the template's unclassified
  campaign high bit, the two transport-padding bytes, other slots, selected
  slot, and unrelated Backup RAM bytes. The in-memory API returns a 2 KiB
  image without filesystem effects. A separate explicit-path writer now
  stages to a unique sibling file, flushes it, verifies the exact bytes and
  original container again, then atomically replaces the requested target.
  It rejects the source path and a symlink alias to that authentic input; the
  caller must provide an existing destination directory.
- ✅ Against the authentic US Akutuba-complete Backup RAM file
  (MD5 `ffabc8d19b0915d4d9632a7ae2e90a97`), Firestaff decoded the original
  body, restored its persistent fields into the real campaign world, encoded
  them back, and reproduced all 2,048 bytes exactly. The focused authentic
  `theron_v1_pce_bram_real_artifact` test passed; no game data was generated or
  modified.
- ✅ Rebuilt the Theron targets and ran all 68 local Theron CTests against the
  available authentic data: 62 passed, six configured media/capture tests
  skipped, zero failed. Both authentic combined-RAR regions, BRAM Continue,
  JP CUE/runtime and projected-ISO paths, and available Track 02/media tests
  passed.
- 🔒 This proves a lossless unchanged-state encoder and an atomic, explicit-
  destination file write, not an in-game save workflow or persistence of
  changed dungeon progress. Binding gameplay progress to the writer and
  reopening a changed save in original Theron remain open. No synthetic game
  state was used as evidence.
- ✅ Follow-up guard: the authentic-artifact test now also changes the
  in-memory party leader to a non-Theron identity and confirms the production
  encoder rejects it without touching the caller's output buffer. Focused
  test and the full 68-test local Theron suite passed (62 passed, six
  configured skips, zero failures).
- ✅ The same authentic-artifact test exercises the explicit-path writer in a
  temporary user-owned directory, verifies a second atomic replacement, and
  confirms both direct and symlink aliases of the authentic source remain
  byte-identical. The full Theron suite passed again (62 passed, six
  configured skips, zero failures); Gitleaks found no leaks in changed files.
- ✅ Production Continue now searches the selected save root for
  `theron-original.bram` before considering the untouched bundled/authentic
  baseline. The real-artifact test writes the original bytes through the new
  writer, resolves that file with the production startup library, then makes
  the user save invalid and confirms startup does not silently fall back to
  baseline progress. The full 68-test suite passed (62 passed, six configured
  skips, zero failures).
- ✅ M11 no longer lets Theron's in-dungeon Save Game input fall through as
  ignored gameplay or route toward the shared DM1 save-disk dialog. It reports
  that the original game has no in-dungeon save transaction; the authenticated
  M11 Continue regression verifies the route, and the full Theron suite still
  passes (62 passed, six configured skips, zero failures).

## 2026-09-27 — authentic quest-artifact test joins Theron suite

- ✅ Registered `theron_v1_track02_quest_item_names` with the `theron`,
  `track02`, and `real-data` CTest labels. Previously the test passed on the
  authentic US and JP Track 02 BINs but was absent from `ctest -L theron`.
  A companion CTest now discovers the original US CloneCD CUE, checks its MD5,
  slices only Track 02 between original CUE indices 3234–6605 in a disposable
  build-directory temporary folder, then verifies the slice MD5
  `168bd6a63784e91885df8c47be62ab5a` before running the same real-data test.
  Missing media skips; mismatched CUE or slice bytes fail. The original disc
  remains unchanged and the temporary slice is removed after the test. The
  complete trv2 Theron selection now runs 70 tests: 62 passed, eight
  configured capture/media-dependent skips, zero failures. US BIN, JP BIN,
  and authentic US CloneCD quest-artifact name checks all passed. A negative
  admission check also supplied the authentic JP CUE through the US CloneCD
  override and confirmed it was rejected on its real, non-US CUE MD5 before
  any temporary slice was created.

## 2026-09-27 — authenticated VRAM replay and Drator route boundary

- ✅ The atomic real trv2 capture
  `/home/trv2/firestaff-theron-evidence/capture/selection.trace.*` was admitted
  with its matching VRAM, VCE, VDC-state, SAT and VDC-I/O sidecars. The full
  Theron CTest selection ran with these inputs: 70 tests, 63 passed, seven
  fixture-dependent skips, zero failures. Hardware-state replay produced
  1,704 BAT tiles and 220 sprite pixels at 320x200; this remains a
  screen-space capture, not game-owned room or tile semantics.
- 🔒 Follow-up cold-start captures used the authentic US CloneCD CUE and
  Track 02 digest `168bd6a63784e91885df8c47be62ab5a`. The corrected isolated
  profile used the authentic 2 KiB campaign BRAM (MD5
  `ffabc8d19b0915d4d9632a7ae2e90a97`); a subsequent run scheduled original
  controller input `run@9600:90`. It recorded 25 raw-sector spans, four SCSI
  commands, 115 CD IRQ callbacks, one game-owned `$E009` dispatch, five loader
  TII transfers, no `$E009` data reads, no authenticated CD-to-RAM receipt and
  no Drator route-hook match. The post-run BRAM remained byte-identical.
- 🔒 At frame 9600 the capture producer recorded `RUN=0x0008`, but the
  configured 65,536-read input trace limit ended at that same boundary. The
  trace has the producer-side input marker but no subsequent original CPU
  read receipt, so it cannot prove that System Card/game code consumed RUN.
  Raw traces remain outside the repository under
  `/home/trv2/firestaff-theron-evidence/capture/drator-generator-authentic-bram-run9600-goal-20260927`.
  Next capture must increase that bound and require a post-event CPU read
  before testing the signed menu route. Do not infer a dungeon transition from
  raw-sector reads or `$E009` dispatch alone.

- 🔒 A 360-second follow-up raised the input limit to 262,144 reads while
  retaining the same authentic US media, unchanged campaign BRAM and
  `run@9600:90` input. The producer applied RUN through frame 9689, but that
  boundary again coincided with the end of the bounded input trace: 262,143
  CPU results were retained, with no subsequent original CPU read of
  `0x0008`. The receipt still reports 25 raw-sector spans, four SCSI commands,
  one `$E009` dispatch, five TII transfers, zero `$E009` data reads and zero
  authenticated CD-to-RAM receipts; the Drator hooks did not fire and the
  BRAM snapshot still matches its authentic input hash. The raw bundle is at
  `/home/trv2/firestaff-theron-evidence/capture/drator-generator-authentic-bram-run9600-input262144-goal-20260927`.
  Next attempt needs the maximum 1,048,576-read bound and at least 600 seconds
  of emulation, so the pre-event trace ceiling cannot hide the post-RUN read.
  No additional capture was started because another agent had resumed using
  trv2.

- 🔒 A 600-second authentic US CloneCD cold-start replay then used the maximum
  1,048,576-read trace bound, original System Card 3.0, and the unchanged
  2 KiB campaign BRAM (`ffabc8d19b0915d4d9632a7ae2e90a97`), with `RUN` held
  from frame 9600 for 90 frames. The input trace contains 37,580 subsequent
  CPU results with `raw=0008 value=37` at `$E4C8`, establishing that the
  authentic input reached an original CPU polling path. The receipt still has
  zero authenticated CD-to-RAM receipts, zero `$E009` data reads, one `$E009`
  dispatch and 25 raw-sector spans; the post-run BRAM digest remains unchanged.
  The strict Stage 2 System Card call verifier rejects this trace because its
  exact expected call receipt is absent. This closes the earlier input-trace
  ceiling gap only; it does not establish Stage 2 completion, menu selection,
  dungeon entry, `$2600` T900 consumption, or inventory semantics. Raw output
  remains outside Git at
  `/home/trv2/work/theron-t900-evidence-run-20260927/capture/authentic-us-bram-run9600-input1048576-600s.trace*`.

- 🔒 A follow-up cold-start used the same authentic US CloneCD, System Card
  3.0 and unchanged campaign BRAM with the signature-gated
  `drator-generator` research route and a scripted PCE `run@1:5` controller
  event. The event was recorded, but none of the route poll PCs or route-hook
  receipts appeared before the bounded input trace reached 2,097,152
  transactions, and the harness was stopped after the trace filled. The final
  receipt reports 25 raw-sector spans, four SCSI READs, 115 CD IRQ callbacks,
  one `$E009` dispatch, five TII transfers, zero `$E009` data reads and zero
  authenticated CD-to-RAM receipts; BRAM remains byte-identical. This is a
  route-instrumentation/startup boundary, not menu or gameplay evidence. The
  private trace is under
  `/home/trv2/work/theron-t900-evidence-run-20260927/capture/authentic-us-bram-drator-generator-run1-5-maxinput-600s-20260927*`.

- 🔒 A 600-second cold-start then combined the correct authentic controller
  replay `run@9600:90` with `drator-generator`. It reproduced 37,580 original
  CPU reads of the RUN mask at `$E4C8`, but no signature-bound menu poll PC or
  route-hook receipt was reached. The 1,048,576-read bound filled; the final
  receipt reports 25 raw-sector spans, four SCSI READs, 115 CD IRQ callbacks,
  one `$E009` dispatch, five TII transfers, zero `$E009` data reads and zero
  authenticated CD-to-RAM receipts. The strict Stage 2 verifier also rejects
  the missing original call receipt. The authentic BRAM digest remains
  `ffabc8d19b0915d4d9632a7ae2e90a97`. This does not establish menu selection,
  dungeon entry or T900 semantics. The timed-out capture remains private at
  `/home/trv2/work/theron-t900-evidence-run-20260927/capture/authentic-us-bram-drator-generator-run9600-replay-600s-20260927*`.

- ✅ Local harness regression: `test_theron_v1_mednafen_live_capture_script.sh`
  passes and verifies that the live capture accepts an operator-selected input
  trace limit from 65,536 through 1,048,576 reads. Its temporary files were
  directed to the task scratch directory. This checks capture configuration
  only; it starts no emulator and adds no original-game evidence. The next
  authentic replay still requires an available trv2 session.

- 🔒 2026-09-28 controlled no-periodic-RUN replay on `trv2`: the capture
  template was seeded with the authentic 2 KiB US campaign BRAM (MD5
  `ffabc8d19b0915d4d9632a7ae2e90a97`), and the original US CUE, Track 02 and
  System Card hashes were checked. The replay applied `run@9600:90`; the
  bounded input trace records 37,600 original CPU results with `raw=0008` at
  `$E4C8`, but no signed Drator menu-route hook or Track 02 loader sidecar
  appeared. Only 49,380 VDC writes were captured, below the required 65,536
  snapshot boundary, so the harness rejected the run and emitted no transition
  receipt. Its private final BRAM snapshot differs from the authentic seed
  (`dbdedb0ec809227b289c2bc5b18b9c9d`), so the run does not qualify as an
  unchanged-BRAM replay. The extra periodic RUN pulses were absent; their
  removal did not establish a route or gameplay consumer. Raw traces and the
  seeded copy remain outside Git under
  `/home/trv2/work/firestaff-theron-inventory-id-integrity-20260928/.codex-scratch/`.

- 🔒 2026-09-29 read-only audit of that retained controller trace: of
  1,048,575 CPU-read rows, 524,287/524,286 are at the System Card poll PCs
  `$E4B7`/`$E4C8`; the two remaining reads are at `$E4B4`/`$E4C5`. The
  title-wait and Drator menu poll
  sites `$0865`, `$7557`, `$6E44`, `$5C97` and `$6DBD` have zero reads. This
  confirms the scheduled RUN was consumed only by the System Card loop; the
  capture never reached the title/menu route. This negative trace does not
  establish a later route; the next experiment must target a source-bound
  title/menu entry instead of treating a larger read bound or the same input
  schedule as gameplay evidence. The trace remains private on trv2 and no game
  payload was copied into the repo.

- 🔒 2026-10-03 stock Mednafen 1.32.1 cold-start confirmation: with the
  authentic JP Rev. 1 CUE/System Card in a fresh isolated profile, holding the
  mapped RUN/Return input for four seconds at the System Card prompt reached
  the Theron's Quest title menu. Selecting NEW GAME and FILE_1 then reached
  the authentic dungeon-selection map; selecting Ak-Tu-Ba and advancing its
  intro reached the first-person dungeon view. This is a real runtime gameplay
  capture. The fresh profile generated a 2-KiB game-specific persistent-RAM
  file. After restarting Mednafen with that SRAM and an isolated empty
  save-state directory, LOAD GAME → FILE_1 → YES returned to the
  dungeon-selection map. A later hash audit found this BRAM byte-identical to
  the known empty menu-only JP image (`de8e415730226a1f0e39666b1ea291b6abec07bcaeb7223dc33ea01a71f89eaa`);
  the after-load map capture is also identical to the after-New-Game capture.
  Therefore this observes a load-menu route but does not prove a non-empty
  native save or restored campaign progress. The authentic title-to-dungeon
  gameplay route remains verified. F5/F7 is emulator-state evidence only.

- 🔒 2026-10-03 Track 02 thing-data test hardening: US BIN and JP BIN checks
  are now separate CTests gated by their authentic image digests; explicit
  paths cannot fall back to another installed edition. Default-media absence
  is region-local; explicit unreadable and wrong-edition input fails, and the
  reader checks seek, size, read, and close results. The optional CloneCD
  raw-data test has its own digest gate but skipped because no explicit
  CloneCD raw image was configured on trv2. The 14 available roster, spawn,
  door, map, descriptor, and thing-data CTests passed three repeated loops
  against authentic US/JP data. Negative checks
  confirmed independent default skips, explicit-path failures, and rejection
  of cross-region media. These source-data checks do not establish complete
  in-game object behavior.

- 🔒 2026-10-03 level-data-block source checks are now isolated by input:
  static, US BIN, JP BIN, US CloneCD raw, US Track 19 ISO, and JP Track 19 ISO.
  Each real input must match its exact source digest, with explicit paths
  authoritative and only absent defaults returning skip 77. Four available
  tests passed three loops against authentic US/JP BIN and US ISO data; the
  CloneCD and JP ISO tests skipped because those inputs were unavailable.

- 🔒 2026-10-04 PCE Fast BaseRAM consumer-read follow-up: broadened the
  instrumented Mednafen 1.32.1 `pce_fast` read trace from six selected bytes to
  the full physical `$1F0000-$1F7FFF` BaseRAM window, retaining a global cap of
  65,536 rows and at most 16 rows per byte. The patch applied and the full
  instrumented emulator built on `trv2` with `-j1`. Replaying the authentic JP
  Ak-Tu-Ba Mednafen state produced 3,354 bounded BaseRAM read rows, with actual
  logical/physical addresses, values, and reader PCs; this confirms the former
  zero-row result was an artifact of the narrow address filter. In these rows,
  neither logical `$2031` nor physical BaseRAM offset `$31` was read, so they do
  not reveal the level/map selector or join any consumer to a verified active
  map identity. An initial attempt used a stale staged script and failed on its
  unconditional PCE VRAM requirement; an isolated rerun with the worktree
  script passed the PCE Fast RAM/snapshot checks, recorded an 8 KiB snapshot,
  and marked VDC data unavailable. That run then correctly stopped at the
  dynamic-receipt gate (`input=1432`, `irq=0`, `authenticated_cd_ram=0`): a
  restored save state does not provide a fresh CD-to-RAM transition. No
  production map or spawn behavior is changed or claimed by this observation.
  The emulator, authentic state, and traces remain in the task-private TRV2
  evidence area.
  Missing-default, explicit-missing, and wrong-region checks behaved as
  expected. This proves the seven level-block receipts and metadata only, not
  decompressed level contents or original-game use.

## 2026-09-27 — authentic closed-door boundary coverage

- ✅ The real-data mechanics probe now classifies every source-backed door in
  all seven US and JP dungeons, requires exact source provenance and a decoded
  door tile, and uses only ordinary floor approaches without active creatures
  on either square. On trv2, the focused CTest passed with 225 checks, zero
  failures and zero skips. Each region loaded 105 doors: 93 had an isolated
  approach and remained blocked by the bounded runtime; 12 had no adjacent
  floor approach; none had an active-creature overlap. This validates the
  current fail-closed boundary against authentic media only. It does not prove
  original door-opening, locking, key, button or movement semantics, so the
  missing original T900 consumer remains open.

## 2026-09-27 — authenticated System Card call-register variance

- ✅ Verified the existing local original US CUE/System Card 3.0 trace against
  the current Track 02 and firmware digests. It records the `$40cd -> $e009`
  call with the expected table and return path, but Y=`$99` instead of `$03`.
  The System Card API reference lists the `$f8..$ff` zero-page call inputs and
  does not list Y as an input. The strict receipt verifier now admits these two
  observed Y values while preserving exact checks on all other fields; its
  focused test passes with both variants and verifies the local authentic trace
  when available. The test is registered with CTest and passes from the
  generated CMake test inventory; its temporary fixture stays in the build
  tree. The trace remains private and is not committed. This establishes the
  call receipt only, not successful stage-two handoff, menu selection, dungeon
  entry or T900 semantics.

## 2026-10-02 — C3A0 caller evidence rejects unrelated rows

- ✅ Tightened `verify_theron_record_table_provenance.py` so a C3A0 caller
  witness must have a logical PC inside the source-locked `$C3A0–$C429`
  window and a physical PC consistent with the captured MPR mapping. The
  regression now rejects both an out-of-window row and a row whose physical
  mapping disagrees with its logical PC; the authentic-shape fixture still
  passes. This validates evidence coordinates only and does not promote the
  `$611D` record table to level, object, creature, or gameplay semantics.
