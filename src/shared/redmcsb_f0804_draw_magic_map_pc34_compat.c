#include "redmcsb_f0804_draw_magic_map_pc34_compat.h"

enum {
    F0804_COLOR_NO_TRANSPARENCY = -1,
    F0804_COLOR_RED = 8,
    F0804_COLOR_CYAN = 4,
    F0804_FAKEWALL = 6,
    F0804_FAKEWALL_IMAGINARY = 1,
    F0804_FOOTPRINTS = 0x8000,
    F0804_PROJECTILE_FIREBALL = -11,
    F0804_PROJECTILE_DEFAULT = -12,
    F0804_PROJECTILE_SLIME = -13,
    F0804_PROJECTILE_POISON = -14,
    F0804_PROJECTILE_LIGHTNING = -4,
    F0804_EXPLOSION_FIREBALL = 0,
    F0804_EXPLOSION_LIGHTNING = 2,
    F0804_EXPLOSION_POISON_BOLT = 6,
    F0804_EXPLOSION_POISON_CLOUD = 7,
    F0804_EXPLOSION_SMOKE = 40,
    F0804_EXPLOSION_FLUXCAGE = 50
};

static int16_t f0804_normalize(int16_t value) {
    return (int16_t)(value & 3);
}

static int16_t f0804_cell(int16_t thing) {
    return (int16_t)(thing & 3);
}

static int f0804_west_east(int16_t direction) {
    return (direction & 1) != 0;
}

static int16_t f0804_random_four(const RedmcsbF0804Callbacks_Compat *callbacks) {
    return callbacks->randomFour != 0 ? (int16_t)(callbacks->randomFour(callbacks->context) & 3) : 0;
}

static void f0804_icon(const RedmcsbF0804Request_Compat *request,
                       const RedmcsbF0804Callbacks_Compat *callbacks,
                       int16_t icon) {
    if (callbacks->drawIcon != 0) {
        callbacks->drawIcon(callbacks->context, icon, request->iconX, request->iconY);
    }
}

int F0804_DrawMagicMap_Compat(const RedmcsbF0804Request_Compat *request,
                              const RedmcsbF0804Callbacks_Compat *callbacks) {
    RedmcsbF0804Square_Compat square;
    int16_t marker = F0804_COLOR_NO_TRANSPARENCY;
    int16_t thing;
    int16_t firstProjectile = REDMCSB_F0804_THING_END_OF_LIST;
    int16_t firstExplosion = REDMCSB_F0804_THING_END_OF_LIST;
    int16_t group = REDMCSB_F0804_THING_END_OF_LIST;
    int imaginary;

    if (request == 0 || callbacks == 0 || callbacks->getSquare == 0 ||
        !callbacks->getSquare(callbacks->context, request->x, request->y,
                              request->direction, &square)) {
        return 0;
    }

    if (request->x == request->partyX && request->y == request->partyY) {
        marker = F0804_COLOR_RED;
    } else if (request->champion.wizardSpellCount != 0 &&
               square.element != REDMCSB_F0804_ELEMENT_WALL &&
               (square.floorOrnamentOrdinalWithFootprints & F0804_FOOTPRINTS) != 0) {
        marker = F0804_COLOR_CYAN;
    }
    if (marker >= 0 && callbacks->fillZone != 0) {
        callbacks->fillZone(callbacks->context,
                            (int16_t)(request->iconX - request->zoneMarginX),
                            (int16_t)(request->iconY - request->zoneMarginY),
                            (int16_t)((request->zoneMarginX << 1) + request->iconWidth),
                            (int16_t)((request->zoneMarginY << 1) + request->iconHeight),
                            marker);
    }

    imaginary = ((square.square >> 5) == F0804_FAKEWALL) &&
                ((square.square & F0804_FAKEWALL_IMAGINARY) != 0);
    switch (square.element) {
        case REDMCSB_F0804_ELEMENT_WALL:
            if (request->champion.ninjaSpellCount != 0 && imaginary &&
                (request->gameTime & 2U) != 0U) {
                f0804_icon(request, callbacks, 1);
                break;
            }
            f0804_icon(request, callbacks, 0);
            for (int ornamentSide = 0; ornamentSide <= 3; ++ornamentSide) {
                int16_t ordinal = square.wallOrnamentOrdinals[ornamentSide];
                int16_t index;
                if (ordinal == 0) {
                    continue;
                }
                index = (int16_t)(ordinal - 1);
                if (index == callbacks->currentViAltarWallOrnamentIndex) {
                    f0804_icon(request, callbacks, (int16_t)(ornamentSide + 26));
                } else if (callbacks->isWallOrnamentAlcove != 0 &&
                           callbacks->isWallOrnamentAlcove(callbacks->context, index)) {
                    f0804_icon(request, callbacks, (int16_t)(ornamentSide + 22));
                } else if (callbacks->isWallOrnamentFountain != 0 &&
                           callbacks->isWallOrnamentFountain(callbacks->context, index)) {
                    f0804_icon(request, callbacks, (int16_t)(ornamentSide + 30));
                }
            }
            break;
        case REDMCSB_F0804_ELEMENT_CORRIDOR:
            f0804_icon(request, callbacks, 2);
            if ((square.floorOrnamentOrdinalWithFootprints & ~F0804_FOOTPRINTS) != 0 &&
                callbacks->floorOrnamentIndex != 0) {
                f0804_icon(request, callbacks,
                            (int16_t)(callbacks->floorOrnamentIndex(
                                callbacks->context,
                                (int16_t)(square.floorOrnamentOrdinalWithFootprints & ~F0804_FOOTPRINTS)) + 78));
            }
            break;
        case REDMCSB_F0804_ELEMENT_TELEPORTER:
            f0804_icon(request, callbacks, (int16_t)(f0804_random_four(callbacks) + 6));
            break;
        case REDMCSB_F0804_ELEMENT_DOOR_SIDE:
        case REDMCSB_F0804_ELEMENT_DOOR_FRONT: {
            int16_t icon = square.element == REDMCSB_F0804_ELEMENT_DOOR_SIDE ? 11 : 10;
            if (callbacks->doorHasButton == 0 ||
                !callbacks->doorHasButton(callbacks->context, square.doorThingIndex)) {
                icon = (int16_t)(icon + 2);
            }
            if (square.doorState == REDMCSB_F0804_DOOR_OPEN ||
                square.doorState == REDMCSB_F0804_DOOR_DESTROYED) {
                icon = (int16_t)(icon + 4);
            }
            f0804_icon(request, callbacks, icon);
            break;
        }
        case REDMCSB_F0804_ELEMENT_STAIRS_SIDE:
        case REDMCSB_F0804_ELEMENT_STAIRS_FRONT: {
            int16_t icon = callbacks->getStairsExitDirection != 0
                               ? f0804_normalize((int16_t)(callbacks->getStairsExitDirection(
                                   callbacks->context, request->x, request->y) - request->direction))
                               : 0;
            if (square.stairsUp != 0) {
                icon = (int16_t)((icon + 2) & 3);
            }
            f0804_icon(request, callbacks, (int16_t)(icon + 18));
            break;
        }
        case REDMCSB_F0804_ELEMENT_PIT: {
            int16_t icon = 3;
            if (square.pitOrTeleporterVisible != 0) {
                ++icon;
                if (request->champion.ninjaSpellCount != 0 && (request->gameTime & 1U) != 0U) {
                    ++icon;
                }
            }
            f0804_icon(request, callbacks, icon);
            break;
        }
        default:
            break;
    }

    if ((request->champion.fighterSpellCount == 0 && request->champion.wizardSpellCount == 0) ||
        callbacks->getNextThing == 0 || callbacks->getThingType == 0) {
        return 1;
    }

    for (thing = square.firstThing; thing != REDMCSB_F0804_THING_END_OF_LIST;
         thing = callbacks->getNextThing(callbacks->context, thing)) {
        int16_t type = callbacks->getThingType(callbacks->context, thing);
        if (type == REDMCSB_F0804_THING_GROUP && request->champion.fighterSpellCount != 0) {
            group = thing;
            continue;
        }
        if (request->champion.wizardSpellCount == 0) {
            continue;
        }
        if (type == REDMCSB_F0804_THING_PROJECTILE && firstProjectile == REDMCSB_F0804_THING_END_OF_LIST) {
            firstProjectile = thing;
        } else if (type == REDMCSB_F0804_THING_EXPLOSION && firstExplosion == REDMCSB_F0804_THING_END_OF_LIST) {
            firstExplosion = thing;
        } else if (type == REDMCSB_F0804_THING_JUNK && callbacks->getObjectType != 0 &&
                   callbacks->getObjectType(callbacks->context, thing) == REDMCSB_F0804_OBJECT_ZOKATHRA) {
            int16_t icon = (int16_t)(f0804_normalize((int16_t)(f0804_cell(thing) - request->direction)) + 34);
            if (square.element == REDMCSB_F0804_ELEMENT_WALL && !imaginary) {
                icon = (int16_t)(icon + 4);
            }
            f0804_icon(request, callbacks, icon);
        }
    }

    if (group != REDMCSB_F0804_THING_END_OF_LIST && callbacks->getCreatureAttributes != 0 &&
        (callbacks->getCreatureAttributes(callbacks->context, group) & REDMCSB_F0804_CREATURE_ATTR_MAGIC_MAP) == 0) {
        f0804_icon(request, callbacks, (int16_t)((request->gameTime & 1U) + 76));
    }
    for (thing = firstProjectile; thing != REDMCSB_F0804_THING_END_OF_LIST;
         thing = callbacks->getNextThing(callbacks->context, thing)) {
        int16_t slot;
        int16_t aspect;
        int16_t icon = 0;
        if (callbacks->getThingType(callbacks->context, thing) != REDMCSB_F0804_THING_PROJECTILE ||
            callbacks->getProjectileSlot == 0 || callbacks->getProjectileAspect == 0) {
            continue;
        }
        slot = callbacks->getProjectileSlot(callbacks->context, thing);
        aspect = callbacks->getProjectileAspect(callbacks->context, slot);
        if (aspect >= 0) {
            if (callbacks->getObjectType != 0 && callbacks->getObjectType(callbacks->context, slot) == REDMCSB_F0804_OBJECT_ZOKATHRA) {
                icon = 34;
            }
        } else if (aspect == F0804_PROJECTILE_FIREBALL) icon = 50;
        else if (aspect == F0804_PROJECTILE_DEFAULT) icon = 54;
        else if (aspect == F0804_PROJECTILE_POISON) icon = 62;
        else if (aspect == F0804_PROJECTILE_SLIME) icon = 58;
        else if (aspect == F0804_PROJECTILE_LIGHTNING) {
            icon = 42;
            if (callbacks->getProjectileDirection != 0 &&
                f0804_west_east(callbacks->getProjectileDirection(callbacks->context, thing)) !=
                    f0804_west_east(request->direction)) {
                icon = (int16_t)(icon + 4);
            }
        }
        if (icon > 0) {
            f0804_icon(request, callbacks,
                        (int16_t)(icon + f0804_normalize((int16_t)(f0804_cell(thing) - request->direction))));
        }
    }
    for (thing = firstExplosion; thing != REDMCSB_F0804_THING_END_OF_LIST;
         thing = callbacks->getNextThing(callbacks->context, thing)) {
        int16_t type;
        int16_t icon = 0;
        if (callbacks->getThingType(callbacks->context, thing) != REDMCSB_F0804_THING_EXPLOSION ||
            callbacks->getExplosionType == 0) {
            continue;
        }
        type = callbacks->getExplosionType(callbacks->context, thing);
        if (type == F0804_EXPLOSION_FIREBALL || type == F0804_EXPLOSION_LIGHTNING) icon = 66;
        else if (type == F0804_EXPLOSION_POISON_BOLT || type == F0804_EXPLOSION_POISON_CLOUD) icon = (int16_t)(f0804_random_four(callbacks) + 68);
        else if (type == F0804_EXPLOSION_FLUXCAGE) icon = (int16_t)(f0804_random_four(callbacks) + 72);
        else if (type < F0804_EXPLOSION_SMOKE) icon = 67;
        if (icon != 0) {
            f0804_icon(request, callbacks, icon);
        }
    }
    return 1;
}
