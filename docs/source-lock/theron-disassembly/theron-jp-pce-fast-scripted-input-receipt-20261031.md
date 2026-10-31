# Theron JP PCE Fast scripted-input receipt

## Result

Three separate runs replayed `up@61:120` on controller slot 0 from the
authentic JP Akutuba F5 dungeon state. Each run loaded the state successfully
and recorded the scripted input reaching a PCE Fast guest controller read in
the same frame:

```text
scripted_pce_input_event frame=61 key=up mask=0010 hold=120 port=0
scripted_pce_input_apply frame=61 scripted=0010 port=0
pce_input_read frame=61 cpu_pc=44c1 physical_pc=000d04c1 register=1000 raw=0010 sel=1 clr=0 index=0 value=3e module=pce_fast device_type=1 device_raw=0000 scripted_port=0 scripted_mask=0010
```

The input verifier passed each receipt. It reports one scripted event, a
same-frame apply, a matching controller-read witness at `$44C1`, zero
System Card poll reads, and one non-System-Card poll read. The result verifies
input delivery to the guest controller read only. It does not establish the
instruction's owner, the action's gameplay meaning, a resulting game-state
change, a visible change, or parity.

## Inputs and capture method

The capture used original JP Rev. 1 media and an emulator-created state that
already shows Akutuba gameplay. All inputs were hash-checked:

| Input | Identity |
|---|---|
| CUE | MD5 `85706e7c2f658bc2792511d618dfc7a5` |
| Track 02 BIN | MD5 `b7afb338ad31be1025b53f9aff12d73a` |
| System Card 3.0 | MD5 `ff1a674273fe3540ccef576376407d1d` |
| Akutuba F5 state | SHA-256 `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee` |
| Instrumented Mednafen | MD5 `09e5cae2a37d7c0734d970d98dc07f40` |

The input script is a deterministic measurement stimulus; the game, state,
disc tracks, and System Card are authentic. Each run was isolated under a
temporary profile and stopped by its bounded timeout after capture. The
instrumented PCE Fast `input.cpp` object was compiled and linked in a private
copy of the configured Mednafen build.

Raw traces, stdout/stderr, and verifier receipts are preserved outside the
repository at
`/home/trv2/firestaff-theron-evidence/capture/theron-pcefast-scripted-input-20261031-r1/`.
Trace SHA-256 values:

- Run 2: `865bda25254118c542d9b557d2afaedaab9c436e33d3ed6578871b2733bced49`
- Run 3: `dce2e8f20c649a940df5c82684ee5180ecb972ccf5b91cd3cc6dbf295a352f85`
- Run 4: `36fe5189051b3d2a4325a5f00b64c99c758b479985467482aa65a5e19f69a84d`

## Follow-up

The active-low guest value `3e` agrees with the internal UP bit `0010` in the
direction bank. A separate same-state comparison now records a repeatable
`$2912` RAM update in three UP runs and no `$D334`/`$D349` consumer path in
three neutral controls. See the [source-locked comparison report](theron-jp-pcefast-input-state-compare-20261031.md).
These original input-only receipts predate the shared-sequence rebuild. The
follow-up [r2 comparison](theron-jp-pcefast-input-state-compare-20261031.md)
orders the frame-61 poll with the `$2912` read/write transaction. The field's
gameplay meaning, movement effect, visible response, and causal relationship
remain unverified.
