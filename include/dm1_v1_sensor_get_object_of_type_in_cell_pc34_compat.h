#ifndef FIRESTAFF_DM1_V1_SENSOR_GET_OBJECT_OF_TYPE_IN_CELL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SENSOR_GET_OBJECT_OF_TYPE_IN_CELL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DEFS.H: C0xFFFF_THING_NONE and CM1_CELL_ANY. */
#define DM1_V1_SENSOR_THING_NONE_PC34 0xffffu
#define DM1_V1_SENSOR_CELL_ANY_PC34    (-1)

/*
 * One object from the already-resolved square chain.  The caller owns both
 * the chain and its traversal order; this query deliberately has no dungeon
 * or object-world dependency.
 */
typedef struct DM1_V1_SensorCellObjectPc34 {
    uint16_t thing;
    uint16_t objectType;
    int16_t cell;
} DM1_V1_SensorCellObjectPc34;

/*
 * ReDMCSB MOVESENS.C F0273_SENSOR_GetObjectOfTypeInCell.
 *
 * Returns the first object in traversal order with objectType and either the
 * requested cell or any cell when cell is DM1_V1_SENSOR_CELL_ANY_PC34.
 * Returns DM1_V1_SENSOR_THING_NONE_PC34 when no entry matches.
 */
uint16_t F0273_SENSOR_GetObjectOfTypeInCell_Compat(
    const DM1_V1_SensorCellObjectPc34 *objects,
    size_t objectCount,
    int16_t cell,
    uint16_t objectType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SENSOR_GET_OBJECT_OF_TYPE_IN_CELL_PC34_COMPAT_H */
