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

/*
 * AI-spec flags provider hook.  The CAII module does not own the
 * AIDefinition table; the session that owns it wires the provider so
 * the DM2_1c9a_0fcb record-delete flag (c_1c9a.cpp:5917-5929) and the
 * ATTACK_CREATURE vl_18 gate (c_creature.cpp:370-385) can resolve
 * DM2_QUERY_CREATURE_AI_SPEC_FLAGS data-backed.  The proven firestaff
 * provider is dm2_v1_creature_ai_spec_flags (dm2_v1_creature.h) over
 * the GDAT extended-mode AI table.  With no provider wired both sites
 * fail closed with their "unknown provenance" outcomes (flag -1).
 * Signature matches dm2_v1_creature_ai_spec_flags: returns 1 and
 * stores the flags word when the type's AI row is loaded, else 0.
 */
typedef int (*DM2_V1_CaiiAiSpecFlagsFn)(int creature_type,
                                        uint16_t *out_flags);
void dm2_v1_caii_set_ai_spec_flags_fn(DM2_V1_CaiiAiSpecFlagsFn fn);

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
  int record_delete_flag;   /* 1/0 data-backed, -1 when flags not loaded */
  int record_delete_branch; /* 1 when the source would enter the
                               DM2_DELETE_CREATURE_RECORD branch (flag
                               set AND a pending timer exists,
                               c_1c9a.cpp:5937-5948) */
  int record_delete_no_timer; /* flag set but slot word@2 was -1: the
                                 source forces RG3L = 0 and skips the
                                 branch (c_1c9a.cpp:5946-5947) */
  int record_delete_x;      /* pending timer payload bytes (valueA lo/hi, */
  int record_delete_y;      /* c_timer.h getxA/getyA) — the branch args */
  int record_delete_head_resolved; /* the bound decision head resolved
                                      the creature at (x, y) */
  int record_index;
  char source_evidence[200];
} DM2_V1_CaiiFreeReceipt;

/*
 * DM2_1c9a_0fcb (c_1c9a.cpp:5896-5957) bounded slice: free one CAII
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
 *     (c_1c9a.cpp:5917-5929) and is computed DATA-BACKED through the
 *     wired AI-spec flags provider (dm2_v1_caii_set_ai_spec_flags_fn —
 *     the proven provider resolves it over the GDAT extended-mode AI
 *     table): flag = ((flags & 0x1) == 0 && slot byte@1a == 0x13),
 *     receipted record_delete_flag (1/0, or -1 when no provider is
 *     wired or the session loaded no AI row for the record's creature
 *     type);
 *   - the DM2_DELETE_CREATURE_RECORD branch head (c_1c9a.cpp:5936-5957)
 *     is now TAKEN data-backed: when the flag is set and the slot timer
 *     word holds a pending ticket the payload bytes are read BEFORE the
 *     timer is deleted (source order: payload read c_1c9a.cpp:5939-5945,
 *     byte@1a clear 5950, DM2_1c9a_0db0 5952) through the ticket peek
 *     accessor — valueA lo/hi = the branch's (x, y) arguments (c_timer.h
 *     getxA/getyA return the valueA bytes); receipted
 *     record_delete_branch + record_delete_x/y.  When the flag is set
 *     but no timer is pending the source forces RG3L = 0
 *     (c_1c9a.cpp:5946-5947) — receipted record_delete_no_timer.  After
 *     the slot is marked free the branch runs the bound
 *     DM2_DELETE_CREATURE_RECORD decision head
 *     (dm2_v1_caii_delete_creature_record_head, c_record.cpp:1357-1425);
 *     its mutating tail (map/message swap, tile-rooted MOVE_RECORD_TO,
 *     DROP_CREATURE_POSSESSION, DM2_1c9a_0247, DEALLOC_RECORD) stays
 *     unbound behind the DM2-002 ground-stack/possession blockers and
 *     the unmodelled dballoc free chain — receipted, never simulated;
 *   - slot byte@1a = 0, the pending timer is deleted through the bound
 *     DM2_1c9a_0db0 path (c_1c9a.cpp:5952), the alloc counter
 *     decrements (ddat.v1d4020--), record byte@5 = -1, and slot word@0
 *     = -1 marking the slot free (c_1c9a.cpp:5950-5955).
 *
 * Returns 1 when the slot was freed; 0 (fail-closed, receipted)
 * otherwise.  `dungeon` may be NULL — the DELETE_CREATURE_RECORD
 * decision head is then skipped (record_delete_head_resolved stays 0)
 * while the branch decision and payload are still receipted.  When
 * `receipt` is non-NULL it always receives the audit record.
 */
int dm2_v1_caii_free_slot(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int slot_index,
    DM2_V1_CaiiFreeReceipt *receipt);

typedef struct {
  int valid;
  int resolved;              /* GET_CREATURE_AT(x, y) found a creature */
  int creature_not_found;    /* source early return (c_record.cpp:1379-1380) */
  int16_t record_handle;     /* the resolved DB4 creature handle */
  int creature_type;         /* record byte@4 */
  int ai_flags_known;        /* provider returned the AI flags word */
  int ai_bit0_clear;         /* jz_test8 gate passed (c_record.cpp:1385) */
  int invoke_message_unbound;/* table1d607e/GDAT-word@1 gate, map swap and
                                DM2_INVOKE_MESSAGE (c_record.cpp:1387-1406) */
  int slot_mode_cleared;     /* record byte@5 != 0xff -> CAII slot byte@1a
                                cleared (c_record.cpp:1408-1413) */
  int move_record_unbound;   /* tile-rooted DM2_MOVE_RECORD_TO cut
                                (c_record.cpp:1419) — DM2-002 blocker */
  int drop_possession_unbound;/* DM2_DROP_CREATURE_POSSESSION
                                 (c_record.cpp:1422) — DM2-002 blocker */
  int dballoc_cleanup_unbound;/* DM2_1c9a_0247 tagged-dballoc cleanup
                                 (c_record.cpp:1423, c_1c9a.cpp:5135-5160) */
  int dealloc_record_unbound;/* DM2_DEALLOC_RECORD pool free-chain
                                (c_record.cpp:1424) — not modelled */
  char source_evidence[224];
} DM2_V1_CaiiDeleteCreatureRecordReceipt;

/*
 * DM2_DELETE_CREATURE_RECORD (skproject/SKULLWIN/c_record.cpp:1357-1425)
 * bounded DECISION-HEAD slice — the part of the body that is provable
 * over the current session state, invoked from the DM2_1c9a_0fcb branch
 * (c_1c9a.cpp:5956-5957) with the pending timer's payload coordinates:
 *   - DM2_GET_CREATURE_AT(x, y) resolves the creature record; -1 takes
 *     the source early return (receipted creature_not_found);
 *   - the record's creature type (byte@4) resolves the AIDefinition
 *     through the wired AI-spec flags provider; the jz_test8 gate
 *     (aidef byte@0 & 1 == 0, c_record.cpp:1385) is computed
 *     data-backed and receipted ai_bit0_clear (-style unknown when no
 *     provider/row: ai_flags_known == 0, gate treated as not taken);
 *   - inside the gate, the table1d607e[GDAT CREATURES word@1] & 0x4
 *     probe, the DM2_CHANGE_CURRENT_MAP_TO swap and DM2_INVOKE_MESSAGE
 *     scheduling (c_record.cpp:1387-1406) stay unbound (table owner and
 *     message system unproven) — receipted invoke_message_unbound;
 *   - record byte@5 != 0xff clears the owning CAII slot's byte@1a
 *     (c_record.cpp:1408-1413) — BOUND when `caii` is non-NULL (in the
 *     0fcb call order byte@5 is already -1, so this only fires for
 *     direct callers);
 *   - the mutating tail — tile-rooted DM2_MOVE_RECORD_TO cut
 *     (c_record.cpp:1419), DM2_DROP_CREATURE_POSSESSION
 *     (c_record.cpp:1422), DM2_1c9a_0247 tagged-dballoc cleanup
 *     (c_record.cpp:1423) and DM2_DEALLOC_RECORD (c_record.cpp:1424) —
 *     stays unbound behind the DM2-002 ground-stack/possession blockers
 *     and the unmodelled dballoc free chain; each receipted, never
 *     simulated.
 *
 * Returns 1 when the creature resolved and the decision head was
 * computed (receipt.valid == 1); 0 (fail-closed, receipted) otherwise.
 */
int dm2_v1_caii_delete_creature_record_head(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    int x, int y,
    DM2_V1_CaiiDeleteCreatureRecordReceipt *receipt);

/*
 * ATTACK_CREATURE CAII-alloc gate (skproject/SKULLWIN/c_creature.cpp:
 * 370-385): when a creature record owns no CAII slot (record byte@5 ==
 * 0xff — caller-owned state, NOT re-checked here), the source resolves
 * the record's creature type (record byte@4) through
 * DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD and reads the vl_18 gate =
 * AIDefinition word@0 & 0x1 (c_creature.cpp:374-378); it allocs the
 * CAII slot only when vl_18 != 0 and returns early otherwise
 * (c_creature.cpp:379-385).
 *
 * This accessor binds that flag gate data-backed through the wired
 * AI-spec flags provider (dm2_v1_caii_set_ai_spec_flags_fn — the proven
 * provider resolves it over the GDAT extended-mode AI table).  Returns
 * 1 when the gate permits allocation (flags bit0 set),
 * 0 when the source would return early (flags bit0 clear) or the handle
 * is not a creature record (fail-closed), and -1 when no provider is
 * wired or the current session loaded no AI row for the record's
 * creature type (unknown — fail-closed for callers that require
 * provenance).
 */
int dm2_v1_caii_attack_guard_allows_alloc(
    DM2_V1_RecordPoolSet *pool_set,
    int16_t record_handle);

#endif
