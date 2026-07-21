#ifndef DM2_V1_CAII_ALLOC_PC34_COMPAT_H
#define DM2_V1_CAII_ALLOC_PC34_COMPAT_H

#include "dm2_v1_creature_schedule_pc34_compat.h"
#include "dm2_v1_drops.h"

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

/* Additional optional word-value providers for the ATTACK_CREATURE
 * body: the AIDefinition BaseHP probe (aidef word@4,
 * c_creature.cpp:420-423) and the GDAT CREATURES word@1 table1d607e
 * index (c_creature.cpp:441 + 612).  The proven firestaff providers are
 * dm2_v1_creature_ai_base_hp and dm2_v1_creature_gdat_word1
 * (dm2_v1_creature.h).  Unwired providers fail closed per call site
 * (aggro undecided / gdat_w1_unknown), never simulated. */
typedef int (*DM2_V1_CaiiWordValueFn)(int creature_type,
                                      uint16_t *out_value);
void dm2_v1_caii_set_ai_base_hp_fn(DM2_V1_CaiiWordValueFn fn);
void dm2_v1_caii_set_gdat_word1_fn(DM2_V1_CaiiWordValueFn fn);

/* Read-only accessors over the module-owned providers and the verbatim
 * table1d607e copy, for composed slices living in their own translation
 * unit (the full DM2_DELETE_CREATURE_RECORD composition).
 * dm2_v1_caii_table1d607e_uc0 returns entry byte@0, or -1 when w1 lies
 * outside the proven span 0x2f (the source would read out of bounds). */
DM2_V1_CaiiAiSpecFlagsFn dm2_v1_caii_get_ai_spec_flags_fn(void);
DM2_V1_CaiiWordValueFn dm2_v1_caii_get_gdat_word1_fn(void);
int dm2_v1_caii_table1d607e_uc0(int w1);

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
  int record_delete_full_ran;      /* the wired COMPLETE composition ran
                                      instead of the standalone head
                                      (c_1c9a.cpp:5956-5957) */
  int record_delete_full_completed;/* the wired composition reached a
                                      source return */
  int record_index;
  char source_evidence[200];
} DM2_V1_CaiiFreeReceipt;

/*
 * Wired full-composition hook (2026-07-21 follow-up): when set, the
 * 0fcb branch (c_1c9a.cpp:5956-5957) runs the COMPLETE
 * DM2_DELETE_CREATURE_RECORD composition
 * (dm2_v1_delete_creature_record_full) instead of the standalone
 * decision head.  The caii module must not depend on the composition's
 * translation unit (its link boundary), so the session that owns the
 * composition wires the hook — exactly like the provider hooks above.
 * The hook mirrors the source call DM2_DELETE_CREATURE_RECORD(x, y, 0,
 * 1) and supplies the session context the composition needs (RNG, map
 * id, gametick, party position, GDAT drop slots).  `dungeon` is
 * mutable because the composition's ground-stack writes land in the
 * dungeon data exactly like the source's map state.  Returns the
 * composition's result (1 = ran to a source return).
 */
typedef int (*DM2_V1_CaiiDeleteCreatureFullFn)(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int x, int y,
    void *context);
void dm2_v1_caii_set_delete_creature_full_fn(
    DM2_V1_CaiiDeleteCreatureFullFn fn, void *context);

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
  int invoke_message_unbound;/* map swap and DM2_INVOKE_MESSAGE
                                (c_record.cpp:1390-1406) stay host-owned */
  int invoke_message_would_run;/* data-backed table1d607e[GDAT word@1]
                                uc[0] & 0x4 probe (c_record.cpp:1387-1388):
                                1 = the swap/message branch WOULD run,
                                0 = skipped, -1 = unknown provenance */
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

typedef struct {
  int valid;
  int cut_performed;         /* BOUND: the tile-rooted ground-stack
                                unlink (c_record.cpp:1419 — the
                                DM2_MOVE_RECORD_TO x == -4 skip00823 path
                                c_moverec.cpp:630-683 observable end
                                state) */
  int cut_miss;              /* fail-closed: the record is not chained
                                on the tile */
  int cut_head_rewritten;    /* the record was the chain head: the
                                ground-stack word was rewritten through
                                dm2_v1_dungeon_set_first_thing */
  int cut_side_effects_unbound;/* the 3CE7D timer/text side effects and
                                the recursive DM2_1c9a_0fcb slot free
                                inside the cut (c_moverec.cpp:670-680)
                                stay host-owned — receipted, never
                                simulated */
  int drop_possession_unbound;/* DM2_DROP_CREATURE_POSSESSION
                                 (c_record.cpp:1422) — the RAND16 scatter
                                 + ALLOC_NEW_DBITEM body stays unbound */
  int dballoc_cleanup_unbound;/* DM2_1c9a_0247 tagged-dballoc cleanup
                                 (c_record.cpp:1423, c_1c9a.cpp:5135-5160)
                                 stays unbound */
  int dealloc_performed;     /* BOUND: DM2_DEALLOC_RECORD
                                (c_record.cpp:1205-1208) — record word@0
                                = 0xffff free marker */
  char source_evidence[224];
} DM2_V1_CaiiDeleteCreatureTailReceipt;

/*
 * DM2_DELETE_CREATURE_RECORD's mutating tail
 * (skproject/SKULLWIN/c_record.cpp:1416-1424) — bounded slice, invoked
 * after dm2_v1_caii_delete_creature_record_head with its resolved
 * record.  Bound in source order:
 *   - the tile-rooted cut (c_record.cpp:1419): the record is unlinked
 *     from the cell's ground-stack chain through the proven record-pool
 *     list-cut semantics (DM2_CUT_RECORD_FROM, c_record.cpp:122+) with
 *     the chain head rewritten in the dungeon ground-stack table when
 *     the record was the head (the DM2_MOVE_RECORD_TO x == -4
 *     skip00823/3CE7D path's observable end state,
 *     c_moverec.cpp:630-683).  A bounded membership pre-walk guarantees
 *     the cut cannot spin on a corrupt chain; a record not chained on
 *     the tile fails closed (cut_miss).  `record_handle` must be the
 *     chain word exactly as DM2_GET_CREATURE_AT returned it (direction
 *     bits preserved — the source compares record words);
 *   - the 3CE7D timer/text side effects and the recursive
 *     DM2_1c9a_0fcb slot free inside the cut stay host-owned
 *     (receipted cut_side_effects_unbound);
 *   - DM2_DROP_CREATURE_POSSESSION (c_record.cpp:1422) and the
 *     DM2_1c9a_0247 tagged-dballoc cleanup (c_record.cpp:1423) stay
 *     unbound — receipted, never simulated;
 *   - DM2_DEALLOC_RECORD (c_record.cpp:1424 via c_record.cpp:1205-1208)
 *     is BOUND: the record's first word becomes the 0xffff free marker.
 *
 * `dungeon` is mutable because the ground-stack head write lands in the
 * dungeon data exactly like the source's map state.  Returns 1 when the
 * tail ran to the source end (receipt.valid == 1); 0 (fail-closed,
 * receipted) otherwise.
 */
int dm2_v1_caii_delete_creature_record_tail(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int16_t record_handle,
    int x, int y,
    DM2_V1_CaiiDeleteCreatureTailReceipt *receipt);

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

typedef struct {
  int valid;
  int completed;             /* final 0db0 + 0cf7 reschedule issued */
  int creature_not_found;    /* handle -1 and no creature at (x, y) */
  int ai_flags_unknown;      /* no provider/AI row: fail-closed */
  int denied_static_no_slot; /* byte@5 == 0xff && vl_18 == 0 early return */
  int alloc_performed;       /* DM2_ALLOC_CAII_TO_CREATURE ran (bound) */
  int alloc_failed;
  int16_t record_handle;     /* resolved DB4 handle (direction bits kept) */
  int creature_type;         /* record byte@4 */
  int hp_word_after;         /* slot word@0x14 after the add (int16) */
  int aggro_evaluated;       /* vl_18 == 0 && strength > 0 && bit2 clear */
  int aggro_set;             /* record word@0xa bit 2 set (c_creature.cpp:433) */
  int aggro_undecided;       /* RNG band without a stream, or zero BaseHP */
  int rng_unbound;           /* a source draw had no session stream */
  int ai_turn_unbound;       /* rg7 != 0 but the turn block could not run
                                bound (gate unknown/out of span, or no
                                session stream) — fail-closed stop */
  int ai_turn_gate_passed;   /* table1d607e uc[0] & 0x80 == 0 (0/1, -1 n/a) */
  int rng_stream_diverged;   /* gate passed but no stream was bound: the
                                turn block's draws are not consumable —
                                stop BEFORE the reaction roll */
  int ai_turn_ran;           /* the bound c_ai turn block executed */
  int ai_turn_entry_roll;    /* c_creature.cpp:444 RANDBIT (0/1, -1 n/a) */
  int ai_turn_vector_dir;    /* CALC_VECTOR_DIR result 0-3 (-1 n/a) */
  int ai_turn_facing;        /* record word@0xe >> 8 & 3 (-1 n/a) */
  int ai_turn_dir;           /* final RG3: 0-3 absolute, 6/7 relative,
                                -1 no turn, -2 block not entered */
  int ai_turn_applied;       /* DM2_ai_13e4_0360 argl0 == 0 wrote slot
                                byte@0x17 (c_ai.cpp:5946) */
  int ai_turn_guard_denied;  /* slot byte@0x17/0x1a == 0x13 guard
                                (c_ai.cpp:5941-5944) */
  int reaction_roll;         /* RAND16(100) draw, -1 when not drawn */
  int reaction_success;      /* vl_14: strength > draw */
  int champion_bit_set;      /* record word@0xa |= (1 << champion) */
  int champion_bit_cleared;  /* record word@0xa &= ~(1 << champion) */
  int final_rg1;             /* the reschedule gate value (0/1) */
  int gdat_w1_unknown;       /* CREATURES word@1 not loaded for the type */
  int gdat_w1_out_of_span;   /* table1d607e span 0x2f exceeded (source OOB) */
  int mode_b1a_out_of_span;  /* slot byte@1a beyond table1d613a span 0x55 */
  int dying_mode;            /* slot byte@1a == 0x13 early return */
  int below_threshold;       /* rg1 == 0 && hp_word < record word@6 */
  int timer_cancelled;       /* bound DM2_1c9a_0db0 removed a pending timer */
  int rescheduled;           /* bound DM2_1c9a_0cf7 enqueued the think timer */
  uint32_t timer_ticket;
  char source_evidence[224];
} DM2_V1_CaiiAttackReceipt;

/*
 * DM2_ATTACK_CREATURE (skproject/SKULLWIN/c_creature.cpp:318-649)
 * bounded slice — the creature-side reaction to a party attack.
 * Source's callers: melee/missile hit paths (c_creature.cpp:996,
 * c_creature.cpp:1335).  Bound in source order:
 *   - handle -1 resolves via DM2_GET_CREATURE_AT(x, y) with the source
 *     early return (c_creature.cpp:345-352);
 *   - the vol_00 flag-word dance (bit 0x4000 RANDBIT clear, bit 0x2000
 *     -> vl_10, c_creature.cpp:353-369) over the caller's attack word;
 *   - vl_18 = AIDefinition word@0 & 1 data-backed through the wired
 *     AI-spec flags provider (c_creature.cpp:374-378); unknown
 *     provenance fails closed (ai_flags_unknown);
 *   - record byte@5 == 0xff: vl_18 == 0 takes the source early return
 *     (denied_static_no_slot), else the BOUND
 *     DM2_ALLOC_CAII_TO_CREATURE runs (c_creature.cpp:379-385);
 *   - slot word@0x14 += hp_delta with the source's 16-bit wrap
 *     (c_creature.cpp:389-393);
 *   - the aggro block (c_creature.cpp:394-435): deterministic bands
 *     bound — hp > 0x1e sets, hp <= 4 uses the BaseHP percentage probe
 *     dm2_v1_creature_ai_base_hp through the wired
 *     dm2_v1_caii_set_ai_base_hp_fn provider; zero BaseHP receipts
 *     undecided — the source would divide by zero); the middle band
 *     (5..30) draws RANDDIR from the session stream (rng_unbound
 *     without one);
 *     aggro sets record word@0xa bit 2 and rg7;
 *   - rg7 != 0 enters the source's c_ai turn block
 *     (c_creature.cpp:438-536), now BOUND: the table1d607e uc[0] & 0x80
 *     entry gate (data-backed through the wired gdat_word1 provider),
 *     the entry RANDBIT, DM2_CALC_VECTOR_DIR (util.cpp:30-46, verbatim —
 *     including its tie-break RANDBIT) from the creature's CCM dispatch
 *     coordinates (target_x/target_y = ddat.v1e0270/v1e0272,
 *     c_dballoc.cpp:438-440) toward the attack origin, the full
 *     skip00247/skip00248/skip00251 direction dance over the record's
 *     facing bits (word@0xe >> 8 & 3), and DM2_ai_13e4_0360 with
 *     argl0 == 0 (c_ai.cpp:5912-5960): the slot byte@0x17/0x1a == 0x13
 *     guards and the byte@0x17 direction write.  The argl0 != 0 tail
 *     (byte@0x21 flag / 0db0+0cf7 requeue, c_ai.cpp:5949-5959) is only
 *     reached by OTHER callers (c_ai.cpp:2114, c_tim_proc.cpp:2988) and
 *     stays unbound.  When the gate cannot be determined, or the gate
 *     passes without a bound session stream, the body still stops
 *     BEFORE the reaction roll (rng_stream_diverged) — fail-closed;
 *   - the reaction roll vl_14 = strength > RAND16(100)
 *     (c_creature.cpp:539-543) over the session stream; on success the
 *     champion bit (1 << (vol_00 low byte)) is OR-ed into record
 *     word@0xa when bit 0x8000 was clear, AND-ed out when set
 *     (c_creature.cpp:546-563);
 *   - the reschedule gate (c_creature.cpp:566-635) bound with both
 *     mdata tables verbatim (table1d613a mdata.c:1615-1639,
 *     table1d607e mdata.c:1564-1613 — per-module source-locked copies
 *     keep the link boundary): rg1 = 1 when vl_18 == 0 && vl_10 != 0 &&
 *     strength == 0; else 0 when the reaction roll failed; else the
 *     table1d613a[slot byte@1a] & 0x10 / table1d607e[GDAT word@1] &
 *     0x410 / & 2 chain (GDAT word@1 data-backed through the wired
 *     dm2_v1_caii_set_gdat_word1_fn provider; span violations fail
 *     closed — the source would read out of bounds);
 *   - slot byte@1a == 0x13 takes the source early return (dying_mode,
 *     c_creature.cpp:638-640); rg1 == 0 && hp_word < record word@6
 *     takes the threshold return (below_threshold,
 *     c_creature.cpp:641-646);
 *   - otherwise the bound DM2_1c9a_0db0 + DM2_1c9a_0cf7 pair deletes
 *     the pending timer and re-queues the think timer
 *     (c_creature.cpp:647-648).
 *
 * `rng` is the session's proven c_random LCG stream (DM2_V1_DropRng,
 * c_random.cpp:13-47); NULL receipts rng_unbound and stops before the
 * first RNG-gated mutation.  `x`/`y` are the attack origin coordinates
 * (vql_08/vql_04); `target_x`/`target_y` are the creature's CCM
 * dispatch coordinates (ddat.v1e0270/v1e0272, c_dballoc.cpp:438-440)
 * the source reads as globals — the bounded slice takes them as
 * parameters.  Returns 1 when the body completed through
 * the final reschedule; 0 (fail-closed, receipted) for every source
 * early return and unproven branch.  When `receipt` is non-NULL it
 * always receives the audit record.
 */
int dm2_v1_caii_attack_creature(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    int target_x, int target_y,
    uint32_t attack_word,
    int16_t attack_strength,
    int32_t hp_delta,
    DM2_V1_CaiiAttackReceipt *receipt);

typedef struct {
  int valid;
  int completed;             /* requested action fully applied */
  int creature_not_found;    /* handle -1 and no creature at (x, y) */
  int not_creature_db;       /* handle is not a DB4 creature record */
  int no_slot;               /* record byte@5 == 0xff early return */
  int guard_denied;          /* slot byte@0x17/0x1a == 0x13
                                (c_ai.cpp:5941-5944) */
  int16_t record_handle;     /* resolved DB4 handle */
  int slot_index;            /* record byte@5 */
  int dir_written;           /* slot byte@0x17 = dir (c_ai.cpp:5946) */
  int argl0_tail;            /* argl0 != 0 tail entered
                                (c_ai.cpp:5947-5959) */
  int mode_b1a_out_of_span;  /* byte@1a beyond table1d613a's proven span
                                0x55 — the source would read OOB */
  int flag_set;              /* slot byte@0x21 = 1 (t613a & 0x10,
                                c_ai.cpp:5949-5952) */
  int timer_cancelled;       /* bound DM2_1c9a_0db0 removed a pending
                                timer (c_ai.cpp:5955) */
  int rescheduled;           /* bound DM2_1c9a_0cf7 re-queued the think
                                timer (c_ai.cpp:5956-5958) */
  uint32_t timer_ticket;
  char source_evidence[224];
} DM2_V1_CaiiAiTurnReceipt;

/*
 * DM2_ai_13e4_0360 (skproject/SKULLWIN/c_ai.cpp:5912-5960) — the
 * creature turn/AI-stop action, complete slice.  Source's callers:
 * ATTACK_CREATURE's c_ai turn block (c_creature.cpp:533, argl0 == 0 —
 * also bound inline inside dm2_v1_caii_attack_creature) and the
 * AI-stop paths that pass dir 0x13 with argl0 == 1
 * (c_creature.cpp:233, c_ai.cpp:2114 DM2_PROCEED_XACT_85,
 * c_tim_proc.cpp:2988 DM2_ACTIVATE_CREATURE_KILLER).  Bound in source
 * order:
 *   - handle -1 resolves via DM2_GET_CREATURE_AT(x, y) with the source
 *     early return (c_ai.cpp:5925-5931);
 *   - record byte@5 == 0xff takes the source early return
 *     (c_ai.cpp:5934-5936) — no_slot;
 *   - slot byte@0x17 == 0x13 or byte@0x1a == 0x13 takes the source
 *     guard return (c_ai.cpp:5941-5944) — guard_denied;
 *   - slot byte@0x17 = dir low byte (c_ai.cpp:5945-5946) —
 *     dir_written; argl0 == 0 returns here;
 *   - the argl0 != 0 tail (c_ai.cpp:5949-5959): table1d613a[slot
 *     byte@1a] & 0x10 sets slot byte@0x21 = 1 (flag_set); otherwise
 *     the bound DM2_1c9a_0db0 + DM2_1c9a_0cf7 pair cancels the pending
 *     timer and re-queues the think timer at (x, y).  table1d613a's
 *     proven span is 0x00-0x55 (mdata.c:1615-1639): a byte@1a beyond
 *     it fails closed AFTER the dir write (mode_b1a_out_of_span) —
 *     the source would read out of bounds.
 *
 * `x`/`y` are the creature's coordinates (the source's edxl/ebxl,
 * reused for the requeue).  `map_id`/`game_tick` carry the CCM
 * dispatch context for the bound schedule producer.  Returns 1 when
 * the requested action completed; 0 (fail-closed, receipted) for
 * every source early return and unproven branch.  When `receipt` is
 * non-NULL it always receives the audit record.
 */
int dm2_v1_caii_ai_13e4_0360(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    int dir,
    int argl0,
    DM2_V1_CaiiAiTurnReceipt *receipt);

typedef struct {
  int valid;
  int completed;             /* timer re-queued and ticket stored */
  int suppressed;            /* s350.v1e0570 != 0: the source returns
                                without requeuing (c_ai.cpp:5612-5613) */
  int not_creature_db;       /* handle is not a DB4 creature record */
  int no_slot;               /* record byte@5 == 0xff */
  int timer_type;            /* 0x21/0x22 from the loop result
                                (c_ai.cpp:5609-5611) */
  int timer_cancelled;       /* slot word@2 pending -> bound
                                DM2_1c9a_0db0 (c_ai.cpp:5641-5643) */
  int enqueued;              /* DM2_QUEUE_TIMER issued a ticket
                                (c_ai.cpp:5644) */
  uint32_t timer_ticket;     /* stored into slot word@2 (c_ai.cpp:5646) */
  char source_evidence[224];
} DM2_V1_CaiiCcmEndRequeueReceipt;

/*
 * The c_ai re-queue at the DM2_PROCEED_CCM end
 * (skproject/SKULLWIN/c_ai.cpp:5608-5614 + 5641-5646) — the tail that
 * re-arms the current creature's CCM timer after the message loop.
 * The loop itself (the CCM stream grammar) stays host-owned; its
 * outputs enter this bounded slice as parameters:
 *   - `timer` carries s350.v1e0562's loop-owned payload fields
 *     (actor, valueA, valueB); THIS slice overwrites the type
 *     (c_ai.cpp:5609-5611: (loop_result != 1 ? 1 : 0) + 0x21) and the
 *     ticks/map word (c_ai.cpp:5614: setmticks(v1e0571, delta),
 *     c_timer.h:66 — the source ORs the delta unmasked);
 *   - `loop_result` is RG4W at m_15785;
 *   - `suppress_requeue` is s350.v1e0570: when set the source returns
 *     WITHOUT touching the timer (c_ai.cpp:5612-5613) — receipted
 *     suppressed, fail-closed;
 *   - `mticks_map` is s350.v1e0571 and `mticks_delta` is the
 *     DM2_CREATURE_SOMETHING_1c9a_0a48() result — that animation-frame
 *     reader (c_1c9a.cpp:5434+) stays host-owned, the caller passes
 *     its value.
 * Bound effects in source order: slot word@2 != -1 cancels the
 * pending timer through the bound DM2_1c9a_0db0 (c_ai.cpp:5641-5643),
 * DM2_QUEUE_TIMER enqueues the rebuilt timer over the session queue
 * (c_ai.cpp:5644, c_timer.cpp:235-257), and the issued ticket lands
 * in slot word@2 (c_ai.cpp:5646).  Returns 1 on completion; 0
 * (fail-closed, receipted) otherwise.
 */
int dm2_v1_caii_ccm_end_requeue(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int16_t record_handle,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    int loop_result,
    int suppress_requeue,
    int16_t mticks_map,
    int32_t mticks_delta,
    DM2_V1_CaiiCcmEndRequeueReceipt *receipt);

#endif
