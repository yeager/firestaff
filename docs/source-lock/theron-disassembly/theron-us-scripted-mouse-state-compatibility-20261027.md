# USA scripted-mouse probe and save-state compatibility

Three isolated loops on TRV2 used the current
`scripts/capture_theron_mednafen_live_trace.sh` with the preserved authentic
USA Ak-Tu-Ba save state, authentic USA Track 02 media, and the instrumented
Mednafen binary at `/home/trv2/work/firestaff-theron-fulltrace-20261026/build/install/bin/mednafen`.
The binary SHA-256 was
`c8e2836e174c8fb87540b48378c6c4e0496040e40eebf1b45fe26dc737145082`; its MD5
was `650105f56a9a1739489effdf766b1564`. The state hash was MD5
`f17f377df210b4a3ae904a13fb85a7f0`, the USA Track 02 hash was MD5
`f23601102138f87c33025877767ebf76`, and the System Card hash was MD5
`ff1a674273fe3540ccef576376407d1d`.

Each run injected `0,0,0;4,0,0;0,0,1;0,0,0` through
`THERON_CAPTURE_MOUSE_INPUT_SCRIPT`. The mouse instrumenter recorded the same
four device updates in all three loops:

```text
device_script sequence=0 update=1 x=0 y=0 buttons=00
device_script sequence=1 update=2 x=4 y=0 buttons=00
device_script sequence=2 update=3 x=4 y=0 buttons=01
device_script sequence=3 update=4 x=4 y=0 buttons=00
```

All three `.trace.mouse` outputs had the same SHA-256:
`952d8be319d190b9a9e890d1304f3a7ea2a16d2a1796aab49abb77400453655f`.
This confirms deterministic injection and instrumentation for that sample
sequence only.

The autoload did not succeed. Mednafen logged `Theron autoload state failed`
and save-state section variable mismatches. Each receipt reports
`transition=missing`, `cd_irq_callbacks=0`,
`authenticated_cd_ram_receipts=0`, and `command_ram_writes=0`. The observed
pointer traces therefore cannot be interpreted as gameplay execution or as
evidence of hitbox and command behavior. No mouse parity claim follows from
these runs.

The instrumenter is `scripts/instrument_mednafen_theron_mouse_trace.py`; it
injects the parsed deltas and button mask in the PCE and PCE Fast mouse device
update paths. The wrapper's environment validation and binary requirements are
in `scripts/capture_theron_mednafen_live_trace.sh`. A useful next capture must
first load a compatible authentic gameplay state or reach gameplay from a
fresh retail CD boot. A save-state capture still does not prove the CD source
sector that supplied runtime bytes.
