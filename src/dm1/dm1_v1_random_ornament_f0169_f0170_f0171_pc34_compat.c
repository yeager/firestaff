#include "dm1_v1_random_ornament_f0169_f0170_f0171_pc34_compat.h"

#include "dm1_v1_dungeon_square_structs_pc34_compat.h"

#include <string.h>

static const uint8_t kWallFlagForDirection[4] = {
    DM1_WALL_NORTH_RANDOM_ORN,
    DM1_WALL_EAST_RANDOM_ORN,
    DM1_WALL_SOUTH_RANDOM_ORN,
    DM1_WALL_WEST_RANDOM_ORN
};

const char *DM1_V1_F0169_F0170_F0171_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:2318 F0169_DUNGEON_GetRandomOrnamentIndex scans the "
        "current map ornament enable table and returns a zero-based index\n"
        "DUNGEON.C:2362 F0170_DUNGEON_GetRandomOrnamentOrdinal converts "
        "the selected index to the one-based ordinal used by dungeon view\n"
        "DUNGEON.C:2369 F0171_DUNGEON_SetSquareAspectRandomWallOrnament"
        "Ordinals writes M551/M552/M553 wall slots and M558 floor slot for "
        "random-enabled wall/corridor squares\n"
        "DEFS.H M551/M552/M553/M558 and DUNGEON.C F0172 feed F0107/F0108 "
        "wall and floor ornament rendering";
}

static void clear_index_result(DM1_V1_RandomOrnamentIndexF0169Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->ornamentIndex = -1;
        out->candidateOrdinal = -1;
    }
}

static void clear_ordinal_result(DM1_V1_RandomOrnamentOrdinalF0170Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->ornamentIndex = -1;
    }
}

static size_t count_enabled(const uint8_t *enabled, size_t count)
{
    size_t i;
    size_t enabledCount = 0;

    if (!enabled) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (enabled[i]) {
            ++enabledCount;
        }
    }
    return enabledCount;
}

int F0169_DUNGEON_GetRandomOrnamentIndex(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentIndexF0169Pc34 *out)
{
    size_t enabledCount;
    size_t wantedOrdinal;
    size_t seen = 0;
    size_t i;

    clear_index_result(out);
    enabledCount = count_enabled(ornamentEnabledByIndex, ornamentCount);
    if (!out || enabledCount == 0) {
        return 0;
    }

    wantedOrdinal = (size_t)(randomValue % enabledCount);
    for (i = 0; i < ornamentCount; ++i) {
        if (!ornamentEnabledByIndex[i]) {
            continue;
        }
        if (seen == wantedOrdinal) {
            out->valid = 1;
            out->ornamentIndex = (int)i;
            out->candidateOrdinal = (int)wantedOrdinal;
            out->admittedCandidateCount = enabledCount;
            return 1;
        }
        ++seen;
    }
    return 0;
}

int F0170_DUNGEON_GetRandomOrnamentOrdinal(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 *out)
{
    DM1_V1_RandomOrnamentIndexF0169Pc34 indexResult;

    clear_ordinal_result(out);
    if (!out ||
        !F0169_DUNGEON_GetRandomOrnamentIndex(
            ornamentEnabledByIndex,
            ornamentCount,
            randomValue,
            &indexResult)) {
        return 0;
    }

    out->valid = 1;
    out->ornamentIndex = indexResult.ornamentIndex;
    out->ornamentOrdinal = indexResult.ornamentIndex + 1;
    out->admittedCandidateCount = indexResult.admittedCandidateCount;
    return 1;
}

static int relative_direction(int direction, int turn)
{
    return (direction + turn) & 3;
}

static int wall_flag_for_relative_direction(int direction, int turn)
{
    return kWallFlagForDirection[relative_direction(direction, turn)];
}

static void write_wall_ordinal(
    const DM1_V1_RandomOrnamentCatalogF0169F0170Pc34 *catalog,
    int *aspect,
    int aspectSlot,
    int *writeFlag,
    const uint16_t *randomValues,
    size_t randomValueCount,
    int *randomCallIndex)
{
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 ordinal;
    uint16_t randomValue = 0;

    if ((size_t)*randomCallIndex < randomValueCount) {
        randomValue = randomValues[*randomCallIndex];
    }
    ++(*randomCallIndex);
    if (F0170_DUNGEON_GetRandomOrnamentOrdinal(
            catalog->wallOrnamentEnabledByIndex,
            catalog->wallOrnamentCount,
            randomValue,
            &ordinal)) {
        aspect[aspectSlot] = ordinal.ornamentOrdinal;
        *writeFlag = 1;
    }
}

int F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
    const DM1_V1_SetSquareAspectRandomOrnamentsF0171InputPc34 *input,
    DM1_V1_SetSquareAspectRandomOrnamentsF0171ResultPc34 *out)
{
    int element;
    int flags;
    int randomCallIndex = 0;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    for (element = 0; element < DM1_SQA_COUNT_V1; ++element) {
        out->aspect[element] = 0;
    }

    if (!input || !input->catalog || input->direction < 0 ||
        input->direction > 3 || !input->randomValues ||
        input->randomValueCount <
            DM1_V1_F0171_RANDOM_ORNAMENT_RANDOM_VALUE_COUNT_PC34) {
        return 0;
    }

    element = DM1_SQUARE_TYPE(input->rawSquare);
    flags = DM1_SQUARE_FLAGS(input->rawSquare);
    out->aspect[DM1_SQA_ELEMENT] =
        dm1_classify_square_aspect_element(input->rawSquare, input->direction);
    out->valid = 1;

    if (element == DM1_ELEMENT_WALL) {
        if (flags & wall_flag_for_relative_direction(input->direction, 1)) {
            write_wall_ordinal(input->catalog,
                               out->aspect,
                               DM1_SQA_RIGHT_WALL_ORN_ORD,
                               &out->wroteRightWallOrnament,
                               input->randomValues,
                               input->randomValueCount,
                               &randomCallIndex);
        }
        if (flags & wall_flag_for_relative_direction(input->direction, 0)) {
            write_wall_ordinal(input->catalog,
                               out->aspect,
                               DM1_SQA_FRONT_WALL_ORN_ORD,
                               &out->wroteFrontWallOrnament,
                               input->randomValues,
                               input->randomValueCount,
                               &randomCallIndex);
        }
        if (flags & wall_flag_for_relative_direction(input->direction, 3)) {
            write_wall_ordinal(input->catalog,
                               out->aspect,
                               DM1_SQA_LEFT_WALL_ORN_ORD,
                               &out->wroteLeftWallOrnament,
                               input->randomValues,
                               input->randomValueCount,
                               &randomCallIndex);
        }
    } else if (element == DM1_ELEMENT_CORRIDOR &&
               (flags & DM1_CORRIDOR_RANDOM_ORN)) {
        DM1_V1_RandomOrnamentOrdinalF0170Pc34 ordinal;
        if (F0170_DUNGEON_GetRandomOrnamentOrdinal(
                input->catalog->floorOrnamentEnabledByIndex,
                input->catalog->floorOrnamentCount,
                input->randomValues[randomCallIndex],
                &ordinal)) {
            out->aspect[DM1_SQA_FLOOR_ORN_ORDINAL] = ordinal.ornamentOrdinal;
            out->wroteFloorOrnament = 1;
        }
        ++randomCallIndex;
    }

    out->randomCalls = randomCallIndex;
    return 1;
}

int DM1_V1_Dungeon_GetRandomOrnamentIndexF0169Pc34Compat(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentIndexF0169Pc34 *out)
{
    return F0169_DUNGEON_GetRandomOrnamentIndex(
        ornamentEnabledByIndex,
        ornamentCount,
        randomValue,
        out);
}

int DM1_V1_Dungeon_GetRandomOrnamentOrdinalF0170Pc34Compat(
    const uint8_t *ornamentEnabledByIndex,
    size_t ornamentCount,
    uint16_t randomValue,
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 *out)
{
    return F0170_DUNGEON_GetRandomOrnamentOrdinal(
        ornamentEnabledByIndex,
        ornamentCount,
        randomValue,
        out);
}
