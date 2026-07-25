#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1023_print_string.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1023_capture {
    unsigned int call_count;
    const char *last_string;
} redmcsb_f1023_capture;

static void capture_console_print(void *context, const char *string)
{
    redmcsb_f1023_capture *capture = context;

    capture->call_count++;
    capture->last_string = string;
}

int main(void)
{
    redmcsb_f1023_capture capture = { 0 };
    const char text[] = "SYSTEM ERROR ";
    const char *evidence;
    (void)evidence;

    redmcsb_f1023_print_string(capture_console_print, &capture, text);
    assert(capture.call_count == 1U);
    assert(capture.last_string == text);
    assert(strcmp(capture.last_string, "SYSTEM ERROR ") == 0);

    evidence = redmcsb_f1023_print_string_source_evidence();
    assert(strstr(evidence, "BASE.C:1144-1150") != NULL);
    assert(strstr(evidence, "IO2.C:250-258") != NULL);
    assert(strstr(evidence, "CEDT025.C:145-157") != NULL);
    puts("ok: ReDMCSB F1023 print string");
    return 0;
}
