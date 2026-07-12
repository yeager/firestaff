#include "theron_v1_irq2_live_trace_gate.h"

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

static int read_trace(const char *path, Theron_Track02Variant expected_variant,
                      Theron_V1Irq2LiveTrace *out_trace) {
    static const char *const keys[] = {
        "source", "variant", "stage3_track02_record", "cd_read_return_pc",
        "irq2_entry_pc", "cd_state_pc", "cd_state_branch_pc",
        "f5_after_cd_read", "f5_at_irq2_entry", "cd_status_1802",
        "cd_status_1803", "f2_before_merge", "f2_at_branch"
    };
    FILE *file;
    char line[128];
    size_t index = 0u;

    if (!path || !out_trace || !(file = fopen(path, "r"))) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    while (fgets(line, sizeof(line), file)) {
        char *equals;
        char *newline;
        uint32_t value;

        newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        if (index >= sizeof(keys) / sizeof(keys[0]) ||
            !(equals = strchr(line, '='))) {
            fclose(file);
            return 0;
        }
        *equals++ = '\0';
        if (strcmp(line, keys[index]) != 0) {
            fclose(file);
            return 0;
        }
        if (index == 0u) {
            if (strcmp(equals, "mednafen-pce") != 0) {
                fclose(file);
                return 0;
            }
        } else if (index == 1u) {
            const char *expected_name = expected_variant == THERON_TRACK02_VARIANT_JP_BIN
                ? "jp_bin" : "us_bin";
            if (strcmp(equals, expected_name) != 0) {
                fclose(file);
                return 0;
            }
        } else if (!parse_u32(equals, &value)) {
            fclose(file);
            return 0;
        } else {
            switch (index) {
                case 2u: out_trace->stage3_track02_record = value; break;
                case 3u: out_trace->cd_read_return_pc = (uint16_t)value; break;
                case 4u: out_trace->irq2_entry_pc = (uint16_t)value; break;
                case 5u: out_trace->cd_state_pc = (uint16_t)value; break;
                case 6u: out_trace->cd_state_branch_pc = (uint16_t)value; break;
                case 7u: out_trace->f5_after_cd_read = (uint8_t)value; break;
                case 8u: out_trace->f5_at_irq2_entry = (uint8_t)value; break;
                case 9u: out_trace->cd_status_1802 = (uint8_t)value; break;
                case 10u: out_trace->cd_status_1803 = (uint8_t)value; break;
                case 11u: out_trace->f2_before_merge = (uint8_t)value; break;
                case 12u: out_trace->f2_at_branch = (uint8_t)value; break;
                default: fclose(file); return 0;
            }
        }
        ++index;
    }
    fclose(file);
    if (index != sizeof(keys) / sizeof(keys[0])) return 0;
    out_trace->magic = 0x54514932u;
    out_trace->version = 1u;
    out_trace->source = THERON_V1_IRQ2_TRACE_SOURCE_MEDNAFEN_PCE;
    out_trace->variant = expected_variant;
    return 1;
}

int main(int argc, char **argv) {
    uint8_t *track02_bytes = NULL;
    uint8_t *system_card_bytes = NULL;
    size_t track02_size;
    size_t system_card_size;
    Theron_Track02Variant variant;
    Theron_V1Irq2LiveTrace trace;
    Theron_V1Irq2FullMediaTraceReceipt receipt;

    if (argc != 5) {
        fprintf(stderr, "usage: %s TRACK02 TRACK02_MD5 SYSCARD3 TRACE\n", argv[0]);
        return 2;
    }
    variant = theron_v1_track02_variant_for_md5(argv[2]);
    if ((variant != THERON_TRACK02_VARIANT_JP_BIN &&
         variant != THERON_TRACK02_VARIANT_US_BIN) ||
        !(track02_bytes = read_file_bytes(argv[1], &track02_size)) ||
        !(system_card_bytes = read_file_bytes(argv[3], &system_card_size)) ||
        !read_trace(argv[4], variant, &trace) ||
        !theron_v1_irq2_live_branch_from_full_track02_media(
            track02_bytes, track02_size, argv[2], system_card_bytes,
            system_card_size, "ff1a674273fe3540ccef576376407d1d", &trace,
            &receipt) || !receipt.valid) {
        free(track02_bytes);
        free(system_card_bytes);
        fprintf(stderr, "FAIL: incomplete or unauthenticated IRQ2 trace\n");
        return 1;
    }
    printf("PASS: variant=%s record=0x%04x f5=0x%02x status=%02x/%02x f2=%02x->%02x\n",
           receipt.variant == THERON_TRACK02_VARIANT_JP_BIN ? "jp_bin" : "us_bin",
           receipt.stage3_track02_record, receipt.f5_at_irq2_entry,
           receipt.cd_status_1802, receipt.cd_status_1803,
           receipt.f2_before_merge, receipt.f2_at_branch);
    free(track02_bytes);
    free(system_card_bytes);
    return 0;
}
