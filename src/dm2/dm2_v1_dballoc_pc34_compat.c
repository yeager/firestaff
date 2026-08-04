/*
 * dm2_v1_dballoc_pc34_compat.c — DM2 database allocation module.
 *
 * Source: skproject c_dballoc.cpp
 * Implements the CPX linked-list free-block allocator, preserved-GFX
 * binary-search table, GFX table compactor, and CPX heap allocator.
 */

#include "dm2_v1_dballoc_pc34_compat.h"
#include <string.h>

/* ========================================================================
 * CPX linked list
 * ======================================================================== */

void dm2_v1_dballoc_cpx_linklist_init(DM2_V1_CpxLinklist *list)
{
    list->ptrlist = NULL;
}

bool dm2_v1_dballoc_cpx_linklist_is_empty(const DM2_V1_CpxLinklist *list)
{
    return list->ptrlist == NULL;
}

/*
 * Insert node into the free list in descending order by amount.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX_LINK_NODE (SKW_3e74_0d32)
 */
void dm2_v1_dballoc_cpx_link_node(DM2_V1_CpxLinklist *list,
                                   DM2_V1_DballocLpp *node)
{
    if (list->ptrlist == NULL) {
        list->ptrlist = node;
        node->next = NULL;
        node->prev = NULL;
        return;
    }

    DM2_V1_DballocLpp *liststart = list->ptrlist;
    int32_t amount = node->amount;

    /* Insert at head if amount >= head's amount */
    if (amount >= liststart->amount) {
        list->ptrlist = node;
        node->prev = NULL;
        node->next = liststart;
        liststart->prev = node;
        return;
    }

    /* Walk to find insertion point */
    DM2_V1_DballocLpp *next;
    while ((next = liststart->next) != NULL) {
        if (amount >= next->amount) {
            next->prev = node;
            liststart->next = node;
            node->prev = liststart;
            node->next = next;
            return;
        }
        liststart = next;
    }

    /* Append at tail */
    liststart->next = node;
    node->prev = liststart;
    node->next = NULL;
}

/*
 * Remove node from the free list.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX_UNLINK_NODE (SKW_3e74_0c8c)
 */
void dm2_v1_dballoc_cpx_unlink_node(DM2_V1_CpxLinklist *list,
                                     DM2_V1_DballocLpp *node)
{
    DM2_V1_DballocLpp *prev = node->prev;
    DM2_V1_DballocLpp *next = node->next;

    if (prev != NULL) {
        prev->next = next;
        if (next != NULL)
            next->prev = prev;
    } else if (next != NULL) {
        list->ptrlist = next;
        next->prev = prev;  /* NULL */
    } else {
        list->ptrlist = NULL;
    }
}

/*
 * Find a free block and optionally split it.
 * skproject: c_cpx_linklist::DM2_ALLOC_CPX1
 *
 * Walks the list looking for an exact match on amount. If no exact match,
 * falls back to the head (largest). Splits the remainder if >= CPX_MIN_SPLIT.
 */
DM2_V1_Cpx1Receipt dm2_v1_dballoc_cpx1(DM2_V1_CpxLinklist *list,
                                         int32_t amount)
{
    DM2_V1_Cpx1Receipt receipt;
    DM2_V1_DballocLpp *lpptr = list->ptrlist;

    /* Search for exact match or fall back to head */
    while (amount != lpptr->amount) {
        if (amount > lpptr->amount || lpptr->next == NULL) {
            lpptr = list->ptrlist;
            break;
        }
        lpptr = lpptr->next;
    }

    dm2_v1_dballoc_cpx_unlink_node(list, lpptr);

    uint8_t *xptr = (uint8_t *)lpptr;
    int32_t diff = lpptr->amount - amount;

    if (diff < DM2_DBALLOC_CPX_MIN_SPLIT) {
        /* Too small to split — use entire block */
        amount = lpptr->amount;
    } else {
        /* Split: create a new free node from the remainder */
        uint8_t *remainder = xptr + amount;
        /* Write the trailing size marker */
        *(int32_t *)(remainder + diff - 4) = diff;
        /* Write the leading size (amount field of new node) */
        *(int32_t *)(remainder) = diff;
        dm2_v1_dballoc_cpx_link_node(list, (DM2_V1_DballocLpp *)remainder);
    }

    receipt.allocated_amount = amount;
    receipt.block_ptr = xptr;
    return receipt;
}

/* ========================================================================
 * Preserved GFX table — binary search
 * ======================================================================== */

/*
 * Binary search for a graphics ID in the preserved GFX table.
 * skproject: c_dballochandler::DM2_FIND_IN_PRESERVED_GFX (SKW_3e74_5420)
 *
 * The table is kept sorted by ID via preservedgfx_idxtable indirection.
 * Returns found=true with the position, or found=false with the insertion point.
 */
DM2_V1_PreservedGfxFindResult dm2_v1_dballoc_find_in_preserved_gfx(
    uint32_t id,
    const uint32_t *preservedgfx_table,
    const int16_t  *preservedgfx_idxtable,
    int16_t         count)
{
    DM2_V1_PreservedGfxFindResult result;
    int16_t lo = -1;
    int16_t hi = count;

    for (;;) {
        int16_t mid = (int16_t)((lo + hi) >> 1);
        if (mid == lo) {
            result.found = false;
            result.index = (int16_t)(mid + 1);
            return result;
        }

        uint32_t val = preservedgfx_table[preservedgfx_idxtable[mid]];
        if (id > val)
            lo = mid;
        else if (id == val) {
            result.found = true;
            result.index = mid;
            return result;
        } else
            hi = mid;
    }
}

/*
 * Add a new entry to the preserved GFX table at insert_pos.
 * skproject: c_dballochandler::DM2_ADD_TO_PRESERVED_GFX (SKW_3e74_54a1)
 */
DM2_V1_AddPreservedGfxReceipt dm2_v1_dballoc_add_to_preserved_gfx(
    uint32_t  id,
    int16_t   insert_pos,
    int16_t  *preservedgfx_idxtable,
    uint32_t *preservedgfx_table,
    int16_t  *count,
    int16_t   max_entries,
    const DM2_V1_AddPreservedGfxCallbacks *cb,
    void     *ctx)
{
    DM2_V1_AddPreservedGfxReceipt receipt;
    receipt.evicted = false;

    /* If table is full, evict via callback */
    if (*count == max_entries) {
        receipt.evicted = true;
        if (cb && cb->evict_cb)
            cb->evict_cb(ctx, id, &insert_pos);
    }

    /* Allocate a slot index */
    int16_t dbidx = DM2_DBALLOC_NODATA;
    if (cb && cb->alloc_slot_cb)
        dbidx = cb->alloc_slot_cb(ctx);

    /* Shift entries right to make room at insert_pos */
    int16_t shift_count = *count - insert_pos;
    if (!receipt.evicted)
        shift_count = *count - insert_pos;
    else
        shift_count = (*count) - insert_pos;

    if (shift_count > 0) {
        memmove(&preservedgfx_idxtable[insert_pos + 1],
                &preservedgfx_idxtable[insert_pos],
                (size_t)shift_count * sizeof(int16_t));
    }

    preservedgfx_idxtable[insert_pos] = dbidx;
    preservedgfx_table[dbidx] = id;

    if (!receipt.evicted)
        (*count)++;

    receipt.dbidx = dbidx;
    return receipt;
}

/* ========================================================================
 * W-table lookup
 * ======================================================================== */

/*
 * Look up a dbidx in w_table1 (if bit 15 set) or w_table2.
 * skproject: c_dballochandler::DM2_ALLOC_CPX_GET_WTABLE_ENTRY (SKW_3e74_0c62)
 */
int16_t dm2_v1_dballoc_get_wtable_entry(int16_t dbidx,
                                         const int16_t *w_table1,
                                         const int16_t *w_table2)
{
    const int16_t *table;
    if (dbidx & (int16_t)0x8000) {
        dbidx &= 0x7fff;
        table = w_table1;
    } else {
        table = w_table2;
    }
    return table[dbidx];
}

/* ========================================================================
 * SET_PPPW_ENTRY
 * ======================================================================== */

/*
 * Set up a pool pointer descriptor.
 * skproject: c_dballochandler::DM2_SET_PPPW_ENTRY (R_2D7EC)
 */
void dm2_v1_dballoc_set_pppw_entry(DM2_V1_DballocPppw *entry,
                                    int16_t *ptr,
                                    int32_t  offset,
                                    int16_t  mode)
{
    entry->wp_00 = ptr;
    entry->wp_04 = ptr;
    entry->wp_08 = ptr;
    if (ptr != NULL) {
        /* Adjust base pointer by offset (in bytes, divided by sizeof(int16_t)) */
        entry->wp_04 = ptr - offset / 2;
    }
    entry->w_0c = mode;
}

/* ========================================================================
 * GFX table compaction
 * ======================================================================== */

/*
 * Compact the GFX table by sliding allocated blocks down, removing free gaps.
 * skproject: c_dballochandler::DM2_INIT_GFX_TABLE (SKW_3e74_2b30)
 */
DM2_V1_GfxCompactReceipt dm2_v1_dballoc_init_gfx_table(
    DM2_V1_CpxLinklist *list,
    DM2_V1_GfxCompactState *state,
    uint8_t *bigpool_start_b,
    uint8_t *bigpool_start_a,
    const DM2_V1_GfxCompactCallbacks *cb,
    void *ctx)
{
    DM2_V1_GfxCompactReceipt receipt;
    receipt.compacted = false;
    receipt.new_start_a = bigpool_start_a;

    if (dm2_v1_dballoc_cpx_linklist_is_empty(list))
        return receipt;

    uint8_t *ptr_src = bigpool_start_b;
    uint8_t *ptr_dest = bigpool_start_b;

    while (ptr_src != bigpool_start_a) {
        DM2_V1_DballocXmalloc *xm = (DM2_V1_DballocXmalloc *)ptr_src;
        int32_t amount = xm->l_00;

        if (amount <= 0) {
            /* Allocated block (negative size) — move it */
            amount = -amount;
            if (ptr_src != ptr_dest) {
                receipt.compacted = true;
                /* Update gfx_table pointer */
                if (cb && cb->get_wtable_entry) {
                    int16_t idx = cb->get_wtable_entry(ctx, xm->dbidx3);
                    state->gfx_table[idx] = (DM2_V1_DballocXmalloc *)ptr_dest;
                }
                memmove(ptr_dest, ptr_src, (size_t)amount);

                DM2_V1_DballocXmalloc *dest_xm = (DM2_V1_DballocXmalloc *)ptr_dest;
                if (xm == state->malloch)
                    state->malloch = dest_xm;
                if (xm == state->mallocg)
                    state->mallocg = dest_xm;
                if (xm == state->malloce)
                    state->malloce = dest_xm;
                if (xm == state->mallocf)
                    state->mallocf = dest_xm;
            }
            ptr_dest += amount;
        }
        /* Free block (positive size) — skip it */
        ptr_src += (amount > 0) ? amount : (-amount);
    }

    receipt.new_start_a = ptr_dest;
    dm2_v1_dballoc_cpx_linklist_init(list);
    return receipt;
}

/* ========================================================================
 * CPX heap allocator
 * ======================================================================== */

/*
 * Ensure at least 'amount' bytes are free in the CPX heap.
 * skproject: c_dballochandler::DM2_GUARANTEE_FREE_CPXHEAP_SIZE
 */
bool dm2_v1_dballoc_guarantee_free_cpxheap_size(
    DM2_V1_CpxHeapState *heap,
    int32_t amount,
    const DM2_V1_GuaranteeFreeCallbacks *cb,
    void *ctx)
{
    while (amount > heap->cpx_available) {
        if (heap->malloch == NULL) {
            if (cb && cb->raise_syserr)
                cb->raise_syserr(ctx, 0x2b);
            return false;
        }
        if (cb && cb->dealloc_entry)
            cb->dealloc_entry(ctx, heap->malloch->dbidx3);
    }
    return true;
}

/*
 * Allocate a block from the CPX heap.
 * skproject: c_dballochandler::DM2_ALLOC_CPXHEAP_CREATE_POINTER
 */
DM2_V1_CpxHeapCreatePtrReceipt dm2_v1_dballoc_cpxheap_create_pointer(
    DM2_V1_CpxHeapState *heap,
    int32_t amount,
    const DM2_V1_GfxCompactCallbacks *compact_cb,
    const DM2_V1_GuaranteeFreeCallbacks *free_cb,
    void *ctx)
{
    DM2_V1_CpxHeapCreatePtrReceipt receipt;
    receipt.compacted = false;

    dm2_v1_dballoc_guarantee_free_cpxheap_size(heap, amount, free_cb, ctx);

    uint8_t *xptr = heap->bigpool_start_a;
    int32_t space = (int32_t)(heap->bigpool_endofunpreservedfree - xptr);

    if (space < amount) {
        if (!dm2_v1_dballoc_cpx_linklist_is_empty(&heap->freelist) &&
            amount <= heap->freelist.ptrlist->amount) {
            /* Use a free block from the list */
            DM2_V1_Cpx1Receipt cpx1 = dm2_v1_dballoc_cpx1(&heap->freelist, amount);
            xptr = cpx1.block_ptr;
            amount = cpx1.allocated_amount;
        } else {
            /* Compact and retry */
            DM2_V1_GfxCompactState compact_state;
            compact_state.gfx_table = heap->gfx_table;
            compact_state.malloch = heap->malloch;
            compact_state.mallocg = heap->mallocg;
            compact_state.malloce = heap->malloce;
            compact_state.mallocf = heap->mallocf;

            DM2_V1_GfxCompactReceipt cr = dm2_v1_dballoc_init_gfx_table(
                &heap->freelist, &compact_state,
                heap->bigpool_start_b, heap->bigpool_start_a,
                compact_cb, ctx);

            heap->malloch = compact_state.malloch;
            heap->mallocg = compact_state.mallocg;
            heap->malloce = compact_state.malloce;
            heap->mallocf = compact_state.mallocf;

            xptr = cr.new_start_a;
            heap->bigpool_start_a = xptr + amount;
            receipt.compacted = true;
        }
    } else {
        heap->bigpool_start_a = xptr + amount;
    }

    DM2_V1_DballocXmalloc *xmptr = (DM2_V1_DballocXmalloc *)xptr;
    heap->cpx_available -= amount;
    xmptr->l_00 = -amount;
    /* Write trailing size marker */
    *(int32_t *)(xptr + amount - 4) = -amount;
    xmptr->prev_idx = DM2_DBALLOC_NODATA;
    xmptr->next_idx = DM2_DBALLOC_NODATA;
    xmptr->dbidx0 = DM2_DBALLOC_NODATA;

    receipt.block = xmptr;
    return receipt;
}

/*
 * Get the next free gfx_table slot.
 * skproject: c_dballochandler::DM2_ALLOC_CPXHEAP_CREATE_INDEX (R_2E4B9)
 */
int16_t dm2_v1_dballoc_cpxheap_create_index(
    DM2_V1_CpxHeapState *heap,
    void (*dealloc_entry)(void *ctx, int16_t dbidx3),
    void *ctx)
{
    int16_t ww = heap->mallocm;
    if (ww == DM2_DBALLOC_NODATA) {
        if (dealloc_entry && heap->malloch)
            dealloc_entry(ctx, heap->malloch->dbidx3);
        ww = heap->mallocm;
    }

    heap->mallock++;
    if (heap->mallock < heap->num_gfx_tableentries) {
        do {
            heap->mallocm++;
        } while (heap->gfx_table[heap->mallocm] != NULL);
    } else {
        heap->mallocm = DM2_DBALLOC_NODATA;
    }

    return ww;
}

/* ========================================================================
 * ALLOC_GENERATE_ID
 * ======================================================================== */

/*
 * Generate a unique ID not present in the preserved GFX table.
 * skproject: c_dballochandler::DM2_ALLOC_GENERATE_ID (DM2_dballoc_3e74_53ea)
 */
uint32_t dm2_v1_dballoc_generate_id(
    int16_t *counter,
    const uint32_t *preservedgfx_table,
    const int16_t  *preservedgfx_idxtable,
    int16_t         count)
{
    uint32_t id;
    do {
        id = (uint32_t)((uint16_t)(*counter)++) | 0xFFFF0000u;
    } while (dm2_v1_dballoc_find_in_preserved_gfx(
                id, preservedgfx_table, preservedgfx_idxtable, count).found);
    return id;
}
