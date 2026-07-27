#ifndef FIRESTAFF_STARTUP_INTRO_M12_H
#define FIRESTAFF_STARTUP_INTRO_M12_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_STARTUP_INTRO_WIDTH = 480,
    M12_STARTUP_INTRO_HEIGHT = 270,
    M12_STARTUP_INTRO_DURATION_MS = 6000
};

/* Draw the standalone Firestaff product intro into a caller-owned RGBA frame. */
void M12_StartupIntro_Render(unsigned char* rgba,
                             int width,
                             int height,
                             uint32_t elapsedMs,
                             uint32_t durationMs,
                             const char* version);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_STARTUP_INTRO_M12_H */
