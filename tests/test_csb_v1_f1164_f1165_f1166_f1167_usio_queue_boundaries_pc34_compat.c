#include "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat.h"

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

static void test_empty_queue_contract(void)
{
    csb_v1_usio_queue_pc34_compat queue;
    csb_v1_usio_data_pc34_compat out = {77, 1, 2, 3, 4, 5, 6};

    csb_v1_usio_queue_init_empty_pc34_compat(&queue);

    CHECK(csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(&queue) ==
          -1);
    CHECK(csb_v1_f1164_usio_15_get_first_queued_usio_data_type_pc34_compat(
              &queue) == 0);
    CHECK(csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_pc34_compat(
              &queue, &out) == 0);
    CHECK(out.usio_type == 0);
    CHECK(out.usio_value == 1);
    CHECK(queue.first_index == 0);
    CHECK(queue.last_index == 10);

    CHECK(F1164_USIO_15_GetFirstQueuedUsioDataType() == 0);
    out.usio_type = 77;
    CHECK(F1166_USIO_16_ExtractFirstUsioDataFromQueue(&out) == 0);
    CHECK(out.usio_type == 0);
    out.usio_type = 77;
    CHECK(F1165_USIO_17_WaitUntilKeyboardOrMouseInput(&out) == 0);
    CHECK(out.usio_type == 0);
}

static void test_queue_index_boundaries_without_input_data(void)
{
    csb_v1_usio_queue_pc34_compat queue;

    csb_v1_usio_queue_init_empty_pc34_compat(&queue);
    queue.first_index = 9;
    queue.last_index = 8;
    CHECK(csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(&queue) ==
          -1);

    queue.first_index = 10;
    queue.last_index = 9;
    CHECK(csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(&queue) ==
          -1);

    queue.first_index = -1;
    queue.last_index = 10;
    CHECK(csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(&queue) ==
          -1);
}

static void test_mouse_status_requires_caller_owned_facts(void)
{
    csb_v1_usio_mouse_status_pc34_compat sentinel = {1, 2, 3, 4, 5, 6};

    CHECK(csb_v1_f1167_usio_14_get_mouse_status_pc34_compat(&sentinel, NULL) ==
          0);
    CHECK(sentinel.mouse_buttons == 4);
    CHECK(sentinel.mouse_x == 5);

    sentinel.mouse_buttons = 7;
    sentinel.mouse_x = 8;
    F1167_USIO_14_GetMouseStatus(&sentinel);
    CHECK(sentinel.mouse_buttons == 7);
    CHECK(sentinel.mouse_x == 8);
}

static void test_evidence_strings(void)
{
    check_contains(
        csb_v1_f1164_usio_15_get_first_queued_usio_data_type_source_evidence_pc34(),
        "USIO2.C:41-56");
    check_contains(
        csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_source_evidence_pc34(),
        "USIO2.C:60-72");
    check_contains(
        csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_source_evidence_pc34(),
        "USIO2.C:75-86");
    check_contains(
        csb_v1_f1167_usio_14_get_mouse_status_source_evidence_pc34(),
        "USIO2.C:89-113");
    check_contains(csb_v1_f1175_f1176_usio_queue_source_evidence_pc34(),
                   "USIO2.C:359-389");
}

int main(void)
{
    test_empty_queue_contract();
    test_queue_index_boundaries_without_input_data();
    test_mouse_status_requires_caller_owned_facts();
    test_evidence_strings();
    return 0;
}
