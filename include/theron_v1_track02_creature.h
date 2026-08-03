#ifndef THERON_V1_TRACK02_CREATURE_H
#define THERON_V1_TRACK02_CREATURE_H

#include <stdint.h>

/* TQ creature types per dungeon, mapped to DM1 creature type indices.
 * Source: DMWeb ChristopheF maps.
 *
 * DM1 types: 0=GiantScorpion, 2=Giggler, 3=PainRat, 5=Screamer,
 * 7=Ghost, 9=Vexirk, 10=Worm, 11=Skeleton, 12=Couatl, 14=Mummy,
 * 15=BlackFlame, 16=MagentaWorm, 17=Trolin, 18=GiantWasp, 20=Materializer,
 * 21=WaterElemental(Dragon), 22=Oitu, 23=Demon */

#define THERON_DUNGEON_COUNT 7
#define THERON_MAX_CREATURE_TYPES_PER_DUNGEON 3

typedef struct {
    uint8_t types[THERON_MAX_CREATURE_TYPES_PER_DUNGEON];
    uint8_t count;
} Theron_DungeonCreatureTypes;

/* Indexed by dungeon_index (0=AKUTUBA..6=DEMON) */
static const Theron_DungeonCreatureTypes theron_creature_types[THERON_DUNGEON_COUNT] = {
    {{ 14,  5,  0 }, 2 },  /* AKUTUBA:  Mummy, Screamer */
    {{ 11,  9, 12 }, 3 },  /* DRATOR:   Skeleton, Vexirk, Couatl */
    {{ 17, 22, 18 }, 3 },  /* FORMICIA: Trolin, Oitu, GiantWasp */
    {{  3,  7,  0 }, 2 },  /* SARMON:   PainRat, Ghost */
    {{ 16, 10, 21 }, 3 },  /* SHADODAN: MagentaWorm, Worm, Dragon(WaterElemental) */
    {{  2,  0,  0 }, 2 },  /* THIEVES:  Giggler, GiantScorpion */
    {{ 20, 15, 23 }, 3 },  /* DEMON:    Materializer, BlackFlame, Demon */
};

/* ── Creature generators ──────────────────────────────────────────────
 * Source: DMWeb ChristopheF maps. */

#define THERON_MAX_GENERATORS 5

typedef struct {
    uint8_t creature_type;
    uint8_t level;
    uint8_t max_spawn;
} Theron_GeneratorDesc;

typedef struct {
    Theron_GeneratorDesc gens[THERON_MAX_GENERATORS];
    uint8_t count;
} Theron_DungeonGenerators;

static const Theron_DungeonGenerators theron_dungeon_generators[THERON_DUNGEON_COUNT] = {
    {{ { 14, 2, 3 }, {0}, {0}, {0}, {0} }, 1 },
    {{ { 11, 0, 2 }, { 11, 1, 1 }, {0}, {0}, {0} }, 2 },
    {{ { 17, 0, 2 }, { 17, 1, 4 }, { 17, 1, 4 }, {0}, {0} }, 3 },
    {{ { 7, 1, 1 }, {0}, {0}, {0}, {0} }, 1 },
    {{ {0}, {0}, {0}, {0}, {0} }, 0 },
    {{ {0}, {0}, {0}, {0}, {0} }, 0 },
    {{ { 15, 1, 1 }, { 15, 1, 1 }, { 15, 1, 1 }, { 15, 1, 1 }, { 15, 1, 1 } }, 5 },
};

#endif
