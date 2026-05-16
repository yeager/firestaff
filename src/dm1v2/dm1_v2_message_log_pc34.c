#include "dm1_v2_message_log_pc34.h"

static M11_V2_LogEntry log_entries[256];
static uint16_t log_count = 0;
static uint16_t log_scroll = 0;
static bool log_visible = true;
static uint32_t log_tick_counter = 0;

void v2_log_init(void) {
    memset(log_entries, 0, sizeof(log_entries));
    log_count = 0;
    log_scroll = 0;
    log_visible = true;
    log_tick_counter = 0;
}

void v2_log_add(const char* text, uint8_t color, M11_V2_LogCategory cat) {
    if (!text) return;
    uint16_t idx = log_count % 256;
    strncpy(log_entries[idx].text, text, 127);
    log_entries[idx].text[127] = '\0';
    log_entries[idx].color = color;
    log_entries[idx].cat = cat;
    log_entries[idx].tick = log_tick_counter++;
    if (log_count < 256) {
        log_count++;
    }
    log_scroll = log_count > 0 ? log_count - 1 : 0;
}

void v2_log_scroll_up(void) {
    if (log_scroll > 0) {
        log_scroll--;
    }
}

void v2_log_scroll_down(void) {
    if (log_scroll < log_count - 1) {
        log_scroll++;
    }
}

void v2_log_toggle(void) {
    log_visible = !log_visible;
}

void v2_log_clear(void) {
    memset(log_entries, 0, sizeof(log_entries));
    log_count = 0;
    log_scroll = 0;
    log_tick_counter = 0;
}

void v2_log_render(uint8_t* fb, int w, int h, int lines) {
    if (!fb || !log_visible || lines <= 0 || w <= 0 || h <= 0) return;

    int draw_lines = lines;
    if (draw_lines > h) draw_lines = h;

    int start_idx = log_scroll;
    int end_idx = start_idx + draw_lines;
    if (end_idx > log_count) end_idx = log_count;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            fb[y * w + x] = 0;
        }
    }

    for (int i = start_idx; i < end_idx; i++) {
        int y = i - start_idx;
        const char* txt = log_entries[i].text;
        int x = 0;
        while (txt[x] != '\0' && x < w) {
            fb[y * w + x] = (uint8_t)txt[x];
            x++;
        }
    }
}

