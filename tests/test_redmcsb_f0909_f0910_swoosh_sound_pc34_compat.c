#include "redmcsb_f0909_f0910_swoosh_sound_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

enum { EVENT_CAPACITY = 16 };

typedef struct {
    char events[EVENT_CAPACITY];
    size_t event_count;
    uint16_t command;
    uint8_t flags;
    void *freed_memory;
    long freed_byte_count;
} fixture;

static void push_event(fixture *state, char event)
{
    assert(state->event_count < EVENT_CAPACITY);
    state->events[state->event_count] = event;
    state->event_count += 1U;
}

static void begin_io(void *context, void *request)
{
    fixture *state = context;
    push_event(state, request == (void *)1 ? 'L' : request == (void *)2 ? 'R' : 'C');
}

static void do_io(void *context, void *request)
{
    (void)request;
    fixture *state = context;
    assert(request == (void *)3);
    push_event(state, 'D');
}

static void wait_io(void *context, void *request)
{
    fixture *state = context;
    push_event(state, request == (void *)1 ? 'l' : 'r');
}

static void set_command(void *context, void *request, uint16_t command)
{
    (void)request;
    fixture *state = context;
    assert(request == (void *)3);
    state->command = command;
    push_event(state, command == 10U ? 'S' : command == 20U ? 'F' : 'T');
}

static void set_flag(void *context, void *request, uint8_t flag)
{
    (void)request;
    fixture *state = context;
    assert(request == (void *)3);
    state->flags = (uint8_t)(state->flags | flag);
    push_event(state, '+');
}

static void clear_flag(void *context, void *request, uint8_t flag)
{
    (void)request;
    fixture *state = context;
    assert(request == (void *)3);
    state->flags = (uint8_t)(state->flags & (uint8_t)~flag);
    push_event(state, '-');
}

static void free_memory(void *context, void *memory, long byte_count)
{
    fixture *state = context;
    state->freed_memory = memory;
    state->freed_byte_count = byte_count;
    push_event(state, 'X');
}

static redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat make_host(
    fixture *state)
{
    redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat host = {
        state, (void *)1, (void *)2, (void *)3, begin_io, do_io, wait_io,
        set_command, set_flag, clear_flag, 10U, 20U, 30U, 0x40U
    };
    return host;
}

int main(void)
{
    fixture state = {{0}, 0U, 0U, 0U, NULL, 0L};
    redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat host = make_host(&state);
    void *sound_data = (void *)4;

    redmcsb_f0909_play_swoosh_sound_pc34_compat(&host);
    assert(state.event_count == 4U);
    assert(memcmp(state.events, "LRSD", 4U) == 0);
    assert(state.command == 10U);

    state.event_count = 0U;
    state.flags = 0x01U;
    redmcsb_f0910_release_swoosh_sound_pc34_compat(&host);
    assert(state.event_count == 8U);
    assert(memcmp(state.events, "F+Clr-TD", 8U) == 0);
    assert(state.command == 30U);
    assert(state.flags == 0x01U);

    state.event_count = 0U;
    redmcsb_f0910_release_swoosh_sound_and_buffer_pc34_compat(
        &host, free_memory, &sound_data, 9078L);
    assert(state.event_count == 9U);
    assert(memcmp(state.events, "F+Clr-TDX", 9U) == 0);
    assert(sound_data == NULL);
    assert(state.freed_memory == (void *)4);
    assert(state.freed_byte_count == 9078L);
    assert(strstr(redmcsb_f0909_f0910_swoosh_sound_source_evidence_pc34(),
                  "SWSHSND.C:26-48") != NULL);
    return 0;
}
