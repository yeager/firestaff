#ifndef FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_F0169_F0170_F0171_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RANDOM_ORNAMENT_F0169_F0170_F0171_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0171_RANDOM_ORNAMENT_RANDOM_VALUE_COUNT_PC34 = 4
};

typedef struct {
    int valid;
    int ornamentIndex;
    int candidateOrdinal;
    size_t admittedCandidateCount;
} DM1_V1_RandomOrnamentIndexF0169Pc34;

typedef struct {
    int valid;
    int ornamentIndex;
    int ornamentOrdinal;
    size_t admittedCandidateCount;
} DM1_V1_RandomOrnamentOrdinalF0170Pc34;

typedef struct {
    const uint8_t *wallOrnamentEnabledByIndex;
    size_t wallOrnamentCount;
    const uint8_t *floorOrnamentEnabledByIndex;
    size_t floorOrnamentCount;
} DM1_V1_RandomOrnamentCatalogF0169F0170Pc34;

typedef struct {
    const DM1_V1_RandomOrnamentCatalogF0169F0170Pc34 *catalog;
    uint8_t rawSquare;
    int direction;
    const uint16_t *randomValues;
    size_t randomValueCount;
} DM1_V1_SetSquareAspectRandomOrnamentsF0171InputPc34;

typedef struct {
    int valid;
    int randomCalls;
    int wroteRightWallOrnament;
    int wroteFrontWallOrnament;
    int wroteLeftWallOrnament;
    int wroteFloorOrnament;
    int aspect[5];
} DM1_V1_SetSquareAspectRandomOrnamentsF0171ResultPc34;

const char *DM1_V1_F0169_F0170_F0171_SourceEvidencePc34(void);

int F0169_DUNGEON_GetRandomOrnamentIndex(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentIndexF0169Pc34 *out);

int F0170_DUNGEON_GetRandomOrnamentOrdinal(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 *out);

int F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
    const DM1_V1_SetSquareAspectRandomOrnamentsF0171InputPc34 *input,
    DM1_V1_SetSquareAspectRandomOrnamentsF0171ResultPc34 *out);

int DM1_V1_Dungeon_GetRandomOrnamentIndexF0169Pc34Compat(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentIndexF0169Pc34 *out);

int DM1_V1_Dungeon_GetRandomOrnamentOrdinalF0170Pc34Compat(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
