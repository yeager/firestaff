#ifndef FIRESTAFF_REDMCSB_F0048_TEXT_MESSAGEAREA_PRINT_CHARACTER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0048_TEXT_MESSAGEAREA_PRINT_CHARACTER_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0048_TEXT_MESSAGEAREA_PrintCharacter.
 *
 * The source builds a two-byte local string from the character and a trailing
 * NUL, then calls F0047_TEXT_MESSAGEAREA_PrintMessage exactly once.
 */
typedef void (*redmcsb_f0048_print_message_fn)(
    void *context,
    int16_t text_color,
    const char *message);

bool F0048_TEXT_MESSAGEAREA_PrintCharacter_PC34(
    redmcsb_f0048_print_message_fn print_message,
    void *context,
    int16_t text_color,
    char character);

#ifdef __cplusplus
}
#endif

#endif
