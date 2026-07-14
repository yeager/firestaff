#ifndef FIRESTAFF_REDMCSB_F0049_TEXT_MESSAGEAREA_PRINT_INTEGER_UNREFERENCED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0049_TEXT_MESSAGEAREA_PRINT_INTEGER_UNREFERENCED_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced.
 *
 * The original PC source formats its unsigned 16-bit argument backwards in
 * an eight-byte local buffer, then passes the resulting decimal suffix to
 * F0047_TEXT_MESSAGEAREA_PrintMessage. This bounded adapter delegates that
 * one source call and returns false without calling it when absent.
 */
typedef void (*redmcsb_f0049_print_message_fn)(
    void *context,
    int16_t text_color,
    const char *message);

bool F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
    redmcsb_f0049_print_message_fn print_message,
    void *context,
    int16_t text_color,
    uint16_t integer);

#ifdef __cplusplus
}
#endif

#endif
