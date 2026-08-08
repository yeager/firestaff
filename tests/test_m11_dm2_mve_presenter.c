#include "dm2_v1_boot.h"
#include "dm2_v1_dos_real_data_manifest.h"
#include "m11_dm2_mve_presenter.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t count;
    uint32_t last_index;
    uint64_t last_time_us;
    uint32_t pixel_hash;
} TestSink;

static int sink_present(void *context, const uint8_t *pixels,
                        const uint8_t palette[256][3], uint32_t index,
                        uint64_t time_us)
{
    TestSink *sink = (TestSink *)context;
    assert(sink && pixels && palette && index == sink->count);
    sink->last_index = index;
    sink->last_time_us = time_us;
    sink->pixel_hash = sink->pixel_hash * 16777619u + pixels[index % 64000u];
    ++sink->count;
    return 1;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const dm2_v1_dos_file_fp_t *fingerprint;
    DM2_V1_BootProfile boot;
    M11_Dm2MvePresenter presenter;
    TestSink sink = {0};
    const uint8_t *bytes = NULL;
    size_t size = 0u;
    uint32_t index;

    if (!root) {
        puts("SKIP: no DM2 DOS DATA root");
        return 0;
    }
    fingerprint = dm2_v1_dos_file_fp_lookup_pc34("intro");
    dm2_v1_boot_profile_init(&boot);
    assert(dm2_v1_boot_scan_assets(&boot, root) == 0);
    assert(boot.assets_verified && boot.platform == DM2_PLATFORM_PC_EN);
    assert(fingerprint &&
           dm2_v1_boot_dos_intro_mve_readonly(&boot, &bytes, &size) == 1 &&
           bytes && size == fingerprint->size_bytes);
    assert(m11_dm2_mve_presenter_open(&presenter, bytes, size, 1000u,
                                      sink_present, &sink) == 1);
    assert(m11_dm2_mve_presenter_advance(&presenter, 999u) == -1);
    m11_dm2_mve_presenter_close(&presenter);

    assert(m11_dm2_mve_presenter_open(&presenter, bytes, size, 1000u,
                                      sink_present, &sink) == 1);
    assert(m11_dm2_mve_presenter_advance(&presenter, 999u) == -1);
    m11_dm2_mve_presenter_close(&presenter);
    sink.count = 0u;
    sink.pixel_hash = 0u;
    assert(m11_dm2_mve_presenter_open(&presenter, bytes, size, 1000u,
                                      sink_present, &sink) == 1);
    assert(m11_dm2_mve_presenter_advance(&presenter, 1000u) == 1);
    for (index = 1u; index < 217u; ++index) {
        const uint64_t deadline = 1000u + (uint64_t)index * 83328u;
        assert(m11_dm2_mve_presenter_advance(&presenter, deadline - 1u) == 0);
        assert(m11_dm2_mve_presenter_advance(&presenter, deadline) == 1);
    }
    assert(presenter.ended && !presenter.failed && sink.count == 217u &&
           sink.last_index == 216u && sink.last_time_us == 216u * 83328u &&
           presenter.audio.queued_source_packets == 217u &&
           presenter.audio.queued_source_bytes == 797426u);
    assert(m11_dm2_mve_presenter_advance(&presenter,
                                         1000u + 217u * 83328u) == 0);
    m11_dm2_mve_presenter_close(&presenter);
    dm2_v1_boot_cleanup(&boot);
    puts("PASS: M11 DM2 MVE seam preserves retail display and PCM order");
    return 0;
}
