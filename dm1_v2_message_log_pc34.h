#ifndef FIRESTAFF_DM1_V2_MESSAGE_LOG_PC34_H
#define FIRESTAFF_DM1_V2_MESSAGE_LOG_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum M11_V2_LogCategory {
    V2_LOG_COMBAT,
    V2_LOG_SYSTEM,
    V2_LOG_LORE,
    V2_LOG_ITEM,
    V2_LOG_SPELL
} M11_V2_LogCategory;

typedef struct M11_V2_LogEntry {
    char text[128];
    uint8_t color;
    uint32_t tick;
    M11_V2_LogCategory cat;
} M11_V2_LogEntry;

void v2_log_init(void);
void v2_log_add(const char* text, uint8_t color, M11_V2_LogCategory cat);
void v2_log_scroll_up(void);
void v2_log_scroll_down(void);
void v2_log_toggle(void);
void v2_log_clear(void);
void v2_log_render(uint8_t* fb, int w, int h, int lines);

#endif
