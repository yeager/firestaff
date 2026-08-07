#include "dm1_v2_auto_save_pc34.h"
#include "dm1_v2_input_remap_pc34.h"
#include "dm1_v2_inventory_sort_pc34.h"
#include <stdio.h>
#include <string.h>
static int failed;
#define OK(x) do { if (!(x)) { fprintf(stderr, "FAIL %s\n", #x); ++failed; } } while (0)
int main(void) {
    unsigned char bytes[20]; char path[32];
    memset(bytes, 0xA5, sizeof(bytes)); memset(path, 0x5A, sizeof(path));
    v2_input_init_defaults(); v2_input_remap(M11_V2_GA_MOVE_FORWARD, 87);
    OK(v2_input_get_action(87) == -1); OK(!v2_input_save("input.bin")); OK(!v2_input_load("input.bin"));
    v2_autosave_init(5, 100); v2_autosave_enable(); OK(!v2_autosave_check(1000));
    v2_autosave_get_path(path, (int)sizeof(path)); OK(path[0] == (char)0x5A);
    OK(v2_autosave_serialize_state(NULL, bytes, (int)sizeof(bytes)) == -1); OK(bytes[0] == 0xA5);
    OK(!v2_autosave_is_enabled()); OK(v2_autosave_current_slot() == -1);
    v2_inv_init(); OK(!v2_inv_add_item("Sword", 5, 99, ICAT_WEAPON)); v2_inv_sort(SORT_BY_VALUE);
    OK(v2_inv_count() == 0); OK(v2_inv_get(0) == NULL); OK(!v2_inv_remove(0));
    if (failed) return 1; puts("dm1_v2_host_state_extensions_pc34: ok"); return 0;
}
