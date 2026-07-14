#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif
int main(void) {
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    long n;
    char *s;
    int ok;
    if (!f || fseek(f, 0, SEEK_END) || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) ||
        !(s = malloc((size_t)n + 1U)) || fread(s, 1, (size_t)n, f) != (size_t)n) return 1;
    s[n] = '\0'; fclose(f);
    ok = strstr(s, "if (actionAsset && spellAsset && actionAsset->loaded") &&
         strstr(s, "Missing real data leaves this source-owned strip black.") &&
         strstr(s, "if (!drewAuthenticFrames && !m11_v1_chrome_mode_enabled())");
    free(s); return ok ? 0 : 1;
}
