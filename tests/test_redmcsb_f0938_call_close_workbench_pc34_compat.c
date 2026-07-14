#include "redmcsb_f0938_call_close_workbench_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    const char *events[6];
    unsigned int event_count;
    void *open_result;
    bool close_result;
    const char *library_name;
    long library_version;
    unsigned long alert_codes[2];
    long alert_parameters[2];
    unsigned int alert_count;
    long delay_ticks;
    void *closed_library;
} redmcsb_f0938_capture_pc34_compat;

static void record_event(redmcsb_f0938_capture_pc34_compat *capture,
                         const char *event)
{
    capture->events[capture->event_count++] = event;
}

static void *capture_open_library(void *context, const char *name,
                                  long version)
{
    redmcsb_f0938_capture_pc34_compat *capture = context;

    record_event(capture, "open");
    capture->library_name = name;
    capture->library_version = version;
    return capture->open_result;
}

static void capture_alert(void *context, unsigned long alert_code,
                          long parameter)
{
    redmcsb_f0938_capture_pc34_compat *capture = context;

    record_event(capture, "alert");
    capture->alert_codes[capture->alert_count] = alert_code;
    capture->alert_parameters[capture->alert_count] = parameter;
    capture->alert_count++;
}

static void capture_delay(void *context, long ticks)
{
    redmcsb_f0938_capture_pc34_compat *capture = context;

    record_event(capture, "delay");
    capture->delay_ticks = ticks;
}

static bool capture_close_workbench(void *context)
{
    redmcsb_f0938_capture_pc34_compat *capture = context;

    record_event(capture, "close_workbench");
    return capture->close_result;
}

static void capture_close_library(void *context, void *library)
{
    redmcsb_f0938_capture_pc34_compat *capture = context;

    record_event(capture, "close_library");
    capture->closed_library = library;
}

int main(void)
{
    const redmcsb_f0938_call_close_workbench_ops_pc34_compat ops = {
        capture_open_library,
        capture_alert,
        capture_delay,
        capture_close_workbench,
        capture_close_library
    };
    int intuition_library;
    redmcsb_f0938_capture_pc34_compat success = {
        { NULL }, 0U, &intuition_library, true, NULL, -1L,
        { 0UL }, { 0L }, 0U, -1L, NULL
    };
    redmcsb_f0938_capture_pc34_compat failures = {
        { NULL }, 0U, NULL, false, NULL, -1L,
        { 0UL }, { 0L }, 0U, -1L, NULL
    };

    redmcsb_f0938_call_close_workbench_pc34_compat(&ops, &success);
    assert(success.event_count == 4U);
    assert(strcmp(success.events[0], "open") == 0);
    assert(strcmp(success.events[1], "delay") == 0);
    assert(strcmp(success.events[2], "close_workbench") == 0);
    assert(strcmp(success.events[3], "close_library") == 0);
    assert(strcmp(success.library_name, "intuition.library") == 0);
    assert(success.library_version == 0L);
    assert(success.alert_count == 0U);
    assert(success.delay_ticks == 100L);
    assert(success.closed_library == &intuition_library);

    redmcsb_f0938_call_close_workbench_pc34_compat(&ops, &failures);
    assert(failures.event_count == 6U);
    assert(strcmp(failures.events[0], "open") == 0);
    assert(strcmp(failures.events[1], "alert") == 0);
    assert(strcmp(failures.events[2], "delay") == 0);
    assert(strcmp(failures.events[3], "close_workbench") == 0);
    assert(strcmp(failures.events[4], "alert") == 0);
    assert(strcmp(failures.events[5], "close_library") == 0);
    assert(failures.alert_count == 2U);
    assert(failures.alert_codes[0] == 0x80444D01UL);
    assert(failures.alert_codes[1] == 0x80444D03UL);
    assert(failures.alert_parameters[0] == 0L);
    assert(failures.alert_parameters[1] == 0L);
    assert(failures.delay_ticks == 100L);
    assert(failures.closed_library == NULL);
    assert(strstr(redmcsb_f0938_call_close_workbench_source_evidence_pc34(),
                  "EXEC.C:275-292") != NULL);
    puts("ok: ReDMCSB F0938 Intuition CloseWorkBench host boundary");
    return 0;
}
