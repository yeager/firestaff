/* DM1 V1 Text/Message Display — ReDMCSB TEXT.C F0046/F0047. Q3.6+Opus. */
#include "dm1_v1_text_message_pc34_compat.h"
#include <string.h>

void m11_text_init(M11_TextState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->fontWidth = 8; s->fontHeight = 8;
}

void m11_text_show(M11_TextState* s, const char* text, int x, int y, int color, int priority) {
    if (!s || !text) return;
    int idx = -1;
    if (s->messageCount < M11_TEXT_MAX_MESSAGES) {
        idx = s->messageCount;
    } else {
        int lowest = 0x7FFFFFFF;
        for (int i = 0; i < M11_TEXT_MAX_MESSAGES; i++) {
            if (s->messages[i].priority < lowest) { lowest = s->messages[i].priority; idx = i; }
        }
    }
    if (idx < 0) return;
    strncpy(s->messages[idx].text, text, M11_TEXT_MAX_LENGTH - 1);
    s->messages[idx].text[M11_TEXT_MAX_LENGTH - 1] = 0;
    s->messages[idx].ticksRemaining = M11_TEXT_DISPLAY_TICKS;
    s->messages[idx].x = x; s->messages[idx].y = y;
    s->messages[idx].color = color; s->messages[idx].priority = priority;
    if (s->messageCount < M11_TEXT_MAX_MESSAGES) s->messageCount++;
}

void m11_text_show_centered(M11_TextState* s, const char* text, int y, int color) {
    if (!s || !text) return;
    int len = (int)strlen(text);
    int x = (224 - len * s->fontWidth) / 2;
    m11_text_show(s, text, x, y, color, 0);
}

void m11_text_tick(M11_TextState* s, int tickMs) {
    if (!s) return;
    int w = 0;
    for (int r = 0; r < s->messageCount; r++) {
        s->messages[r].ticksRemaining -= tickMs;
        if (s->messages[r].ticksRemaining > 0) {
            if (w != r) s->messages[w] = s->messages[r];
            w++;
        }
    }
    s->messageCount = w;
}

int m11_text_get_active_count(const M11_TextState* s) { return s ? s->messageCount : 0; }

const M11_TextMessage* m11_text_get_message(const M11_TextState* s, int index) {
    if (!s || index < 0 || index >= s->messageCount) return (void*)0;
    return &s->messages[index];
}
