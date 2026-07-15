#include "redmcsb_f0925_check_utility_disk_in_drive_pc34_compat.h"

#include <string.h>

static int redmcsb_f0925_bounded_cstr_len_pc34(const char *text,
                                               unsigned int capacity,
                                               unsigned int *out_length)
{
    unsigned int length;

    if (!text || !out_length || capacity == 0u) return 0;
    for (length = 0u; length < capacity; ++length) {
        if (text[length] == '\0') {
            *out_length = length;
            return 1;
        }
    }
    return 0;
}

int redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
    int device_number,
    int first_loop_iteration,
    redmcsb_f0925_force_media_change_detection_pc34_compat
        force_media_change_detection,
    redmcsb_f0925_floprd_pc34_compat floprd,
    void *context)
{
    char buffer[600];
    static char saved_sector_string[104];
    static char saved_title_string[50];
    unsigned int sector_string_length;
    unsigned int title_string_length;
    int result;

    if (!force_media_change_detection || !floprd) return -1;
    force_media_change_detection(context, device_number);
    result = floprd(context, buffer, 0L, device_number, 7, 0, 0, 1);
    if (result != 0) return -1;
    if (!redmcsb_f0925_bounded_cstr_len_pc34(
            buffer, (unsigned int)sizeof(saved_sector_string),
            &sector_string_length) ||
        !redmcsb_f0925_bounded_cstr_len_pc34(
            &buffer[128], (unsigned int)sizeof(saved_title_string),
            &title_string_length)) {
        return -1;
    }

    if (strcmp(buffer, saved_sector_string) == 0 &&
        strcmp(&buffer[128], saved_title_string) == 0) {
        result = 1;
    } else {
        result = first_loop_iteration;
    }
    if (first_loop_iteration != 0) {
        (void)memcpy(saved_sector_string, buffer,
                     (size_t)sector_string_length + 1u);
        (void)memcpy(saved_title_string, &buffer[128],
                     (size_t)title_string_length + 1u);
    }
    return result;
}

const char *
redmcsb_f0925_check_utility_disk_in_drive_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C F0925_CheckUtilityDiskInDrive:272-295 "
           "calls F0926, reads Floprd(buf,0,device,7,0,0,1), compares "
           "sector strings at offsets 0 and 128, and refreshes the saved "
           "strings only on a nonzero first-loop flag.";
}
