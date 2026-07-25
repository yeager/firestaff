#include "redmcsb_f1001_japanese_load_ank_character_patterns_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

enum {
    REDMCSB_F1001_WRITES_PER_CHARACTER = 18u,
    REDMCSB_F1001_WRITE_COUNT =
        REDMCSB_F1001_ANK_CHARACTER_COUNT * REDMCSB_F1001_WRITES_PER_CHARACTER
};

typedef struct {
    uint16_t write_ports[REDMCSB_F1001_WRITE_COUNT];
    uint8_t write_values[REDMCSB_F1001_WRITE_COUNT];
    unsigned int write_count;
    unsigned int read_count;
    unsigned int vertical_blank_count;
    unsigned int interrupt_wait_count;
    unsigned int chained_interrupt_count;
    unsigned int set_vector_count;
    unsigned int enter_critical_count;
    unsigned int leave_critical_count;
    unsigned int critical_depth;
    uint8_t current_character;
    redmcsb_f1001_interrupt_handler_pc34_compat installed_handler;
    void *installed_handler_context;
} redmcsb_f1001_capture_pc34_compat;

static void capture_port_write(void *context, uint16_t port, uint8_t value)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(capture->write_count < REDMCSB_F1001_WRITE_COUNT);
    capture->write_ports[capture->write_count] = port;
    capture->write_values[capture->write_count] = value;
    ++capture->write_count;
    if (port == UINT16_C(0x00a3)) {
        capture->current_character = value;
    }
}

static uint8_t capture_port_read(void *context, uint16_t port)
{
    (void)port;
    redmcsb_f1001_capture_pc34_compat *capture = context;
    unsigned int row = capture->read_count %
                       REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES;

    assert(port == UINT16_C(0x00a9));
    ++capture->read_count;
    return (uint8_t)(capture->current_character + row);
}

static void capture_vertical_blank(void *context)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(capture->write_count == capture->vertical_blank_count *
                                   REDMCSB_F1001_WRITES_PER_CHARACTER);
    ++capture->vertical_blank_count;
}

static void capture_enter_critical_section(void *context)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(capture->critical_depth == 0u);
    capture->critical_depth = 1u;
    ++capture->enter_critical_count;
}

static void capture_leave_critical_section(void *context)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(capture->critical_depth == 1u);
    capture->critical_depth = 0u;
    ++capture->leave_critical_count;
}

static void capture_previous_interrupt(void *context)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    ++capture->chained_interrupt_count;
}

static void capture_get_interrupt_vector(
    void *context,
    uint8_t interrupt_number,
    redmcsb_f1001_interrupt_handler_pc34_compat *handler,
    void **handler_context)
{
    (void)interrupt_number;
    assert(interrupt_number == REDMCSB_F1001_TIMER_INTERRUPT);
    *handler = capture_previous_interrupt;
    *handler_context = context;
}

static void capture_set_interrupt_vector(
    void *context,
    uint8_t interrupt_number,
    redmcsb_f1001_interrupt_handler_pc34_compat handler,
    void *handler_context)
{
    (void)interrupt_number;
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(interrupt_number == REDMCSB_F1001_TIMER_INTERRUPT);
    ++capture->set_vector_count;
    capture->installed_handler = handler;
    capture->installed_handler_context = handler_context;
}

static void capture_wait_for_interrupt(void *context)
{
    redmcsb_f1001_capture_pc34_compat *capture = context;

    assert(capture->installed_handler != NULL);
    ++capture->interrupt_wait_count;
    capture->installed_handler(capture->installed_handler_context);
}

static void assert_port_sequence(const redmcsb_f1001_capture_pc34_compat *capture)
{
    (void)capture;
    unsigned int character_index;

    assert(capture->write_count == REDMCSB_F1001_WRITE_COUNT);
    assert(capture->read_count == REDMCSB_F1001_ANK_SEGMENT_BYTES);
    for (character_index = 0u;
         character_index < REDMCSB_F1001_ANK_CHARACTER_COUNT;
         ++character_index) {
        unsigned int write_index =
            character_index * REDMCSB_F1001_WRITES_PER_CHARACTER;
        (void)write_index;
        unsigned int row;

        assert(capture->write_ports[write_index] == UINT16_C(0x00a3));
        assert(capture->write_values[write_index] == (uint8_t)character_index);
        assert(capture->write_ports[write_index + 1u] == UINT16_C(0x00a1));
        assert(capture->write_values[write_index + 1u] == 0u);
        for (row = 0u; row < REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES;
             ++row) {
            assert(capture->write_ports[write_index + 2u + row] ==
                   UINT16_C(0x00a5));
            assert(capture->write_values[write_index + 2u + row] ==
                   (uint8_t)row);
        }
    }
}

int main(void)
{
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES] = { 0u };
    redmcsb_f1001_capture_pc34_compat capture = { { 0u }, { 0u }, 0u, 0u,
                                                    0u, 0u, 0u, 0u, 0u, 0u,
                                                    0u, 0u, NULL, NULL };
    const redmcsb_f1001_japanese_io_pc34_compat io = {
        capture_port_write,
        capture_port_read,
        capture_vertical_blank,
        capture_enter_critical_section,
        capture_leave_critical_section,
        capture_get_interrupt_vector,
        capture_set_interrupt_vector,
        capture_wait_for_interrupt
    };
    const char *evidence;
    (void)evidence;
    unsigned int index;

    redmcsb_f1001_japanese_load_ank_character_patterns_p20ja_pc34_compat(
        ank_segment, &io, &capture);
    assert(capture.vertical_blank_count == REDMCSB_F1001_ANK_CHARACTER_COUNT);
    assert(capture.enter_critical_count == REDMCSB_F1001_ANK_CHARACTER_COUNT);
    assert(capture.leave_critical_count == REDMCSB_F1001_ANK_CHARACTER_COUNT);
    assert(capture.critical_depth == 0u);
    assert_port_sequence(&capture);
    for (index = 0u; index < REDMCSB_F1001_ANK_SEGMENT_BYTES; ++index) {
        assert(ank_segment[index] ==
               (uint8_t)((index / REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES) +
                         (index % REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES)));
    }

    memset(ank_segment, 0, sizeof(ank_segment));
    memset(&capture, 0, sizeof(capture));
    redmcsb_f1001_japanese_load_ank_character_patterns_p20jb_pc34_compat(
        ank_segment, &io, &capture);
    assert(capture.interrupt_wait_count == REDMCSB_F1001_ANK_CHARACTER_COUNT);
    assert(capture.chained_interrupt_count == REDMCSB_F1001_ANK_CHARACTER_COUNT);
    assert(capture.set_vector_count == 2u);
    assert(capture.enter_critical_count == 0u);
    assert(capture.leave_critical_count == 0u);
    assert(capture.critical_depth == 0u);
    assert(capture.installed_handler == capture_previous_interrupt);
    assert(capture.installed_handler_context == &capture);
    assert_port_sequence(&capture);
    for (index = 0u; index < REDMCSB_F1001_ANK_SEGMENT_BYTES; ++index) {
        assert(ank_segment[index] ==
               (uint8_t)((index / REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES) +
                         (index % REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES)));
    }

    evidence = redmcsb_f1001_japanese_load_ank_character_patterns_source_evidence_pc34();
    assert(strstr(evidence, "JAPANESE.C:97-188") != NULL);
    assert(strstr(evidence, "MEDIA457_P20JA") != NULL);
    assert(strstr(evidence, "MEDIA469_P20JB") != NULL);
    assert(strstr(evidence, "0x0A") != NULL);
    puts("ok: ReDMCSB F1001 Japanese ANK character-pattern loading");
    return 0;
}
