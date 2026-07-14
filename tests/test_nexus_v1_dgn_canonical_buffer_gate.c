#include "nexus_v1_engine.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    uint8_t bytes[65];
    int i;
    static const struct {
        int size;
        const char *md5;
    } fixtures[] = {
        {0, "d41d8cd98f00b204e9800998ecf8427e"},
        {55, "6912ee65fff2d9f9ce2508cddf8bcda0"},
        {56, "51fdd1acda72405dfdfa03fcb85896d7"},
        {63, "48a6295221902e8e0938f773a7185e72"},
        {64, "b2d3f56bc197fd985d5965079b5e7148"},
        {65, "8bd7053801c768420faf816fadba971c"}
    };

    for (i = 0; i < (int)sizeof(bytes); ++i) {
        bytes[i] = (uint8_t)i;
    }
    for (i = 0; i < (int)(sizeof(fixtures) / sizeof(fixtures[0])); ++i) {
        expect(nexus_v1_dgn_bytes_match_canonical_md5(
                   bytes, fixtures[i].size, fixtures[i].md5),
               "exact DGN buffer accepts its canonical MD5 at every padding boundary");
    }

    expect(nexus_v1_dgn_bytes_match_canonical_md5(
               bytes, 65, "8BD7053801C768420FAF816FADBA971C"),
           "canonical DGN catalog spelling is case-insensitive");

    ++bytes[64];
    expect(!nexus_v1_dgn_bytes_match_canonical_md5(
               bytes, 65, "8bd7053801c768420faf816fadba971c"),
           "a one-byte reopened DGN mismatch stays blocked");
    --bytes[64];
    expect(!nexus_v1_dgn_bytes_match_canonical_md5(
               bytes, 64, "8bd7053801c768420faf816fadba971c"),
           "a truncated reopened DGN buffer stays blocked");
    expect(!nexus_v1_dgn_bytes_match_canonical_md5(
               bytes, -1, "d41d8cd98f00b204e9800998ecf8427e"),
           "invalid DGN lengths never match a canonical identity");

    if (failures) {
        fprintf(stderr, "Nexus DGN canonical-buffer gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN canonical-buffer gate passed");
    return 0;
}
