#ifndef FIRESTAFF_REDMCSB_F0050_TEXT_MESSAGEAREA_PRINT_SPACE_UNREFERENCED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0050_TEXT_MESSAGEAREA_PRINT_SPACE_UNREFERENCED_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced.
 *
 * The source emits one white space through F0047_TEXT_MESSAGEAREA_PrintMessage.
 * This bounded adapter delegates that one source call to print_message and
 * returns false without calling it when no delegate is supplied.
 */
typedef void (*redmcsb_f0050_print_message_fn)(
    void *context,
    int16_t text_color,
    const char *message);

bool F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
    redmcsb_f0050_print_message_fn print_message,
    void *context);

#define F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced \
    F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34

const char *redmcsb_f0050_text_messagearea_print_space_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
