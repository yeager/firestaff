#include "redmcsb_f1048_setjmp.h"
#include "redmcsb_f1049_longjmp_pc34_compat.h"
#include "redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat.h"
#include "redmcsb_f1055_post_f0380_command_process_queue_pc34_compat.h"
#include "redmcsb_f1061_pre_unreferenced_pc34_compat.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_f1048_setjmp_alias(void)
{
    jmp_buf environment;
    const volatile int value = F1048_setjmp(environment);

    CHECK(value == 0);
    check_contains(redmcsb_f1048_setjmp_source_evidence(), "DEFS.H:3209");
    check_contains(redmcsb_f1048_setjmp_source_evidence(), "F1048_setjmp");
}

static void test_f1049_longjmp_disabled_alias(void)
{
    CHECK(!redmcsb_f1049_longjmp_pc34_compat());
    CHECK(!F1049_longjmp());
    check_contains(redmcsb_f1049_longjmp_source_evidence_pc34(),
                   "DEFS.H:3210");
    check_contains(redmcsb_f1049_longjmp_source_evidence_pc34(),
                   "F1049_longjmp");
}

static void test_command_queue_padding_hooks_are_noop(void)
{
    int sentinel = 0x5a17;

    redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat();
    F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC();
    CHECK(sentinel == 0x5a17);
    check_contains(
        redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34(),
        "AMIGA.H:400");
    check_contains(
        redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34(),
        "F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC");

    redmcsb_f1055_post_f0380_command_process_queue_pc34_compat();
    F1055_Post_F0380_COMMAND_ProcessQueue_CPSC();
    CHECK(sentinel == 0x5a17);
    check_contains(
        redmcsb_f1055_post_f0380_command_process_queue_source_evidence_pc34(),
        "AMIGA.H:401");
    check_contains(
        redmcsb_f1055_post_f0380_command_process_queue_source_evidence_pc34(),
        "F1055_Post_F0380_COMMAND_ProcessQueue_CPSC");
}

static void test_unreferenced_padding_hook_is_noop(void)
{
    int sentinel = 0x1061;

    redmcsb_f1061_pre_unreferenced_pc34_compat();
    F1061_Pre_Unreferenced();
    CHECK(sentinel == 0x1061);
    check_contains(redmcsb_f1061_pre_unreferenced_source_evidence_pc34(),
                   "READWRIT.C:75");
    check_contains(redmcsb_f1061_pre_unreferenced_source_evidence_pc34(),
                   "F1061_Pre_Unreferenced");
}

int main(void)
{
    test_f1048_setjmp_alias();
    test_f1049_longjmp_disabled_alias();
    test_command_queue_padding_hooks_are_noop();
    test_unreferenced_padding_hook_is_noop();
    return 0;
}
