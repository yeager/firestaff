#include <stdio.h>

#include "memory_cache_defrag_result_pc34_compat.h"

static int test_apply_defrag_result_moves_cache_top_and_clears_unused_list(void) {
    struct MemoryCacheUnusedBlock_Compat unused = {24, 0, 0};
    struct MemoryCacheDefragResultState_Compat state;
    state.cacheMemoryTopOffset = 100;
    state.firstUnusedBlock = &unused;
    MEMORY_CACHE_ApplyDefragResult_Compat(72, &state);
    return state.cacheMemoryTopOffset == 72 && state.firstUnusedBlock == 0;
}

static int test_apply_defrag_result_can_keep_zero_unused_state(void) {
    struct MemoryCacheDefragResultState_Compat state;
    state.cacheMemoryTopOffset = 40;
    state.firstUnusedBlock = 0;
    MEMORY_CACHE_ApplyDefragResult_Compat(40, &state);
    return state.cacheMemoryTopOffset == 40 && state.firstUnusedBlock == 0;
}

int main(void) {
    if (!test_apply_defrag_result_moves_cache_top_and_clears_unused_list()) {
        fprintf(stderr, "test_apply_defrag_result_moves_cache_top_and_clears_unused_list failed\n");
        return 1;
    }
    if (!test_apply_defrag_result_can_keep_zero_unused_state()) {
        fprintf(stderr, "test_apply_defrag_result_can_keep_zero_unused_state failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
