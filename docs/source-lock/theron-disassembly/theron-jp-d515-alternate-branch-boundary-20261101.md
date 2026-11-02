# Theron JP `$D515` alternate-branch boundary

Date: 2026-11-01

## Authentic state-only trace

The read-only review used the retained PCE Fast runtime code-follow trace from
an autoloaded authentic Japanese Ak-Tu-Ba state. The transition receipt
identifies Japanese Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`), the System Card, and the loaded state
(MD5 `d5c0daa227c04d55bdf20244803c00bd`). The 269,795-byte
`live.trace.3879-indirect-target` has SHA-256
`6bd8222e1b1f378397772d71a5490c0e92893bad43ace7e285bf7f765d5f88ad`.
Its companion scripted-input receipt records one event consumed at the
non-System-Card poll `$44D2`.

The instruction trace enters `$D515` at step 55, physical `$0D7515`, with
MPR6 `$6B` and accumulator `$0C`. It reaches `$D528` with Y=`$02`; the next
instruction sees accumulator `$FF`. The authenticated JP `$D515..$D5A6`
candidate disassembly has `CMP #$FE` at `$D52A` and `BNE $D534` at `$D52C`,
so this path skips the `$D530` call to `$D588`. This confirms execution of the
indexed path and explains why this replay does not exercise the alternate
branch.

## Evidence boundary

The autoloaded state bypasses the original CD loader: the transition receipt
contains no CD data-port reads or authenticated CD-to-RAM receipts. Seven
identical JP Track 02 code windows remain possible source copies, so the MPR
mapping does not identify a unique source occurrence or LBA. The single
controller event is not mouse-coordinate evidence. The trace establishes no
pointer meaning, hit region, Button-I action, target, gameplay effect, or
native hit-test behavior.

Next, vary one authentic input dimension at a time from the same state and
capture the controller/mouse reads, `$D4EC..$D5A6` instruction and register
context, indexed-table reads, and resulting command/display state. First seek
a reproducible path that takes `$D52C` to `$D530` and executes `$D588`; then
bind its descriptor and result through the Button-I consumer. A separate cold
original-media capture is still required to bind the executed bank to a
specific Track 02 read before enabling world-space clicks in Firestaff.

The static US/JP disassembly and its seven-candidate source census are in the
[`$D515/$D588` regional source-lock report](theron-jp-runtime-input-poll-bank-alias-20261009.md).
