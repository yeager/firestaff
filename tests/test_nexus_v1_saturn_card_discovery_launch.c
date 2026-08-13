#include "nexus_v1_saturn_card_discovery.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_saturn_save_capture.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char card_path[128];
    const char *paths[2];
    Nexus_V1_SaturnCardDiscoveryInput input;
    Nexus_V1_SaturnCardDiscoveryReceipt discovery;
    Nexus_V1_Engine engine;
    Nexus_V1_SaturnSaveCaptureReceipt save;
    Nexus_V1_LauncherSaturnCardStartupReceipt startup;
    unsigned char empty_backup_ram[8192] = {0};
    FILE *file;
    int result = 1;

    /* Several agents/CTest jobs may run this test at the same time. A fixed
     * /tmp pathname made one process overwrite or remove another process's
     * card, producing a nondeterministic discovery failure. */
    snprintf(card_path, sizeof(card_path), "/tmp/firestaff-nexus-card-%ld",
             (long)getpid());
    paths[0] = card_path;
    paths[1] = "/tmp/cards.zip::SATURN.BUP";
    input.paths = paths;
    input.path_count = 1;

    file = fopen(card_path, "wb");
    if (!file || fwrite(empty_backup_ram, 1, sizeof(empty_backup_ram), file) !=
                    sizeof(empty_backup_ram)) {
        if (file) fclose(file);
        remove(card_path);
        return 1;
    }
    fclose(file);

    if (!nexus_v1_saturn_card_discover(&input, &discovery) ||
        !discovery.valid || !discovery.direct_launch_permitted) {
        goto done;
    }

    memset(&engine, 0, sizeof(engine));
    memset(&save, 0, sizeof(save));
    save.valid = 1;
    save.status = NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE;
    save.opaque_only = 1;
    save.title_route_bound = 1;
    save.champion_route_bound = 1;
    save.image_bytes = sizeof(empty_backup_ram);
    save.image_fnv1a64 = discovery.image_fnv1a64;
    if (!nexus_v1_engine_set_saturn_save_capture_receipt(
            &engine, 3, &save) ||
        !nexus_v1_launcher_select_saturn_card_startup(
            &engine, 3, discovery.image_fnv1a64, &startup) ||
        nexus_v1_launcher_select_saturn_card_startup(
            &engine, 4, discovery.image_fnv1a64, &startup)) {
        goto done;
    }

    input.paths = &paths[1];
    if (nexus_v1_saturn_card_discover(&input, &discovery) ||
        !discovery.virtual_candidate_seen || discovery.image_fnv1a64 ||
        discovery.path[0] || discovery.direct_launch_permitted) {
        goto done;
    }

    input.paths = paths;
    input.path_count = 2;
    if (nexus_v1_saturn_card_discover(&input, &discovery) ||
        !discovery.ambiguous || !discovery.virtual_candidate_seen) {
        goto done;
    }

    result = 0;

done:
    remove(card_path);
    return result;
}
