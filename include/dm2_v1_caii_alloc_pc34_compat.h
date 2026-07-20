#ifndef DM2_V1_CAII_ALLOC_PC34_COMPAT_H
#define DM2_V1_CAII_ALLOC_PC34_COMPAT_H

#include "dm2_v1_creature_schedule_pc34_compat.h"

/*
 * DM2 v1 PC 3.4 CAII (creature-array) slot allocator — bounded slice.
 *
 * Binds the observable slice of DM2_ALLOC_CAII_TO_CREATURE
 * (skproject/SKULLWIN/c_1c9a.cpp:5772-5894): the lazy creature-activation
 * allocator invoked when a creature without a CAII slot is activated
 * (DM2_ATTACK_CREATURE c_creature.cpp:318-386 resolves the record via
 * DM2_GET_CREATURE_AT when its record argument is -1, then allocs;
 * c_moverec.cpp:983, c_tim_proc.cpp:2887, c_1c9a.cpp:9982 call the
 * allocator directly).  There is NO map-load CAII loop in the source:
 * activation is event-driven, so this module binds the allocator itself
 * and leaves the event sites as future wiring.
 *
 * Observable slice (c_1c9a.cpp:5779-5893):
 *   - record byte@5 != 0xff  -> source early return (already has a slot);
 *   - the word@0xe bit-10 read-modify-write dance (or8/or16/and8 at
 *     c_1c9a.cpp:5788-5807) restores the bit to its original value — a
 *     decompiled bitfield rewrite, receipted as a no-op, never simulated;
 *   - free-slot scan over the session CAII array (34-byte slots, source
 *     c_creature stride 0x22), free = signed word@0 < 0; when no slot is
 *     free the source recycles records from the world
 *     (DM2_RECYCLE_A_RECORD_FROM_THE_WORLD, c_1c9a.cpp:5880-5891) which is
 *     unproven — the bounded slice fails closed WITHOUT mutation;
 *   - slot init (c_1c9a.cpp:5809-5866): zero 34 bytes; word@0 = record
 *     index only (handle & 0x3ff — DM2_1c9a_0fcb rebuilds the DB4 handle
 *     by OR-ing 0x1000, c_1c9a.cpp:5915); word@2 = -1 (timer index);
 *     byte@6 = (gametick >> 2) - 1; byte@4 = gametick - 0x7f; word@0xc =
 *     (x & 0x1f) | ((y & 0x1f) << 5) | ((map & 0x3f) << 10);
 *     byte@0x16/0x17 = -1; byte@7 = 0; record byte@5 = slot index;
 *     alloc counter++ (ddat.v1d4020);
 *   - DM2_PREPARE/UNPREPARE_LOCAL_CREATURE_VAR, DM2_14cd_0802 (s350
 *     bytes 0x12/0x13) and the s350.v1e0552/v1e054e group scan with
 *     DM2_CREATURE_SOMETHING_1c9a_0a48 (c_1c9a.cpp:5868-5875) stay
 *     host-owned until the CCM body is proven — receipted, never
 *     simulated;
 *   - the bound creature-scheduling producer DM2_1c9a_0cf7
 *     (dm2_v1_creature_schedule_at) runs with the activation cell,
 *     queueing the creature's first 0x21/0x22 timer (c_1c9a.cpp:5860);
 *     the issued stable session ticket is stored in the slot timer word
 *     (word@2) exactly like the producer's post-queue store
 *     (c_1c9a.cpp:5724-5728);
 *   - slot byte@1a = 0 when the record group/leader link word@8 is not
 *     0xffff, else 0x11 (c_1c9a.cpp:5861-5866).
 *
 * The CAII array capacity stands in for ddat.v1e08a0 (computed at
 * DM2_INIT by DM2_1c9a_3c30, startend.cpp:467-494, as
 * min(eligible-creature-count + 0x64, savegame word@0x14) — the savegame
 * word owner is unproven), so the caller owns the capacity.
 */

#define DM2_V1_CAII_SLOT_SIZE 34 /* source c_creature stride 0x22 */

typedef struct {
  uint8_t *slots;    /* capacity * DM2_V1_CAII_SLOT_SIZE bytes, owned */
  int capacity;      /* caller-owned stand-in for ddat.v1e08a0 */
  int alloc_count;   /* session stand-in for ddat.v1d4020 */
  int valid;
} DM2_V1_CaiiArray;

/* Allocates the slot array and marks every slot free (word@0 = -1).
 * capacity <= 0 fails closed (valid == 0). */
void dm2_v1_caii_array_init(DM2_V1_CaiiArray *caii, int capacity);
void dm2_v1_caii_array_free(DM2_V1_CaiiArray *caii);

/* Read-only slot accessor; NULL for out-of-range indexes or invalid
 * arrays (fail-closed). */
const uint8_t *dm2_v1_caii_slot(const DM2_V1_CaiiArray *caii, int index);

typedef struct {
  int valid;
  int allocated;
  int already_allocated;
  int no_free_slot;
  int record_flag_rewrite_noop;
  int slot_index;
  int record_index;       /* handle & 0x3ff stored at slot word@0 */
  int timer_scheduled;
  int timer_type;
  unsigned long due_tick;
  int local_var_unbound;  /* PREPARE/UNPREPARE + 14cd_0802 host-owned */
  int group_scan_unbound; /* s350 group scan + 1c9a_0a48 host-owned */
  char source_evidence[224];
} DM2_V1_CaiiAllocReceipt;

/*
 * Allocate one CAII slot for `record_handle` and schedule the creature's
 * first think timer, mirroring DM2_ALLOC_CAII_TO_CREATURE
 * (c_1c9a.cpp:5772-5894).  `map_id` stands in for ddat.v1d3248 and
 * `game_tick` for timdat.gametick (both caller-owned).  Returns 1 when a
 * slot was allocated, 0 for the source early return (already_allocated),
 * the fail-closed no-free-slot path (no_free_slot), or invalid input.
 * When `receipt` is non-NULL it always receives the audit record.
 */
int dm2_v1_caii_alloc_to_creature(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    DM2_V1_CaiiAllocReceipt *receipt);

typedef struct {
  int valid;
  int deleted;
  int no_caii_slot;
  int no_pending_timer;
  int not_creature_db;
  int slot_index;
  uint32_t cancelled_ticket;
  char source_evidence[200];
} DM2_V1_CaiiDeleteTimerReceipt;

/*
 * DM2_1c9a_0db0 (c_1c9a.cpp:5734-5763) bounded slice: delete the pending
 * think timer owned by a creature record.  The source verifies the
 * record DB ((handle >> 10) & 0xf == dbCreature 4), reads record byte@5
 * (CAII slot), and when the slot timer word (word@2) is not -1 deletes
 * that timer (DM2_DELETE_TIMER, c_timer.cpp:215-232) and writes the word
 * back to -1.  The bounded slice cancels the stable session ticket
 * stored in word@2.  Returns 1 when a timer was deleted; 0 (fail-closed,
 * receipted) otherwise.  When `receipt` is non-NULL it always receives
 * the audit record.
 */
int dm2_v1_caii_delete_timer(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int16_t record_handle,
    DM2_V1_CaiiDeleteTimerReceipt *receipt);

/*
 * CAII-aware creature scheduling — the COMPLETE DM2_1c9a_0cf7 slice
 * (c_1c9a.cpp:5695-5728) the way the source's direct callers reach it
 * (c_creature.cpp:648, c_move.cpp:700): resolve the creature record at
 * (x, y), delete any previously queued timer through the bound
 * DM2_1c9a_0db0 path (receipt.replaced_existing == 1), enqueue the new
 * 0x21/0x22 timer, and store the issued ticket in the CAII slot timer
 * word (word@2, c_1c9a.cpp:5724-5728).  A record without a CAII slot
 * (byte@5 == 0xff) fails closed with receipt.no_caii_slot == 1 — the
 * source would index the creatures array out of bounds, so activation
 * must allocate the slot first (dm2_v1_caii_alloc_to_creature).
 * Returns 1 when a timer was enqueued; 0 (fail-closed) otherwise.
 */
int dm2_v1_caii_schedule_creature_at(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int x, int y,
    DM2_V1_CreatureScheduleReceipt *receipt);

typedef struct {
  int valid;
  int freed;
  int already_free;
  int out_of_range;
  int had_pending_timer;
  int record_delete_unbound;
  int record_index;
  char source_evidence[200];
} DM2_V1_CaiiFreeReceipt;

/*
 * DM2_1c9a_0fcb (c_1c9a.cpp:5896-5944) bounded slice: free one CAII
 * slot, completing the slot lifecycle (alloc -> schedule -> delete ->
 * free).  The source's callers are creature despawn/death paths and
 * load-game cleanup (c_ai.cpp:5775, c_moverec.cpp:684 + 997,
 * c_savegame.cpp:2049).  Observable slice:
 *   - slot index bounds: the source compares slot > ddat.v1e08a0
 *     unsigned (c_1c9a.cpp:5905) and would index out of bounds at
 *     slot == capacity; the bounded slice fails closed for any index
 *     outside [0, capacity) — receipted out_of_range;
 *   - already-free slot (signed word@0 < 0): source early return
 *     (c_1c9a.cpp:5908-5909), receipted already_free;
 *   - the DB4 handle is rebuilt as slot word@0 | 0x1000
 *     (c_1c9a.cpp:5915) — confirming word@0 holds the bare record
 *     index;
 *   - the record-delete flag derives from DM2_QUERY_CREATURE_AI_SPEC_FLAGS
 *     (c_1c9a.cpp:5917-5929) whose AI-spec table owner is unproven, so
 *     the bounded slice NEVER takes the DM2_DELETE_CREATURE_RECORD
 *     branch (c_1c9a.cpp:5930-5944, including the timer payload read);
 *     receipted record_delete_unbound, never simulated;
 *   - slot byte@1a = 0, the pending timer is deleted through the bound
 *     DM2_1c9a_0db0 path (c_1c9a.cpp:5933), the alloc counter
 *     decrements (ddat.v1d4020--), record byte@5 = -1, and slot word@0
 *     = -1 marking the slot free (c_1c9a.cpp:5932-5941).
 *
 * Returns 1 when the slot was freed; 0 (fail-closed, receipted)
 * otherwise.  When `receipt` is non-NULL it always receives the audit
 * record.
 */
int dm2_v1_caii_free_slot(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int slot_index,
    DM2_V1_CaiiFreeReceipt *receipt);

#endif
