#include "nexus_v1_audio_receipt.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    int level;
    int checked = 0;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }

    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        char path[2048];
        uint8_t *data;
        uint32_t size;
        Nexus_V1_AudioReceipt asset;
        Nexus_V1_SalOpaquePrefixReceipt prefix;

        snprintf(path, sizeof(path), "%s/SNDLEV%02d.SAL", data_dir, level);
        data = read_file(path, &size);
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
            return 1;
        }
        if (level == 0) {
            data[0] ^= 0x01u;
            if (nexus_v1_audio_sal_opaque_prefix_receipt(data, size, &prefix)) {
                puts("FAIL: altered SAL opaque prefix was accepted");
                free(data);
                return 1;
            }
            data[0] ^= 0x01u;
            if (nexus_v1_audio_sal_opaque_prefix_receipt(data, 32u, &prefix)) {
                puts("FAIL: truncated SAL opaque prefix was accepted");
                free(data);
                return 1;
            }
        }
        free(data);
        ++checked;
    }

    {
        char path[2048];
        uint8_t *data;
        uint32_t size;
        Nexus_V1_SddrvsDisassemblyReceipt driver;

        snprintf(path, sizeof(path), "%s/SDDRVS.TSK", data_dir);
        data = read_file(path, &size);
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
            driver.pcm_voice_handler_offset != 0x1f0eU ||
            !driver.m68k_instruction_stream_proven ||
            !driver.command_dispatch_proven ||
            !driver.pcm_voice_register_route_proven ||
            driver.event_dispatch_proven || driver.playback_permitted) {
            puts("FAIL: SDDRVS.TSK 68k disassembly receipt was not admitted");
            free(data);
            return 1;
        }
        free(data);
        puts("SDDRVS.TSK disassembly: 68k entry/dispatch/PCM corridors bound; playback blocked");
    }

    printf("SAL opaque-prefix corpus: banks=%d prefix_bytes=33 decode=blocked\n",
           checked);
    return checked == NEXUS_V1_AUDIO_LEVEL_COUNT ? 0 : 1;
}
