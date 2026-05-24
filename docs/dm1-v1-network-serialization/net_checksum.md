# DM1 V1 — Checksum / Error Detection

## Source-locked
ReDMCSB: READWRIT.C (lines 191–240), SAVEHEAD.C (lines 16–107), DEFS.H:500–501
Firestaff: src/dm1/dm1_v1_save_load.c, src/engine/firestaff_save.c

## Checksum Algorithm — Two-Layer Design

### Layer 1: Header Checksum (SAVEHEAD.C:35–52)
Validates the 512-byte save header in two halves:

**First 256 bytes (words 0–127):**
Iterate 32 times, accumulating into checksum:
  sum += word[n]; sum ^= word[n+1]; sum -= word[n+2]; sum ^= word[n+3]
(Repeating 4-word pattern gives 128 words total)

**Second 256 bytes (words 128–255):**
Deobfuscate first via F0417_SAVEUTIL_GetChecksumAndObfuscate with platform key,
then sum all int16_t values: checksum = sum(words[128..255])

**Validation:** expected_checksum == computed_checksum

### Layer 2: Obfuscation-based Integrity (READWRIT.C:191–240)
F0417_SAVEUTIL_GetChecksumAndObfuscate simultaneously:
1. Computes cumulative checksum: sum += buffer[i] + (buffer[i] XOR key)
2. XORs each word with key: buffer[i] ^= key
3. Key increments by word_count each iteration

This is reversible: calling it again with same key deobfuscates and recomputes same checksum.

Platform-specific keys:
  DM Atari ST: C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX = 10 (Noise[10])
  CSB/Amiga/PC: C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX = 29 (Noise[29])

### Layer 3: Per-Section Checksums (LOADSAVE.C:1632–1682)
16-key/16-checksum arrays in header. Each data section (champions, maps, things, text)
is written with its own key and checksum entry at its index in the Keys[]/Checksums[] arrays.

### Layer 4: Write Verification (READWRIT.C:159–168)
F0416_SAVEUTIL_IsWriteBytesSuccessful: verifies fwrite() returned exact byte count.
F0415_SAVEUTIL_IsReadBytesSuccessful: verifies fread() returned exact byte count.

## Firestaff Implementation
firestaff_save.c: FS_SAVE_HEADER with magic[4] (FS_SAVE_MAGIC), version, game_id,
level, party_x/y/dir, timestamp. Simple CRC approach via memcmp(magic,4).
dm1_v1_save_load.c: fuller reimplementation of the multi-layer DM1 scheme.

## Error Messages
LOADSAVE.C: G0547_pc_SAVEDGAMEDAMAGED — displayed when any checksum fails.
Firestaff: DM1_SAVE_ERROR_BAD_CRC — "CRC MISMATCH — SAVE DAMAGED"
