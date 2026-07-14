#include <stdint.h>
#include <string.h>

#include "redmcsb_f0808_seek_to_cylinder0_pc34_compat.h"

struct disk_bios_call {
    uint8_t ah;
    uint8_t al;
    uint8_t cl;
};

struct disk_bios_log {
    struct disk_bios_call calls[2];
    int count;
};

static void record_disk_bios_call(uint8_t ah, uint8_t al, uint8_t cl, void *context)
{
    struct disk_bios_log *log = (struct disk_bios_log *)context;

    if (log->count < 2) {
        log->calls[log->count].ah = ah;
        log->calls[log->count].al = al;
        log->calls[log->count].cl = cl;
    }
    ++log->count;
}

int main(void)
{
    struct disk_bios_log log = {{{0u, 0u, 0u}, {0u, 0u, 0u}}, 0};

    redmcsb_f0808_seek_to_cylinder0_pc34_compat(
        0x83u, record_disk_bios_call, &log);

    if (log.count != 2 ||
        log.calls[0].ah != 0x10u || log.calls[0].al != 0x83u || log.calls[0].cl != 0u ||
        log.calls[1].ah != 0x07u || log.calls[1].al != 0x83u || log.calls[1].cl != 0u) {
        return 1;
    }
    if (strcmp(redmcsb_f0808_seek_to_cylinder0_source_evidence_pc34(),
               "ReDMCSB IO.C:3980-3995; DISK_BOOT, PC-98 INT 1Bh seek and recalibrate") != 0) {
        return 1;
    }

    return 0;
}
