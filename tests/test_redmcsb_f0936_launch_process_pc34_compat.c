#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0936_launch_process_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

enum redmcsb_f0936_event_pc34_compat {
    REDMCSB_F0936_EVENT_LOAD_SEG,
    REDMCSB_F0936_EVENT_CREATE_PROC,
    REDMCSB_F0936_EVENT_PUT_MSG,
    REDMCSB_F0936_EVENT_WAIT_PORT,
    REDMCSB_F0936_EVENT_GET_MSG,
    REDMCSB_F0936_EVENT_UNLOAD_SEG
};

typedef struct {
    enum redmcsb_f0936_event_pc34_compat events[6];
    unsigned int event_count;
    const char *name;
    long priority;
    long stack_size;
    void *message;
    void *reply_port;
    void *segment_list;
    void *process;
} redmcsb_f0936_capture_pc34_compat;

static void record_event(redmcsb_f0936_capture_pc34_compat *capture,
                         enum redmcsb_f0936_event_pc34_compat event)
{
    capture->events[capture->event_count] = event;
    ++capture->event_count;
}

static void *capture_load_seg(void *context, const char *name)
{
    redmcsb_f0936_capture_pc34_compat *capture = context;

    capture->name = name;
    record_event(capture, REDMCSB_F0936_EVENT_LOAD_SEG);
    return capture->segment_list;
}

static void *capture_create_proc(void *context,
                                 const char *name,
                                 long priority,
                                 void *segment_list,
                                 long stack_size)
{
    (void)segment_list;
    (void)name;
    redmcsb_f0936_capture_pc34_compat *capture = context;

    assert(name == capture->name);
    assert(segment_list == capture->segment_list);
    capture->priority = priority;
    capture->stack_size = stack_size;
    record_event(capture, REDMCSB_F0936_EVENT_CREATE_PROC);
    return capture->process;
}

static void capture_put_msg(void *context, void *process, void *message)
{
    (void)process;
    redmcsb_f0936_capture_pc34_compat *capture = context;

    assert(process == capture->process);
    capture->message = message;
    record_event(capture, REDMCSB_F0936_EVENT_PUT_MSG);
}

static void capture_wait_port(void *context, void *reply_port)
{
    redmcsb_f0936_capture_pc34_compat *capture = context;

    capture->reply_port = reply_port;
    record_event(capture, REDMCSB_F0936_EVENT_WAIT_PORT);
}

static void *capture_get_msg(void *context, void *reply_port)
{
    (void)reply_port;
    redmcsb_f0936_capture_pc34_compat *capture = context;

    assert(reply_port == capture->reply_port);
    record_event(capture, REDMCSB_F0936_EVENT_GET_MSG);
    return NULL;
}

static void capture_unload_seg(void *context, void *segment_list)
{
    (void)segment_list;
    redmcsb_f0936_capture_pc34_compat *capture = context;

    assert(segment_list == capture->segment_list);
    record_event(capture, REDMCSB_F0936_EVENT_UNLOAD_SEG);
}

static const redmcsb_f0936_launch_process_ops_pc34_compat capture_ops = {
    capture_load_seg,
    capture_create_proc,
    capture_put_msg,
    capture_wait_port,
    capture_get_msg,
    capture_unload_seg
};

int main(void)
{
    int message;
    int reply_port;
    int segment_list;
    int process;
    redmcsb_f0936_capture_pc34_compat capture = {
        { REDMCSB_F0936_EVENT_LOAD_SEG }, 0U, NULL, 1L, 1L,
        NULL, NULL, &segment_list, &process
    };

    redmcsb_f0936_launch_process_pc34_compat(
        "DungeonMaster:swoosh", true, &message, &reply_port, &capture_ops,
        &capture);
    assert(capture.event_count == 6U);
    assert(capture.events[0] == REDMCSB_F0936_EVENT_LOAD_SEG);
    assert(capture.events[1] == REDMCSB_F0936_EVENT_CREATE_PROC);
    assert(capture.events[2] == REDMCSB_F0936_EVENT_PUT_MSG);
    assert(capture.events[3] == REDMCSB_F0936_EVENT_WAIT_PORT);
    assert(capture.events[4] == REDMCSB_F0936_EVENT_GET_MSG);
    assert(capture.events[5] == REDMCSB_F0936_EVENT_UNLOAD_SEG);
    assert(capture.priority == 0L);
    assert(capture.stack_size == 8000L);
    assert(capture.message == &message);
    assert(capture.reply_port == &reply_port);

    capture.event_count = 0U;
    redmcsb_f0936_launch_process_pc34_compat(
        "DungeonMaster:dm", false, &message, &reply_port, &capture_ops,
        &capture);
    assert(capture.event_count == 3U);
    assert(capture.events[0] == REDMCSB_F0936_EVENT_LOAD_SEG);
    assert(capture.events[1] == REDMCSB_F0936_EVENT_CREATE_PROC);
    assert(capture.events[2] == REDMCSB_F0936_EVENT_PUT_MSG);

    assert(strstr(redmcsb_f0936_launch_process_source_evidence_pc34(),
                  "EXEC.C:66-82") != NULL);
    assert(strstr(redmcsb_f0936_launch_process_source_evidence_pc34(),
                  "No F0936_ComputeChecksum") != NULL);
    puts("ok: ReDMCSB F0936 process launch sequence");
    return 0;
}
