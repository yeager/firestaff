/*
 * ReDMCSB TEXT.C F0819_TEXT_MESSAGEAREA_PrintMessageAsJapanese, PC-98 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0819_TEXT_MESSAGEAREA_PRINT_MESSAGE_AS_JAPANESE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0819_TEXT_MESSAGEAREA_PRINT_MESSAGE_AS_JAPANESE_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_text_message_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Preserves the PC-98 TEXT.C convention: prepend ESC (0x1B) before
 * dispatching the original byte string to F0047's compatible message path.
 * As in the source, the caller must supply a string that fits the 100-byte
 * local message buffer after the prefix.
 */
void redmcsb_f0819_text_messagearea_print_message_as_japanese_pc34_compat(
    DM1_V1_TextMessageState *state,
    int16_t text_color,
    char *string);

const char *redmcsb_f0819_text_messagearea_print_message_as_japanese_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
