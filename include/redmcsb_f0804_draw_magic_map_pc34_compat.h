#ifndef REDMCSB_F0804_DRAW_MAGIC_MAP_PC34_COMPAT_H
#define REDMCSB_F0804_DRAW_MAGIC_MAP_PC34_COMPAT_H

#include <stdint.h>

/*
 * ReDMCSB PANEL.C:545-752 (PC 3.4).
 *
 * This is deliberately a renderer adapter, not a replacement magic-map
 * renderer.  F0804 only selects source icon indices and invokes F0735/F0803;
 * the supplied callbacks retain ownership of real graphics and palettes.
 */

enum {
    REDMCSB_F0804_ELEMENT_WALL = 0,
    REDMCSB_F0804_ELEMENT_CORRIDOR = 1,
    REDMCSB_F0804_ELEMENT_PIT = 2,
    REDMCSB_F0804_ELEMENT_TELEPORTER = 5,
    REDMCSB_F0804_ELEMENT_DOOR_SIDE = 16,
    REDMCSB_F0804_ELEMENT_DOOR_FRONT = 17,
    REDMCSB_F0804_ELEMENT_STAIRS_SIDE = 18,
    REDMCSB_F0804_ELEMENT_STAIRS_FRONT = 19,
    REDMCSB_F0804_THING_END_OF_LIST = -2,
    REDMCSB_F0804_THING_GROUP = 4,
    REDMCSB_F0804_THING_JUNK = 10,
    REDMCSB_F0804_THING_PROJECTILE = 14,
    REDMCSB_F0804_THING_EXPLOSION = 15,
    REDMCSB_F0804_OBJECT_ZOKATHRA = 197,
    REDMCSB_F0804_CREATURE_ATTR_MAGIC_MAP = 0x4000,
    REDMCSB_F0804_DOOR_OPEN = 0,
    REDMCSB_F0804_DOOR_DESTROYED = 5
};

typedef struct RedmcsbF0804Square_Compat {
    int16_t element;
    int16_t square;
    int16_t wallOrnamentOrdinals[4];
    int16_t floorOrnamentOrdinalWithFootprints;
    int16_t firstThing;
    int16_t doorThingIndex;
    int16_t doorState;
    int16_t stairsUp;
    int16_t pitOrTeleporterVisible;
} RedmcsbF0804Square_Compat;

typedef struct RedmcsbF0804Champion_Compat {
    uint8_t wizardSpellCount;
    uint8_t ninjaSpellCount;
    uint8_t fighterSpellCount;
} RedmcsbF0804Champion_Compat;

typedef struct RedmcsbF0804Callbacks_Compat {
    void *context;
    int (*getSquare)(void *context, int16_t x, int16_t y, int16_t direction,
                     RedmcsbF0804Square_Compat *outSquare);
    void (*fillZone)(void *context, int16_t x, int16_t y, int16_t width,
                     int16_t height, int16_t color);
    void (*drawIcon)(void *context, int16_t iconIndex, int16_t x, int16_t y);
    int (*isWallOrnamentAlcove)(void *context, int16_t ornamentIndex);
    int (*isWallOrnamentFountain)(void *context, int16_t ornamentIndex);
    int16_t (*getStairsExitDirection)(void *context, int16_t x, int16_t y);
    int (*doorHasButton)(void *context, int16_t doorThingIndex);
    int16_t (*randomFour)(void *context);
    int16_t (*getNextThing)(void *context, int16_t thing);
    int16_t (*getThingType)(void *context, int16_t thing);
    int16_t (*getObjectType)(void *context, int16_t thingOrSlot);
    int16_t (*getCreatureAttributes)(void *context, int16_t groupThing);
    int16_t (*getProjectileSlot)(void *context, int16_t projectileThing);
    int16_t (*getProjectileAspect)(void *context, int16_t projectileSlot);
    int16_t (*getProjectileDirection)(void *context, int16_t projectileThing);
    int16_t (*getExplosionType)(void *context, int16_t explosionThing);
    int16_t (*floorOrnamentIndex)(void *context, int16_t ornamentOrdinal);
    int16_t currentViAltarWallOrnamentIndex;
} RedmcsbF0804Callbacks_Compat;

typedef struct RedmcsbF0804Request_Compat {
    int16_t x;
    int16_t y;
    int16_t direction;
    int16_t iconX;
    int16_t iconY;
    int16_t partyX;
    int16_t partyY;
    uint32_t gameTime;
    int16_t iconWidth;
    int16_t iconHeight;
    int16_t zoneMarginX;
    int16_t zoneMarginY;
    RedmcsbF0804Champion_Compat champion;
} RedmcsbF0804Request_Compat;

/* Returns 1 after dispatching source-equivalent callbacks, 0 if getSquare fails. */
int F0804_DrawMagicMap_Compat(const RedmcsbF0804Request_Compat *request,
                              const RedmcsbF0804Callbacks_Compat *callbacks);

#endif
