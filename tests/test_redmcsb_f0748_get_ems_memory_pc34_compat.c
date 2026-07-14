#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0748_get_ems_memory_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

#define REQUIRE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "requirement failed: %s at line %d\n", #condition, \
                    __LINE__); \
            return 1; \
        } \
    } while (0)

typedef struct {
    bool present;
    uint16_t unallocated_page_count;
    int16_t version;
    redmcsb_f0748_physical_page_pc34_compat pages[64];
    uint16_t page_count;
    uint16_t page_frame_segment;
    uint16_t handle_to_return;
    int allocate_call_count;
    uint16_t allocated_page_count;
    int map_multiple_call_count;
    uint16_t mapped_handle;
    uint16_t mapped_entry_count;
    redmcsb_f0748_physical_page_pc34_compat mapped_entries[64];
    int map_page_call_count;
    bool map_page_handle_mismatch;
    uint16_t mapped_physical_pages[4];
    uint16_t mapped_logical_pages[4];
} capture;

static bool is_present(void *context) { return ((capture *)context)->present; }
static uint16_t get_pages(void *context)
{
    return ((capture *)context)->unallocated_page_count;
}
static int16_t get_version(void *context) { return ((capture *)context)->version; }
static uint16_t get_addresses(
    void *context,
    redmcsb_f0748_physical_page_pc34_compat *entries,
    uint16_t capacity)
{
    capture *state = context;

    if (state->page_count > capacity) {
        return state->page_count;
    }
    memcpy(entries, state->pages,
           (size_t)state->page_count * sizeof(state->pages[0]));
    return state->page_count;
}
static uint16_t allocate(void *context, uint16_t page_count)
{
    capture *state = context;

    state->allocate_call_count++;
    state->allocated_page_count = page_count;
    return state->handle_to_return;
}
static void map_multiple(
    void *context,
    uint16_t handle,
    const redmcsb_f0748_physical_page_pc34_compat *entries,
    uint16_t entry_count)
{
    capture *state = context;

    state->map_multiple_call_count++;
    state->mapped_handle = handle;
    state->mapped_entry_count = entry_count;
    memcpy(state->mapped_entries, entries,
           (size_t)entry_count * sizeof(state->mapped_entries[0]));
}
static uint16_t get_frame(void *context)
{
    return ((capture *)context)->page_frame_segment;
}
static void map_one(void *context, uint16_t handle, uint16_t physical,
                    uint16_t logical)
{
    capture *state = context;
    int index = state->map_page_call_count++;

    if (handle != state->handle_to_return) {
        state->map_page_handle_mismatch = true;
    }
    state->mapped_physical_pages[index] = physical;
    state->mapped_logical_pages[index] = logical;
}

int main(void)
{
    capture state = {0};
    redmcsb_f0748_ems_pc34_compat ems = {
        is_present, get_pages, get_version, get_addresses, allocate,
        map_multiple, get_frame, map_one, &state};
    uint16_t handle = UINT16_C(0xbeef);
    uint16_t page_frame = UINT16_C(0xffff);

    REQUIRE(redmcsb_f0748_get_ems_memory_pc34_compat(
                &ems, &handle, &page_frame, UINT16_C(0)) == 0);
    REQUIRE(page_frame == 0);
    REQUIRE(state.allocate_call_count == 0);

    state.present = true;
    state.version = 4;
    state.unallocated_page_count = 3;
    state.handle_to_return = UINT16_C(0x31);
    state.page_count = 5;
    state.pages[0] = (redmcsb_f0748_physical_page_pc34_compat){
        UINT16_C(0xb800), UINT16_C(0)};
    state.pages[1] = (redmcsb_f0748_physical_page_pc34_compat){
        UINT16_C(0xc800), UINT16_C(2)};
    state.pages[2] = (redmcsb_f0748_physical_page_pc34_compat){
        UINT16_C(0xcc00), UINT16_C(3)};
    state.pages[3] = (redmcsb_f0748_physical_page_pc34_compat){
        UINT16_C(0xd000), UINT16_C(4)};
    state.pages[4] = (redmcsb_f0748_physical_page_pc34_compat){
        UINT16_C(0xe000), UINT16_C(5)};
    REQUIRE(redmcsb_f0748_get_ems_memory_pc34_compat(
                &ems, &handle, &page_frame, UINT16_C(0)) ==
            (INT32_C(3) << 14));
    REQUIRE(handle == UINT16_C(0x31));
    REQUIRE(page_frame == UINT16_C(0xc800));
    REQUIRE(state.allocated_page_count == 3);
    REQUIRE(state.map_multiple_call_count == 1);
    REQUIRE(state.mapped_handle == UINT16_C(0x31));
    REQUIRE(state.mapped_entry_count == 3);
    REQUIRE(state.mapped_entries[0].page_frame_segment == 0);
    REQUIRE(state.mapped_entries[0].physical_page == UINT16_C(0xc800));
    REQUIRE(state.mapped_entries[2].page_frame_segment == 2);
    REQUIRE(state.mapped_entries[2].physical_page == UINT16_C(0xd000));

    memset(&state, 0, sizeof(state));
    state.present = true;
    state.version = 3;
    state.unallocated_page_count = 9;
    state.page_frame_segment = UINT16_C(0xe000);
    state.handle_to_return = UINT16_C(0x44);
    REQUIRE(redmcsb_f0748_get_ems_memory_pc34_compat(
                &ems, &handle, &page_frame, UINT16_C(0)) ==
            (INT32_C(4) << 14));
    REQUIRE(page_frame == UINT16_C(0xe000));
    REQUIRE(state.allocated_page_count == 4);
    REQUIRE(state.map_page_call_count == 4);
    REQUIRE(!state.map_page_handle_mismatch);
    REQUIRE(state.mapped_physical_pages[3] == 3);
    REQUIRE(state.mapped_logical_pages[3] == 3);

    puts("ok: ReDMCSB F0748 PC 3.4 EMS allocation");
    return 0;
}
