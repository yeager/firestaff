#ifndef FIRESTAFF_DM1_V1_C15_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_C15_LAYOUT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

/* ReDMCSB DUNGEON.C F0166 reserves a raw C15 slot by replacing its complete
 * record with an end-of-list record.  The later F0213 publication stages own
 * initialization and linking; this first transaction stage owns only that
 * reversible pool reservation. */
typedef struct {
    struct DungeonThings_Compat* things;
    struct DungeonDatState_Compat* dungeon;
    unsigned short thing;
    unsigned char raw[4];
    struct DungeonExplosion_Compat decoded;
    int mapIndex;
    int mapX;
    int mapY;
    int linked;
    int active;
} DM1_C15PoolReservationPc34;

/* PC34 C25 is EVENT.MapTime plus B.Location and C.Slot.  The receipt keeps
 * that original union separate from a host ExplosionList index. */
typedef struct {
    uint32_t mapTime;
    uint16_t slot;
    uint8_t mapX;
    uint8_t mapY;
    uint8_t priority;
    uint32_t c15Fingerprint;
    int active;
} DM1_C15C25PublicationReceiptPc34;

uint32_t dm1_v1_c15_layout_fingerprint_pc34(const unsigned char* bytes,
                                             size_t byte_count);

int dm1_v1_c15_pool_reserve_pc34(
    struct DungeonThings_Compat* things,
    DM1_C15PoolReservationPc34* out_reservation);

int dm1_v1_c15_pool_rollback_pc34(
    DM1_C15PoolReservationPc34* reservation);

/* ReDMCSB C15 layout: Generic.Next, then Type in low seven bits of byte two,
 * Centered in bit seven, and Attack in byte three.  Linking is delegated to
 * DUNGEON.C F0163 and any failed stage restores the exact reserved pool row. */
int dm1_v1_c15_pool_initialize_and_link_pc34(
    DM1_C15PoolReservationPc34* reservation,
    struct DungeonDatState_Compat* dungeon,
    int explosion_type,
    int attack,
    int centered,
    int cell,
    int map_index,
    int map_x,
    int map_y);

int dm1_v1_c15_c25_publish_pc34(
    DM1_C15PoolReservationPc34* reservation,
    struct DungeonDatState_Compat* dungeon,
    int explosion_type,
    int attack,
    int centered,
    int cell,
    int map_index,
    int map_x,
    int map_y,
    uint32_t fire_at_tick,
    int priority,
    DM1_C15C25PublicationReceiptPc34* out_receipt);

int dm1_v1_c15_c25_receipt_is_live_pc34(
    const DM1_C15C25PublicationReceiptPc34* receipt,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things);

/* ReDMCSB PROJEXPL.C F0221 (lines 883-903) walks the source square's SFT
 * chain for a live C050 fluxcage.  A C15 row is accepted only when its raw
 * bytes and decoded mirror agree; malformed source ownership fails closed. */
int dm1_v1_f0221_fluxcage_on_square_pc34(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int map_index,
    int map_x,
    int map_y,
    int* out_has_fluxcage);

#endif
