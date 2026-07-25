#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1035_unreferenced.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1035_capture {
    unsigned int call_count;
    void *context;
    char string[2];
} redmcsb_f1035_capture;

static void capture_f1034(void *context, const char *string)
{
    redmcsb_f1035_capture *capture = context;

    capture->call_count++;
    capture->context = context;
    capture->string[0] = string[0];
    capture->string[1] = string[1];
}

int main(void)
{
    redmcsb_f1035_capture capture = { 0 };
    const char *evidence;
    (void)evidence;

    redmcsb_f1035_unreferenced(capture_f1034, &capture, 'A');
    assert(capture.call_count == 1U);
    assert(capture.context == &capture);
    assert(capture.string[0] == 'A');
    assert(capture.string[1] == '\0');
    assert(strcmp(capture.string, "A") == 0);

    redmcsb_f1035_unreferenced(capture_f1034, &capture, 0x142);
    assert(capture.call_count == 2U);
    assert(capture.string[0] == 'B');
    assert(capture.string[1] == '\0');

    evidence = redmcsb_f1035_unreferenced_source_evidence();
    assert(strstr(evidence, "IO2.C:219-225") != NULL);
    assert(strstr(evidence, "IO2.C:227-236") != NULL);
    assert(strstr(evidence, "MEDIA746 F1035_Unreferenced") != NULL);
    assert(strstr(evidence, "FindTask(0L)") != NULL);
    puts("ok: ReDMCSB F1035 one-character F1034 dispatch");
    return 0;
}
