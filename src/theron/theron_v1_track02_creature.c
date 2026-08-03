/* theron_v1_track02_creature.c — TQ creature type table.
 *
 * TQ creatures are NOT stored in ground ref chains like DM1.
 * Instead, each map level has a creature_count in its header,
 * and creature types are determined per-dungeon from ROM tables.
 * The creature type assignments are defined as static data in
 * theron_v1_track02_creature.h (sourced from DMWeb ChristopheF maps).
 */

#include "theron_v1_track02_creature.h"
