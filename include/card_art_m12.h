#ifndef FIRESTAFF_CARD_ART_M12_H
#define FIRESTAFF_CARD_ART_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_CARD_ART_PATH_CAPACITY = 512
};

typedef struct {
    char gameId[8];
    char resolvedPath[M12_CARD_ART_PATH_CAPACITY];
    char fileName[64];
    char slotLabel[64];
    int hasImageFile;
    int hasExternalFile;
} M12_GameCardArt;

void M12_CardArt_Resolve(M12_GameCardArt* art,
                         const char* gameId,
                         const char* dataDir);
int M12_CardArt_HasImage(const M12_GameCardArt* art);
int M12_CardArt_HasExternalFile(const M12_GameCardArt* art);
const char* M12_CardArt_GetResolvedPath(const M12_GameCardArt* art);
const char* M12_CardArt_GetFileName(const M12_GameCardArt* art);
const char* M12_CardArt_GetSlotLabel(const M12_GameCardArt* art);
void M12_CardArt_DrawPreview(const M12_GameCardArt* art,
                             unsigned char* framebuffer,
                             int framebufferWidth,
                             int framebufferHeight,
                             int x,
                             int y,
                             int w,
                             int h,
                             unsigned char paper,
                             unsigned char ink,
                             unsigned char accent,
                             unsigned char glow);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CARD_ART_M12_H */
