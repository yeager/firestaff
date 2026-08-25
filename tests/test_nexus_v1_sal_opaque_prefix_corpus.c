#include "nexus_v1_audio_receipt.h"
#include "nexus_v1_iso_reader.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, uint32_t *out_size);

static int open_retail_iso_if_present(const char *directory,
                                      Nexus_ISOReader *out_iso)
{
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        NULL
    };
    char path[2048];
    int index;

    if (!directory || !directory[0] || !out_iso) return 0;
    memset(out_iso, 0, sizeof(*out_iso));
    if (strlen(directory) >= 4U &&
        strcmp(directory + strlen(directory) - 4U, ".cue") == 0)
        return nexus_iso_open_cue(out_iso, directory) > 0;
    for (index = 0; cue_names[index]; ++index) {
        snprintf(path, sizeof(path), "%s/%s", directory, cue_names[index]);
        if (nexus_iso_open_cue(out_iso, path) > 0) return 1;
    }
    return 0;
}

static uint8_t *read_retail_member(const char *directory,
                                   Nexus_ISOReader *iso,
                                   const char *name,
                                   uint32_t *out_size)
{
    char path[2048];
    const Nexus_ISOFile *member;
    uint8_t *data;

    snprintf(path, sizeof(path), "%s/%s", directory, name);
    data = read_file(path, out_size);
    if (data) return data;
    if (!iso || !iso->valid || !(member = nexus_iso_find(iso, name)) ||
        member->size == 0U || member->size > (uint32_t)INT_MAX) return NULL;
    data = (uint8_t *)malloc(member->size);
    if (!data || nexus_iso_read_file(iso, member, data, (int)member->size) !=
                     (int)member->size) {
        free(data);
        return NULL;
    }
    *out_size = member->size;
    return data;
}

static uint8_t *read_file(const char *path, uint32_t *out_size) {
    FILE *file;
    long file_size;
    uint8_t *data;

    if (out_size) *out_size = 0;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 ||
        (unsigned long)file_size > UINT32_MAX ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data || fread(data, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    if (out_size) *out_size = (uint32_t)file_size;
    return data;
}

int main(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_ISOReader iso;
    int iso_opened;
    int level;
    int checked = 0;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }
    iso_opened = open_retail_iso_if_present(data_dir, &iso);

    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        char name[32];
        uint8_t *data;
        uint32_t size;
        Nexus_V1_AudioReceipt asset;
        Nexus_V1_SalOpaquePrefixReceipt prefix;

        snprintf(name, sizeof(name), "SNDLEV%02d.SAL", level);
        data = read_retail_member(data_dir, iso_opened ? &iso : NULL, name,
                                  &size);
        if (!data ||
            nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_SAL_BANK,
                                          level, &asset) != NEXUS_V1_AUDIO_OK ||
            size != asset.expected_size ||
            !nexus_v1_audio_sal_opaque_prefix_receipt(data, size, &prefix) ||
            !prefix.valid || prefix.opaque_prefix_bytes != 33u ||
            !prefix.signature_matches || !prefix.reserved_zero_bytes_match ||
            !prefix.marker_matches || prefix.codec_semantics_proven ||
            prefix.sample_semantics_proven || prefix.playback_semantics_proven ||
            !prefix.blocks_decode) {
            printf("FAIL: SNDLEV%02d.SAL did not retain the opaque prefix receipt\n",
                   level);
            free(data);
            if (iso_opened) nexus_iso_close(&iso);
            return 1;
        }
        if (level == 0) {
            data[0] ^= 0x01u;
            if (nexus_v1_audio_sal_opaque_prefix_receipt(data, size, &prefix)) {
                puts("FAIL: altered SAL opaque prefix was accepted");
                free(data);
                if (iso_opened) nexus_iso_close(&iso);
                return 1;
            }
            data[0] ^= 0x01u;
            if (nexus_v1_audio_sal_opaque_prefix_receipt(data, 32u, &prefix)) {
                puts("FAIL: truncated SAL opaque prefix was accepted");
                free(data);
                if (iso_opened) nexus_iso_close(&iso);
                return 1;
            }
        }
        free(data);
        ++checked;
    }

    {
        uint8_t *data;
        uint32_t size;
        Nexus_V1_SddrvsDisassemblyReceipt driver;

        data = read_retail_member(data_dir, iso_opened ? &iso : NULL,
                                  "SDDRVS.TSK", &size);
        if (!data || !nexus_v1_audio_sddrvs_disassembly_receipt(
                         data, size, &driver) ||
            driver.source_size != 26610U ||
            driver.code_entry_offset != 0x1000U ||
            driver.sound_cpu_ram_base != 0x00100000U ||
            driver.work_ram_base != 0x00007000U ||
            driver.stack_base != 0x0000a000U ||
            driver.command_dispatch_offset != 0x1c08U ||
            driver.command_jump_table_offset != 0x1c2aU ||
            driver.command_jump_table_count != 16U ||
            !driver.command_jump_targets_bound ||
            driver.command_jump_entry_kinds[0] != NEXUS_V1_SDDRVS_JUMP_PC_RELATIVE ||
            driver.command_jump_entry_kinds[11] != NEXUS_V1_SDDRVS_JUMP_ABSOLUTE_LONG ||
            driver.command_jump_target_offsets[0] != 0x1e6eU ||
            driver.command_jump_target_offsets[4] != 0x1f0eU ||
            driver.command_jump_target_offsets[11] != 0x1cbaU ||
            driver.command_jump_target_offsets[15] != 0x1cbcU ||
            driver.command_handler_offset != 0x2220U ||
            driver.command_handler_runtime_pc != 0x3224U ||
            driver.command_handler_valid_command_limit != 0x12U ||
            driver.command_handler_state_offset != 0x187eU ||
            driver.command_handler_channel_stride != 0x20U ||
            driver.command_handler_scsp_register_offset != 0x17U ||
            driver.command_handler_return_offset != 0x223cU ||
            driver.pcm_voice_handler_offset != 0x1f0eU ||
            !driver.m68k_instruction_stream_proven ||
            !driver.command_dispatch_proven ||
            !driver.command_handler_proven ||
            !driver.pcm_voice_register_route_proven ||
            driver.event_dispatch_proven || driver.playback_permitted) {
            puts("FAIL: SDDRVS.TSK 68k disassembly receipt was not admitted");
            free(data);
            if (iso_opened) nexus_iso_close(&iso);
            return 1;
        }
        data[0x1c2e] ^= 0x01u;
        if (nexus_v1_audio_sddrvs_disassembly_receipt(data, size, &driver)) {
            puts("FAIL: altered SDDRVS jump-table entry was accepted");
            free(data);
            if (iso_opened) nexus_iso_close(&iso);
            return 1;
        }
        free(data);
        puts("SDDRVS.TSK disassembly: 68k entry/dispatch/PCM corridors bound; playback blocked");
    }

    printf("SAL opaque-prefix corpus: banks=%d prefix_bytes=33 decode=blocked\n",
           checked);
    if (iso_opened) nexus_iso_close(&iso);
    return checked == NEXUS_V1_AUDIO_LEVEL_COUNT ? 0 : 1;
}
