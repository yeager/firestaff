#ifndef FIRESTAFF_DM1_V2_SCREENSHOT_PC34_H
#define FIRESTAFF_DM1_V2_SCREENSHOT_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void v2_screenshot_init(void);
int v2_screenshot_capture(uint8_t* fb, int w, int h, uint32_t* palette, int palette_size, const char* path);
void v2_screenshot_auto_name(char* buf, int bufsize);

#ifdef __cplusplus
}
#endif

#endif
