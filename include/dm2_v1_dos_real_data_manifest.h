#ifndef DM2_V1_DOS_REAL_DATA_MANIFEST_H
#define DM2_V1_DOS_REAL_DATA_MANIFEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Byte-verified real-data manifest for DM2 (Skullkeep) DOS English.
 * Fingerprinted 2026-08-07 against the extracted shipping install
 * `Dungeon-Master-II-Skullkeep_DOS_EN.zip`.
 *
 * User backup files (`data/sksaveN.bak`) are intentionally excluded
 * — those are per-user save-backups, not shipping payloads.
 *
 * FORMAT NOTES:
 *   * skull.exe is the main game binary — DOS/4GW-extended
 *     LE-format 32-bit application (dispatched via dm2.bat, which
 *     invokes dos4gw.exe).
 *   * data/graphics.dat uses signature word 0x8005 (extended v5,
 *     5624 assets) — the DM2 DOS variant of the ext_v4 format the
 *     FM Towns disc ships. Both share the 4-byte-per-record header
 *     stride; ext walker handles both via
 *     `dm2_v1_fmtowns_graphics_dat_ext_walker`.
 *   * data/dungeon.dat is 39437 bytes — DM2's own dungeon format,
 *     larger than DM1's ~33 KB.
 *   * intro / end / splash are large multi-frame animation blobs.
 *   * hmidrv.386, hmimdrv.386, hmidet.386 are HMI (Human Machine
 *     Interfaces) sound-driver LE .386 modules for the DOS/4GW
 *     host — sound.
 */

typedef struct dm2_v1_dos_file_fp {
    const char *name;
    size_t      size_bytes;
    uint8_t     sha256[32];
} dm2_v1_dos_file_fp_t;

#define DM2_V1_DOS_FILE_COUNT 30

extern const dm2_v1_dos_file_fp_t dm2_v1_dos_files[DM2_V1_DOS_FILE_COUNT];

const dm2_v1_dos_file_fp_t *dm2_v1_dos_file_fp_lookup_pc34(const char *name);

int dm2_v1_dos_file_fp_matches_pc34(
    const char *name, size_t actual_size,
    const uint8_t sha256_actual[32]);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_DOS_REAL_DATA_MANIFEST_H */
