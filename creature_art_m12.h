#ifndef FIRESTAFF_CREATURE_ART_M12_H
#define FIRESTAFF_CREATURE_ART_M12_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_CREATURE_THUMB_WIDTH  = 64,
    M12_CREATURE_THUMB_HEIGHT = 42,
    M12_CREATURE_COUNT        = 5
};

typedef struct {
    const char* name;
    unsigned char pixels[M12_CREATURE_THUMB_WIDTH * M12_CREATURE_THUMB_HEIGHT];
    int loaded;
} M12_CreatureThumb;

typedef struct {
    M12_CreatureThumb creatures[M12_CREATURE_COUNT];
    int selectedIndex;
    int anyLoaded;
} M12_CreatureArtState;

/* Initialize and attempt to load creature thumbnails from dataDir.
   Picks a random creature index using the provided seed. */
void M12_CreatureArt_Init(M12_CreatureArtState* state,
                          const char* dataDir,
                          unsigned int seed);

/* Returns 1 if a creature thumbnail is available for display. */
int M12_CreatureArt_HasSelection(const M12_CreatureArtState* state);

/* Get the name of the selected creature (e.g. "RED DRAGON"). */
const char* M12_CreatureArt_GetSelectedName(const M12_CreatureArtState* state);

/* Blit the selected creature thumbnail into a framebuffer region.
   Pixels with value 0 are treated as transparent.
   The thumbnail is scaled to fill (dstW x dstH). */
void M12_CreatureArt_Draw(const M12_CreatureArtState* state,
                          unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int dstX,
                          int dstY,
                          int dstW,
                          int dstH);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CREATURE_ART_M12_H */
