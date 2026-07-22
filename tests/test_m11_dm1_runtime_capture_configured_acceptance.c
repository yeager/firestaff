/* Source-level acceptance lock for M11's all-route real-data capture gate. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int has_between(const char* begin, const char* end, const char* text)
{
    size_t length;
    const char* cursor;
    if (!begin || !end || end < begin || !text) return 0;
    length = strlen(text);
    for (cursor = begin; cursor + length <= end; ++cursor) {
        if (memcmp(cursor, text, length) == 0) return 1;
    }
    return 0;
}

int main(void)
{
    FILE* file = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char* source;
    char* route;
    char* hash;
    char* publish;
    long size;
    int ok;

    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0 ||
        !(source = (char*)malloc((size_t)size + 1u)) ||
        fread(source, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        return 1;
    }
    fclose(file);
    source[size] = '\0';
    route = strstr(source, "static void m11_dm1_runtime_capture_route_evidence(");
    hash = strstr(source, "static unsigned int m11_dm1_runtime_capture_fnv1a(");
    publish = strstr(source, "static void m11_dm1_runtime_capture_publish(");
    ok = route && hash && publish && route < hash && hash < publish &&
         has_between(route, hash,
             "accepted && sourceOwned && suppressSyntheticFallback") &&
         has_between(route, hash, "sourceTick != 0u && materialFNV1a != 0u") &&
         has_between(route, hash, "routes->acceptedRoutes |= route") &&
         has_between(route, hash, "sourceTick < bridge->lastSourceTick[index]") &&
         has_between(publish, source + size,
             "routes->requiredRoutes != routes->acceptedRoutes") &&
         has_between(publish, source + size,
             "!routes->evidence[index].sourceOwned") &&
         has_between(publish, source + size,
             "!routes->evidence[index].suppressSyntheticFallback") &&
         has_between(publish, source + size,
             "receipt->routeMask = routes->acceptedRoutes") &&
         has_between(source, route, "M11_DM1_RUNTIME_CAPTURE_C13") &&
         has_between(source, route, "M11_DM1_RUNTIME_CAPTURE_HOC") &&
         has_between(source, route, "M11_DM1_RUNTIME_CAPTURE_TOP_ROW") &&
         has_between(source, route, "M11_DM1_RUNTIME_CAPTURE_ACTION_SPELL");
    free(source);
    return ok ? 0 : 1;
}
