# Theron JP PCE Fast input-gated VRAM trace — 2026-10-31

## Result

An isolated reference-only Mednafen 1.32.1 PCE Fast capture loaded an
authentic Japanese Akutuba dungeon save state and replayed `up@61:120`. The
guest input sidecar records the matching read at frame 61:

```text
pce_input_read frame=61 cpu_pc=44c1 physical_pc=000d04c1 register=1000 raw=0010 sel=1 clr=0 index=0 value=3e module=pce_fast device_type=1 device_raw=0000 scripted_port=0 scripted_mask=0010 event_sequence=58758
```

The VRAM trace starts with V5's exact PC/raw gate and records its first commit
at shared event sequence `58761`, immediately after the matching guest read
at `58758`. The wrapper validated 4,096 contiguous VRAM commit rows and the
same-frame scripted-input receipt. This confirms that the instrumentation
gate admits CPU-port VRAM commits after the configured guest input read in
this authentic state route. Event ordering alone does not prove the input
caused a particular commit, identify the VRAM contents, show a visible change,
or establish gameplay or Firestaff parity.

The state-only capture mode explicitly skips cold-start System Card checks
that save-state autoload bypasses. Its receipt says `transition=missing`; no
cold game-owned transfer, runtime CD source, map binding, or renderer join is
claimed. The 4,096-row limit was reached, so this is a bounded prefix rather
than a complete run. Raw evidence remains outside Git.

## Provenance and verification

Inputs were the hash-verified JP Rev. 1 CUE and System Card 3.0, the authentic
Akutuba F5 state, and the corrected instrumented PCE Fast build. The scripted
UP event is a measurement stimulus; original game data and state are
authentic.

| Artifact | SHA-256 |
|---|---|
| JP CUE | `7bbbe8b077f35b2e9b05c68848f8cb86ea9aa74021a076e882eee8841bc56545` |
| System Card input | `df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1` |
| Akutuba F5 save state | `b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068` |
| Corrected instrumented Mednafen binary | `d673c3bbd7d8f27fe6b717b29af2ffaaa68d2c74e93b27abe6609a68cb019e56` |
| Input trace | `e2c383753a7d7f5c157dab250ae548ba5137764b5f10fc95a426adc8a060f1b3` |
| V5 VRAM commit trace | `5adb10f9e22af3381cbcdb289fcadc13cdbb6bad1fe774723c5fac6569d596d8` |
| Scripted-input receipt | `bfe48cb53c452eec55fc45e21406efc13858b594920fbe54f201b3e9c078663c` |
| Transition receipt | `4fb08c208cb6d6ae434fe22e4bd4c218da9ca43b3b57c86e18dcc49f949bb816` |

TRV2 capture directory:
`/home/trv2/.cache/firestaff-theron-vwr-input-gate-20261031/authentic-up-input-gate-corrected-2/`.
The corrected input hook passes `pce_jp_data[index]` to the trace gate. The
complete patch chain and instrumented build/install passed; the focused patch
application regression passed three loops. The capture wrapper reports:

```text
PASS: bounded PCE Fast save-state reference traces validated; cold-start and CD handoff are not claimed (exit=124)
```

Exit 124 is the expected bounded timeout used to close this reference run;
the wrapper completed its requested trace and snapshot validation before
reporting the state-only success.
