#ifndef DM1_V1_DOS_PC34_REAL_DATA_MANIFEST_H
#define DM1_V1_DOS_PC34_REAL_DATA_MANIFEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Byte-verified real-data manifest for DM1 DOS PC 3.4 (English).
 * Fingerprinted 2026-08-07 against the shipping disc:
 *   `Dungeon-Master_DOS_EN_Version-34.zip` (contents extracted).
 *
 * DM1 DOS 3.4 is the canonical PC release referenced throughout
 * the Firestaff codebase (see CLAUDE.md). This manifest lets a
 * runtime that has been pointed at a data directory positively
 * identify each file as an authentic PC 3.4 payload before binding
 * it to any consumer.
 *
 * FORMAT NOTES (byte-verified 2026-08-07):
 *   * GRAPHICS.DAT starts with signature word 0x8001 (little-endian
 *     `01 80`) — the SAME signature the CSB FM Towns disc uses.
 *     Word1 = 0x02c9 = 713. This is NOT the "575-asset DM1 legacy
 *     format" some prior documentation claimed; the DOS 3.4 payload
 *     uses an FTL-pack container.
 *   * SONG.DAT starts with the same `01 80` prefix (FTL-pack).
 *   * DUNGEON.DAT starts with `63 00 fb 2f …` — standard DM1
 *     dungeon header (word0 = 99, checksum-like word follows).
 *   * TITLE and END are Electronic Arts "ANIM" (IFF-adjacent)
 *     320x200 4-plane animation payloads (`41 4e 00 08 … 01 40
 *     00 c8 00 04 …` prefix), NOT DOS executables.
 *   * DM.EXE, STATS.EXE, INSTALL.EXE, and the ANIM/EGA/FIRES/
 *     IBMIO/SELECTOR/SWOOSH/TANDY/VGA overlay files are all
 *     standard DOS MZ executables (`4d 5a`).
 */

typedef struct dm1_v1_dos_pc34_file_fp {
    const char *name;          /* filename inside the disc root */
    size_t      size_bytes;
    uint8_t     sha256[32];    /* big-endian SHA-256 digest */
} dm1_v1_dos_pc34_file_fp_t;

#define DM1_V1_DOS_PC34_FILE_COUNT 16

extern const dm1_v1_dos_pc34_file_fp_t
    dm1_v1_dos_pc34_files[DM1_V1_DOS_PC34_FILE_COUNT];

/* Look up a fingerprint entry by exact filename. Returns NULL if
 * no entry matches. */
const dm1_v1_dos_pc34_file_fp_t *
dm1_v1_dos_pc34_file_fp_lookup_pc34(const char *name);

/* Compare a candidate blob against the fingerprint entry named
 * `name`. Returns 1 iff the size and SHA-256 match. `sha256_actual`
 * is the SHA-256 already computed by the caller (this module does
 * NOT ship a SHA-256 implementation — that lives elsewhere in the
 * codebase). Returns 0 on any mismatch or if `name` is unknown. */
int dm1_v1_dos_pc34_file_fp_matches_pc34(
    const char *name, size_t actual_size,
    const uint8_t sha256_actual[32]);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_DOS_PC34_REAL_DATA_MANIFEST_H */
