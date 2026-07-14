#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1034_write_current_task_diagnostic.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1034_capture {
    void *found_task;
    unsigned int find_task_calls;
    unsigned int string_length_calls;
    unsigned int write_calls;
    void *written_handle;
    const char *written_string;
    long written_byte_count;
} redmcsb_f1034_capture;

static void *capture_find_task(void *context)
{
    redmcsb_f1034_capture *capture = context;

    capture->find_task_calls++;
    return capture->found_task;
}

static int16_t capture_string_length(void *context, const char *string)
{
    redmcsb_f1034_capture *capture = context;

    capture->string_length_calls++;
    return (int16_t)strlen(string);
}

static void capture_write(void *context,
                          void *file_handle,
                          const char *string,
                          long byte_count)
{
    redmcsb_f1034_capture *capture = context;

    capture->write_calls++;
    capture->written_handle = file_handle;
    capture->written_string = string;
    capture->written_byte_count = byte_count;
}

int main(void)
{
    int current_task_token;
    int other_task_token;
    int nil_handle_token;
    const char text[] = "diagnostic";
    const char *evidence;
    redmcsb_f1034_capture capture = { 0 };

    capture.found_task = &other_task_token;
    redmcsb_f1034_write_current_task_diagnostic(
        capture_find_task, capture_string_length, capture_write, &capture,
        &current_task_token, &nil_handle_token, text);
    assert(capture.find_task_calls == 1U);
    assert(capture.string_length_calls == 0U);
    assert(capture.write_calls == 0U);

    capture.found_task = &current_task_token;
    redmcsb_f1034_write_current_task_diagnostic(
        capture_find_task, capture_string_length, capture_write, &capture,
        &current_task_token, &nil_handle_token, text);
    assert(capture.find_task_calls == 2U);
    assert(capture.string_length_calls == 1U);
    assert(capture.write_calls == 1U);
    assert(capture.written_handle == &nil_handle_token);
    assert(capture.written_string == text);
    assert(capture.written_byte_count == 10L);

    evidence = redmcsb_f1034_write_current_task_diagnostic_source_evidence();
    assert(strstr(evidence, "IO2.C:219-225") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:239") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:341") != NULL);
    puts("ok: ReDMCSB F1034 current-task diagnostic write");
    return 0;
}
