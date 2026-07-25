#include "redmcsb_f0925_check_utility_disk_in_drive_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int force_call_count;
    unsigned int read_call_count;
    int seen_device_number;
    long seen_reserved;
    int seen_sector;
    int seen_track;
    int seen_side;
    int seen_count;
    int read_result;
    const char *sector_string;
    const char *title_string;
} redmcsb_f0925_capture_pc34_compat;

static __attribute__((unused)) void capture_force_media_change_detection(void *context,
                                                  int device_number)
{
    redmcsb_f0925_capture_pc34_compat *capture = context;

    capture->force_call_count++;
    capture->seen_device_number = device_number;
}

static __attribute__((unused)) int capture_floprd(void *context,
                          char *buffer,
                          long reserved,
                          int device_number,
                          int sector,
                          int track,
                          int side,
                          int count)
{
    redmcsb_f0925_capture_pc34_compat *capture = context;

    capture->read_call_count++;
    capture->seen_reserved = reserved;
    capture->seen_device_number = device_number;
    capture->seen_sector = sector;
    capture->seen_track = track;
    capture->seen_side = side;
    capture->seen_count = count;
    if (capture->read_result == 0) {
        (void)memset(buffer, 0, 600U);
        (void)strcpy(buffer, capture->sector_string);
        (void)strcpy(&buffer[128], capture->title_string);
    }
    return capture->read_result;
}

int main(void)
{
    redmcsb_f0925_capture_pc34_compat capture = {
        0U, 0U, 0, 0L, 0, 0, 0, 0, -1,
        "copyright (c) 1987, Software Heaven, Inc.", "Chaos Strikes Back"
    };

    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 1, capture_force_media_change_detection, capture_floprd,
               &capture) == -1);
    assert(capture.force_call_count == 1U);
    assert(capture.read_call_count == 1U);
    assert(capture.seen_device_number == 2);
    assert(capture.seen_reserved == 0L);
    assert(capture.seen_sector == 7);
    assert(capture.seen_track == 0);
    assert(capture.seen_side == 0);
    assert(capture.seen_count == 1);

    capture.read_result = 0;
    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 1, capture_force_media_change_detection, capture_floprd,
               &capture) == 1);
    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 0, capture_force_media_change_detection, capture_floprd,
               &capture) == 1);

    capture.title_string = "Wrong title";
    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 0, capture_force_media_change_detection, capture_floprd,
               &capture) == 0);
    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 7, capture_force_media_change_detection, capture_floprd,
               &capture) == 7);
    assert(redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
               2, 0, capture_force_media_change_detection, capture_floprd,
               &capture) == 1);
    assert(capture.force_call_count == 6U);
    assert(capture.read_call_count == 6U);

    assert(strstr(
               redmcsb_f0925_check_utility_disk_in_drive_source_evidence_pc34(),
               "PRIM1.C:272-295") != NULL);
    puts("ok: ReDMCSB F0925 utility-disk sector signature gate");
    return 0;
}
