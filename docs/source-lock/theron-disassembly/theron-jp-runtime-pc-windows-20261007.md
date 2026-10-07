# Theron's Quest JP Runtime-Sampled HuC6280 PC Windows

## Evidence and limits

The trace is from
`/home/trv2/firestaff-theron-evidence/capture/stage2-mpr1-probe-authentic-jp-f5-x11-diagonal-20261006/`.
Its transition receipt identifies the authentic JP Rev. 1 Track 02 BIN by
MD5 `b7afb338ad31be1025b53f9aff12d73a`, in `MODE1/2352`; the media file is
8,102,640 bytes and independently has that MD5. The captured transition says
`mednafen_module=pce_fast` and `input_delivery=scripted_pce_replay`.

This is not a gameplay-state or stage-two provenance receipt. The trace header
leaves `variant` and `stage3_track02_record` unknown, and the transition says
`transition=missing` and `post_dungeon_overlay_replay=0`. No gameplay meaning,
stage identity, or handler name is inferred below.

The evidence sidecars are kept outside the repository with the authentic game
media. Their SHA-256 values are:

| Sidecar | SHA-256 |
|---|---|
| `live.trace` | `fa1cb2331299570ccb79e28a0f668bb3d09845b670e1a3104ec8ca07eda50f15` |
| `live.trace.main-ram-consumer` | `d2f884cadd433b874213761522dd7801f465ad49b2991ae4f6a8c2111000029b` |
| `live.trace.transition` | `52638a24771a0fec5585dd061b2bd02ed90a32af938d447619b6d26792a54d04` |

## Observed windows

Filtering `live.trace.main-ram-consumer` for logical PC `$C10E..$C44D` and
physical PC `$0D2000..$0D3FFF` yields 228 ordered reads at 27 unique logical
PCs. Repeated rows at each PC contain the same eight `reader_code_bytes`.
Each row records the active PC and physical mapping; the bytes below come
directly from those rows. MAME `unidasm -arch h6280` was run separately from
each sampled PC, three times per window, and gave identical output on all
three passes. The final column gives only the first instruction at the
sampled PC; subsequent linear instructions in each eight-byte window are not
claimed to have executed.

| Logical PC | Physical PC | Captured bytes | First HuC6280 instruction |
|---:|---:|---|---|
| `$C10E` | `$0D210E` | `BD 3F 29 AA A9 0D 20 E7` | `LDA $293F,X` |
| `$C1E7` | `$0D21E7` | `73 90 20 97 28 04 00 43` | `TII $2090,$2897,$0004` |
| `$C1FA` | `$0D21FA` | `AD 2B 27 4C A3 59 64 27` | `LDA $272B` |
| `$C268` | `$0D2268` | `CE 23 27 C8 B1 01 AF 98` | `DEC $2723` |
| `$C2D5` | `$0D22D5` | `AE 23 27 10 07 6D 22 27` | `LDX $2723` |
| `$C2DA` | `$0D22DA` | `6D 22 27 90 33 80 07 6D` | `ADC $2722` |
| `$C2E1` | `$0D22E1` | `6D 22 27 90 02 A9 FF 85` | `ADC $2722` |
| `$C2EE` | `$0D22EE` | `AE 24 27 10 0D 6D 24 27` | `LDX $2724` |
| `$C2F3` | `$0D22F3` | `6D 24 27 B0 0B 49 FF 1A` | `ADC $2724` |
| `$C300` | `$0D2300` | `6D 24 27 85 92 18 A5 93` | `ADC $2724` |
| `$C308` | `$0D2308` | `AE 24 27 10 0A 6D 24 27` | `LDX $2724` |
| `$C30D` | `$0D230D` | `6D 24 27 B0 0C 68 68 A9` | `ADC $2724` |
| `$C317` | `$0D2317` | `6D 24 27 90 02 A9 FF 85` | `ADC $2724` |
| `$C321` | `$0D2321` | `AD 1F 27 C5 92 90 0F F0` | `LDA $271F` |
| `$C337` | `$0D2337` | `AD 20 27 0A 0A 0A 85 14` | `LDA $2720` |
| `$C346` | `$0D2346` | `AD 1E 27 C5 91 B0 02 85` | `LDA $271E` |
| `$C358` | `$0D2358` | `ED 21 27 85 93 A5 92 C5` | `SBC $2721` |
| `$C372` | `$0D2372` | `AE 9F 27 18 7D 44 48 85` | `LDX $279F` |
| `$C3D4` | `$0D23D4` | `AE 23 27 10 10 6D 22 27` | `LDX $2723` |
| `$C3E9` | `$0D23E9` | `6D 22 27 B0 05 CD 1E 27` | `ADC $2722` |
| `$C3EE` | `$0D23EE` | `CD 1E 27 90 03 A9 00 60` | `CMP $271E` |
| `$C40A` | `$0D240A` | `AD 1D 27 C5 90 90 12 F0` | `LDA $271D` |
| `$C41E` | `$0D241E` | `AD 1D 27 85 90 20 21 C3` | `LDA $271D` |
| `$C426` | `$0D2426` | `AD 20 27 18 65 23 85 23` | `LDA $2720` |
| `$C42E` | `$0D242E` | `AD 21 27 18 65 24 85 24` | `LDA $2721` |
| `$C436` | `$0D2436` | `AD 21 27 29 07 85 14 A5` | `LDA $2721` |
| `$C44D` | `$0D244D` | `CD 1B 27 90 03 AD 1B 27` | `CMP $271B` |

## Static-source correlation

The 20-byte anchor reconstructed from the observed physical-PC byte windows
at `$0D22D5..$0D22E8` has SHA-256
`0ee4e603fa0f1ce551be1ad185bdbba69a526aa0f5dbe5a9d05bc21a4959e898`.
It occurs at seven raw offsets in the authentic JP Track 02 BIN:
`$097335`, `$0E0B35`, `$12A335`, `$173B35`, `$1BD335`, `$206B35`, and
`$250335`. Mapping each occurrence to the sampled logical PC `$C10E` gives
candidate code-window bases `$09716E`, `$0E096E`, `$12A16E`, `$17396E`,
`$1BD16E`, `$20696E`, and `$25016E`.

Comparing all 27 unique captured windows against those candidate bases gives
four candidates with 27/27 matches, two with 26/27 matches (each misses
`$C10E`), and one with 24/27 matches (misses `$C10E`, `$C1FA`, and `$C372`).
Therefore the authentic edition and captured runtime bytes are identified,
but the trace does not identify which repeated source copy supplied the
executed code. There is no source-offset-to-RAM-load receipt joined to these
PC windows.

The next provenance step is to capture a contemporaneous source-offset/RAM
load receipt that distinguishes the repeated code copies and also binds a
known game transition. Until then, retain these as runtime-sampled disassembly
windows only; do not assign a routine purpose or use them as proof of stage or
gameplay parity.
