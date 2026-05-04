/* DM1 V1 Wall Ornament Rendering — source-locked from ReDMCSB
 * DUNVIEW.C: F0107_IsDrawnWallOrnamentAnAlcove_CPSF (line 3502)
 * DUNVIEW.C: ornament coordinate sets from graphic 558
 * DRAWVIEW.C: wall ornament overlay on viewport walls
 * Ornaments: alcoves, switches, keyholes, inscriptions, fountains */
#ifndef FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_WALL_ORN_MAX       16   /* Max wall ornament types per level */
#define DM1_FLOOR_ORN_MAX      16   /* Max floor ornament types per level */
#define DM1_DOOR_ORN_MAX        1   /* Max door ornament types per level */
#define DM1_ORN_COORD_SETS     12   /* Coordinate sets for depth/side */

/* Ornament types from ReDMCSB */
typedef enum {
    M11_ORN_NONE = 0,
    M11_ORN_ALCOVE,            /* Interactive alcove (item storage) */
    M11_ORN_SWITCH,            /* Wall switch (lever/button) */
    M11_ORN_KEYHOLE,           /* Keyhole */
    M11_ORN_INSCRIPTION,       /* Text inscription */
    M11_ORN_FOUNTAIN,          /* Fountain (water source) */
    M11_ORN_TORCH_HOLDER,      /* Wall torch */
    M11_ORN_GENERIC            /* Generic decorative ornament */
} M11_WO_OrnamentKind;

/* Ornament coordinate set — where to draw at each depth/side */
typedef struct {
    int16_t x, y;             /* Position relative to wall zone */
    int16_t w, h;             /* Ornament bitmap dimensions at this depth */
    int16_t depth;            /* View depth (0=closest, 3=farthest) */
    int16_t side;             /* 0=left, 1=center, 2=right */
} M11_WO_OrnCoord;

/* Ornament definition */
typedef struct {
    uint16_t gfx_index;       /* Bitmap index in GRAPHICS.DAT */
    M11_WO_OrnamentKind kind;
    bool is_alcove;            /* F0107 check result */
    bool is_interactive;       /* Can be clicked */
    M11_WO_OrnCoord coords[DM1_ORN_COORD_SETS]; /* Per depth/side */
} M11_WO_OrnamentDef;

typedef struct {
    M11_WO_OrnamentDef wall_ornaments[DM1_WALL_ORN_MAX];
    M11_WO_OrnamentDef floor_ornaments[DM1_FLOOR_ORN_MAX];
    M11_WO_OrnamentDef door_ornaments[DM1_DOOR_ORN_MAX];
    uint8_t wall_count;
    uint8_t floor_count;
    uint8_t door_count;
} M11_WO_OrnamentState;

void m11_wo_init(M11_WO_OrnamentState* state);
void m11_wo_set_level_ornaments(M11_WO_OrnamentState* state,
                                 uint8_t wall_count, uint8_t floor_count,
                                 uint8_t door_count);
bool m11_wo_is_alcove(const M11_WO_OrnamentDef* orn);
const M11_WO_OrnCoord* m11_wo_get_coord(const M11_WO_OrnamentDef* orn,
                                          int16_t depth, int16_t side);
void m11_wo_setup_default_coords(M11_WO_OrnamentDef* orn);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H */
