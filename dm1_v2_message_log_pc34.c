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

/* V2 Message Log — ring buffer for combat/event messages */

#define V2_MSGLOG_MAX 128
#define V2_MSGLOG_TEXT 96

typedef struct {
    char text[V2_MSGLOG_TEXT];
    uint32_t color;
    uint32_t tick;
    float alpha; /* for fade-out */
} V2_MsgLogEntry;

static V2_MsgLogEntry g_msglog[V2_MSGLOG_MAX];
static int g_msglog_head = 0;
static int g_msglog_count = 0;

void v2_msglog_add(const char *text, uint32_t color, uint32_t tick) {
    V2_MsgLogEntry *e;
    int idx = (g_msglog_head + g_msglog_count) % V2_MSGLOG_MAX;
    if (g_msglog_count >= V2_MSGLOG_MAX) {
        g_msglog_head = (g_msglog_head + 1) % V2_MSGLOG_MAX;
    } else {
        g_msglog_count++;
    }
    e = &g_msglog[idx];
    if (text) {
        strncpy(e->text, text, V2_MSGLOG_TEXT - 1);
        e->text[V2_MSGLOG_TEXT - 1] = '\0';
    }
    e->color = color;
    e->tick = tick;
    e->alpha = 1.0f;
}

void v2_msglog_tick(float dt) {
    int i;
    for (i = 0; i < g_msglog_count; i++) {
        int idx = (g_msglog_head + i) % V2_MSGLOG_MAX;
        if (g_msglog[idx].alpha > 0.0f) {
            g_msglog[idx].alpha -= dt * 0.1f; /* fade over 10 seconds */
        }
    }
}

int v2_msglog_get_visible(const V2_MsgLogEntry **out, int max_count) {
    int i, count = 0;
    if (!out) return 0;
    for (i = g_msglog_count - 1; i >= 0 && count < max_count; i--) {
        int idx = (g_msglog_head + i) % V2_MSGLOG_MAX;
        if (g_msglog[idx].alpha > 0.05f) {
            out[count++] = &g_msglog[idx];
        }
    }
    return count;
}

void v2_msglog_clear(void) { g_msglog_count = 0; g_msglog_head = 0; }

