# Theron's Quest Stage-2 static palette source gap

## Evidence

The authenticated US and JP Track 02 BINs contain the same Stage-2 dispatch
entry for HuC6280 address `$46B8` at file offsets `0x2bf448` (US) and
`0x2beb18` (JP). The 13-byte handler prefix has FNV-1a `1e9975ff` in both
editions. The next two bytes are a `JSR` target: US calls `$5E4D` and JP calls
`$5E7D`. The dispatch table at `$410D` selects `$46B8` at opcode index 53
in both editions.

The current US Stage-2 listing labels `$5E4D` as a palette routine that
selects VCE index `$E0` and transfers 64 bytes from `$5E5F` to `$0404`.
Linear Stage-2 mapping places the US target at `0x2c0bdd`; those authenticated
bytes do not match the listed routine. The JP call target `$5E7D` maps to
`0x2c02dd`; its bytes also do not match a corresponding verified palette
body. The US listing's 18-byte routine was not found in either complete
Track 02 BIN or Track 19 ISO. Each caller and target address share the same
HuC6280 `$4000..$5FFF` MPR2 window, and the caller does not remap MPR2 before
the call. A simple bank-window remap therefore does not explain the
discrepancy.

## Scope and next evidence

The common handler prefix and dispatch entry are authenticated; the US and
JP call targets are also present in their respective source bytes. They do
not prove that the currently listed `$5E4D` body or `Stage2TiaPayload5E5F`
belongs to either image or is executed. Do not publish this palette as native
game state, associate it with a startup/dungeon screen, or use it to admit
pixels.

Resolve the Stage-2 listing's provenance and any source-to-RAM copying or
byte transformation that could supply `$5E4D`, using exact JP/US Track 02
bytes and the caller's live memory mapping. Until then, native palette and
graphics presentation remain blocked at this route. This evidence is static
disassembly analysis only; Theron continues to run in Firestaff's native
C/SDL runtime.
