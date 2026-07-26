# pass1092 — DM1 V1 F0803/F0433 New-Game Save Ownership (PC 3.4)

## Claim

F0803 (Save and Quit vanilla export) and F0433 (save serializer) ownership
is proven by existing round-trip tests. A synthetic world survives
F0803 → F0796 import with no manifest marker, and F0802 preserves
byte-identical C3/C4 event/heap data through a full export → import cycle.

## Evidence

### F0803 vanilla export (LOADSAVE.C F0433 path)

- `test_dm1_v1_original_save_pc34_handoff.c` line 2352: calls
  `F0803_SAVEGAME_ExportVanillaPC34FromWorld_Compat`, verifies result
  contains no Firestaff manifest (`MANIFEST_ERR_NOT_PRESENT`).
- The exported bytes are a plain PC 3.4 DMSAVE.DAT: 512-byte header with
  F0417 obfuscation, 5 length-prefixed parts (GLOBAL_DATA, ACTIVE_GROUP,
  PARTY, EVENTS, TIMELINE), optional portraits, optional dungeon tail.

### F0433 round-trip (F0802 → F0435 → F0802)

- `test_dm1_v1_original_save_pc34_handoff.c` line 2347-2377: F0802
  export → handoff_bytes import → compare C3/C4 raw event and heap bytes.
  Both byte count and content are identical.

### F0417/F0418 obfuscation primitive

- `test_dm1_v1_savegame_pc34_native_export_pc34_compat.c`: F0417 is its
  own inverse, F0418 read-only checksum matches, header round-trips,
  per-part checksums validate, F0797 error strings are stable.

### LSV-02 per-game manifest gate

- `test_dm1_v1_savegame_pc34_native_export_pc34_compat.c`: DM1 export
  is stamped with gameCode=DM1, CSB manifest is refused, vanilla
  PC 3.4 file (no manifest) is accepted under legacy interop, magic
  tampering flips the verdict.

## Source anchors

- ReDMCSB LOADSAVE.C F0433 (save serializer)
- ReDMCSB LOADSAVE.C F0435 (load deserializer)
- ReDMCSB READWRIT.C F0417 (obfuscation)
- ReDMCSB READWRIT.C F0418 (checksum)
- ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)

## Verdict

F0803/F0433 ownership is source-locked. The vanilla PC 3.4 save format
is fully exercised by synthetic round-trip tests. Real-corpus admission
(Q-DM1-01) remains blocked on an original DMSAVE.DAT from the DOS engine.
