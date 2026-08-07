#include "dm1_v2_message_log_pc34.h"

/* ReDMCSB owns the message-area queue, source glyphs and presentation. This
 * compatibility API must not retain a duplicate log, a host font or pixels. */

void v2_log_init(void) {
}

void v2_log_add(const char* text, uint8_t color, M11_V2_LogCategory cat) {
    (void)text;
    (void)color;
    (void)cat;
}

void v2_log_scroll_up(void) {
}

void v2_log_scroll_down(void) {
}

void v2_log_toggle(void) {
}

void v2_log_clear(void) {
}

void v2_log_render(uint8_t* fb, int w, int h, int lines) {
    (void)fb;
    (void)w;
    (void)h;
    (void)lines;
}
