#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

/* Skip-safe real-media probe. The parser exposes only existing Structure1B
 * material selectors; opaque DGN spans are never examined here. */
int main(int argc, char **argv) {
    Nexus_V1_Engine engine;
    Nexus_V1_DgnMaterialCorpusReceipt receipt;
    const char *root = argc > 1 ? argv[1] : NULL;

    memset(&engine, 0, sizeof(engine));
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_init(&engine, root) < 0) {
        puts("SKIP: Nexus Track 1 data unavailable");
        return 0;
    }
    (void)nexus_v1_inspect_dgn_material_corpus(&engine, &receipt);
    printf("LEV corpus: readable=%d parsed=%d geometry=%d expected=%d\n",
           receipt.readable_level_count, receipt.parsed_level_count,
           receipt.geometry_ready_level_count, receipt.expected_level_count);
    printf("Material refs: floor=%u ceiling=%u wall=%u\n",
           receipt.floor_coverage.command_count,
           receipt.ceiling_coverage.command_count,
           receipt.wall_coverage.command_count);
    printf("Coverage: floor=%d ceiling=%d wall=%d bpk-host=%d complete=%d\n",
           receipt.floor_coverage.covered, receipt.ceiling_coverage.covered,
           receipt.wall_coverage.covered, receipt.bpk_host_routes_complete,
           receipt.host_route_evidence_complete);
    printf("Containers: floors present=%d format=%d identity=%d host=%d; "
           "walls present=%d format=%d identity=%d host=%d\n",
           receipt.floor_container.source_present,
           receipt.floor_container.format_valid,
           receipt.floor_container.identity_verified,
           receipt.floor_container.host_route_permitted,
           receipt.wall_container.source_present,
           receipt.wall_container.format_valid,
           receipt.wall_container.identity_verified,
           receipt.wall_container.host_route_permitted);
    nexus_v1_shutdown(&engine);
    return receipt.parsed_level_count == receipt.expected_level_count &&
           receipt.geometry_ready_level_count == receipt.expected_level_count
        ? 0 : 1;
}
