#include "dm1_v2_message_log_pc34.h"
#include "dm1_v2_stat_tracker_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t framebuffer[16] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    };
    uint8_t before[sizeof(framebuffer)];
    unsigned char serialized[sizeof(M11_V2_GameStats)];
    const M11_V2_GameStats *all;
    int ok = 1;

    memcpy(before, framebuffer, sizeof(framebuffer));
    v2_log_init();
    v2_log_add("HOST LOG", 255, V2_LOG_COMBAT);
    v2_log_scroll_up();
    v2_log_scroll_down();
    v2_log_toggle();
    v2_log_render(framebuffer, 4, 4, 4);
    v2_log_clear();
    ok &= memcmp(framebuffer, before, sizeof(framebuffer)) == 0;

    memset(serialized, 0xa5, sizeof(serialized));
    v2_stats_init();
    v2_stats_increment(M11_V2_STAT_TOTAL_KILLS, 99);
    ok &= v2_stats_get(M11_V2_STAT_TOTAL_KILLS) == 0;
    ok &= !v2_stats_save("synthetic.stats");
    ok &= !v2_stats_load("synthetic.stats");
    ok &= v2_stats_serialize(serialized, (int)sizeof(serialized)) == -1;
    ok &= v2_stats_deserialize(serialized, (int)sizeof(serialized)) == -1;
    ok &= serialized[0] == 0xa5;
    all = v2_stats_get_all();
    ok &= all && all->total_steps == 0 && all->damage_dealt == 0;

    puts(ok ? "PASS dm1_v2_message_stats_pc34" :
              "FAIL dm1_v2_message_stats_pc34");
    return ok ? 0 : 1;
}
