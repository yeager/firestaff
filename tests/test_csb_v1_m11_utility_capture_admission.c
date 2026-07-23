/* Focused CSB ENTRANCE.C utility capture admission.
 * The optional DM1 payload only reaches the real CSB boot route; it is not
 * presentation material.  M11 must therefore show the source raster only
 * while the package and capture identities are current. */

#include "csb_v1_boot.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) \
    do { if (!(condition)) { fprintf(stderr, "FAIL: %s\n", (message)); ++failures; } } while (0)

static void write_le16(unsigned char *buf, size_t off, unsigned short value)
{
    buf[off] = (unsigned char)(value & 0xffu);
    buf[off + 1u] = (unsigned char)(value >> 8);
}

static int write_dm1_import_save(const char *path)
{
    unsigned char buf[1024];
    size_t off = CSB_V1_DM1_HDR_CHAMPION_START;
    size_t equip_off = off + CSB_V1_DM1_CHAMP_OFF_EQUIP;
    FILE *file;
    int slot;

    if (!path) {
        return 0;
    }
    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 1u;
    memcpy(buf + off + CSB_V1_DM1_CHAMP_OFF_NAME, "UTILITY ", 8u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_HEALTH, 80u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH, 100u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_STAMINA, 60u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA, 100u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MANA, 30u);
    write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA, 50u);
    for (slot = 0; slot < CSB_V1_SLOT_COUNT; ++slot) {
        write_le16(buf, equip_off + (size_t)slot * 2u, 0xffffu);
    }
    file = fopen(path, "wb");
    if (!file) {
        return 0;
    }
    if (fwrite(buf, 1u, sizeof(buf), file) != sizeof(buf)) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static int nonzero_count(const unsigned char *pixels, size_t count)
{
    size_t i;
    int result = 0;
    for (i = 0u; pixels && i < count; ++i) {
        result += pixels[i] != 0u;
    }
    return result;
}

static int advance_to_utility_wait(M11_GameViewState *view)
{
    int guard = csb_v1_startup_title_total_ticks_pc34() + 8;
    while (view && guard-- > 0 && view->csbState.startup_entrance_source_step < 4) {
        if (M11_GameView_AdvanceIdleTick(view) != M11_GAME_INPUT_REDRAW) {
            return 0;
        }
    }
    return view && view->csbState.startup_entrance_active &&
           view->csbState.startup_entrance_source_step == 4 &&
           view->csbState.startup_import_available;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_DATA_DIR");
    char import_template[] = "/tmp/firestaff-csb-utility-XXXXXX";
    unsigned char framebuffer[320 * 200];
    CSB_V1_StartupRuntimeAssetSession_PC34 *session;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    int fd;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_DATA_DIR is not set");
        return 0;
    }
    fd = mkstemp(import_template);
    if (fd < 0) {
        fputs("FAIL: unable to allocate DM1 import fixture\n", stderr);
        return 1;
    }
    close(fd);
    CHECK(write_dm1_import_save(import_template),
          "utility fixture supplies a structurally valid DM1 import record");
    if (failures) {
        remove(import_template);
        return 1;
    }
    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = data_dir;
    spec.csbImportDm1SavePath = import_template;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    M11_GameView_Init(&view);
    CHECK(M11_GameView_Start(&view, &spec),
          "hash-verified CSB boot opens the utility route");
    CHECK(advance_to_utility_wait(&view),
          "ENTRANCE.C utility wait receipt reaches M11");
    session = (CSB_V1_StartupRuntimeAssetSession_PC34 *)
        view.csbStartupRuntimeAssetSession;
    CHECK(session && session->csbStartupPackageIdentity != 0u &&
              view.csbStartupReleaseLifecycleReceipt.valid,
          "utility route retains package and capture identities");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(nonzero_count(framebuffer, sizeof(framebuffer)) > 0,
          "current utility receipt reaches the verified startup raster");

    view.csbStartupReleaseLifecycleReceipt.release_capture_hash ^= 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(nonzero_count(framebuffer, sizeof(framebuffer)) > 0,
          "utility raster admission uses the current C004/C002/C003 session surface");
    view.csbStartupReleaseLifecycleReceipt.release_capture_hash ^= 1u;

    session->csbStartupPackageIdentity ^= 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(nonzero_count(framebuffer, sizeof(framebuffer)) == 0,
          "mismatched utility package identity admits no raster or text fallback");
    session->csbStartupPackageIdentity ^= 1u;

    view.csbStartupReleaseAppCaptureReceipt.valid = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(nonzero_count(framebuffer, sizeof(framebuffer)) > 0,
          "utility raster does not use a synthetic release-ready wrapper");
    M11_GameView_Shutdown(&view);
    remove(import_template);
    if (failures) {
        fprintf(stderr, "FAIL: csb_v1_m11_utility_capture_admission (%d failures)\n",
                failures);
        return 1;
    }
    puts("PASS: csb_v1_m11_utility_capture_admission");
    return 0;
}
