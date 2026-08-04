#ifndef DM2_V1_DBALLOC_PC34_COMPAT_H
#define DM2_V1_DBALLOC_PC34_COMPAT_H

/*
 * dm2_v1_dballoc_pc34_compat.h — DM2 database allocation module.
 *
 * Source: skproject c_dballoc.cpp (35 functions).
 * Covers the CPX linked-list free-block allocator, the preserved-GFX
 * binary-search table, the GFX index table compactor, and the CPX heap
 * pointer/index allocator.  All public functions use callback-based
 * architecture with opaque context pointers.
 *
 * The module is split into three layers:
 *   1. CPX linked list  — sorted doubly-linked free-block list
 *   2. Preserved GFX    — binary-search table for cached graphics IDs
 *   3. CPX heap          — pointer/index allocator with compaction
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_DBALLOC_NODATA          ((int16_t)-1)
#define DM2_DBALLOC_CPX_MIN_SPLIT   30

/* ========================================================================
 * Core data structures
 * ======================================================================== */

/*
 * s_lpp — linked-list node for free memory blocks.
 * Stored at the start of each free block in the CPX heap.
 * skproject: s_lpp in c_alloc.h
 */
typedef struct DM2_V1_DballocLpp {
    int32_t  amount;    /* size of the free block in bytes */
    struct DM2_V1_DballocLpp *prev;
    struct DM2_V1_DballocLpp *next;
} DM2_V1_DballocLpp;

/*
 * s_xmalloc — allocated block header in the CPX heap.
 * skproject: s_xmalloc in c_alloc.h
 * The l_00 field is negative for allocated blocks, positive for free.
 */
typedef struct DM2_V1_DballocXmalloc {
    int32_t  l_00;       /* negative = allocated size, positive = free size */
    DM2_V1_DballocLpp lpp; /* embedded free-list node (used when free) */
    int16_t  dbidx0;    /* allocation status / usage counter */
    int16_t  dbidx3;    /* owner db index (with 0x8000 flag) */
    int16_t  prev_idx;  /* previous in LRU chain */
    int16_t  next_idx;  /* next in LRU chain */
} DM2_V1_DballocXmalloc;

/*
 * s_pppw — pool pointer descriptor.
 * skproject: s_pppw in c_dballoc.h
 */
typedef struct DM2_V1_DballocPppw {
    int16_t *wp_00;     /* current pointer */
    int16_t *wp_04;     /* base pointer (adjusted by offset) */
    int16_t *wp_08;     /* original pointer */
    int16_t  w_0c;      /* pool mode */
} DM2_V1_DballocPppw;

/* ========================================================================
 * CPX linked list — sorted doubly-linked free-block list
 * skproject: c_cpx_linklist class
 * ======================================================================== */

typedef struct DM2_V1_CpxLinklist {
    DM2_V1_DballocLpp *ptrlist;   /* head of sorted free list */
} DM2_V1_CpxLinklist;

/* Receipt for CPX1 (find-and-split) */
typedef struct DM2_V1_Cpx1Receipt {
    int32_t  allocated_amount;   /* actual amount allocated (>= requested) */
    uint8_t *block_ptr;          /* pointer to the allocated block */
} DM2_V1_Cpx1Receipt;

void dm2_v1_dballoc_cpx_linklist_init(DM2_V1_CpxLinklist *list);
bool dm2_v1_dballoc_cpx_linklist_is_empty(const DM2_V1_CpxLinklist *list);

/*
 * DM2_ALLOC_CPX_LINK_NODE — insert node in descending-amount order.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX_LINK_NODE (SKW_3e74_0d32)
 */
void dm2_v1_dballoc_cpx_link_node(DM2_V1_CpxLinklist *list,
                                   DM2_V1_DballocLpp *node);

/*
 * DM2_ALLOC_CPX_UNLINK_NODE — remove node from the list.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX_UNLINK_NODE (SKW_3e74_0c8c)
 */
void dm2_v1_dballoc_cpx_unlink_node(DM2_V1_CpxLinklist *list,
                                     DM2_V1_DballocLpp *node);

/*
 * DM2_ALLOC_CPX1 — find a free block of at least 'amount' bytes,
 * split if remainder >= CPX_MIN_SPLIT.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX1
 */
DM2_V1_Cpx1Receipt dm2_v1_dballoc_cpx1(DM2_V1_CpxLinklist *list,
                                         int32_t amount);

/* ========================================================================
 * Preserved GFX table — binary search for cached graphics IDs
 * skproject: DM2_FIND_IN_PRESERVED_GFX / DM2_ADD_TO_PRESERVED_GFX
 * ======================================================================== */

typedef struct DM2_V1_PreservedGfxFindResult {
    bool    found;
    int16_t index;     /* if found: position; if not: insertion point */
} DM2_V1_PreservedGfxFindResult;

/*
 * DM2_FIND_IN_PRESERVED_GFX — binary search for a graphics ID.
 * skproject: c_dballochandler::DM2_FIND_IN_PRESERVED_GFX (SKW_3e74_5420)
 *
 * preservedgfx_table: array of uint32_t IDs (indexed by dbidx)
 * preservedgfx_idxtable: array of int16_t indices into preservedgfx_table
 * count: number of valid entries in preservedgfx_idxtable
 */
DM2_V1_PreservedGfxFindResult dm2_v1_dballoc_find_in_preserved_gfx(
    uint32_t id,
    const uint32_t *preservedgfx_table,
    const int16_t  *preservedgfx_idxtable,
    int16_t         count);

/*
 * DM2_ADD_TO_PRESERVED_GFX — insert a new entry at the given position.
 * skproject: c_dballochandler::DM2_ADD_TO_PRESERVED_GFX (SKW_3e74_54a1)
 * Returns the dbidx of the newly allocated slot.
 *
 * Callbacks:
 *   alloc_slot_cb: allocates a new preserved slot (DM2_dballoc_2E52C logic)
 *   evict_cb: called when the table is full and an entry must be evicted
 */
typedef struct DM2_V1_AddPreservedGfxCallbacks {
    int16_t (*alloc_slot_cb)(void *ctx);
    void    (*evict_cb)(void *ctx, uint32_t id, int16_t *out_index);
} DM2_V1_AddPreservedGfxCallbacks;

typedef struct DM2_V1_AddPreservedGfxReceipt {
    int16_t dbidx;     /* slot index assigned to the new entry */
    bool    evicted;   /* true if an existing entry was evicted */
} DM2_V1_AddPreservedGfxReceipt;

DM2_V1_AddPreservedGfxReceipt dm2_v1_dballoc_add_to_preserved_gfx(
    uint32_t  id,
    int16_t   insert_pos,
    int16_t  *preservedgfx_idxtable,
    uint32_t *preservedgfx_table,
    int16_t  *count,
    int16_t   max_entries,
    const DM2_V1_AddPreservedGfxCallbacks *cb,
    void     *ctx);

/* ========================================================================
 * W-table lookup
 * skproject: c_dballochandler::DM2_ALLOC_CPX_GET_WTABLE_ENTRY (SKW_3e74_0c62)
 * ======================================================================== */

int16_t dm2_v1_dballoc_get_wtable_entry(int16_t dbidx,
                                         const int16_t *w_table1,
                                         const int16_t *w_table2);

/* ========================================================================
 * SET_PPPW_ENTRY — pool pointer descriptor setup
 * skproject: c_dballochandler::DM2_SET_PPPW_ENTRY (R_2D7EC)
 * ======================================================================== */

void dm2_v1_dballoc_set_pppw_entry(DM2_V1_DballocPppw *entry,
                                    int16_t *ptr,
                                    int32_t  offset,
                                    int16_t  mode);

/* ========================================================================
 * GFX table compaction
 * skproject: c_dballochandler::DM2_INIT_GFX_TABLE (SKW_3e74_2b30)
 * ======================================================================== */

typedef struct DM2_V1_GfxCompactCallbacks {
    /*
     * get_wtable_entry: resolve dbidx3 -> gfx_table index.
     * Used to update gfx_table pointers after compaction moves a block.
     */
    int16_t (*get_wtable_entry)(void *ctx, int16_t dbidx);
} DM2_V1_GfxCompactCallbacks;

typedef struct DM2_V1_GfxCompactState {
    DM2_V1_DballocXmalloc **gfx_table;
    DM2_V1_DballocXmalloc  *malloch;
    DM2_V1_DballocXmalloc  *mallocg;
    DM2_V1_DballocXmalloc  *malloce;
    DM2_V1_DballocXmalloc  *mallocf;
} DM2_V1_GfxCompactState;

typedef struct DM2_V1_GfxCompactReceipt {
    bool     compacted;           /* true if any blocks were moved */
    uint8_t *new_start_a;         /* new bigpool_start_a after compaction */
} DM2_V1_GfxCompactReceipt;

DM2_V1_GfxCompactReceipt dm2_v1_dballoc_init_gfx_table(
    DM2_V1_CpxLinklist *list,
    DM2_V1_GfxCompactState *state,
    uint8_t *bigpool_start_b,
    uint8_t *bigpool_start_a,
    const DM2_V1_GfxCompactCallbacks *cb,
    void *ctx);

/* ========================================================================
 * CPX heap allocator — pointer creation
 * skproject: c_dballochandler::DM2_ALLOC_CPXHEAP_CREATE_POINTER
 *            (SKW_ALLOC_LOWER_CPXHEAP)
 * ======================================================================== */

typedef struct DM2_V1_CpxHeapState {
    DM2_V1_CpxLinklist      freelist;
    uint8_t                *bigpool_start_a;  /* next-alloc pointer */
    uint8_t                *bigpool_start_b;  /* base of CPX heap */
    uint8_t                *bigpool_endofunpreservedfree; /* upper bound */
    int32_t                 cpx_available;
    DM2_V1_DballocXmalloc **gfx_table;
    int16_t                 num_gfx_tableentries;
    int16_t                 mallock;   /* count of allocated entries */
    int16_t                 mallocm;   /* next free gfx_table slot */
    DM2_V1_DballocXmalloc  *malloch;
    DM2_V1_DballocXmalloc  *mallocg;
    DM2_V1_DballocXmalloc  *malloce;
    DM2_V1_DballocXmalloc  *mallocf;
} DM2_V1_CpxHeapState;

typedef struct DM2_V1_CpxHeapCreatePtrReceipt {
    DM2_V1_DballocXmalloc *block;    /* allocated block header */
    bool                   compacted; /* true if GFX table was compacted */
} DM2_V1_CpxHeapCreatePtrReceipt;

/*
 * DM2_GUARANTEE_FREE_CPXHEAP_SIZE — ensure at least 'amount' bytes free.
 * skproject: c_dballochandler::DM2_GUARANTEE_FREE_CPXHEAP_SIZE
 */
typedef struct DM2_V1_GuaranteeFreeCallbacks {
    void (*dealloc_entry)(void *ctx, int16_t dbidx3);
    void (*raise_syserr)(void *ctx, int32_t code);
} DM2_V1_GuaranteeFreeCallbacks;

bool dm2_v1_dballoc_guarantee_free_cpxheap_size(
    DM2_V1_CpxHeapState *heap,
    int32_t amount,
    const DM2_V1_GuaranteeFreeCallbacks *cb,
    void *ctx);

/*
 * DM2_ALLOC_CPXHEAP_CREATE_POINTER — allocate a block from the CPX heap.
 * skproject: c_dballochandler::DM2_ALLOC_CPXHEAP_CREATE_POINTER
 */
DM2_V1_CpxHeapCreatePtrReceipt dm2_v1_dballoc_cpxheap_create_pointer(
    DM2_V1_CpxHeapState *heap,
    int32_t amount,
    const DM2_V1_GfxCompactCallbacks *compact_cb,
    const DM2_V1_GuaranteeFreeCallbacks *free_cb,
    void *ctx);

/*
 * DM2_ALLOC_CPXHEAP_CREATE_INDEX — get the next free gfx_table slot.
 * skproject: c_dballochandler::DM2_ALLOC_CPXHEAP_CREATE_INDEX (R_2E4B9)
 */
int16_t dm2_v1_dballoc_cpxheap_create_index(
    DM2_V1_CpxHeapState *heap,
    void (*dealloc_entry)(void *ctx, int16_t dbidx3),
    void *ctx);

/* ========================================================================
 * ALLOC_GENERATE_ID — unique ID generator
 * skproject: c_dballochandler::DM2_ALLOC_GENERATE_ID (DM2_dballoc_3e74_53ea)
 * ======================================================================== */

uint32_t dm2_v1_dballoc_generate_id(
    int16_t *counter,
    const uint32_t *preservedgfx_table,
    const int16_t  *preservedgfx_idxtable,
    int16_t         count);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_DBALLOC_PC34_COMPAT_H */
