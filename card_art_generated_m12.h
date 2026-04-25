#ifndef FIRESTAFF_CARD_ART_GENERATED_M12_H
#define FIRESTAFF_CARD_ART_GENERATED_M12_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* gameId;
    int width;
    int height;
    const unsigned char* rgb;
} M12_GeneratedCardArt;

const M12_GeneratedCardArt* M12_GeneratedCardArt_Find(const char* gameId);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CARD_ART_GENERATED_M12_H */
