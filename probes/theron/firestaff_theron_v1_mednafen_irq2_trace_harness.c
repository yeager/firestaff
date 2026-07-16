#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_system_card_irq2_cd_state_gate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Track 02's unrelated level entry points are not part of this media/trace
 * harness. These stubs keep the manual link focused on the loader path. */
Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                          const uint8_t *data,
                                          int data_size,
                                          int dungeon_id,
                                          int sub_level_index) {
    (void)level;
    (void)data;
    (void)data_size;
    (void)dungeon_id;
    (void)sub_level_index;
    return THERON_MAP_ERR_NULL;
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    (void)world;
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int parse_u32(const char *text, uint32_t *out_value) {
    char *end = NULL;
    unsigned long value;

    if (!text || !out_value) return 0;
    value = strtoul(text, &end, 0);
    if (!end || *end != '\0' || value > 0xfffffffful) return 0;
    *out_value = (uint32_t)value;
    return 1;
}

typedef struct {
    int valid;
    unsigned int snapshot_count;
    unsigned int io_write_count;
    int system_card_before_stage3;
    int io_write_before_stage3;
    uint32_t stage3_seq;
    uint32_t irq2_seq;
    uint32_t cd_state_seq;
    uint32_t branch_seq;
    uint8_t f5_after_cd_read;
    uint8_t f5_at_irq2_entry;
    uint8_t cd_status_1802;
    uint8_t cd_status_1803;
    uint8_t f2_before_merge;
    uint8_t f2_at_branch;
} TheronIrq2CaptureReceipt;

static int read_trace(const char *path, Theron_Track02Variant expected_variant,
                      TheronIrq2CaptureReceipt *out_receipt) {
    FILE *file;
    char line[256];
    size_t index = 0u;
    uint32_t previous_sequence = 0u;
    int have_previous_sequence = 0;

    if (!path || !out_receipt || !(file = fopen(path, "r"))) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    while (fgets(line, sizeof(line), file)) {
        char *equals;
        char *newline;

        newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        if (index < 4u) {
            if (!(equals = strchr(line, '='))) {
                fclose(file);
                return 0;
            }
            *equals++ = '\0';
            if ((index == 0u && (strcmp(line, "trace_format") != 0 ||
                                 strcmp(equals, "theron_irq2_capture_v2") != 0)) ||
                (index == 1u && (strcmp(line, "source") != 0 ||
                                 strcmp(equals, "mednafen-pce-instrumented") != 0)) ||
                (index == 2u && strcmp(line, "variant") != 0) ||
                (index == 3u && strcmp(line, "stage3_track02_record") != 0)) {
                fclose(file);
                return 0;
            }
            if (index == 2u) {
            const char *expected_name = expected_variant == THERON_TRACK02_VARIANT_JP_BIN
                ? "jp_bin" : "us_bin";
            if (strcmp(equals, expected_name) != 0) {
                fclose(file);
                return 0;
            }
            }
            if (index == 3u) {
                uint32_t expected_record = expected_variant == THERON_TRACK02_VARIANT_JP_BIN
                    ? 0x4dfu : 0x4e0u;
                uint32_t record;
                if (!parse_u32(equals, &record) || record != expected_record) {
                    fclose(file);
                    return 0;
                }
            }
            ++index;
            continue;
        }
        if (strncmp(line, "io_write ", 9u) == 0) {
            unsigned int sequence;
            unsigned int timestamp;
            unsigned int logical_pc;
            unsigned int physical_addr;
            unsigned int port;
            unsigned int data;
            int consumed = 0;
            if (sscanf(line,
                       "io_write seq=%u timestamp=%u logical_pc=%x physical_addr=%x port=%x data=%x%n",
                       &sequence, &timestamp, &logical_pc, &physical_addr,
                       &port, &data, &consumed) != 6 || line[consumed] != '\0' ||
                logical_pc > 0xffffu || port > 0x0fu || data > 0xffu ||
                (have_previous_sequence && sequence <= previous_sequence)) {
                fclose(file);
                return 0;
            }
            (void)timestamp;
            (void)physical_addr;
            previous_sequence = sequence;
            have_previous_sequence = 1;
            ++out_receipt->io_write_count;
            if (out_receipt->stage3_seq == 0u) {
                out_receipt->io_write_before_stage3 = 1;
            }
            continue;
        }
        {
            unsigned int sequence;
            unsigned int logical_pc;
            unsigned int physical_pc;
            unsigned int cd_1800;
            unsigned int cd_1801;
            unsigned int cd_1802;
            unsigned int cd_1803;
            unsigned int f5;
            unsigned int f2;
            int consumed = 0;
            if (sscanf(line,
                       "snapshot seq=%u logical_pc=%x physical_pc=%x cd_1800=%x cd_1801=%x cd_1802=%x cd_1803=%x f5=%x f2=%x%n",
                       &sequence, &logical_pc, &physical_pc, &cd_1800,
                       &cd_1801, &cd_1802, &cd_1803, &f5, &f2,
                       &consumed) != 9 || line[consumed] != '\0' ||
                logical_pc > 0xffffu || cd_1800 > 0xffu || cd_1801 > 0xffu ||
                cd_1802 > 0xffu || cd_1803 > 0xffu || f5 > 0xffu || f2 > 0xffu ||
                (have_previous_sequence && sequence <= previous_sequence)) {
                fclose(file);
                return 0;
            }
            (void)physical_pc;
            previous_sequence = sequence;
            have_previous_sequence = 1;
            ++out_receipt->snapshot_count;
            if (logical_pc >= 0xe000u && logical_pc < 0xf000u &&
                out_receipt->stage3_seq == 0u) {
                out_receipt->system_card_before_stage3 = 1;
            }
            if (logical_pc == 0x4093u && out_receipt->stage3_seq == 0u) {
                out_receipt->stage3_seq = sequence;
                out_receipt->f5_after_cd_read = (uint8_t)f5;
            } else if (logical_pc == 0xe736u && out_receipt->irq2_seq == 0u) {
                out_receipt->irq2_seq = sequence;
                out_receipt->f5_at_irq2_entry = (uint8_t)f5;
            } else if (logical_pc == 0xe742u && out_receipt->cd_state_seq == 0u) {
                out_receipt->cd_state_seq = sequence;
                out_receipt->cd_status_1802 = (uint8_t)cd_1802;
                out_receipt->cd_status_1803 = (uint8_t)cd_1803;
                out_receipt->f2_before_merge = (uint8_t)f2;
            } else if (logical_pc == 0xe74cu && out_receipt->branch_seq == 0u) {
                out_receipt->branch_seq = sequence;
                out_receipt->f2_at_branch = (uint8_t)f2;
            }
        }
    }
    fclose(file);
    out_receipt->valid = index == 4u && out_receipt->snapshot_count > 0u &&
        out_receipt->io_write_count > 0u && out_receipt->system_card_before_stage3 &&
        out_receipt->io_write_before_stage3 && out_receipt->stage3_seq > 0u &&
        out_receipt->irq2_seq > out_receipt->stage3_seq &&
        out_receipt->cd_state_seq > out_receipt->irq2_seq &&
        out_receipt->branch_seq > out_receipt->cd_state_seq &&
        out_receipt->f5_after_cd_read == out_receipt->f5_at_irq2_entry &&
        out_receipt->f2_at_branch == (uint8_t)(
            (out_receipt->cd_status_1802 & out_receipt->cd_status_1803) |
            out_receipt->f2_before_merge);
    return out_receipt->valid;
}

int main(int argc, char **argv) {
    uint8_t *track02_bytes = NULL;
    uint8_t *system_card_bytes = NULL;
    size_t track02_size;
    size_t system_card_size;
    Theron_Track02Variant variant;
    TheronIrq2CaptureReceipt capture;
    Theron_V1Stage2RuntimeHandoff handoff;
    Theron_V1SystemCardIrq2CdStateGate state_gate;

    if (argc != 5) {
        fprintf(stderr, "usage: %s TRACK02 TRACK02_MD5 SYSCARD3 TRACE\n", argv[0]);
        return 2;
    }
    variant = theron_v1_track02_variant_for_md5(argv[2]);
    if ((variant != THERON_TRACK02_VARIANT_JP_BIN &&
         variant != THERON_TRACK02_VARIANT_US_BIN) ||
        !(track02_bytes = read_file_bytes(argv[1], &track02_size)) ||
        !(system_card_bytes = read_file_bytes(argv[3], &system_card_size)) ||
        !read_trace(argv[4], variant, &capture) ||
        !theron_v1_stage2_runtime_handoff_from_original_media(
            track02_bytes, track02_size, argv[2], &handoff) || !handoff.valid ||
        !theron_v1_system_card_irq2_cd_state_gate_from_full_track02_media(
            track02_bytes, track02_size, argv[2], system_card_bytes,
            system_card_size, "ff1a674273fe3540ccef576376407d1d", &state_gate) ||
        !state_gate.valid || handoff.variant != variant ||
        state_gate.variant != variant ||
        state_gate.stage3_track02_record != handoff.track02_record) {
        free(track02_bytes);
        free(system_card_bytes);
        fprintf(stderr, "FAIL: incomplete or unauthenticated IRQ2 trace\n");
        return 1;
    }
    printf("PASS: capture-only variant=%s record=0x%04x snapshots=%u io-writes=%u "
           "f5=0x%02x status=%02x/%02x f2=%02x->%02x branch=unselected\n",
           variant == THERON_TRACK02_VARIANT_JP_BIN ? "jp_bin" : "us_bin",
           handoff.track02_record, capture.snapshot_count, capture.io_write_count,
           capture.f5_at_irq2_entry, capture.cd_status_1802,
           capture.cd_status_1803, capture.f2_before_merge,
           capture.f2_at_branch);
    free(track02_bytes);
    free(system_card_bytes);
    return 0;
}
