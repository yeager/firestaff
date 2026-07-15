/*
 * csb_v1_csbwin_graphics_signature_gate.h
 *
 * CSBWin SaveGame/Graphics signature contract for graphics.dat and
 * CSBgraphics.dat.  The input is the MD5 identity recorded by a real asset
 * cache; this module never opens a file by name or substitutes graphics.
 *
 * Source references:
 *   - CSBWin/data.cpp:1936-1964 Signature
 *   - CSBWin/Graphics.cpp:1760-1820 openGraphicsFile
 *   - CSBWin/Graphics.cpp:1838-1899 OpenCSBgraphicsFile
 */

#ifndef FIRESTAFF_CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_H
#define FIRESTAFF_CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD = 0,
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM = 1
} CSB_V1_CSBWinGraphicsSignatureKind;

typedef enum {
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK = 0,
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_MD5 = -2,
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_EXPECTED = -3,
    CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_RUNTIME = -4
} CSB_V1_CSBWinGraphicsSignatureGateResult;

typedef struct {
    uint32_t actual_signature1;
    uint32_t actual_signature2;
    uint32_t actual_folded_signature;
    int expected_signature_checked;
    int runtime_signature_checked;
} CSB_V1_CSBWinGraphicsSignatureReceipt;

/* Validate a lowercase or uppercase 32-character MD5 identity using the
 * little-endian first-eight-byte signature made by CSBWin Signature().
 * `expected_signature*` are EXTENDEDFEATURESBLOCK fields. `runtime_signature`
 * is the EDBT_RuntimeFileSignatures value from EXPOOL.  A mismatch is an
 * admission failure, never a request for replacement graphics. */
int csb_v1_csbwin_graphics_signature_gate_validate_md5(
    const char *md5_hex,
    CSB_V1_CSBWinGraphicsSignatureKind kind,
    uint32_t expected_signature1,
    uint32_t expected_signature2,
    uint32_t runtime_signature,
    uint32_t debugging_data,
    CSB_V1_CSBWinGraphicsSignatureReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
