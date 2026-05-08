# Pass320 — DM1 V1 F0097 VIDRV window after-F0128 sequencing probe

Status: `BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER`

## Source anchors

- `DRAWVIEW.C`: F0097 entry line 709, viewport zone line 847, VIDRV call line 857.
- `DUNVIEW.C`: F0128 calls F0097 at lines 8606 and 8610.
- `DEFS.H`: video driver table slot 9 line 4279.
- `IBMIO.C`: PC runtime binds `G2156_VideoDriver` from interrupt vector line 2518.
- `VIDEODRV.C`: slot-9 table entry line 950, implementation line 3566.

## Runtime decision

- Strict F0128 stop recaptured: `False`.
- F0097/VIDRV window hit: `False`.
- Exact VIDRV candidate: `2809:1EFF`.
- Blocker: Pass320 strict stop filter did not recapture F0128. Pass318/older scripts confuse BPLIST setup echoes with stops; the next blocker is debugger stop/control sequencing before the post-F0128 F0097 window can be trusted.

Manifest: `parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe/manifest.json`
