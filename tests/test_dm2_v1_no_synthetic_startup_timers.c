#include "dm2_v1_timeline.h"
int main(void) {
    dm2_v1_timeline_reset_state();
    return dm2_v1_timeline_init() == DM2_TIMELINE_RESULT_OK &&
           dm2_v1_timeline_get_builtin_count() == 0 &&
           dm2_v1_timeline_queue_size() == 0 &&
           dm2_v1_timeline_tick(60000) == 0 &&
           dm2_v1_timeline_total_fires() == 0 ? 0 : 1;
}
