#include "redmcsb_f0927_print_loader_error_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

/* Source-faithful F0933 dependency used to isolate the F0927 adapter. */
int F0933_GetHexStringFromValue(uint32_t value, char *string)
{
    static const char digits[] = "0123456789ABCDEF";
    char reversed[8];
    size_t count = 0U;

    do {
        reversed[count++] = digits[value % UINT32_C(16)];
        value >>= 4;
    } while (value != 0U);
    while (count != 0U) {
        *string++ = reversed[--count];
    }
    *string = '\0';
    return 0;
}

typedef struct {
    char writes[3][32];
    size_t write_count;
    size_t key_available_count;
    size_t read_key_count;
    bool key_states[3];
} redmcsb_f0927_capture_pc34_compat;

static void capture_write(void *context, const char *text)
{
    redmcsb_f0927_capture_pc34_compat *capture = context;

    (void)strcpy(capture->writes[capture->write_count++], text);
}

static bool capture_key_available(void *context)
{
    redmcsb_f0927_capture_pc34_compat *capture = context;

    return capture->key_states[capture->key_available_count++];
}

static void capture_read_key(void *context)
{
    redmcsb_f0927_capture_pc34_compat *capture = context;

    capture->read_key_count++;
}

int main(void)
{
    redmcsb_f0927_capture_pc34_compat capture = {
        { { "" }, { "" }, { "" } }, 0U, 0U, 0U, { false, false, true }
    };
    const redmcsb_f0927_print_loader_error_callbacks_pc34_compat callbacks = {
        &capture, capture_write, capture_key_available, capture_read_key
    };
    (void)callbacks;
    const char *evidence;
    (void)evidence;

    assert(redmcsb_f0927_print_loader_error_pc34_compat(
               -INT16_C(1), &callbacks) == -INT16_C(1));
    assert(capture.write_count == 3U);
    assert(strcmp(capture.writes[0], "Loader error:\a 0x") == 0);
    assert(strcmp(capture.writes[1], "FFFFFFFF") == 0);
    assert(strcmp(capture.writes[2], "\n\r") == 0);
    assert(capture.key_available_count == 3U);
    assert(capture.read_key_count == 1U);

    evidence = redmcsb_f0927_print_loader_error_source_evidence_pc34();
    assert(strstr(evidence, "PRIM1.C:398-416") != NULL);
    assert(strstr(evidence, "loaderError") != NULL);
    return 0;
}
