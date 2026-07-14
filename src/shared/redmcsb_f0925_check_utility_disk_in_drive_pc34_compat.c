#include "redmcsb_f0925_check_utility_disk_in_drive_pc34_compat.h"

#include <string.h>

int redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
    int device_number,
    int first_loop_iteration,
    redmcsb_f0925_force_media_change_detection_pc34_compat
        force_media_change_detection,
    redmcsb_f0925_floprd_pc34_compat floprd,
    void *context)
{
    char buffer[600];
    int return_value;
    static char saved_sector_string[104];
    static char saved_title_string[50];

    force_media_change_detection(context, device_number);
    if (floprd(context, buffer, 0L, device_number, 7, 0, 0, 1) != 0) {
        return -1;
    }
    if (strcmp(buffer, saved_sector_string) == 0 &&
        strcmp(&buffer[128], saved_title_string) == 0) {
        return_value = 1;
    } else {
        return_value = first_loop_iteration;
    }
    if (first_loop_iteration != 0) {
        (void)strcpy(saved_sector_string, buffer);
        (void)strcpy(saved_title_string, &buffer[128]);
    }
    return return_value;
}

const char *
redmcsb_f0925_check_utility_disk_in_drive_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C:272-295 defines "
           "F0925_CheckUtilityDiskInDrive: it calls F0926, reads sector 7 "
           "with Floprd(buffer, 0L, device, 7, 0, 0, 1), returns -1 on a "
           "nonzero read result, compares strings at offsets 0 and 128 "
           "against static 104- and 50-byte buffers, and copies both after "
           "a nonzero first-loop iteration.";
}
