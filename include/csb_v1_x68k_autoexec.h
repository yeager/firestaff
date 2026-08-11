#ifndef FIRESTAFF_CSB_V1_X68K_AUTOEXEC_H
#define FIRESTAFF_CSB_V1_X68K_AUTOEXEC_H
#include <stddef.h>
#include <stdint.h>
#include "csb_v1_x68k_hdm.h"
#define CSB_V1_X68K_AUTOEXEC_MAX_COMMANDS 8u
typedef struct { CSB_V1_X68kHdmReceipt media; uint8_t command_count; char commands[CSB_V1_X68K_AUTOEXEC_MAX_COMMANDS][32]; int dos_eof_terminated; int host_execution_permitted; } CSB_V1_X68kAutoexecReceipt;
/* Parse the original Human68k AUTOEXEC.BAT command order. This is a source
 * receipt only; neither CK.R nor VIDSET.X is executed by the host. */
int csb_v1_x68k_autoexec_receipt(const uint8_t *hdm,size_t hdm_size,CSB_V1_X68kAutoexecReceipt*out);
#endif
