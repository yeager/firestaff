#ifndef FIRESTAFF_DM1_V2_MESSAGE_LOG_PC34_H
#define FIRESTAFF_DM1_V2_MESSAGE_LOG_PC34_H

#include <stdint.h>

typedef enum M11_V2_LogCategory {
    V2_LOG_COMBAT,
    V2_LOG_SYSTEM,
    V2_LOG_LORE,
    V2_LOG_ITEM,
    V2_LOG_SPELL
} M11_V2_LogCategory;

/* Compatibility-only. PC34 message text, glyphs and placement remain in the
 * V1 message-area route; these entries retain no duplicate text or pixels. */
void v2_log_init(void);
void v2_log_add(const char* text, uint8_t color, M11_V2_LogCategory cat);
void v2_log_scroll_up(void);
void v2_log_scroll_down(void);
void v2_log_toggle(void);
void v2_log_clear(void);
void v2_log_render(uint8_t* fb, int w, int h, int lines);

#endif
