# Theron US cold scripted-input return-mask capture — 2026-10-31

## Result

A 45-second cold run in standard Mednafen PCE used authentic US Track 02 and
System Card media. No autoload state or configured profile SRAM was present.
The plan scheduled `run@480:300`, `select@900:120`, and `i@1200:300`.

The revised verifier matched all three scripted events to device applies and
subsequent controller reads. The reads occurred at PC `$E4C8` in the System
Card BIOS; `system_card_poll_reads=3` and
`non_system_card_poll_reads=0`. This validates delivery into the BIOS-visible
controller path, not a game-owned poll or menu action.

The transition receipt remains `transition=missing`. The run observed 25 raw
CD sectors and one `$E009` dispatch/entry, but no CD data-port reads, no
source-bound reads, no byte-exact origin-to-RAM receipt, and no authenticated
CD-to-RAM handoff. The dispatch is not joined to Track 02 content. This run
does not establish that the title menu consumed the inputs or that any gameplay
action occurred. Native Firestaff parity is unchanged.

## Run identity and retained receipts

- Module: standard `pce`.
- Track 02: authentic US MODE1/2352, MD5
  `f23601102138f87c33025877767ebf76`.
- System Card: MD5 `ff1a674273fe3540ccef576376407d1d`.
- Autoload state: none; configured profile SRAM files: zero.
- Instrumented Mednafen binary MD5: `fd2250e10dbe5c91e639a611fd2af624`.
- Plan: `run@480:300,select@900:120,i@1200:300`.
- Scripted-input receipt: 3 events, 3 applies, 3 subsequent controller reads,
  3 selected-bank mask matches; BIOS poll boundary only.
- Transition receipt: 25 raw sectors, 1 `$E009` dispatch, 0 CD data-port reads,
  0 source-bound CD reads, and 0 authenticated CD-to-RAM receipts.
- Input sidecar SHA-256:
  `359500ac646e0e67516aa2c113a251b0d0ff8f1902dee8bd5787d3b4c1fd2b2b`.
- Transition receipt SHA-256:
  `34eb6bdc0509b209dde0dd704956674ffe73d791fb02cc5fcdc59e2e1643d191`.

Raw traces, game media and RAM snapshots remain in temporary TRV2 storage and
are not repository assets. The script validates the active-low value returned
to the guest: standard PCE restores the physical input buffer after device
update, so its raw sidecar field may omit scripted bits even though the device
has latched them. The PCE Fast raw-field behavior is also covered by the same
returned-bank check.

## Follow-up

Keep the corrected returned-value check and its fixtures. The next reference
capture must observe a non-System-Card controller poll and bind the relevant
game-owned state transition to authenticated Track 02 source reads before
assigning meaning to `$E009` or claiming a title-menu action. Continue to use
reference instrumentation only as diagnostic evidence; native Firestaff
behavior requires independent authentic-data verification.
