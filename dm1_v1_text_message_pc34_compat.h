#ifndef FIRESTAFF_DM1_V1_TEXT_MESSAGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TEXT_MESSAGE_PC34_COMPAT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Based on ReDMCSB TEXT.C F0046_TEXT_Print, F0047_TEXT_DrawCharacter */
#define M11_TEXT_MAX_MESSAGES 8
#define M11_TEXT_MAX_LENGTH 64
#define M11_TEXT_DISPLAY_TICKS 150
typedef struct { char text[M11_TEXT_MAX_LENGTH]; int ticksRemaining; int priority; int x; int y; int color; } M11_TextMessage;
typedef struct { M11_TextMessage messages[M11_TEXT_MAX_MESSAGES]; int messageCount; int fontWidth; int fontHeight; } M11_TextState;
void m11_text_init(M11_TextState* s);
void m11_text_show(M11_TextState* s, const char* text, int x, int y, int color, int priority);
void m11_text_show_centered(M11_TextState* s, const char* text, int y, int color);
void m11_text_tick(M11_TextState* s, int tickMs);
int m11_text_get_active_count(const M11_TextState* s);
const M11_TextMessage* m11_text_get_message(const M11_TextState* s, int index);
#ifdef __cplusplus
}
#endif
#endif
