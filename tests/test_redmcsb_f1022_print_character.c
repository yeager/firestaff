#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1022_print_character.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1022_capture {
    unsigned int call_count;
    const char *last_string;
    char copied_string[2];
} redmcsb_f1022_capture;

static void capture_console_print(void *context, const char *string)
{
    redmcsb_f1022_capture *capture = context;

    capture->call_count++;
    capture->last_string = string;
    capture->copied_string[0] = string[0];
    capture->copied_string[1] = string[1];
}

int main(void)
{
    redmcsb_f1022_capture capture = { 0 };
    const char *evidence;
    (void)evidence;

    redmcsb_f1022_print_character(capture_console_print, &capture, '4');
    assert(capture.call_count == 1U);
    assert(capture.copied_string[0] == '4');
    assert(capture.copied_string[1] == '\0');
    assert(strcmp(capture.last_string, "4") == 0);

    redmcsb_f1022_print_character(capture_console_print, &capture, '2');
    assert(capture.call_count == 2U);
    assert(capture.copied_string[0] == '2');
    assert(capture.copied_string[1] == '\0');
    assert(strcmp(capture.last_string, "2") == 0);

    evidence = redmcsb_f1022_print_character_source_evidence();
    assert(strstr(evidence, "BASE.C:1144-1147") != NULL);
    assert(strstr(evidence, "IO2.C:239-248") != NULL);
    puts("ok: ReDMCSB F1022 print character");
    return 0;
}
