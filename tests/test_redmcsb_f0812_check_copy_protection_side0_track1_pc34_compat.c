#include <stdint.h>
#include <string.h>

#include "redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat.h"

struct read_log {
    uint8_t device;
    uint16_t length;
    uint8_t length_code;
    uint8_t cylinder;
    uint8_t head;
    uint8_t sector;
    uint8_t *buffer;
    uint8_t status;
    int calls;
};

static uint8_t record_read(uint8_t device,
                           uint16_t length,
                           uint8_t length_code,
                           uint8_t cylinder,
                           uint8_t head,
                           uint8_t sector,
                           uint8_t *buffer,
                           void *context)
{
    struct read_log *log = (struct read_log *)context;

    log->device = device;
    log->length = length;
    log->length_code = length_code;
    log->cylinder = cylinder;
    log->head = head;
    log->sector = sector;
    log->buffer = buffer;
    ++log->calls;
    return log->status;
}

int main(void)
{
    uint8_t buffer[256] = {0};
    struct read_log log = {0u, 0u, 0u, 0u, 0u, 0u, NULL, 0xA0u, 0};

    if (!redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat(
            0x83u, buffer, record_read, &log) ||
        log.calls != 1 || log.device != 0x83u || log.length != 256u ||
        log.length_code != 7u || log.cylinder != 1u || log.head != 0u ||
        log.sector != 240u || log.buffer != buffer) {
        return 1;
    }

    log.status = 0x00u;
    if (redmcsb_f0812_check_copy_protection_side0_track1_pc34_compat(
            0x83u, buffer, record_read, &log)) {
        return 1;
    }
    if (strcmp(redmcsb_f0812_check_copy_protection_side0_track1_source_evidence_pc34(),
               "ReDMCSB IO.C:4153-4172; PC-98 INT 1Bh sector 240 CRC-status gate") != 0) {
        return 1;
    }
    return 0;
}
