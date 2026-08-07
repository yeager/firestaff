#ifndef DM1_V1_FMTOWNS_TBIOS_ID_H
#define DM1_V1_FMTOWNS_TBIOS_ID_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked TBIOS (TownsOS BIOS) version fingerprint detector.
 *
 * TownsOS BIOS blobs (loaded from FMT_F20.ROM or captured from a
 * live FM Towns machine) embed a 32-byte identifier at a fixed
 * physical address: four 8-byte NUL-terminated string fields
 * naming the version, date, product ("towns"), and subsystem
 * ("tbios"). The identifier locations and version strings below
 * are byte-verified against Tsugaru emulator's `tbiosid.cpp`
 * physical-memory dumps (see docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md).
 *
 * Firestaff uses this identity to gate any TBIOS-dependent code
 * paths (currently: none live; the DM1 menu-render pipeline is
 * display-only). Once a TBIOS integration lands, the detector
 * lets the runtime confirm the host BIOS matches what the disc
 * expects — DM1 HMA-240 needs V31L31 (1992-10-16 or 1993-01-07);
 * refusing to launch against a mismatched TBIOS keeps the
 * source-lock guarantee.
 */

typedef enum {
    DM1_V1_FMTOWNS_TBIOS_UNKNOWN     = 0,
    DM1_V1_FMTOWNS_TBIOS_V31L22A     = 1,   /* 1989-03-08, base 0x1F750 */
    DM1_V1_FMTOWNS_TBIOS_V31L23A     = 2,   /* 1990-09-21, base 0x1F3D0 */
    DM1_V1_FMTOWNS_TBIOS_V31L31_90   = 3,   /* 1990-11-21, base 0x1F3D0 */
    DM1_V1_FMTOWNS_TBIOS_V31L31_91   = 4,   /* 1991-10-05, base 0x100000 */
    DM1_V1_FMTOWNS_TBIOS_V31L31_92   = 5,   /* 1992-10-16, base 0x100000 */
    DM1_V1_FMTOWNS_TBIOS_V31L31_93   = 6,   /* 1993-01-07, base 0x100000 */
    DM1_V1_FMTOWNS_TBIOS_V31L35      = 7,   /* 1993-10-15, base 0x100000 */
    DM1_V1_FMTOWNS_TBIOS_V31L35_94   = 8    /* 1994-12-03, base 0x100000 (FSC11) */
} dm1_v1_fmtowns_tbios_version_t;

typedef struct {
    dm1_v1_fmtowns_tbios_version_t version;
    char     version_str[9];   /* e.g. "V31L31" */
    char     date_str[9];      /* e.g. "92/10/16" */
    char     product_str[9];   /* always "towns"  */
    char     subsystem_str[9]; /* always "tbios"  */
    uint32_t identified_at_offset; /* file offset where match happened */
} dm1_v1_fmtowns_tbios_identity_t;

/* Scan `bios_bytes` (size `bios_size`) for a TBIOS identifier at
 * any of the shipping base addresses (0x1F750, 0x1F3D0, 0x100000).
 * On match, populate `out` with the recovered strings and version
 * enum. Returns 1 on identified match, 0 if no known version was
 * found (or if inputs are invalid). Fail-closed: unknown strings
 * classify as DM1_V1_FMTOWNS_TBIOS_UNKNOWN, not a guess. */
int dm1_v1_fmtowns_tbios_identify_pc34(
    const uint8_t                   *bios_bytes,
    size_t                           bios_size,
    dm1_v1_fmtowns_tbios_identity_t *out);

/* Return the required TBIOS version for the DM1 HMA-240 English
 * disc. The disc was released 1992, so V31L31_92 or later matches;
 * strictly-earlier versions may miss subfunctions DM1 uses. */
dm1_v1_fmtowns_tbios_version_t dm1_v1_fmtowns_tbios_required_for_dm1_hma240_pc34(void);

/* Return non-zero if `have` is at least as new as `need` (in the
 * enum ordering). Simple gate for admission checks. */
int dm1_v1_fmtowns_tbios_meets_pc34(
    dm1_v1_fmtowns_tbios_version_t have,
    dm1_v1_fmtowns_tbios_version_t need);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TBIOS_ID_H */
