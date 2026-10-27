# USA scripted-mouse command trace and save-state compatibility

TRV2 captures used the current `scripts/capture_theron_mednafen_live_trace.sh`
with the preserved authentic USA Ak-Tu-Ba save state, authentic USA Track 02
media, and the instrumented Mednafen binary at
`/home/trv2/work/firestaff-theron-fulltrace-20261026/build/install/bin/mednafen`.
The binary SHA-256 was
`c8e2836e174c8fb87540b48378c6c4e0496040e40eebf1b45fe26dc737145082`; its MD5
was `650105f56a9a1739489effdf766b1564`. The state hash was MD5
`f17f377df210b4a3ae904a13fb85a7f0`, the USA Track 02 hash was MD5
`f23601102138f87c33025877767ebf76`, and the System Card hash was MD5
`ff1a674273fe3540ccef576376407d1d`.

The state fails to load under the `pce_fast` module. The same binary accepts
it under the standard `pce` module, logging `Theron autoload state loaded`
despite save-state variable mismatch warnings. This is a module-specific
compatibility result, not evidence that the loaded RAM was sourced from the
current CD session.

Four separate standard-PCE probes injected horizontal mouse deltas 4, 16, 32
and 64 followed by a brief Button I sample. Each loaded the save state; the
observed hover values remained `$00/$50`, and none queued a `$2905` command.

Three further isolated standard-PCE captures injected a 64-unit horizontal
delta and held Button I for 180 device updates. Each loaded the state and
recorded 181 nonzero `device_script` rows. In every run, the original guest
code queued `$50`, consumed it twice, and cleared the queue:

```text
device_script sequence=0 x=64 y=0 buttons=00
device_script sequence=2 x=0 y=0 buttons=01
```

```text
command_ram_write logical_address=2905 value=50 writer_pc=ccdb
command_consumer_read logical_address=2905 value=50 reader_pc=d34d
command_consumer_read logical_address=2905 value=50 reader_pc=d395
command_ram_write logical_address=2905 value=00 writer_pc=d3a0
```

The `$50` command agrees with the earlier source-locked viewport capture,
which records the original hover path and `$2905=$50` dispatch. This delta
script does not bind its host-relative motion to that capture's exact
`$3C/$78` cursor coordinate; see
[`theron-original-viewport-command-capture-2026-08-21.md`](../theron-original-viewport-command-capture-2026-08-21.md).

Button II held for the same 180 updates did not write `$2905`. Party RAM
`$203F/$2040/$2041` remained `$01/$02/$03` in all three Button I captures.
The full mouse trace hashes differ between runs because they include other
runtime observations; the ordered queue writes and consumer PCs above matched
in all three loops.

Each save-state capture reports `transition=missing` and
`authenticated_cd_ram_receipts=0`. The `$2905` queue evidence is therefore a
runtime command-dispatch observation from the authentic saved state, not a
CD-sector-to-RAM provenance receipt. The observed `$50` command and unchanged
party coordinates do not establish a world-click effect or complete mouse
parity.

The instrumenter is `scripts/instrument_mednafen_theron_mouse_trace.py`; it
injects the parsed deltas and button mask in the PCE and PCE Fast mouse device
update paths. The wrapper's environment validation and binary requirements are
in `scripts/capture_theron_mednafen_live_trace.sh`. Next, bind the executing
command bytes and their mouse-coordinate inputs to a source-LBA-backed session,
then determine whether a viewport target causes an authentic world-state
change. A save-state capture alone cannot prove the CD sector that supplied
runtime bytes.
