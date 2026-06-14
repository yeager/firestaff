/*
 * csb_v1_save_import_path_pc34_compat.h
 *
 * CSB V1 Champion Transfer/Import (Champions GAP 3, HoC
 * delta).  Source-locked per M13_PLAN.md:280-294 (HoC
 * delta between DM1 and CSB) and DEFS.H:1289
 * (CSBGAME.DAT magic byte).
 *
 * v1 (2026-06-14): bounded implementation that detects the
 * save-file variant by magic bytes and dispatches to the
 * right load path.  DM1 PC 3.4 saves are 1:1 compatible;
 * CSB v2.x saves use a different layout.  Full CSB-specific
 * import support is OPEN-OMFATTANDE; this helper is the
 * dispatcher + a stub for the CSB-specific path that
 * gracefully returns a "not yet implemented" sentinel.
 */
#ifndef REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H
#define REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Save-file variant detection.  The first 8 bytes of
 * the DM1 / CSB save file are the magic field.  v1 PC 3.4
 * saves use "RDMCSB15" (8 bytes, no NUL); CSB v2.x
 * saves have a different magic.  The HoC delta between
 * DM1 and CSB is precisely this magic byte + the
 * version field that follows. */
typedef enum {
    CSB_V1_SAVE_VARIANT_DM1_PC34 = 0,
    CSB_V1_SAVE_VARIANT_CSB_V20 = 1,
    CSB_V1_SAVE_VARIANT_CSB_V21 = 2,
    CSB_V1_SAVE_VARIANT_UNKNOWN = -1
} CSB_V1_SaveVariant;

/* Detect the save-file variant from the first 16 bytes
 * of the file.  Returns CSB_V1_SAVE_VARIANT_UNKNOWN for
 * unrecognized magic. */
CSB_V1_SaveVariant csb_v1_detect_save_variant(
    const unsigned char* header, int headerLen);

/* CSB-specific import path.  This is OPEN-OMFATTANDE
 * (the full CSB save layout differs from DM1 in many
 * fields: champion roster length, party stat bytes, etc.).
 * v1 returns 0 (= not implemented) and lets the caller
 * fall back to the DM1 1:1 loader.  When the helper
 * is extended with the CSB-specific code, callers should
 * check the return code and treat 0 as 'use DM1 path'. */
int  csb_v1_import_csb_save(const char* path);
int  csb_v1_save_import_path_implemented(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H */
