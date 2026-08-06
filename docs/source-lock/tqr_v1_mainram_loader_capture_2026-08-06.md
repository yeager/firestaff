# Theron Track 02 main-RAM loader capture — 2026-08-06

This note records the latest operator-local replay against the supplied
hash-verified US Track 02 ISO and System Card 3.0. The capture sidecars are
not repository assets and contain no synthesized game data.

## Observed provenance

| Witness | Result |
|---|---:|
| loader sidecar | MD5 `2827cb429d0b97f0e1fc26185a9bb28c` |
| first loader transfer | `$2286` / bank `$1f`, `TIA`, `$c800 → $0404`, `0x80` bytes |
| loader transfer rows | 13 |
| RTS rows | 24 |
| post-RTS rows | 24 |
| main-RAM consumer sidecar | MD5 `9d19ad9b993f1853e868f381756eb1d0` |
| consumer reads | 4,096 |
| executed code witness | `$2c54–$2c69` |

The loader sidecar includes later `TII` transfers and additional branch/call
rows. Firestaff now accepts those bounded, known HuC6280 witness rows instead
of incorrectly treating the first `TIA` row as the entire capture.

## What this does not prove

The capture did not produce a valid VDC VRAM/VCE snapshot, a `$2600` dynamic
consumer read, or a source-owned decision identifying level-grid/object fields.
It therefore remains provenance-only. The forcefield transition must continue
to stop before dungeon promotion until a clean authentic capture supplies
those consumer and presentation bindings.

Source lock: `docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`
and the checked-in Mednafen instrumentation patches under `scripts/`.
