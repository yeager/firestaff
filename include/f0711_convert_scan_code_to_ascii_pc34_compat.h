#ifndef FIRESTAFF_F0711_CONVERT_SCAN_CODE_TO_ASCII_PC34_COMPAT_H
#define FIRESTAFF_F0711_CONVERT_SCAN_CODE_TO_ASCII_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IO2.C:F0711_ConvertScanCodeToASCII for MEDIA709_I34E_I34M_P31J
 * delegates to the PC driver's IODRV_26 slot. IBMIO.C:F8092 implements that
 * slot with G8038/G8039's original 128-entry US scan-code tables. Bit 9
 * selects the uppercase table; only bits 0..6 select the table entry.
 */
enum {
    F0711_PC34_SCAN_CODE_MASK = 0x007F,
    F0711_PC34_UPPERCASE_MASK = 0x0200
};

int16_t f0711_convert_scan_code_to_ascii_pc34_compat(int16_t scan_code);

const char *f0711_convert_scan_code_to_ascii_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
