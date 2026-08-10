#include "nexus_v1_scsp_trace.h"
#include "nexus_v1_sound.h"
#include "nexus_v1_audio_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_bytes(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *data;

    if (out_size) *out_size = 0U;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)length);
    if (!data || fread(data, 1U, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    if (out_size) *out_size = (size_t)length;
    return data;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *scsp_path = getenv("FIRESTAFF_NEXUS_SCSP_TRACE");
    const char *main_path = getenv("FIRESTAFF_NEXUS_MAIN_SCSP_TRACE");
    char sal_path[1024];
    char map_path[1024];
    char driver_path[1024];
    unsigned char *sal = NULL;
    unsigned char *map = NULL;
    unsigned char *driver = NULL;
    unsigned char *scsp = NULL;
    unsigned char *main_trace = NULL;
    size_t sal_size = 0U;
    size_t map_size = 0U;
    size_t driver_size = 0U;
    size_t scsp_size = 0U;
    size_t main_size = 0U;
    Nexus_V1_ScspTraceReceipt scsp_receipt;
    Nexus_V1_MainScspTraceReceipt main_receipt;
    Nexus_V1_SddrvsDisassemblyReceipt driver_receipt;
    Nexus_V1_ScspRuntimeJoinReceipt join_receipt;
    Nexus_V1_ScspTraceReceipt complete_trace_fixture;
    Nexus_V1_ScspRuntimeJoinReceipt complete_join_receipt;
    Nexus_SoundEngine engine;
    Nexus_SfxRuntimeReceipt sound_receipt;
    int ok = 0;

    if (!data_dir || !data_dir[0] || !scsp_path || !scsp_path[0] ||
        !main_path || !main_path[0]) {
        puts("SKIP: Nexus runtime join inputs are not mounted");
        return 77;
    }
    snprintf(sal_path, sizeof(sal_path), "%s/SNDLEV00.SAL", data_dir);
    snprintf(map_path, sizeof(map_path), "%s/SNDLEV00.MAP", data_dir);
    snprintf(driver_path, sizeof(driver_path), "%s/SDDRVS.TSK", data_dir);
    sal = read_bytes(sal_path, &sal_size);
    map = read_bytes(map_path, &map_size);
    driver = read_bytes(driver_path, &driver_size);
    scsp = read_bytes(scsp_path, &scsp_size);
    main_trace = read_bytes(main_path, &main_size);
    if (!sal || !map || !driver || !scsp || !main_trace ||
        !nexus_v1_scsp_write_trace_parse(scsp, scsp_size, &scsp_receipt) ||
        !nexus_v1_main_scsp_write_trace_parse(
            main_trace, main_size, &main_receipt) ||
        !nexus_v1_audio_sddrvs_disassembly_receipt(
            driver, (uint32_t)driver_size, &driver_receipt)) {
        goto cleanup;
    }
    memset(&join_receipt, 0, sizeof(join_receipt));
    if (nexus_v1_scsp_runtime_join(
            &scsp_receipt, &main_receipt, &driver_receipt,
            1, 1, 1, 1, &join_receipt)) {
        if (!join_receipt.valid || !join_receipt.runtime_corridor_proven ||
            join_receipt.event_selector_semantics_proven ||
            join_receipt.sal_codec_proven || join_receipt.playback_permitted) {
            goto cleanup;
        }
    } else if (join_receipt.runtime_corridor_proven ||
               !join_receipt.blocks_real_sfx_playback ||
               join_receipt.event_selector_semantics_proven ||
               join_receipt.sal_codec_proven || join_receipt.playback_permitted) {
        /* A partial capture must stay blocked, never become a false join. */
        goto cleanup;
    }
    complete_trace_fixture = scsp_receipt;
    complete_trace_fixture.scsp_voice_register_write_count = 1U;
    complete_trace_fixture.intra_trace_observation_order_proven = 1;
    memset(&complete_join_receipt, 0, sizeof(complete_join_receipt));
    if (!nexus_v1_scsp_runtime_join(
            &complete_trace_fixture, &main_receipt, &driver_receipt,
            1, 1, 1, 1, &complete_join_receipt) ||
        !complete_join_receipt.valid ||
        !complete_join_receipt.source_identities_bound ||
        !complete_join_receipt.runtime_corridor_proven ||
        !complete_join_receipt.producer_command_observed ||
        !complete_join_receipt.sound_cpu_command_observed ||
        !complete_join_receipt.driver_command_handler_observed ||
        !complete_join_receipt.pcm_voice_register_route_observed ||
        complete_join_receipt.event_selector_semantics_proven ||
        complete_join_receipt.sal_codec_proven ||
        complete_join_receipt.playback_permitted ||
        !complete_join_receipt.blocks_real_sfx_playback) {
        goto cleanup;
    }
    if (!scsp_receipt.driver_command_handler_observed ||
        scsp_receipt.mailbox_value_02_count == 0U ||
        !main_receipt.producer_command_observed ||
        main_receipt.mailbox_value_02_count == 0U ||
        main_receipt.mailbox_value_0200_count == 0U ||
        !driver_receipt.command_handler_proven ||
        !driver_receipt.pcm_voice_register_route_proven ||
        driver_receipt.event_dispatch_proven ||
        driver_receipt.playback_permitted) {
        goto cleanup;
    }

    memset(&engine, 0, sizeof(engine));
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    if (nexus_sound_init(&engine) != 0 ||
        nexus_sound_load_canonical_level(
            &engine, 0, sal, (int)sal_size, map, (int)map_size, 1, 1) != 0) {
        nexus_sound_shutdown(&engine);
        goto cleanup;
    }
    nexus_sound_set_driver_canonical_source_verified(&engine, 1);
    if (nexus_sound_level_runtime_receipt(&engine, &sound_receipt) != 0 ||
        !sound_receipt.sal_tone_bank_directory_supported ||
        sound_receipt.sal_tone_entry_count_decoded <= 4 ||
        sound_receipt.map_record_table_supported == 0 ||
        sound_receipt.map_record_count == 0 ||
        sound_receipt.event_dispatch_source_verified ||
        sound_receipt.playback_enabled ||
        !sound_receipt.blocks_real_sfx_playback ||
        sound_receipt.status != NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE) {
        nexus_sound_shutdown(&engine);
        goto cleanup;
    }
    nexus_sound_shutdown(&engine);
    ok = 1;

cleanup:
    free(sal);
    free(map);
    free(driver);
    free(scsp);
    free(main_trace);
    if (!ok) {
        puts("FAIL: Nexus SLEV/SAL/SCSP runtime join");
        return 1;
    }
    puts("test_nexus_v1_slev_scsp_runtime_join: PASS (source-bound, playback blocked)");
    return 0;
}
