#ifndef REDMCSB_F7090_MAKE_NEW_ADVENTURE_PC34_COMPAT_H
#define REDMCSB_F7090_MAKE_NEW_ADVENTURE_PC34_COMPAT_H

#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"

#include <stdint.h>

/* ReDMCSB CEDTINCI.C F7089/F7090 utility-disk import transition.
 *
 * The source first transfers portrait bytes through F7088, then normalizes
 * the already-copied destination party for a new CSB adventure.  Firestaff
 * keeps those two ownership boundaries separate: callers must run F7088's
 * included-to-excluded portrait transfer before this function.  This routine
 * never manufactures a portrait, object modifier, or random value. */

#define REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT 4U
#define REDMCSB_F7090_PC34_HEADER_TAIL_OFFSET 256U
#define REDMCSB_F7090_PC34_HEADER_TAIL_SIZE 40U
#define REDMCSB_F7090_PC34_THING_NONE 0xFFFFU

typedef uint8_t (*RedmcsbF7090RandomBytePc34)(void *context);
typedef void (*RedmcsbF7090RemoveObjectModifierPc34)(
    void *context, uint16_t champion_index, uint16_t slot_index,
    uint16_t thing);

/* F7089 returns a free party cell in source scan order.  A return value of
 * four is intentional when all four cells are occupied: it is the original
 * CEDT008.C loop result, not a modern clamp. */
uint8_t redmcsb_f7089_get_first_cell_with_no_champion_pc34(
    const CSB_V1_PartyState *party, uint16_t loaded_champion_count);

/* F7090 mutates a destination party that has already received its imported
 * champion records.  It copies the original two header tails as one exact
 * [256,296) range, increments GameID, carries Platform and sets Useless=2.
 * The modifier and RNG callbacks are required source dependencies. The
 * original F7020 mutator has no failure return; neither does this boundary. */
int redmcsb_f7090_make_new_adventure_pc34(
    const CSB_V1_SaveHeader *source_header, uint16_t source_platform,
    CSB_V1_SaveHeader *destination_header, uint16_t *destination_platform,
    uint16_t *destination_useless, const CSB_V1_PartyState *source_party,
    CSB_V1_PartyState *destination_party, uint16_t loaded_champion_count,
    RedmcsbF7090RandomBytePc34 random_byte,
    RedmcsbF7090RemoveObjectModifierPc34 remove_object_modifier,
    void *context);

const char *redmcsb_f7090_make_new_adventure_pc34_source_evidence(void);

#endif
