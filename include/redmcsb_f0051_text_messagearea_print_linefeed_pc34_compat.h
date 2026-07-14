#ifndef FIRESTAFF_REDMCSB_F0051_TEXT_MESSAGEAREA_PRINT_LINEFEED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0051_TEXT_MESSAGEAREA_PRINT_LINEFEED_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0051_TEXT_MESSAGEAREA_PrintLineFeed.
 *
 * The source emits the Graphic562 line-feed string through F0047 with the
 * black text color. This bounded adapter makes exactly that one callback,
 * returning false without a delegate.
 */
typedef void (*redmcsb_f0051_print_message_fn)(
    void *context,
    int16_t text_color,
    const char *message);

bool F0051_TEXT_MESSAGEAREA_PrintLineFeed_PC34(
    redmcsb_f0051_print_message_fn print_message,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
