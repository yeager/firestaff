#include "redmcsb_f0950_japanese_character_pattern_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

enum {
    REDMCSB_F0950_WRITE_EVENT_COUNT = 36,
    REDMCSB_F0950_READ_EVENT_COUNT = 32
};

typedef struct {
    uint16_t write_ports[REDMCSB_F0950_WRITE_EVENT_COUNT];
    uint8_t write_values[REDMCSB_F0950_WRITE_EVENT_COUNT];
    uint8_t read_values[REDMCSB_F0950_READ_EVENT_COUNT];
    unsigned int write_count;
    unsigned int read_count;
    unsigned int enter_count;
    unsigned int leave_count;
    unsigned int critical_depth;
} redmcsb_f0950_capture_pc34_compat;

static void capture_port_write(void *context, uint16_t port, uint8_t value)
{
    redmcsb_f0950_capture_pc34_compat *capture = context;

    assert(capture->write_count < REDMCSB_F0950_WRITE_EVENT_COUNT);
    if (port == UINT16_C(0x00A5)) {
        assert(capture->critical_depth == 1U);
    }
    capture->write_ports[capture->write_count] = port;
    capture->write_values[capture->write_count] = value;
    ++capture->write_count;
}

static uint8_t capture_port_read(void *context, uint16_t port)
{
    (void)port;
    redmcsb_f0950_capture_pc34_compat *capture = context;

    assert(port == UINT16_C(0x00A9));
    assert(capture->critical_depth == 1U);
    assert(capture->read_count < REDMCSB_F0950_READ_EVENT_COUNT);
    return capture->read_values[capture->read_count++];
}

static void capture_enter_critical_section(void *context)
{
    redmcsb_f0950_capture_pc34_compat *capture = context;

    assert(capture->critical_depth == 0U);
    capture->critical_depth = 1U;
    ++capture->enter_count;
}

static void capture_leave_critical_section(void *context)
{
    redmcsb_f0950_capture_pc34_compat *capture = context;

    assert(capture->critical_depth == 1U);
    capture->critical_depth = 0U;
    ++capture->leave_count;
}

int main(void)
{
    uint8_t character_pattern[
        REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_BYTE_COUNT] = { 0U };
    redmcsb_f0950_capture_pc34_compat capture = { { 0U }, { 0U }, { 0U },
                                                   0U, 0U, 0U, 0U, 0U };
    const redmcsb_f0950_japanese_io_pc34_compat io = {
        capture_port_write,
        capture_port_read,
        capture_enter_critical_section,
        capture_leave_critical_section
    };
    unsigned int index;
    const char *evidence;
    (void)evidence;

    for (index = 0U; index < REDMCSB_F0950_READ_EVENT_COUNT; ++index) {
        capture.read_values[index] = (uint8_t)(UINT8_C(0x80) + index);
    }

    redmcsb_f0950_japanese_character_pattern_pc34_compat(
        INT16_C(0x7423), character_pattern, &io, &capture);

    assert(capture.write_count == REDMCSB_F0950_WRITE_EVENT_COUNT);
    assert(capture.read_count == REDMCSB_F0950_READ_EVENT_COUNT);
    assert(capture.enter_count == REDMCSB_F0950_READ_EVENT_COUNT);
    assert(capture.leave_count == REDMCSB_F0950_READ_EVENT_COUNT);
    assert(capture.critical_depth == 0U);
    assert(capture.write_ports[0] == UINT16_C(0x0068));
    assert(capture.write_values[0] == UINT8_C(0x0B));
    assert(capture.write_ports[1] == UINT16_C(0x00A3));
    assert(capture.write_values[1] == UINT8_C(0x54));
    assert(capture.write_ports[2] == UINT16_C(0x00A1));
    assert(capture.write_values[2] == UINT8_C(0x23));

    for (index = 0U; index < 16U; ++index) {
        unsigned int selected_write = 3U + (index * 2U);
        unsigned int unselected_write = selected_write + 1U;
        (void)unselected_write;

        assert(capture.write_ports[selected_write] == UINT16_C(0x00A5));
        assert(capture.write_values[selected_write] ==
               (uint8_t)(UINT8_C(0x20) + index));
        assert(capture.write_ports[unselected_write] == UINT16_C(0x00A5));
        assert(capture.write_values[unselected_write] == (uint8_t)index);
    }
    assert(capture.write_ports[35] == UINT16_C(0x0068));
    assert(capture.write_values[35] == UINT8_C(0x0A));
    assert(memcmp(character_pattern, capture.read_values,
                  sizeof(character_pattern)) == 0);

    evidence = redmcsb_f0950_japanese_character_pattern_source_evidence_pc34();
    assert(strstr(evidence, "JAPANESE.C:36-74") != NULL);
    assert(strstr(evidence, "pushf/cli/popf") != NULL);
    puts("ok: ReDMCSB F0950 Japanese character-pattern port sequence");
    return 0;
}
