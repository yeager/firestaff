#include "theron_v1_track02_dungeon_loader.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_door.h"
#include "theron_v1_track02_creature_names.h"
#include "theron_v1_track02_creature_spawn.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_boot.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_world.h"
#include "menu_input_m12.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2352
#define UD_PER_SECTOR 2048
#define SYNC_OFFSET 16

static uint8_t *load_track02_ud(const char *path, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsize <= 0) { fclose(fp); return NULL; }
    uint8_t *raw = malloc((size_t)fsize);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, (size_t)fsize, fp);
    fclose(fp);
    size_t sectors = (size_t)fsize / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }
    for (size_t s = 0; s < sectors; s++)
        memcpy(ud + s * UD_PER_SECTOR, raw + s * SECTOR_SIZE + SYNC_OFFSET, UD_PER_SECTOR);
    free(raw);
    *out_size = ud_size;
    return ud;
}

static uint8_t *load_raw_bytes(const char *path, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *raw;

    if (!path || !out_size) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0 ||
        (file_size = ftell(fp)) <= 0 ||
        fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)file_size);
    if (!raw || fread(raw, 1u, (size_t)file_size, fp) != (size_t)file_size) {
        free(raw);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (size_t)file_size;
    return raw;
}

static void bind_real_track02_party(
    Theron_V1_World *world,
    const uint8_t *track02,
    size_t track02_size,
    const char *md5_hex) {
    Theron_Track02StartupRosterNameCatalog catalog;
    Theron_DungeonProgression progression;
    Theron_StartupFlow flow;
    const char *names[THERON_TRACK02_MAX_STARTUP_ROSTER_NAMES] = {0};

    assert(world != NULL && track02 != NULL && md5_hex != NULL);
    memset(&catalog, 0, sizeof(catalog));
    assert(theron_v1_track02_catalog_startup_roster_names(
               track02, track02_size, md5_hex, &catalog) ==
           THERON_TRACK02_SIGNAL_OK);
    assert(catalog.name_count == THERON_TRACK02_MAX_STARTUP_ROSTER_NAMES);
    for (size_t i = 0u; i < catalog.name_count; ++i)
        names[i] = catalog.names[i].name;

    /* TAKE owns an inventory transition and therefore requires a live
     * champion.  Bind Theron's regional source record through the same
     * authenticated Track 02 roster route as production; never seed a test
     * champion into this real-data regression.  Theron persists between
     * dungeon banks, so the unlocked Akutuba entry is the correct source
     * handoff even while the loop inspects later banks independently. */
    theron_v1_startup_flow_init(&flow);
    theron_v1_dungeon_progression_init(&progression);
    assert(theron_v1_startup_choose_stage(
               &flow, &progression, THERON_DUNGEON_1_AKUTUBA) ==
           THERON_STARTUP_OK);
    assert(theron_v1_startup_enter_forcefield_with_track02_roster(
               &flow, &world->party, track02, track02_size, md5_hex,
               names, (int)catalog.name_count) == THERON_STARTUP_OK);
    assert(world->party.champion_count >= 1);
    assert(world->party.active_slot == THERON_CHAMPION_SLOT_THERON);
    assert(world->party.champions[THERON_CHAMPION_SLOT_THERON].health > 0);
}

static void test_real_item_name_sources(
    uint8_t *ud, size_t ud_size, int variant) {
    static const size_t us_offsets[THERON_DUNGEON_COUNT] = {
        0x099517u, 0x0d9b32u, 0x11a22bu, 0x159a71u,
        0x19a397u, 0x1d9737u, 0x21a08eu
    };
    static const size_t jp_offsets[THERON_DUNGEON_COUNT] = {
        0x098d77u, 0x0d938au, 0x119a97u, 0x1592b1u,
        0x199bfbu, 0x1d8f7fu, 0x21988eu
    };
    static const uint8_t counts[THERON_DUNGEON_COUNT] = {
        80u, 65u, 69u, 69u, 67u, 63u, 66u
    };
    static const size_t us_property_offsets[THERON_DUNGEON_COUNT] = {
        0x099825u, 0x0d9dc5u, 0x11a4d4u, 0x159d1du,
        0x19a64eu, 0x1d999fu, 0x21a32du
    };
    static const size_t jp_property_offsets[THERON_DUNGEON_COUNT] = {
        0x0990a2u, 0x0d9616u, 0x119d4du, 0x15955du,
        0x199eb1u, 0x1d91d9u, 0x219b13u
    };
    static const uint8_t jp_compass[8] = {
        0x83u, 0x52u, 0x83u, 0x93u,
        0x83u, 0x70u, 0x83u, 0x58u
    };
    static const char *const us_quest_names[THERON_DUNGEON_COUNT] = {
        "SHIELD DEFIANT", "TAZA BOOTS", "TAZA POLEYN", "SOUL CAGE",
        "TAZA ARMOR", "TAZAHELM", "THE RETALIATOR"
    };
    static const uint8_t jp_quest_names[THERON_DUNGEON_COUNT][20] = {
        {0x83u,0x66u,0x83u,0x74u,0x83u,0x42u,0x83u,0x41u,0x83u,0x93u,
         0x83u,0x67u,0x83u,0x56u,0x81u,0x5bu,0x83u,0x8bu,0x83u,0x68u},
        {0x83u,0x5eu,0x83u,0x55u,0x83u,0x75u,0x81u,0x5bu,0x83u,0x63u},
        {0x83u,0x5eu,0x83u,0x55u,0x83u,0x4fu,0x83u,0x8au,0x81u,0x5bu,
         0x83u,0x75u},
        {0x83u,0x5cu,0x83u,0x45u,0x83u,0x8bu,0x83u,0x50u,0x81u,0x5bu,
         0x83u,0x57u},
        {0x83u,0x5eu,0x83u,0x55u,0x83u,0x41u,0x81u,0x5bu,0x83u,0x7du,
         0x81u,0x5bu},
        {0x83u,0x5eu,0x83u,0x55u,0x83u,0x77u,0x83u,0x8bu,0x83u,0x81u,
         0x83u,0x62u,0x83u,0x67u},
        {0x95u,0x9cu,0x8fu,0x51u,0x82u,0xccu,0x8cu,0x95u}
    };
    static const uint8_t jp_quest_name_sizes[THERON_DUNGEON_COUNT] = {
        20u, 10u, 12u, 12u, 12u, 14u, 8u
    };
    const size_t *offsets = variant == 1 ? jp_offsets : us_offsets;
    const size_t *property_offsets =
        variant == 1 ? jp_property_offsets : us_property_offsets;
    Theron_V1_World *world = calloc(1u, sizeof(*world));
    unsigned int dungeon;
    assert(world != NULL);
    theron_v1_world_init(world);
    {
        Theron_Track02RetrievalTextSource retrieval;
        Theron_Track02CampaignMaskSource campaign_mask;
        const uint8_t *message = NULL;
        size_t message_size = 0u;
        assert(theron_v1_track02_decode_retrieval_text_source(
                   ud, ud_size, variant, &retrieval) == 1);
        assert(theron_v1_world_bind_track02_retrieval_text_source(
                   world, &retrieval, variant) == 1);
        assert(theron_v1_world_retrieval_text_record_raw(
                   world, 0u, &message, &message_size) == 1);
        assert(message_size == retrieval.raw_message_sizes[0]);
        assert(memcmp(message, retrieval.raw_messages[0], message_size) == 0);
        assert(theron_v1_world_retrieval_text_record_raw(
                   world, THERON_TRACK02_RETRIEVAL_TEXT_COUNT,
                   &message, &message_size) == 0);
        {
            Theron_Track02RetrievalTextSource forged = retrieval;
            forged.resource_offset += 2048u;
            assert(theron_v1_world_bind_track02_retrieval_text_source(
                       world, &forged, variant) == 0);
            forged = retrieval;
            forged.track02_resource_block += 1u;
            assert(theron_v1_world_bind_track02_retrieval_text_source(
                       world, &forged, variant) == 0);
            forged = retrieval;
            forged.post_dungeon_text_selector_fnv1a ^= 1u;
            assert(theron_v1_world_bind_track02_retrieval_text_source(
                       world, &forged, variant) == 0);
        }
        retrieval.retrieval_event_relation_proven = 0;
        assert(theron_v1_world_bind_track02_retrieval_text_source(
                   world, &retrieval, variant) == 0);
        assert(theron_v1_world_retrieval_text_record_raw(
                   world, 0u, &message, &message_size) == 0);
        retrieval.retrieval_event_relation_proven = 1;
        assert(theron_v1_world_bind_track02_retrieval_text_source(
                   world, &retrieval, variant) == 1);
        assert(theron_v1_track02_decode_campaign_mask_source(
                   ud, ud_size, variant, &campaign_mask) == 1);
        assert(theron_v1_world_bind_track02_campaign_mask_source(
                   world, &campaign_mask, variant) == 1);
        assert(world->track02_campaign_mask.runtime_address == 0x267cu);
        assert(world->track02_campaign_mask.campaign_bits_mask == 0x7fu);
        assert(world->track02_campaign_mask
                   .artifact_collection_relation_proven == 1);
        {
            uint8_t artifact_mask = 0u;
            Theron_DungeonProgression saved_progression = world->progression;
            assert(theron_v1_world_campaign_artifact_mask(
                       world, 0xd5u, &artifact_mask) == 1);
            assert(artifact_mask == 0x55u);
            world->progression.current_dungeon = THERON_DUNGEON_5_SHADO;
            world->progression.current_level = 2u;
            world->progression.dungeon_playtime_seconds = 321u;
            assert(theron_v1_world_apply_campaign_artifact_byte(
                       world, 0xd5u) == 1);
            assert(world->progression.quest_items_collected == 0x55u);
            assert(world->progression.quest_complete == 0u);
            assert(world->progression.current_dungeon ==
                   THERON_DUNGEON_5_SHADO);
            assert(world->progression.current_level == 2u);
            assert(world->progression.dungeon_playtime_seconds == 321u);
            assert(world->progression.dungeon_states[0] ==
                   THERON_DUNGEON_STATE_COMPLETE);
            assert(world->progression.dungeon_states[4] ==
                   THERON_DUNGEON_STATE_COMPLETE);
            assert(world->dungeon_complete == 1);
            world->progression = saved_progression;
            world->dungeon_complete = 0;
        }
        {
            Theron_Track02CampaignMaskSource forged = campaign_mask;
            forged.post_dungeon_cd_base_track_bcd = 0x02u;
            assert(theron_v1_world_bind_track02_campaign_mask_source(
                       world, &forged, variant) == 0);
            forged = campaign_mask;
            forged.descriptor_loader_fnv1a ^= 1u;
            assert(theron_v1_world_bind_track02_campaign_mask_source(
                       world, &forged, variant) == 0);
            forged = campaign_mask;
            forged.post_dungeon_record_stride = 8u;
            assert(theron_v1_world_bind_track02_campaign_mask_source(
                       world, &forged, variant) == 0);
            forged = campaign_mask;
            forged.post_dungeon_shared_record = 0x040du;
            assert(theron_v1_world_bind_track02_campaign_mask_source(
                       world, &forged, variant) == 0);
        }
        campaign_mask.artifact_collection_relation_proven = 1;
        assert(theron_v1_world_bind_track02_campaign_mask_source(
                   world, &campaign_mask, variant) == 0);
        assert(world->track02_campaign_mask.valid == 0);
        campaign_mask.artifact_collection_relation_proven = 0;
        assert(theron_v1_world_bind_track02_campaign_mask_source(
                   world, &campaign_mask, variant) == 1);
        assert(world->track02_campaign_mask
                   .artifact_collection_relation_proven == 1);
    }
    for (dungeon = 1u; dungeon <= THERON_DUNGEON_COUNT; ++dungeon) {
        Theron_Track02ItemNameSource source;
        const uint8_t *name = NULL;
        size_t name_size = 0u;
        uint8_t saved;
        assert(theron_v1_track02_decode_item_name_source(
                   ud, ud_size, variant, dungeon, &source) == 1);
        assert(source.valid == 1);
        assert(source.variant == variant);
        assert(source.dungeon_id == dungeon);
        assert(source.count == counts[dungeon - 1u]);
        assert(source.source_offset == offsets[dungeon - 1u]);
        assert(source.type_code_source_offset ==
               offsets[dungeon - 1u] - counts[dungeon - 1u] - 6u);
        assert(source.type_code_source_fnv1a != 0u);
        assert(source.property_source_offset ==
               property_offsets[dungeon - 1u]);
        assert(source.property_source_fnv1a ==
               ((variant == 1 && dungeon == 2u) ?
                    0x6c4d1386u : 0xb97787efu));
        assert(source.object_item_index_relation_proven == 1);
        assert(source.host_text_rendering_proven == 0);
        assert(theron_v1_world_bind_track02_item_name_source(
                   world, &source, variant) == 1);
        assert(theron_v1_world_track02_item_name_raw(
                   world, dungeon, 0u, &name, &name_size) == 1);
        if (variant == 1) {
            assert(name_size == sizeof(jp_compass));
            assert(memcmp(name, jp_compass, sizeof(jp_compass)) == 0);
        } else {
            assert(name_size == 7u);
            assert(memcmp(name, "COMPASS", 7u) == 0);
        }
        assert(theron_v1_track02_quest_item_name_raw(
                   &source, &name, &name_size) == 1);
        assert(theron_v1_world_quest_item_name_raw(
                   world, dungeon - 1u, &name, &name_size) == 1);
        if (variant == 1) {
            assert(name_size == jp_quest_name_sizes[dungeon - 1u]);
            assert(memcmp(name, jp_quest_names[dungeon - 1u], name_size) == 0);
        } else {
            assert(name_size == strlen(us_quest_names[dungeon - 1u]));
            assert(memcmp(name, us_quest_names[dungeon - 1u], name_size) == 0);
        }
        /* Dungeon 6 has only 63 source entries.  Do not consume the first
         * property-table zero as a fabricated empty name. */
        if (dungeon == 6u) {
            assert(theron_v1_world_track02_item_name_raw(
                       world, dungeon, 64u, &name, &name_size) == 0);
        }
        saved = ud[offsets[dungeon - 1u]];
        ud[offsets[dungeon - 1u]] ^= 1u;
        assert(theron_v1_track02_decode_item_name_source(
                   ud, ud_size, variant, dungeon, &source) == 0);
        ud[offsets[dungeon - 1u]] = saved;
        saved = ud[offsets[dungeon - 1u] - counts[dungeon - 1u] - 6u];
        ud[offsets[dungeon - 1u] - counts[dungeon - 1u] - 6u] ^= 1u;
        assert(theron_v1_track02_decode_item_name_source(
                   ud, ud_size, variant, dungeon, &source) == 0);
        ud[offsets[dungeon - 1u] - counts[dungeon - 1u] - 6u] = saved;
        saved = ud[property_offsets[dungeon - 1u]];
        ud[property_offsets[dungeon - 1u]] ^= 1u;
        assert(theron_v1_track02_decode_item_name_source(
                   ud, ud_size, variant, dungeon, &source) == 0);
        ud[property_offsets[dungeon - 1u]] = saved;
    }
    assert(theron_v1_world_track02_item_name_raw(
               world, 0u, 0u, NULL, NULL) == 0);
    assert(theron_v1_world_quest_item_name_raw(
               world, THERON_DUNGEON_COUNT, NULL, NULL) == 0);
    free(world);
    printf("  authentic %s dungeon-local item name/type-code tables bind losslessly\n",
           variant == 1 ? "JP" : "US");
}

static void test_real_sarmon_track19_mapping(
        const uint8_t *ud, size_t ud_size, int variant) {
    const char *home = getenv("HOME");
    char track19_path[1024];
    Theron_Track02ItemNameSource track02;
    Theron_V1Track19ItemNameBank track19;
    Theron_V1_World *world;
    const Theron_V1_Object *object = NULL;
    Theron_DungeonLoadResult result;
    const uint8_t *name = NULL;
    size_t name_size = 0u;

    if (!home) return;
    snprintf(track19_path, sizeof(track19_path),
             "%s/.firestaff/data/theron/%s", home,
             variant == 1 ? "TQJP19.iso" : "TQUS19.iso");
    if (!theron_v1_track19_item_name_bank_file(track19_path, &track19)) return;
    assert(theron_v1_track02_decode_item_name_source(
               ud, ud_size, variant, 4u, &track02) == 1);
    world = (Theron_V1_World *)calloc(1u, sizeof(*world));
    assert(world != NULL);
    theron_v1_world_init(world);
    assert(theron_v1_world_bind_track19_item_name_bank(
               world, &track19, variant) == 1);
    assert(world->track19_item_names.item_mapping_proven == 0);
    assert(theron_v1_world_bind_track02_item_name_source(
               world, &track02, variant) == 1);
    assert(world->track19_item_names.item_mapping_proven == 1);
    assert(world->track19_item_names.mapped_track02_dungeon_mask == (1u << 3));
    world->current_dungeon = 4;
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 4, ud, ud_size, variant, &result) == 0);
    for (int i = 0; i < world->object_count; ++i) {
        if (world->objects[i].source_origin_valid &&
            world->objects[i].source_dungeon == 4u &&
            world->objects[i].source_property_valid) {
            object = &world->objects[i];
            break;
        }
    }
    assert(object != NULL);
    assert(theron_v1_world_object_track19_item_name_raw(
               world, object, &name, &name_size) == 1);
    assert(name != NULL && name_size > 0u);
    {
        Theron_V1_Object wrong_dungeon = *object;
        wrong_dungeon.source_dungeon = 3u;
    assert(theron_v1_world_object_track19_item_name_raw(
               world, &wrong_dungeon, &name, &name_size) == 0);
    }
    track19.raw_properties[0][0] ^= 1u;
    assert(theron_v1_world_bind_track19_item_name_bank(
               world, &track19, variant) == 0);
    free(world);
}

static void test_authenticated_world_spawn_binding(
    const char *us_path, const char *jp_path) {
    size_t raw_size = 0u;
    uint8_t *raw = load_raw_bytes(us_path, &raw_size);
    Theron_Track02SpawnSource source;
    Theron_V1_World world;

    assert(raw != NULL);
    memset(&source, 0, sizeof(source));
    assert(theron_v1_track02_decode_spawn_source(
               raw, raw_size, THERON_V1_TRACK02_VARIANT_US_BIN,
               &source) == 1);
    theron_v1_world_init(&world);
    assert(theron_v1_world_bind_track02_spawn_source(
               &world, &source, THERON_V1_TRACK02_VARIANT_US_BIN) == 1);
    assert(world.track02_spawn_source.authenticated == 1);
    assert(world.track02_spawn_source.variant ==
           THERON_V1_TRACK02_VARIANT_US_BIN);
    for (unsigned int i = 0u; i < THERON_TRACK02_SPAWN_ZONE_COUNT; ++i) {
        const Theron_SpawnZoneDesc *zone = theron_v1_track02_spawn_zone(i);
        assert(zone != NULL);
        assert(world.track02_spawn_source.zones[i].map_width == zone->map_width);
        assert(world.track02_spawn_source.zones[i].map_height == zone->map_height);
        assert(theron_v1_world_track02_spawn_category(&world, i) ==
               world.track02_spawn_source.zones[i].category);
    }
    assert(theron_v1_world_track02_spawn_category(
               &world, THERON_TRACK02_SPAWN_ZONE_COUNT) == 0xffu);
    free(raw);

    if (jp_path) {
        raw = load_raw_bytes(jp_path, &raw_size);
        assert(raw != NULL);
        memset(&source, 0, sizeof(source));
        assert(theron_v1_track02_decode_spawn_source(
                   raw, raw_size, THERON_V1_TRACK02_VARIANT_JP_BIN,
                   &source) == 1);
        theron_v1_world_init(&world);
        assert(theron_v1_world_bind_track02_spawn_source(
                   &world, &source,
                   THERON_V1_TRACK02_VARIANT_JP_BIN) == 1);
        assert(world.track02_spawn_source.authenticated == 1);
        assert(world.track02_spawn_source_variant ==
               THERON_V1_TRACK02_VARIANT_JP_BIN);
        /* JP source bytes are now retained, but category semantics remain
         * closed until the JP runtime consumer is authenticated. */
        assert(theron_v1_world_track02_spawn_category(&world, 0u) == 0xffu);
        free(raw);
    }
    printf("  authenticated Track 02 spawn source reaches world binding\n");
}

static const char *find_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    const char *candidates[3] = { explicit_path, NULL, NULL };
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin",
                 home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                 home);
        candidates[2] = path + 256;
    }
    for (unsigned int i = 0; i < 3u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static const char *find_jp_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_JP_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    if (explicit_path && explicit_path[0]) return explicit_path;
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQJP02.bin",
             home);
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fclose(fp);
    return path;
}

static void assert_source_category_census(
    const Theron_DungeonLoadResult *result) {
    unsigned int total = 0;
    for (unsigned int category = 0; category < THERON_ITEM_CATEGORY_COUNT;
         ++category) {
        total += result->source_category_counts[category];
        if ((category > THERON_CAT_MISC && category < THERON_CAT_MISSILE) ||
            category > THERON_CAT_CLOUD)
            assert(result->source_category_counts[category] == 0);
    }
    assert(total == result->source_object_count);
}

static void assert_source_type_census(
    const Theron_DungeonLoadResult *result) {
    uint32_t expected[THERON_ITEM_CATEGORY_COUNT][8] = {{0}};
    for (unsigned int i = 0; i < result->source_object_count; ++i) {
        const Theron_Track02SourceObjectOccurrence *occ =
            &result->source_objects[i];
        assert(occ->source_index < THERON_MAX_ITEMS_PER_CAT);
        unsigned int type_value = 0;
        int has_type = 1;
        switch (occ->category) {
        case THERON_CAT_MONSTER:
            type_value = occ->decoded.value.monster.type;
            break;
        case THERON_CAT_WEAPON:
            type_value = occ->decoded.value.weapon.type;
            break;
        case THERON_CAT_CLOTHING:
            type_value = occ->decoded.value.clothing.type;
            break;
        case THERON_CAT_SCROLL:
            type_value = occ->decoded.value.scroll.type;
            break;
        case THERON_CAT_POTION:
            type_value = occ->decoded.value.potion.type;
            break;
        case THERON_CAT_MISC:
            type_value = occ->decoded.value.misc.type;
            break;
        default:
            has_type = 0;
            break;
        }
        if (has_type)
            expected[occ->category][type_value >> 5] |=
                1u << (type_value & 31u);
    }
    assert(memcmp(expected, result->source_type_value_mask,
                  sizeof(expected)) == 0);
}

static unsigned int expected_live_monsters(const Theron_V1_World *world) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id != world->current_dungeon ||
            record->level != world->current_level) continue;
        unsigned int members = (unsigned int)record->number + 1u;
        if (members > 4u) members = 4u;
        for (unsigned int slot = 0; slot < members; ++slot)
            if (record->health[slot] != 0u) ++count;
    }
    return count;
}

static void assert_real_monster_ledger_roundtrip(
    const Theron_V1_World *source_world) {
    Theron_V1_World *restored;
    uint8_t *bytes;
    size_t size;
    int later_level = -1;

    assert(source_world != NULL);
    assert(source_world->source_monster_count > 0u);
    size = theron_v1_world_serialize_size(source_world);
    assert(size > 0u);
    bytes = (uint8_t *)malloc(size);
    restored = (Theron_V1_World *)malloc(sizeof(*restored));
    assert(bytes != NULL && restored != NULL);
    assert(theron_v1_world_serialize(source_world, bytes, size) == size);

    /* Keep the already authenticated level/media envelope, but erase the
     * category-4 ledger so success cannot come from the destination world. */
    *restored = *source_world;
    restored->source_monster_count = 0u;
    memset(restored->source_monsters, 0, sizeof(restored->source_monsters));
    assert(theron_v1_world_deserialize(restored, bytes, size) == 0);
    assert(restored->source_monster_count ==
           source_world->source_monster_count);
    for (unsigned int i = 0; i < source_world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *left =
            &source_world->source_monsters[i];
        const Theron_V1_SourceMonsterRecord *right =
            &restored->source_monsters[i];
        assert(right->dungeon_id == left->dungeon_id);
        assert(right->level == left->level);
        assert(right->x == left->x && right->y == left->y);
        assert(right->source_ref == left->source_ref);
        assert(right->source_index == left->source_index);
        assert(right->chested == left->chested);
        assert(right->type == left->type);
        assert(right->position == left->position);
        assert(right->number == left->number);
        assert(right->direction_flags == left->direction_flags);
        assert(right->flags_word == left->flags_word);
        assert(right->unknown_word == left->unknown_word);
        assert(memcmp(right->health, left->health,
                      sizeof(left->health)) == 0);
        assert(right->raw_size == left->raw_size);
        assert(memcmp(right->raw, left->raw, sizeof(left->raw)) == 0);
        if (left->level != source_world->current_level)
            later_level = left->level;
    }

    /* A Continue must be able to rebuild another real level from the saved
     * ledger, not merely retain the creatures that were live at save time. */
    assert(later_level >= 0);
    restored->current_level = later_level;
    restored->creature_count = 0;
    memset(restored->creatures, 0, sizeof(restored->creatures));
    assert(theron_v1_world_spawn_level_creatures(restored) == 0);
    assert((unsigned int)restored->creature_count ==
           expected_live_monsters(restored));
    assert(restored->creature_count > 0);

    free(restored);
    free(bytes);
}

static int assert_real_item_roundtrip(Theron_V1_World *world) {
    int found = 0;
    int selected_nonfirst = 0;
    int original_level = world->current_level;
    int initial_objects;

    /* Exercise one object from the loaded level, not a hand-built fixture.
     * T900 ownership remains source-gated: the item must carry its real
     * category, source reference, item type and verified 6-byte property row
     * through TAKE and DROP. */
    for (int i = 0; i < world->object_count; ++i) {
        Theron_V1_Object *object = &world->objects[i];
        int inventory_slot = -1;
        int earlier_carryable = 0;
        int facing = -1;
        uint16_t source_ref;
        uint16_t source_next_ref;
        uint16_t source_index;
        uint8_t source_category;
        uint8_t source_position;
        uint8_t source_dungeon;
        uint8_t source_level;
        uint8_t source_x;
        uint8_t source_y;
        uint8_t source_raw_size;
        uint8_t source_raw[16];
        Theron_V1_BootRuntimeInputReceipt input_receipt;
        if (object->level != world->current_level ||
            !object->source_ref || !object->source_property_valid ||
            (object->source_category != THERON_CAT_WEAPON &&
             object->source_category != THERON_CAT_CLOTHING &&
             object->source_category != THERON_CAT_SCROLL &&
             object->source_category != THERON_CAT_POTION &&
             object->source_category != THERON_CAT_MISC)) {
            continue;
        }
        for (int j = 0; j < i; ++j) {
            const Theron_V1_Object *earlier = &world->objects[j];
            if (earlier->dungeon_id == object->dungeon_id &&
                earlier->level == object->level &&
                earlier->x == object->x && earlier->y == object->y &&
                !(earlier->flags & THERON_OBJ_F_PICKED_UP) &&
                (earlier->source_category == THERON_CAT_WEAPON ||
                 earlier->source_category == THERON_CAT_CLOTHING ||
                 earlier->source_category == THERON_CAT_SCROLL ||
                 earlier->source_category == THERON_CAT_POTION ||
                 earlier->source_category == THERON_CAT_MISC)) {
                earlier_carryable = 1;
                break;
            }
        }
        if (earlier_carryable) continue;
        for (int dir = 0; dir < THERON_DIR_COUNT; ++dir) {
            int px = object->x - g_theron_dir_dx[dir];
            int py = object->y - g_theron_dir_dy[dir];
            const Theron_V1_Level *level =
                &world->levels[world->current_dungeon - 1][object->level];
            if (px >= 0 && py >= 0 && px < level->width && py < level->height &&
                level->squares[py][px] != THERON_SQUARE_WALL) {
                theron_v1_party_place(world, px, py, dir);
                facing = dir;
                break;
            }
        }
        if (facing < 0) continue;
        /* Prefer a real occurrence hidden behind another source record.  It
         * is the regression case for TAKE's category-aware chain walk. */
        if (theron_v1_object_at_in_dungeon(
                world, world->current_dungeon, world->current_level,
                object->x, object->y) == object && selected_nonfirst == 0)
            continue;
        selected_nonfirst = 1;
        world->current_level = object->level;
        initial_objects = world->object_count;
        source_ref = object->source_ref;
        source_next_ref = object->source_next_ref;
        source_index = object->source_index;
        source_category = object->source_category;
        source_position = object->source_position;
        assert(object->source_origin_valid);
        source_dungeon = object->source_dungeon;
        source_level = object->source_level;
        source_x = object->source_x;
        source_y = object->source_y;
        source_raw_size = object->source_raw_size;
        memcpy(source_raw, object->source_raw, sizeof(source_raw));
        assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                   world, NULL, M12_MENU_INPUT_PICKUP_ITEM, -1,
                   &input_receipt) == 1);
        assert(input_receipt.picked_up == 1);
        inventory_slot = input_receipt.inventory_slot;
        assert(inventory_slot >= 0);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_ref == source_ref);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_position == source_position);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_origin_valid);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_dungeon == source_dungeon);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_level == source_level);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_x == source_x);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_y == source_y);
        assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                   world, NULL, M12_MENU_INPUT_INVENTORY_TOGGLE, -1,
                   &input_receipt) == 1);
        assert(input_receipt.inventory_selected == 1);
        assert(input_receipt.inventory_slot == inventory_slot);
        inventory_slot = input_receipt.inventory_slot;
        {
            size_t save_size = theron_v1_world_serialize_size(world);
            uint8_t *save_bytes = malloc(save_size);
            Theron_V1_World *restored = malloc(sizeof(*restored));
            Theron_V1_BootRuntimeInputReceipt restored_receipt;
            assert(save_bytes != NULL && restored != NULL);
            /* Resume starts from the same authenticated, already loaded
             * Track 02 bank, then applies the saved mutable world. */
            memcpy(restored, world, sizeof(*restored));
            assert(theron_v1_world_serialize(world, save_bytes, save_size) ==
                   save_size);
            assert(theron_v1_world_deserialize(restored, save_bytes,
                                               save_size) == 0);
            assert(restored->inventory_source[restored->party.active_slot]
                       [inventory_slot].source_origin_valid);
            assert(restored->inventory_source[restored->party.active_slot]
                       [inventory_slot].source_dungeon == source_dungeon);
            assert(restored->inventory_source[restored->party.active_slot]
                       [inventory_slot].source_level == source_level);
            assert(restored->inventory_source[restored->party.active_slot]
                       [inventory_slot].source_x == source_x);
            assert(restored->inventory_source[restored->party.active_slot]
                       [inventory_slot].source_y == source_y);
            assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                       restored, NULL, M12_MENU_INPUT_INVENTORY_TOGGLE, -1,
                       &restored_receipt) == 1);
            assert(restored_receipt.inventory_selected == 1 &&
                   restored_receipt.inventory_slot == inventory_slot);
            assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                       restored, NULL, M12_MENU_INPUT_DROP_ITEM,
                       restored_receipt.inventory_slot,
                       &restored_receipt) == 1);
            assert(restored_receipt.dropped == 1);
            free(restored);
            free(save_bytes);
        }
        assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                   world, NULL, M12_MENU_INPUT_DROP_ITEM, inventory_slot,
                   &input_receipt) == 1);
        assert(input_receipt.dropped == 1 &&
               input_receipt.inventory_slot == inventory_slot);
        assert(world->object_count == initial_objects);
        assert(object->source_ref == source_ref);
        assert(object->source_next_ref == source_next_ref);
        assert(object->source_index == source_index);
        assert(object->source_category == source_category);
        assert(object->source_position == source_position);
        assert(object->source_origin_valid);
        assert(object->source_dungeon == source_dungeon);
        assert(object->source_level == source_level);
        assert(object->source_x == source_x);
        assert(object->source_y == source_y);
        assert(((object->flags >> THERON_OBJ_F_SOURCE_POSITION_SHIFT) & 3u) ==
               source_position);
        assert(object->source_raw_size == source_raw_size);
        assert(memcmp(object->source_raw, source_raw, sizeof(source_raw)) == 0);

        /* The dropped occurrence must remain source-backed on the next real
         * input cycle; otherwise DROP merely created a host-side lookalike. */
        facing = -1;
        for (int dir = 0; dir < THERON_DIR_COUNT; ++dir) {
            int px = object->x - g_theron_dir_dx[dir];
            int py = object->y - g_theron_dir_dy[dir];
            const Theron_V1_Level *level =
                &world->levels[world->current_dungeon - 1][object->level];
            if (px >= 0 && py >= 0 && px < level->width &&
                py < level->height &&
                level->squares[py][px] != THERON_SQUARE_WALL) {
                theron_v1_party_place(world, px, py, dir);
                facing = dir;
                break;
            }
        }
        assert(facing >= 0);
        assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                   world, NULL, M12_MENU_INPUT_PICKUP_ITEM, -1,
                   &input_receipt) == 1);
        assert(input_receipt.picked_up == 1);
        inventory_slot = input_receipt.inventory_slot;
        assert(inventory_slot >= 0);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_ref == source_ref);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_position == source_position);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_origin_valid);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_dungeon == source_dungeon);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_level == source_level);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_x == source_x);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_y == source_y);
        assert(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
                   world, NULL, M12_MENU_INPUT_DROP_ITEM, inventory_slot,
                   &input_receipt) == 1);
        assert(input_receipt.dropped == 1);
        assert(world->object_count == initial_objects);
        assert(object->source_ref == source_ref);
        assert(object->source_position == source_position);
        assert(object->source_origin_valid);
        assert(object->source_dungeon == source_dungeon);
        assert(object->source_level == source_level);
        assert(object->source_x == source_x);
        assert(object->source_y == source_y);
        assert(object->source_raw_size == source_raw_size);
        assert(memcmp(object->source_raw, source_raw, sizeof(source_raw)) == 0);
        found = 1;
        break;
    }
    world->current_level = original_level;
    assert(found);
    return selected_nonfirst;
}

static void assert_real_misc_roundtrip(Theron_V1_World *world) {
    int found = 0;
    int original_level = world->current_level;

    /* Category 10 is the newly admitted neutral source-item carrier.  Walk
     * every loaded level because MISC occurrences are not guaranteed on the
     * initial map. */
    for (int i = 0; i < world->object_count; ++i) {
        Theron_V1_Object *object = &world->objects[i];
        int inventory_slot = -1;
        int before;
        if (object->source_category != THERON_CAT_MISC ||
            !object->source_ref || !object->source_property_valid ||
            object->type != THERON_OBJTYPE_SOURCE_ITEM ||
            theron_v1_object_at_in_dungeon(
                world, world->current_dungeon, object->level,
                object->x, object->y) != object) {
            continue;
        }
        world->current_level = object->level;
        before = world->object_count;
        assert(theron_v1_click_route(world, object->x, object->y,
                                     THERON_CMD_TAKE) == 0);
        for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            if (world->party.champions[world->party.active_slot]
                    .inventory[slot] == object->source_item_type &&
                world->inventory_source[world->party.active_slot][slot]
                    .valid) {
                inventory_slot = slot;
                break;
            }
        }
        assert(inventory_slot >= 0);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].category == THERON_CAT_MISC);
        assert(theron_v1_drop_inventory_source_item(
                   world, world->party.active_slot, inventory_slot,
                   object->x, object->y) > 0);
        assert(world->object_count == before);
        assert(object->source_ref != 0u);
        assert(!(object->flags & THERON_OBJ_F_PICKED_UP));
        found = 1;
        break;
    }
    world->current_level = original_level;
    assert(found);
}

static void assert_real_chests_are_not_itemrecords(
    const Theron_V1_World *world, unsigned int expected_chests) {
    unsigned int chest_count = 0;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        if (object->source_category != THERON_CAT_CHEST) continue;
        ++chest_count;
        assert(object->type == THERON_OBJTYPE_CHEST);
        assert(object->source_property_valid == 0);
        assert(object->source_item_category == 0);
        assert(object->source_item_type == 0);
    }
    /* This is a real-data regression guard, not a synthetic chest fixture. */
    assert(chest_count == expected_chests);
}

static void assert_real_doors_preserve_record_without_runtime_aliases(
    const Theron_V1_World *world, unsigned int expected_doors) {
    unsigned int doors = 0u;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        const Theron_V1_SourceObjectRecord *source = NULL;
        Theron_Door door;
        if (object->type != THERON_OBJTYPE_DOOR) continue;
        for (unsigned int j = 0; j < world->source_object_count; ++j) {
            const Theron_V1_SourceObjectRecord *candidate =
                &world->source_objects[j];
            if (candidate->category == THERON_CAT_DOOR &&
                candidate->dungeon_id == object->dungeon_id &&
                candidate->level == object->level &&
                candidate->x == object->x && candidate->y == object->y &&
                candidate->position ==
                    ((object->flags &
                      THERON_OBJ_F_SOURCE_POSITION_MASK) >>
                     THERON_OBJ_F_SOURCE_POSITION_SHIFT)) {
                source = candidate;
                break;
            }
        }
        assert(source != NULL);
        assert(source->raw_size == 4u);
        assert(object->source_origin_valid == 1u);
        assert(object->source_dungeon == source->dungeon_id);
        assert(object->source_level == source->level);
        assert(object->source_x == source->x);
        assert(object->source_y == source->y);
        assert(object->source_ref == source->source_ref);
        assert(object->source_next_ref == source->next_ref);
        assert(object->source_index == source->source_index);
        assert(object->source_category == THERON_CAT_DOOR);
        assert(object->source_position == source->position);
        assert(object->source_raw_size == source->raw_size);
        assert(memcmp(object->source_raw, source->raw, source->raw_size) == 0);
        assert(theron_v1_track02_door_decode(source->raw, &door) == 0);
        assert(object->state == THERON_DOOR_STATE_CLOSED);
        assert((object->flags & (THERON_DOOR_F_LOCKED |
                                 THERON_DOOR_F_SECRET |
                                 THERON_DOOR_F_BROKEN |
                                 THERON_DOOR_F_AUTO_OPEN |
                                 THERON_DOOR_F_MANUAL)) == 0u);
        assert(((object->flags & THERON_OBJ_F_SOURCE_POSITION_MASK) >>
                THERON_OBJ_F_SOURCE_POSITION_SHIFT) == source->position);
        assert(!!(object->flags & THERON_OBJ_F_SOURCE_DOOR_IRON) ==
               !!door.type);
        assert(!!(object->flags & THERON_OBJ_F_SOURCE_DOOR_OPENS_UP) ==
               !!door.opens_up);
        assert(!!(object->flags & THERON_OBJ_F_SOURCE_DOOR_BUTTON) ==
               !!door.button);
        assert(!!(object->flags & THERON_OBJ_F_SOURCE_DOOR_DESTROYABLE) ==
               !!door.destroyable);
        assert(!!(object->flags & THERON_OBJ_F_SOURCE_DOOR_BASHABLE) ==
               !!door.bashable);
        assert(object->quantity == door.ornate);
        /* Category 0 does not create host geometry.  Its independently
         * decoded map byte must already identify the square as a door. */
        assert(world->levels[object->dungeon_id - 1][object->level]
                            .squares[object->y][object->x] ==
               THERON_SQUARE_DOOR);
        ++doors;
    }
    assert(doors == expected_doors);
}

static unsigned int assert_real_teleporters_preserve_map_state(
    const Theron_V1_World *world, unsigned int expected_teleporters,
    unsigned int *active_to_inactive) {
    unsigned int teleporters = 0u;
    unsigned int active = 0u;

    if (active_to_inactive) *active_to_inactive = 0u;
    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        Theron_Teleporter source;
        uint8_t tile;
        if (object->type != THERON_OBJTYPE_TELEPORTER) continue;
        assert(object->source_raw_size == 6u);
        assert(theron_v1_track02_teleporter_decode(
                   object->source_raw, &source) == 0);
        assert(object->source_origin_valid == 1u);
        tile = world->levels[object->dungeon_id - 1][object->level]
                    .source_tiles[object->y][object->x];
        assert(theron_tile_type(tile) == THERON_TILE_TELEPORTER);
        assert(object->state == ((tile & 0x08u) ? 1u : 0u));
        assert(((object->flags &
                 THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_MASK) >>
                THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_SHIFT) == source.scope);
        assert(world->levels[object->dungeon_id - 1][object->level]
                            .squares[object->y][object->x] ==
               THERON_SQUARE_TELEPORTER);
        if (object->state != 0u) {
            int target_level = (object->linked_id >> 10) & 0x3f;
            int target_y = (object->linked_id >> 5) & 0x1f;
            int target_x = object->linked_id & 0x1f;
            ++active;
            for (int j = 0; j < world->object_count; ++j) {
                const Theron_V1_Object *target = &world->objects[j];
                if (target->type == THERON_OBJTYPE_TELEPORTER &&
                    target->dungeon_id == object->dungeon_id &&
                    target->level == target_level && target->x == target_x &&
                    target->y == target_y && target->state == 0u &&
                    active_to_inactive)
                    ++*active_to_inactive;
            }
        }
        ++teleporters;
    }
    assert(teleporters == expected_teleporters);
    return active;
}

static unsigned int assert_real_active_to_inactive_teleporter_links(
    Theron_V1_World *world) {
    unsigned int verified = 0u;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *source = &world->objects[i];
        int target_level;
        int target_y;
        int target_x;
        if (source->type != THERON_OBJTYPE_TELEPORTER ||
            !(source->flags & THERON_OBJ_F_TRACK02_COORD_LINK) ||
            source->state == 0u)
            continue;
        target_level = (source->linked_id >> 10) & 0x3f;
        target_y = (source->linked_id >> 5) & 0x1f;
        target_x = source->linked_id & 0x1f;
        for (int j = 0; j < world->object_count; ++j) {
            const Theron_V1_Object *target = &world->objects[j];
            if (target->type != THERON_OBJTYPE_TELEPORTER ||
                target->dungeon_id != source->dungeon_id ||
                target->level != target_level || target->x != target_x ||
                target->y != target_y || target->state != 0u)
                continue;
            world->current_dungeon = source->dungeon_id;
            world->current_level = source->level;
            world->transition_pending = 0;
            assert(theron_v1_teleporter_resolve(
                       world, source->x, source->y) == 0);
            assert(world->transition_pending == 1);
            assert(world->transition_target_level == target_level);
            assert(world->transition_spawn_x == target_x);
            assert(world->transition_spawn_y == target_y);
            assert(world->party.leader_x == target_x);
            assert(world->party.leader_y == target_y);
            world->transition_pending = 0;
            ++verified;
            break;
        }
    }
    return verified;
}

static void assert_real_pit_open_gate(
    Theron_V1_World *world, unsigned int *closed_count,
    unsigned int *open_count) {
    int exercised_closed_move = 0;
    int exercised_open_move = 0;
    int saved_level = world->current_level;

    for (int level_index = 0;
         level_index < THERON_MAX_LEVELS_PER_DUNGEON; ++level_index) {
        Theron_V1_Level *level;
        if (!world->level_loaded[world->current_dungeon - 1][level_index])
            continue;
        level = &world->levels[world->current_dungeon - 1][level_index];
        for (int y = 0; y < level->height; ++y) {
            for (int x = 0; x < level->width; ++x) {
                uint8_t source_tile = level->source_tiles[y][x];
                int is_open;
                if (theron_tile_type(source_tile) != THERON_TILE_PIT)
                    continue;
                is_open = (source_tile & 0x08u) != 0u &&
                          (source_tile & 0x01u) == 0u;
                if (is_open) ++*open_count;
                else ++*closed_count;
                for (int dir = 0; dir < THERON_DIR_COUNT; ++dir) {
                    int px = x - g_theron_dir_dx[dir];
                    int py = y - g_theron_dir_dy[dir];
                    Theron_V1_Party saved_party;
                    int before_pending;
                    if (px < 0 || px >= level->width ||
                        py < 0 || py >= level->height ||
                        level->squares[py][px] != THERON_SQUARE_FLOOR)
                        continue;
                    saved_party = world->party;
                    before_pending = world->transition_pending;
                    world->current_level = level_index;
                    world->party.leader_x = px;
                    world->party.leader_y = py;
                    world->party.leader_dir = (int8_t)dir;
                    world->party.levitating = 0;
                    assert(theron_v1_get_move_result(world, dir) ==
                           (is_open ? THERON_MOVE_BLOCKED : THERON_MOVE_OK));
                    if (is_open) {
                        world->party.levitating = 1;
                        assert(theron_v1_get_move_result(world, dir) ==
                               THERON_MOVE_OK);
                        world->party.levitating = 0;
                    }
                    if ((is_open && !exercised_open_move) ||
                        (!is_open && !exercised_closed_move)) {
                        Theron_V1_Creature *creature =
                            theron_v1_creature_at_in_dungeon(
                                world, world->current_dungeon,
                                level_index, x, y);
                        if (creature && (creature->flags & THERON_CF_ACTIVE)) {
                            world->party = saved_party;
                            world->transition_pending = before_pending;
                            break;
                        }
                        int rc = theron_v1_move_party(world, dir);
                        assert(rc == (is_open ? THERON_MOVE_BLOCKED :
                                               THERON_MOVE_OK));
                        if (is_open) {
                            assert(world->party.leader_x == px);
                            assert(world->party.leader_y == py);
                            assert(world->transition_pending == before_pending);
                            world->party.levitating = 1;
                            assert(theron_v1_move_party(world, dir) ==
                                   THERON_MOVE_OK);
                            assert(world->party.leader_x == x);
                            assert(world->party.leader_y == y);
                            exercised_open_move = 1;
                        } else {
                            assert(world->party.leader_x == x);
                            assert(world->party.leader_y == y);
                            exercised_closed_move = 1;
                        }
                    }
                    world->party = saved_party;
                    world->transition_pending = before_pending;
                    break;
                }
            }
        }
    }
    world->current_level = saved_level;
}

static void assert_real_door_source_gate_survives_save(
    Theron_V1_World *world) {
    const Theron_V1_Object *door = NULL;
    Theron_V1_World *restored;
    uint8_t *bytes;
    size_t size;

    for (int i = 0; i < world->object_count; ++i) {
        if (world->objects[i].type == THERON_OBJTYPE_DOOR) {
            door = &world->objects[i];
            break;
        }
    }
    if (!door) return;
    size = theron_v1_world_serialize_size(world);
    bytes = (uint8_t *)malloc(size);
    restored = (Theron_V1_World *)malloc(sizeof(*restored));
    assert(size > 0u && bytes != NULL && restored != NULL);
    memcpy(restored, world, sizeof(*restored));
    assert(theron_v1_world_serialize(world, bytes, size) == size);
    assert(theron_v1_world_deserialize(restored, bytes, size) == 0);
    restored->current_dungeon = door->dungeon_id;
    restored->current_level = door->level;
    {
        Theron_V1_Object *restored_door = NULL;
        for (int i = 0; i < restored->object_count; ++i) {
            Theron_V1_Object *candidate = &restored->objects[i];
            if (candidate->type == THERON_OBJTYPE_DOOR &&
                candidate->source_ref == door->source_ref &&
                candidate->source_dungeon == door->source_dungeon &&
                candidate->source_level == door->source_level &&
                candidate->source_x == door->source_x &&
                candidate->source_y == door->source_y) {
                restored_door = candidate;
                break;
            }
        }
        assert(restored_door != NULL);
        assert(restored_door->source_origin_valid == 1u);
        assert(restored_door->source_ref == door->source_ref);
        assert(restored_door->source_raw_size == door->source_raw_size);
        assert(memcmp(restored_door->source_raw, door->source_raw,
                      door->source_raw_size) == 0);
        assert(restored->level_loaded[door->source_dungeon - 1u]
                                     [door->source_level]);
        assert(restored->levels[door->source_dungeon - 1u]
                               [door->source_level].source_header_verified);
        {
            int occurrence_found = 0;
            for (unsigned int i = 0; i < restored->source_object_count; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &restored->source_objects[i];
                if (source->dungeon_id == restored_door->source_dungeon &&
                    source->level == restored_door->source_level &&
                    source->x == restored_door->source_x &&
                    source->y == restored_door->source_y &&
                    source->source_ref == restored_door->source_ref &&
                    source->next_ref == restored_door->source_next_ref &&
                    source->source_index == restored_door->source_index &&
                    source->category == restored_door->source_category &&
                    source->position == restored_door->source_position &&
                    source->raw_size == restored_door->source_raw_size &&
                    memcmp(source->raw, restored_door->source_raw,
                           source->raw_size) == 0) {
                    occurrence_found = 1;
                    break;
                }
            }
            assert(occurrence_found);
        }
        {
            Theron_V1_Object *selected = NULL;
            for (int i = 0; i < restored->object_count; ++i) {
                Theron_V1_Object *candidate = &restored->objects[i];
                if (candidate->dungeon_id == restored->current_dungeon &&
                    candidate->level == restored->current_level &&
                    candidate->x == door->x && candidate->y == door->y &&
                    candidate->type == THERON_OBJTYPE_DOOR) {
                    selected = candidate;
                    break;
                }
            }
            assert(selected != NULL);
            assert(selected->state == THERON_DOOR_STATE_CLOSED);
            assert(selected->source_origin_valid == 1u);
            assert(selected->source_category == THERON_CAT_DOOR);
            assert(selected->source_raw_size == 4u);
        }
        assert(theron_v1_door_open(restored, door->x, door->y) == -1);
        assert(restored_door->state == THERON_DOOR_STATE_CLOSED);
    }
    free(restored);
    free(bytes);
}

static void assert_real_teleporter_source_gate_survives_save(
    Theron_V1_World *world) {
    const Theron_V1_Object *teleporter = NULL;
    Theron_V1_World *restored;
    uint8_t *bytes;
    size_t size;
    int target_level;
    int target_x;
    int target_y;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *candidate = &world->objects[i];
        if (candidate->type == THERON_OBJTYPE_TELEPORTER &&
            candidate->state != 0u &&
            (candidate->flags & THERON_OBJ_F_TRACK02_COORD_LINK)) {
            teleporter = candidate;
            break;
        }
    }
    if (!teleporter) return;
    target_level = (teleporter->linked_id >> 10) & 0x3f;
    target_y = (teleporter->linked_id >> 5) & 0x1f;
    target_x = teleporter->linked_id & 0x1f;
    size = theron_v1_world_serialize_size(world);
    bytes = (uint8_t *)malloc(size);
    restored = (Theron_V1_World *)malloc(sizeof(*restored));
    assert(size > 0u && bytes != NULL && restored != NULL);
    memcpy(restored, world, sizeof(*restored));
    assert(theron_v1_world_serialize(world, bytes, size) == size);
    assert(theron_v1_world_deserialize(restored, bytes, size) == 0);
    {
        Theron_V1_Object *restored_teleporter = NULL;
        for (int i = 0; i < restored->object_count; ++i) {
            Theron_V1_Object *candidate = &restored->objects[i];
            if (candidate->type == THERON_OBJTYPE_TELEPORTER &&
                candidate->source_ref == teleporter->source_ref &&
                candidate->source_dungeon == teleporter->source_dungeon &&
                candidate->source_level == teleporter->source_level &&
                candidate->source_x == teleporter->source_x &&
                candidate->source_y == teleporter->source_y) {
                restored_teleporter = candidate;
                break;
            }
        }
        assert(restored_teleporter != NULL);
        assert(restored_teleporter->state == 1u);
        assert(restored_teleporter->linked_id == teleporter->linked_id);
        assert(restored_teleporter->source_raw_size == 6u);
        assert(memcmp(restored_teleporter->source_raw,
                      teleporter->source_raw, 6u) == 0);
        assert((restored_teleporter->flags &
                THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_MASK) ==
               (teleporter->flags &
                THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_MASK));
        assert(restored->levels[teleporter->source_dungeon - 1u]
                               [teleporter->source_level]
                                   .source_tiles[teleporter->source_y]
                                                [teleporter->source_x] & 0x08u);
        restored->current_dungeon = teleporter->dungeon_id;
        restored->current_level = teleporter->level;
        restored->transition_pending = 0;
        assert(theron_v1_teleporter_resolve(
                   restored, teleporter->x, teleporter->y) == 0);
        assert(restored->transition_pending == 1);
        assert(restored->transition_target_level == target_level);
        assert(restored->transition_spawn_x == target_x);
        assert(restored->transition_spawn_y == target_y);
    }
    free(restored);
    free(bytes);
}

static void assert_real_control_records_have_no_fixture_mutation_aliases(
    const Theron_V1_World *world, unsigned int expected_teleporters,
    unsigned int expected_actuators) {
    unsigned int teleporters = 0u;
    unsigned int actuators = 0u;
    const uint32_t mutation_mask =
        THERON_OBJ_F_PICKED_UP | THERON_OBJ_F_OPENED |
        THERON_OBJ_F_ACTIVATED | THERON_OBJ_F_DESTROYED |
        THERON_OBJ_F_LOCKED | THERON_OBJ_F_USED;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        const Theron_V1_SourceObjectRecord *source = NULL;
        if (object->type != THERON_OBJTYPE_TELEPORTER &&
            object->type != THERON_OBJTYPE_SOURCE_ACTUATOR) continue;
        for (unsigned int j = 0; j < world->source_object_count; ++j) {
            const Theron_V1_SourceObjectRecord *candidate =
                &world->source_objects[j];
            if (candidate->dungeon_id == object->dungeon_id &&
                candidate->level == object->level &&
                candidate->x == object->x && candidate->y == object->y &&
                candidate->source_ref == object->source_ref &&
                candidate->source_index == object->source_index) {
                source = candidate;
                break;
            }
        }
        assert(source != NULL);
        assert(object->source_origin_valid == 1u);
        assert(object->source_dungeon == source->dungeon_id);
        assert(object->source_level == source->level);
        assert(object->source_x == source->x);
        assert(object->source_y == source->y);
        assert(object->source_next_ref == source->next_ref);
        assert(object->source_category == source->category);
        assert(object->source_position == source->position);
        assert(object->source_raw_size == source->raw_size);
        assert(memcmp(object->source_raw, source->raw, source->raw_size) == 0);
        assert((object->flags & mutation_mask) == 0u);
        assert(((object->flags & THERON_OBJ_F_SOURCE_POSITION_MASK) >>
                THERON_OBJ_F_SOURCE_POSITION_SHIFT) <= 3u);
        if (object->type == THERON_OBJTYPE_TELEPORTER) {
            assert(object->flags & THERON_OBJ_F_TRACK02_COORD_LINK);
            ++teleporters;
        } else {
            ++actuators;
        }
    }
    assert(teleporters == expected_teleporters);
    assert(actuators == expected_actuators);
}

static void census_real_actuator_types(
    const Theron_V1_World *world, unsigned int floor_types[128],
    unsigned int wall_types[128], unsigned int effects[8],
    unsigned int *local_effects, unsigned int *reverted, unsigned int *once,
    unsigned int *high_values, unsigned int *max_value,
    unsigned int party_tiles[8], unsigned int party_positions[4],
    unsigned int party_effects[8], unsigned int *party_once,
    unsigned int *party_local_effects, unsigned int *party_reverted,
    unsigned int *party_with_teleporter,
    unsigned int *party_with_open_teleporter,
    unsigned int *party_reverted_set_shared_entry,
    unsigned int *party_reverted_set_other) {
    static const uint8_t shared_entry_raw[8] = {
        0xfe, 0xff, 0x03, 0x00, 0xa4, 0x07, 0x80, 0x18
    };
    static const uint8_t formicia_reverted_set_raw[8] = {
        0xfe, 0xff, 0x03, 0x01, 0x60, 0x02, 0xa0, 0x21
    };
    for (unsigned int i = 0; i < world->source_object_count; ++i) {
        const Theron_V1_SourceObjectRecord *source = &world->source_objects[i];
        Theron_Actuator actuator;
        const Theron_V1_Level *level;
        uint8_t tile_type;
        if (source->category != THERON_CAT_ACTUATOR) continue;
        assert(source->raw_size == 8u);
        assert(theron_v1_track02_actuator_decode(source->raw, &actuator) == 0);
        level = &world->levels[source->dungeon_id - 1][source->level];
        tile_type = (uint8_t)(level->source_tiles[source->y][source->x] & 0xe0u);
        if (tile_type == THERON_TILE_WALL)
            ++wall_types[actuator.type];
        else
            ++floor_types[actuator.type];
        ++effects[actuator.effect];
        *local_effects += actuator.local_effect != 0u;
        *reverted += actuator.revert_effect != 0u;
        *once += actuator.once != 0u;
        *high_values += actuator.value > 0xffu;
        if (actuator.value > *max_value) *max_value = actuator.value;
        if (tile_type != THERON_TILE_WALL &&
            actuator.type == TQ_ACT_FLOOR_PARTY) {
            ++party_tiles[tile_type >> 5];
            ++party_positions[source->position];
            ++party_effects[actuator.effect];
            *party_once += actuator.once != 0u;
            *party_local_effects += actuator.local_effect != 0u;
            *party_reverted += actuator.revert_effect != 0u;
            if (actuator.effect == 0u && actuator.revert_effect != 0u) {
                if (source->level == 0 && source->x == 2 && source->y == 1 &&
                    level->source_tiles[source->y][source->x] == 0xb4u &&
                    memcmp(source->raw, shared_entry_raw,
                           sizeof(shared_entry_raw)) == 0)
                    ++*party_reverted_set_shared_entry;
                else {
                    assert(source->dungeon_id == 3);
                    assert(source->level == 3 && source->x == 11 &&
                           source->y == 8);
                    assert(level->source_tiles[source->y][source->x] == 0x30u);
                    assert(memcmp(source->raw, formicia_reverted_set_raw,
                                  sizeof(formicia_reverted_set_raw)) == 0);
                    assert(actuator.once == 0u && actuator.local_effect == 0u);
                    assert(actuator.value == 2u);
                    assert(actuator.target_x == 6u && actuator.target_y == 4u &&
                           actuator.target_facing == 2u);
                    ++*party_reverted_set_other;
                }
            }
            for (unsigned int j = 0; j < world->source_object_count; ++j) {
                const Theron_V1_SourceObjectRecord *candidate =
                    &world->source_objects[j];
                if (candidate->category == THERON_CAT_TELEPORTER &&
                    candidate->dungeon_id == source->dungeon_id &&
                    candidate->level == source->level &&
                    candidate->x == source->x && candidate->y == source->y) {
                    ++*party_with_teleporter;
                    *party_with_open_teleporter +=
                        (level->source_tiles[source->y][source->x] & 0x08u) != 0u;
                    break;
                }
            }
        }
    }
}

static void print_real_actuator_census(
    const char *region, const unsigned int floor_types[128],
    const unsigned int wall_types[128], const unsigned int effects[8],
    unsigned int local_effects, unsigned int reverted, unsigned int once,
    unsigned int high_values, unsigned int max_value) {
    unsigned int type_total = 0u;
    unsigned int effect_total = 0u;
    printf("  %s source actuators:", region);
    for (unsigned int i = 0; i < 128u; ++i) {
        type_total += floor_types[i] + wall_types[i];
        if (floor_types[i]) printf(" floor%u=%u", i, floor_types[i]);
        if (wall_types[i]) printf(" wall%u=%u", i, wall_types[i]);
    }
    printf("; effects:");
    for (unsigned int i = 0; i < 8u; ++i) {
        effect_total += effects[i];
        if (effects[i]) printf(" %u=%u", i, effects[i]);
    }
    printf("; once=%u local-effect=%u reverted=%u value>255=%u max-value=%u\n",
           once, local_effects, reverted, high_values, max_value);
    /* Both authenticated retail regions currently carry this exact corpus.
     * Keep the dominant party-square and generator families explicit so a
     * decoder regression cannot silently turn real records into another
     * host actuator kind. */
    assert(type_total == 1109u);
    assert(effect_total == type_total);
    assert(effects[0] == 632u && effects[1] == 158u &&
           effects[2] == 103u && effects[3] == 216u);
    assert(effects[4] == 0u && effects[5] == 0u && effects[6] == 0u &&
           effects[7] == 0u);
    assert(floor_types[TQ_ACT_FLOOR_PARTY] == 393u);
    assert(floor_types[TQ_ACT_FLOOR_MONSTER_GEN] == 46u);
    assert(once == 286u);
    assert(local_effects == 116u);
    assert(reverted == 70u);
    assert(high_values == 0u);
    assert(max_value == 255u);
}

static void print_real_party_actuator_context(
    const char *region, const unsigned int tiles[8],
    const unsigned int positions[4], const unsigned int effects[8],
    unsigned int once, unsigned int local_effects, unsigned int reverted,
    unsigned int with_teleporter, unsigned int with_open_teleporter,
    unsigned int reverted_set_shared_entry, unsigned int reverted_set_other) {
    printf("  %s floor-party actuator context: tiles", region);
    for (unsigned int i = 0; i < 8u; ++i)
        if (tiles[i]) printf(" %u=%u", i, tiles[i]);
    printf("; positions");
    for (unsigned int i = 0; i < 4u; ++i)
        if (positions[i]) printf(" %u=%u", i, positions[i]);
    printf("; effects");
    for (unsigned int i = 0; i < 8u; ++i)
        if (effects[i]) printf(" %u=%u", i, effects[i]);
    printf("; once=%u local-effect=%u reverted=%u teleporter=%u "
           "open-teleporter=%u reverted-set-entry=%u reverted-set-other=%u\n",
           once, local_effects, reverted, with_teleporter,
           with_open_teleporter, reverted_set_shared_entry,
           reverted_set_other);
    /* Exact US/JP retail corpus.  Floor-party records occur on several map
     * families, including both closed and open teleporters; no one tile kind
     * or OPEN bit may therefore be promoted into the missing event gate. */
    assert(tiles[1] == 210u);
    assert(tiles[2] == 92u);
    assert(tiles[3] == 8u);
    assert(tiles[5] == 83u);
    assert(tiles[0] + tiles[1] + tiles[2] + tiles[3] + tiles[4] +
               tiles[5] + tiles[6] + tiles[7] == 393u);
    assert(positions[0] == 393u && positions[1] == 0u &&
           positions[2] == 0u && positions[3] == 0u);
    assert(effects[0] == 190u && effects[1] == 66u &&
           effects[2] == 40u && effects[3] == 97u &&
           effects[4] == 0u && effects[5] == 0u &&
           effects[6] == 0u && effects[7] == 0u);
    assert(once == 91u);
    assert(local_effects == 12u);
    assert(reverted == 19u);
    assert(with_teleporter == 83u);
    assert(with_open_teleporter == 7u);
    assert(reverted_set_shared_entry == 7u);
    assert(reverted_set_other == 1u);
}

static void census_real_actuator_dispatch_context(
    const Theron_V1_World *world, unsigned int local_multiples[4096],
    unsigned int remote_target_tiles[9],
    unsigned int target_categories[8][16],
    unsigned int targets_without_things[8],
    unsigned int target_actuator_types[8][128]) {
    for (unsigned int i = 0; i < world->source_object_count; ++i) {
        const Theron_V1_SourceObjectRecord *source = &world->source_objects[i];
        Theron_Actuator actuator;
        const Theron_V1_Level *level;
        if (source->category != THERON_CAT_ACTUATOR || source->raw_size != 8u)
            continue;
        assert(theron_v1_track02_actuator_decode(source->raw, &actuator) == 0);
        if (actuator.type != TQ_ACT_FLOOR_PARTY) continue;
        level = &world->levels[source->dungeon_id - 1][source->level];
        if ((level->source_tiles[source->y][source->x] & 0xe0u) ==
                THERON_TILE_WALL)
            continue;
        if (actuator.local_effect) {
            ++local_multiples[actuator.local_multiple];
            continue;
        }
        if (actuator.target_x >= level->width ||
            actuator.target_y >= level->height) {
            static const uint8_t thieves_outside_raw[8] = {
                0xfe, 0xff, 0x03, 0x00, 0x78, 0x15, 0x40, 0x50
            };
            ++remote_target_tiles[8];
            assert(source->dungeon_id == 6 && source->level == 0 &&
                   source->x == 5 && source->y == 5 &&
                   actuator.target_x == 1u && actuator.target_y == 10u &&
                   memcmp(source->raw, thieves_outside_raw,
                          sizeof(thieves_outside_raw)) == 0);
        } else {
            const unsigned int family =
                level->source_tiles[actuator.target_y][actuator.target_x] >> 5;
            int found = 0;
            ++remote_target_tiles[family];
            for (unsigned int j = 0; j < world->source_object_count; ++j) {
                const Theron_V1_SourceObjectRecord *target =
                    &world->source_objects[j];
                if (target->dungeon_id == source->dungeon_id &&
                    target->level == source->level &&
                    target->x == actuator.target_x &&
                    target->y == actuator.target_y &&
                    target->category < 16u) {
                    ++target_categories[family][target->category];
                    if (target->category == THERON_CAT_ACTUATOR &&
                        target->raw_size == 8u) {
                        Theron_Actuator target_actuator;
                        assert(theron_v1_track02_actuator_decode(
                                   target->raw, &target_actuator) == 0);
                        assert(target_actuator.type < 128u);
                        if (family == THERON_TILE_OPEN &&
                            target_actuator.type == TQ_ACT_FLOOR_MONSTER_GEN) {
                            Theron_ActuatorGeneratorPlan generator_plan;
                            static const uint8_t drator_raw[8] = {
                                0xfe, 0xff, 0x06, 0x06,
                                0x80, 0x08, 0x20, 0x40
                            };
                            static const uint8_t formicia_raw[8] = {
                                0xfe, 0xff, 0x06, 0x08,
                                0x00, 0x0e, 0x10, 0x20
                            };
                            static const uint8_t thieves_raw[8] = {
                                0xfe, 0xff, 0x06, 0x01,
                                0x80, 0x08, 0x10, 0x40
                            };
                            static const uint8_t demon_raw[8] = {
                                0xfe, 0xff, 0x06, 0x0b,
                                0x80, 0x08, 0x10, 0xf0
                            };
                            assert(theron_v1_track02_actuator_generator_plan(
                                       &target_actuator, &generator_plan));
                            if (source->dungeon_id == 2) {
                                static const uint8_t drator_event_raw[4][8] = {
                                    {0x33,0x0c,0x03,0x00,0xc4,0x10,0x00,0x19},
                                    {0xfe,0xff,0x03,0x00,0x98,0x00,0x00,0x19},
                                    {0xfe,0xff,0x03,0x00,0x80,0x00,0x10,0x19},
                                    {0xfe,0xff,0x03,0x00,0xd8,0x00,0x00,0x19}
                                };
                                const unsigned int event_slot =
                                    source->source_ref == 0x0c81u ? 0u :
                                    source->source_ref == 0x0c94u ? 1u :
                                    source->source_ref == 0x0cacu ? 2u : 3u;
                                static const uint8_t event_xy[4][2] = {
                                    {2,3}, {11,7}, {12,2}, {12,3}
                                };
                                assert(source->level == 2 &&
                                       target->source_ref == 0x0c38u &&
                                       target->x == 4 && target->y == 3 &&
                                       (source->source_ref == 0x0c81u ||
                                        source->source_ref == 0x0c94u ||
                                        source->source_ref == 0x0cacu ||
                                       source->source_ref == 0x0c9eu) &&
                                       source->x == event_xy[event_slot][0] &&
                                       source->y == event_xy[event_slot][1] &&
                                       memcmp(source->raw,
                                              drator_event_raw[event_slot],
                                              sizeof(drator_event_raw[event_slot])) == 0 &&
                                       memcmp(target->raw, drator_raw,
                                              sizeof(drator_raw)) == 0 &&
                                       target_actuator.value == 12u &&
                                       target_actuator.generator_generation == 1u &&
                                       target_actuator.generator_toughness == 32u &&
                                       target_actuator.generator_pause == 64u &&
                                       generator_plan.creature_type_value == 12u &&
                                       !generator_plan.count_is_random &&
                                       generator_plan.fixed_count_minus_one == 0u);
                            } else if (source->dungeon_id == 3) {
                                static const uint8_t formicia_event_raw[8] = {
                                    0xfe,0xff,0x03,0x00,0x80,0x00,0xc0,0x11
                                };
                                assert(source->level == 1 &&
                                       source->source_ref == 0x0c6bu &&
                                       source->x == 8 && source->y == 6 &&
                                       memcmp(source->raw, formicia_event_raw,
                                              sizeof(formicia_event_raw)) == 0 &&
                                       target->source_ref == 0x0c57u &&
                                       target->x == 7 && target->y == 2 &&
                                       memcmp(target->raw, formicia_raw,
                                              sizeof(formicia_raw)) == 0 &&
                                       target_actuator.value == 16u &&
                                       target_actuator.generator_generation == 12u &&
                                       target_actuator.generator_toughness == 16u &&
                                       target_actuator.generator_pause == 32u &&
                                       generator_plan.creature_type_value == 16u &&
                                       generator_plan.count_is_random &&
                                       generator_plan.random_count_bound == 4u);
                            } else if (source->dungeon_id == 6) {
                                static const uint8_t thieves_event_raw[8] = {
                                    0xfe,0xff,0x03,0x00,0x04,0x00,0x40,0x1c
                                };
                                assert(source->level == 2 &&
                                       source->source_ref == 0x0c7fu &&
                                       source->x == 18 && source->y == 16 &&
                                       memcmp(source->raw, thieves_event_raw,
                                              sizeof(thieves_event_raw)) == 0 &&
                                       target->source_ref == 0x0c70u &&
                                       target->x == 17 && target->y == 3 &&
                                       memcmp(target->raw, thieves_raw,
                                              sizeof(thieves_raw)) == 0 &&
                                       target_actuator.value == 2u &&
                                       target_actuator.generator_generation == 1u &&
                                       target_actuator.generator_toughness == 16u &&
                                       target_actuator.generator_pause == 64u &&
                                       generator_plan.creature_type_value == 2u &&
                                       !generator_plan.count_is_random &&
                                       generator_plan.fixed_count_minus_one == 0u);
                            } else {
                                static const uint8_t demon_event_raw[8] = {
                                    0xfe,0xff,0x03,0x00,0x50,0x20,0x80,0x2c
                                };
                                assert(source->dungeon_id == 7 &&
                                       source->level == 2 &&
                                       source->source_ref == 0x0c58u &&
                                       source->x == 16 && source->y == 9 &&
                                       memcmp(source->raw, demon_event_raw,
                                              sizeof(demon_event_raw)) == 0 &&
                                       target->source_ref == 0x0c64u &&
                                       target->x == 18 && target->y == 5 &&
                                       memcmp(target->raw, demon_raw,
                                              sizeof(demon_raw)) == 0 &&
                                       target_actuator.value == 22u &&
                                       target_actuator.generator_generation == 1u &&
                                       target_actuator.generator_toughness == 16u &&
                                       target_actuator.generator_pause == 240u &&
                                       generator_plan.creature_type_value == 22u &&
                                       !generator_plan.count_is_random &&
                                       generator_plan.fixed_count_minus_one == 0u);
                            }
                        }
                        ++target_actuator_types[family]
                                               [target_actuator.type];
                    }
                    found = 1;
                }
            }
            if (!found) ++targets_without_things[family];
        }
    }
}

static void print_real_actuator_dispatch_context(
    const char *region, const unsigned int local_multiples[4096],
    const unsigned int remote_target_tiles[9],
    const unsigned int target_categories[8][16],
    const unsigned int targets_without_things[8],
    const unsigned int target_actuator_types[8][128]) {
    printf("  %s actuator dispatch context: local", region);
    for (unsigned int i = 0; i < 4096u; ++i)
        if (local_multiples[i]) printf(" %u=%u", i, local_multiples[i]);
    printf("; remote-target-tiles");
    for (unsigned int i = 0; i < 8u; ++i)
        if (remote_target_tiles[i])
            printf(" %u=%u", i, remote_target_tiles[i]);
    if (remote_target_tiles[8]) printf(" outside=%u", remote_target_tiles[8]);
    printf("\n");
    printf("  %s actuator target C03 types:", region);
    for (unsigned int family = 0; family < 8u; ++family) {
        printf(" family%u[", family);
        for (unsigned int type = 0; type < 128u; ++type)
            if (target_actuator_types[family][type])
                printf(" t%u=%u", type,
                       target_actuator_types[family][type]);
        printf(" ]");
    }
    printf("\n");
    printf("  %s actuator target thing context:", region);
    for (unsigned int family = 0; family < 8u; ++family) {
        printf(" family%u[empty=%u", family, targets_without_things[family]);
        for (unsigned int category = 0; category < 16u; ++category)
            if (target_categories[family][category])
                printf(" c%u=%u", category,
                       target_categories[family][category]);
        printf("]");
    }
    printf("\n");
    {
        static const unsigned int expected_types[8][128] = {
            { [0] = 4u, [1] = 4u, [2] = 4u, [3] = 4u, [4] = 3u,
              [5] = 16u, [15] = 1u },
            { [1] = 1u, [2] = 1u, [3] = 8u, [6] = 7u, [7] = 4u },
            { [3] = 10u },
            { [3] = 2u },
            { 0u },
            { [1] = 2u, [3] = 4u },
            { 0u },
            { 0u }
        };
        assert(memcmp(target_actuator_types, expected_types,
                      sizeof(expected_types)) == 0);
    }
    {
        static const unsigned int expected_categories[8][16] = {
            { [3] = 36u, [5] = 1u, [6] = 1u, [8] = 1u, [10] = 1u },
            { [2] = 1u, [3] = 21u, [4] = 7u, [5] = 1u, [10] = 4u },
            { [3] = 10u },
            { [3] = 2u },
            { [0] = 9u },
            { [1] = 18u, [3] = 6u },
            { 0u },
            { 0u }
        };
        static const unsigned int expected_empty_us[8] = {
            186u, 86u, 13u, 5u, 2u, 0u, 5u, 6u
        };
        static const unsigned int expected_empty_jp[8] = {
            185u, 86u, 13u, 5u, 2u, 2u, 6u, 4u
        };
        const unsigned int *expected_empty =
            strcmp(region, "US") == 0 ? expected_empty_us : expected_empty_jp;
        assert(memcmp(target_categories, expected_categories,
                      sizeof(expected_categories)) == 0);
        assert(memcmp(targets_without_things, expected_empty,
                      sizeof(expected_empty_us)) == 0);
    }
    unsigned int local_total = 0u;
    unsigned int remote_total = 0u;
    for (unsigned int i = 0; i < 4096u; ++i) {
        local_total += local_multiples[i];
        if (i != 176u) assert(local_multiples[i] == 0u);
    }
    for (unsigned int i = 0; i < 9u; ++i)
        remote_total += remote_target_tiles[i];
    assert(local_multiples[176] == 12u && local_total == 12u);
    assert(remote_total == 381u && remote_target_tiles[8] == 1u);
    assert(remote_target_tiles[1] == 114u &&
           remote_target_tiles[2] == 18u &&
           remote_target_tiles[3] == 7u &&
           remote_target_tiles[4] == 11u);
    if (strcmp(region, "US") == 0) {
        assert(remote_target_tiles[0] == 201u &&
               remote_target_tiles[5] == 18u &&
               remote_target_tiles[6] == 5u &&
               remote_target_tiles[7] == 6u);
    } else {
        assert(remote_target_tiles[0] == 200u &&
               remote_target_tiles[5] == 20u &&
               remote_target_tiles[6] == 6u &&
               remote_target_tiles[7] == 4u);
    }
}

static void census_real_control_square_first_object_aliases(
    const Theron_V1_World *world, unsigned int *out_door_aliases,
    unsigned int *out_teleporter_aliases) {
    unsigned int door_aliases = 0u;
    unsigned int teleporter_aliases = 0u;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *control = &world->objects[i];
        const Theron_V1_Object *first;
        if (control->type != THERON_OBJTYPE_DOOR &&
            control->type != THERON_OBJTYPE_TELEPORTER) continue;
        first = theron_v1_object_at_in_dungeon(
            (Theron_V1_World *)world, control->dungeon_id, control->level,
            control->x, control->y);
        assert(first != NULL);
        if (first->type != control->type) {
            if (control->type == THERON_OBJTYPE_DOOR) ++door_aliases;
            else ++teleporter_aliases;
        }
    }
    *out_door_aliases += door_aliases;
    *out_teleporter_aliases += teleporter_aliases;
}

static unsigned int census_real_carryable_not_first(
    const Theron_V1_World *world) {
    unsigned int count = 0u;
    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        const Theron_V1_Object *first;
        if (object->source_category != THERON_CAT_WEAPON &&
            object->source_category != THERON_CAT_CLOTHING &&
            object->source_category != THERON_CAT_SCROLL &&
            object->source_category != THERON_CAT_POTION &&
            object->source_category != THERON_CAT_MISC) continue;
        first = theron_v1_object_at_in_dungeon(
            (Theron_V1_World *)world, object->dungeon_id, object->level,
            object->x, object->y);
        if (first != object) ++count;
    }
    return count;
}

static void test_all_dungeons(
    const uint8_t *ud, size_t ud_size,
    const uint8_t *track02, size_t track02_size) {
    static const size_t property_offsets[THERON_DUNGEON_COUNT] = {
        0x099825u, 0x0d9dc5u, 0x11a4d4u, 0x159d1du,
        0x19a64eu, 0x1d999fu, 0x21a32du
    };
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };
    /* Only generators reachable from real ground-reference chains are
     * placed in the world ledger; the remaining category-3 records are
     * source table entries without a map occurrence. */
    const unsigned int expected_source_generators[] = {3, 7, 9, 2, 1, 14, 10};
    unsigned int door_aliases = 0u;
    unsigned int teleporter_aliases = 0u;
    unsigned int carryable_not_first = 0u;
    unsigned int nonfirst_take_roundtrips = 0u;
    unsigned int active_teleporters = 0u;
    unsigned int active_to_inactive = 0u;
    unsigned int closed_pits = 0u;
    unsigned int open_pits = 0u;
    unsigned int floor_actuators[128] = {0};
    unsigned int wall_actuators[128] = {0};
    unsigned int actuator_effects[8] = {0};
    unsigned int local_effect_actuators = 0u;
    unsigned int reverted_actuators = 0u;
    unsigned int once_actuators = 0u;
    unsigned int high_actuator_values = 0u;
    unsigned int max_actuator_value = 0u;
    unsigned int party_tiles[8] = {0};
    unsigned int party_positions[4] = {0};
    unsigned int party_effects[8] = {0};
    unsigned int party_once = 0u;
    unsigned int party_local_effects = 0u;
    unsigned int party_reverted = 0u;
    unsigned int party_with_teleporter = 0u;
    unsigned int party_with_open_teleporter = 0u;
    unsigned int party_reverted_set_shared_entry = 0u;
    unsigned int party_reverted_set_other = 0u;
    unsigned int local_multiples[4096] = {0};
    unsigned int remote_target_tiles[9] = {0};
    unsigned int target_categories[8][16] = {{0}};
    unsigned int targets_without_things[8] = {0};
    unsigned int target_actuator_types[8][128] = {{0}};

    for (int d = 0; d < 7; d++) {
        Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
        assert(world);
        theron_v1_world_init(world);
        world->current_dungeon = d + 1;
        bind_real_track02_party(
            world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);

        {
            Theron_Track02ItemNameSource name_source;
            assert(theron_v1_track02_decode_item_name_source(
                       ud, ud_size, 2, (unsigned int)d + 1u,
                       &name_source) == 1);
            assert(theron_v1_world_bind_track02_item_name_source(
                       world, &name_source, 2) == 1);
        }

        Theron_DungeonLoadResult result;
        int rc = theron_v1_track02_load_full_dungeon(
            world, d + 1, ud, ud_size, &result);
        assert(rc == 0);
        Theron_DungeonData source_maps;
        assert(theron_v1_track02_dungeon_map_load(
                   ud, ud_size, (unsigned int)d, &source_maps));
        assert(source_maps.map_count == (uint8_t)result.levels_loaded);
        assert(world->source_thing_directory_verified[d] == 1);
        assert(world->source_column_thing_count_total[d] ==
               source_maps.column_thing_count_total);
        assert(memcmp(world->source_thing_descriptor_sizes[d],
                      source_maps.thing_descriptor_sizes,
                      sizeof(source_maps.thing_descriptor_sizes)) == 0);
        for (unsigned int m = 0; m < source_maps.map_count; ++m) {
            const Theron_MapHeader *src = &source_maps.maps[m].header;
            const Theron_V1_Level *dst = &world->levels[d][m];
            assert(dst->source_header_verified == 1);
            assert(dst->source_header_level_index == src->map_id);
            assert(dst->source_map_x_offset == src->x_offset);
            assert(dst->source_map_y_offset == src->y_offset);
            assert(dst->source_header_unk1 == src->unk1);
            assert(dst->source_header_unk2 == src->unk2);
            assert(dst->source_xp_modifier == src->xp_modifier);
            assert(dst->source_door_type1 == src->door_type1);
            assert(dst->source_door_type2 == src->door_type2);
            assert(dst->source_creature_gfx_bank ==
                   source_maps.creature_gfx_bank[m]);
            assert(dst->source_cumulative_column_items ==
                   source_maps.cumulative_column_items[m]);
            assert(dst->creature_budget == src->creature_count);
        }

        printf("  %s: %d levels, %d things placed "
               "(%d doors, %d telep, %d act[%d fixed], %d source records, "
               "%d unbound, %d raw-only, %d materialized, %d properties) "
               "%d refs linked\n",
               names[d],
               result.levels_loaded,
               result.total_things_placed,
               result.doors_placed,
               result.teleporters_placed,
               result.actuators_placed,
               result.actuator_value_fixes,
               result.source_records_decoded,
               result.unbound_item_refs,
               result.raw_only_item_refs,
               result.source_objects_materialized,
               result.source_item_properties_bound,
               result.ground_refs_linked);

        printf("    source categories: monster=%u weapon=%u clothing=%u "
               "scroll=%u potion=%u chest=%u misc=%u missile=%u cloud=%u\n",
               result.source_category_counts[THERON_CAT_MONSTER],
               result.source_category_counts[THERON_CAT_WEAPON],
               result.source_category_counts[THERON_CAT_CLOTHING],
               result.source_category_counts[THERON_CAT_SCROLL],
               result.source_category_counts[THERON_CAT_POTION],
               result.source_category_counts[THERON_CAT_CHEST],
               result.source_category_counts[THERON_CAT_MISC],
               result.source_category_counts[THERON_CAT_MISSILE],
               result.source_category_counts[THERON_CAT_CLOUD]);

        assert(result.levels_loaded > 0);
        assert(result.total_things_placed > 0);
        assert(result.source_records_decoded > 0);
        assert(result.unbound_item_refs == result.source_records_decoded +
               result.raw_only_item_refs);
        assert(result.raw_only_item_refs == 0);
        assert(result.source_objects_materialized > 0);
        assert(result.source_item_properties_bound > 0);
        assert((unsigned int)result.source_item_properties_bound ==
               (unsigned int)result.source_objects_materialized -
                   result.source_category_counts[THERON_CAT_CHEST]);
        assert(result.source_property_table_verified == 1);
        assert(result.source_property_table_offset == property_offsets[d]);
        assert(result.source_text_data_count <= THERON_TRACK02_SOURCE_TEXT_MAX);
        assert(theron_v1_world_source_dungeon_text_count(world) ==
               result.source_text_data_count);
        if (result.source_text_data_count > 0u) {
            uint16_t source_word = 0;
            assert(theron_v1_world_source_dungeon_text_word(
                world, 0u, &source_word));
            assert(source_word == result.source_text_data[0]);
            assert(world->source_dungeon_text_dungeon_id == (int)d + 1);
        }
        if (d == 0)
            assert(result.source_text_data_count == 0x013C);
        assert(result.source_object_count ==
               (unsigned int)result.source_occurrences_decoded);
        assert(result.source_occurrences_decoded ==
               (int)result.source_object_count);
        assert(world->source_monster_count ==
               result.source_category_counts[THERON_CAT_MONSTER]);
        assert(world->source_generator_count == expected_source_generators[d]);
        /* Every decoded Track 02 ground-reference occurrence is retained in
         * the world provenance ledger; gameplay consumers remain separately
         * gated until their source semantics are authenticated. */
        assert(world->source_object_count ==
               (unsigned int)result.source_object_count);
        /* Static category-4 group records are now admitted to the live pool
         * for the current level.  This is source materialization, not the
         * still-gated random generator path. */
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int ci = 0; ci < world->creature_count; ++ci) {
            const Theron_V1_Creature *creature = &world->creatures[ci];
            const Theron_V1_SourceMonsterRecord *source = NULL;
            for (unsigned int si = 0; si < world->source_monster_count; ++si) {
                const Theron_V1_SourceMonsterRecord *candidate =
                    &world->source_monsters[si];
                if (candidate->source_ref == creature->source_ref &&
                    candidate->source_index == creature->source_index) {
                    source = candidate;
                    break;
                }
            }
            assert(creature->flags & THERON_CF_ACTIVE);
            assert(creature->source_ref != 0u);
            assert(source != NULL);
            assert(source->raw_size == 16u);
            assert(creature->type == source->type);
            assert(creature->source_chested == source->chested);
            assert(creature->source_cell ==
                   (uint8_t)((creature->source_position >>
                              (creature->source_slot * 2u)) & 0x03u));
            assert(creature->source_slot < 4u);
            assert(creature->source_group_count ==
                   (uint8_t)(source->number + 1u));
            assert(creature->hp == (int)source->health[creature->source_slot]);
            assert(creature->source_raw_size == source->raw_size);
            assert(memcmp(creature->source_raw, source->raw,
                          source->raw_size) == 0);
            assert(creature->max_hp == creature->hp);
            assert(creature->attack == 0);
            assert(creature->defense == 0);
            assert(creature->speed == 0);
            assert(creature->hp == creature->max_hp);
            assert(creature->primary_attack == THERON_ATTACK_NONE);
            assert(creature->ai == THERON_AI_UNAVAILABLE);
            assert(creature->secondary_attack == THERON_ATTACK_NONE);
            /* This direct user-data loader has no authenticated spawn-source
             * receipt.  Static creature type must not be promoted into a
             * regular-spawn category; the authenticated binding test above
             * covers the real source-consumer path. */
            assert(creature->source_spawn_category == 0xffu);
        }
        theron_v1_world_init_generators(world);
        world->world_tick = 60;
        theron_v1_world_tick_generators(world);
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int i = 0; i < world->generator_active_count; ++i)
            assert(world->generator_spawn_count[i] == 0);
        if (d == 0)
            assert_real_monster_ledger_roundtrip(world);
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        for (unsigned int i = 0; i < result.source_object_count; ++i) {
            const Theron_Track02SourceObjectOccurrence *occ =
                &result.source_objects[i];
            assert(occ->category <= THERON_CAT_MISC ||
                   occ->category == THERON_CAT_MISSILE ||
                   occ->category == THERON_CAT_CLOUD);
            assert(occ->raw_size == theron_item_bytes[occ->category]);
            assert(occ->next_ref ==
                   ((uint16_t)occ->raw[0] | ((uint16_t)occ->raw[1] << 8)));
            assert(occ->decoded_valid);
            assert(occ->decoded.category == occ->category);
            assert(occ->decoded.next_ref == occ->next_ref);
        }
        for (unsigned int i = 0; i < world->source_monster_count; ++i) {
            const Theron_V1_SourceMonsterRecord *monster =
                &world->source_monsters[i];
            const Theron_Track02SourceObjectOccurrence *source = NULL;
            assert(monster->dungeon_id == d + 1);
            assert(monster->level >= 0 &&
                   monster->level < THERON_MAX_LEVELS_PER_DUNGEON);
            assert(monster->x < THERON_MAX_MAP_SIZE);
            assert(monster->y < THERON_MAX_MAP_SIZE);
            for (unsigned int j = 0; j < result.source_object_count; ++j) {
                const Theron_Track02SourceObjectOccurrence *candidate =
                    &result.source_objects[j];
                if (candidate->category == THERON_CAT_MONSTER &&
                    candidate->source_ref == monster->source_ref &&
                    candidate->source_index == monster->source_index) {
                    source = candidate;
                    break;
                }
            }
            assert(source != NULL);
            assert(monster->level == (int)source->map);
            assert(monster->x == (int)source->x);
            assert(monster->y == (int)source->y);
            assert(monster->type == source->decoded.value.monster.type);
            assert(monster->position == source->decoded.value.monster.position);
            assert(monster->number == source->decoded.value.monster.number);
            assert(monster->direction_flags ==
                   source->decoded.value.monster.direction_flags);
            assert(memcmp(monster->health, source->decoded.value.monster.health,
                          sizeof(monster->health)) == 0);
            assert(source->raw_size == 16u);
            assert(source->next_ref ==
                   ((uint16_t)source->raw[0] |
                    ((uint16_t)source->raw[1] << 8)));
            assert(monster->chested ==
                   (int16_t)((uint16_t)source->raw[2] |
                             ((uint16_t)source->raw[3] << 8)));
            assert(monster->type == source->raw[4]);
            assert(monster->position == source->raw[5]);
            for (unsigned int hi = 0; hi < 4u; ++hi)
                assert(monster->health[hi] ==
                       ((uint16_t)source->raw[6u + hi * 2u] |
                        ((uint16_t)source->raw[7u + hi * 2u] << 8)));
            assert(monster->flags_word ==
                   ((uint16_t)source->raw[14] |
                    ((uint16_t)source->raw[15] << 8)));
            assert(monster->direction_flags == source->raw[15]);
            assert(monster->unknown_word == 0u);
        }
        for (unsigned int i = 0; i < world->source_generator_count; ++i) {
            const Theron_V1_SourceGeneratorRecord *generator =
                &world->source_generators[i];
            assert(generator->dungeon_id == d + 1);
            assert(generator->level >= 0 &&
                   generator->level < THERON_MAX_LEVELS_PER_DUNGEON);
            assert(generator->x < THERON_MAX_MAP_SIZE);
            assert(generator->y < THERON_MAX_MAP_SIZE);
            assert(generator->type == TQ_ACT_FLOOR_MONSTER_GEN);
        }
        for (unsigned int i = 0; i < world->source_object_count; ++i) {
            const Theron_V1_SourceObjectRecord *object =
                &world->source_objects[i];
            const Theron_Track02SourceObjectOccurrence *source = NULL;
            for (unsigned int j = 0; j < result.source_object_count; ++j) {
                const Theron_Track02SourceObjectOccurrence *candidate =
                    &result.source_objects[j];
                if (candidate->source_ref == object->source_ref &&
                    candidate->source_index == object->source_index &&
                    candidate->category == object->category) {
                    source = candidate;
                    break;
                }
            }
            assert(source != NULL);
            assert(object->dungeon_id == d + 1);
            assert(object->level == (int)source->map);
            assert(object->x == (int)source->x);
            assert(object->y == (int)source->y);
            assert(object->source_ref == source->source_ref);
            assert(object->next_ref == source->next_ref);
            assert(object->source_index == source->source_index);
            assert(object->category == source->category);
            assert(object->position == source->position);
            assert(object->raw_size == source->raw_size);
            assert(memcmp(object->raw, source->raw, object->raw_size) == 0);
        }
        /* Champions and creatures are linked through actuators (champion
         * mirror type 127), not directly through ground refs. */
        assert(world->object_count == result.total_things_placed);
        carryable_not_first += census_real_carryable_not_first(world);
        nonfirst_take_roundtrips +=
            (unsigned int)assert_real_item_roundtrip(world);
        if (d == 0)
            assert_real_misc_roundtrip(world);
        assert_real_chests_are_not_itemrecords(
            world, result.source_category_counts[THERON_CAT_CHEST]);
        assert_real_doors_preserve_record_without_runtime_aliases(
            world, (unsigned int)result.doors_placed);
        {
            unsigned int chained = 0u;
            active_teleporters += assert_real_teleporters_preserve_map_state(
                world, (unsigned int)result.teleporters_placed, &chained);
            assert(assert_real_active_to_inactive_teleporter_links(world) ==
                   chained);
            active_to_inactive += chained;
        }
        assert_real_door_source_gate_survives_save(world);
        assert_real_teleporter_source_gate_survives_save(world);
        assert_real_pit_open_gate(world, &closed_pits, &open_pits);
        assert_real_control_records_have_no_fixture_mutation_aliases(
            world, (unsigned int)result.teleporters_placed,
            (unsigned int)result.actuators_placed);
        census_real_control_square_first_object_aliases(
            world, &door_aliases, &teleporter_aliases);
        census_real_actuator_types(
            world, floor_actuators, wall_actuators, actuator_effects,
            &local_effect_actuators, &reverted_actuators, &once_actuators,
            &high_actuator_values, &max_actuator_value,
            party_tiles, party_positions, party_effects,
            &party_once, &party_local_effects, &party_reverted,
            &party_with_teleporter, &party_with_open_teleporter,
            &party_reverted_set_shared_entry, &party_reverted_set_other);
        census_real_actuator_dispatch_context(
            world, local_multiples, remote_target_tiles,
            target_categories, targets_without_things,
            target_actuator_types);
        for (int oi = 0; oi < world->object_count; ++oi) {
            const Theron_V1_Object *object = &world->objects[oi];
            const uint8_t *raw_name = NULL;
            size_t raw_name_size = 0u;
            uint8_t type_code = 0u;
            if (!object->source_property_valid) continue;
            assert(theron_v1_world_object_item_name_raw(
                       world, object, &raw_name, &raw_name_size) == 1);
            assert(raw_name != NULL);
            assert(raw_name_size < THERON_TRACK02_ITEM_NAME_SOURCE_CAPACITY);
            assert(theron_v1_world_object_item_type_code(
                       world, object, &type_code) == 1);
            assert(type_code ==
                   world->track02_item_names[d]
                       .raw_type_codes[object->source_item_type]);
        }

        free(world);
    }
    printf("  US first-object aliases: doors=%u teleporters=%u\n",
           door_aliases, teleporter_aliases);
    printf("  US carryable records not first at square: %u\n",
           carryable_not_first);
    printf("  US active teleporters=%u active-to-inactive targets=%u\n",
           active_teleporters, active_to_inactive);
    printf("  US source pits: closed/passable=%u open/fail-closed=%u\n",
           closed_pits, open_pits);
    print_real_actuator_census(
        "US", floor_actuators, wall_actuators, actuator_effects,
        local_effect_actuators, reverted_actuators, once_actuators,
        high_actuator_values, max_actuator_value);
    print_real_party_actuator_context(
        "US", party_tiles, party_positions, party_effects,
        party_once, party_local_effects, party_reverted,
        party_with_teleporter, party_with_open_teleporter,
        party_reverted_set_shared_entry, party_reverted_set_other);
    print_real_actuator_dispatch_context(
        "US", local_multiples, remote_target_tiles,
        target_categories, targets_without_things,
        target_actuator_types);
    assert(nonfirst_take_roundtrips > 0u);
}

static void test_all_jp_dungeons(
    const uint8_t *ud, size_t ud_size,
    const uint8_t *track02, size_t track02_size) {
    static const size_t property_offsets[THERON_DUNGEON_COUNT] = {
        0x0990a2u, 0x0d9616u, 0x119d4du, 0x15955du,
        0x199eb1u, 0x1d91d9u, 0x219b13u
    };
    unsigned int door_aliases = 0u;
    unsigned int teleporter_aliases = 0u;
    unsigned int carryable_not_first = 0u;
    unsigned int nonfirst_take_roundtrips = 0u;
    unsigned int active_teleporters = 0u;
    unsigned int active_to_inactive = 0u;
    unsigned int closed_pits = 0u;
    unsigned int open_pits = 0u;
    unsigned int floor_actuators[128] = {0};
    unsigned int wall_actuators[128] = {0};
    unsigned int actuator_effects[8] = {0};
    unsigned int local_effect_actuators = 0u;
    unsigned int reverted_actuators = 0u;
    unsigned int once_actuators = 0u;
    unsigned int high_actuator_values = 0u;
    unsigned int max_actuator_value = 0u;
    unsigned int party_tiles[8] = {0};
    unsigned int party_positions[4] = {0};
    unsigned int party_effects[8] = {0};
    unsigned int party_once = 0u;
    unsigned int party_local_effects = 0u;
    unsigned int party_reverted = 0u;
    unsigned int party_with_teleporter = 0u;
    unsigned int party_with_open_teleporter = 0u;
    unsigned int party_reverted_set_shared_entry = 0u;
    unsigned int party_reverted_set_other = 0u;
    unsigned int local_multiples[4096] = {0};
    unsigned int remote_target_tiles[9] = {0};
    unsigned int target_categories[8][16] = {{0}};
    unsigned int targets_without_things[8] = {0};
    unsigned int target_actuator_types[8][128] = {{0}};
    for (int d = 0; d < 7; d++) {
        Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
        assert(world);
        theron_v1_world_init(world);
        world->current_dungeon = d + 1;
        bind_real_track02_party(
            world, track02, track02_size, THERON_TRACK02_MD5_JP_BIN);

        {
            Theron_Track02ItemNameSource name_source;
            assert(theron_v1_track02_decode_item_name_source(
                       ud, ud_size, 1, (unsigned int)d + 1u,
                       &name_source) == 1);
            assert(theron_v1_world_bind_track02_item_name_source(
                       world, &name_source, 1) == 1);
        }

        Theron_DungeonLoadResult result;
        assert(theron_v1_track02_load_full_dungeon_for_variant(
                   world, d + 1, ud, ud_size,
                   THERON_TRACK02_VARIANT_JP_BIN, &result) == 0);
        assert(result.levels_loaded > 0);
        assert(result.source_records_decoded > 0);
        assert(result.raw_only_item_refs == 0);
        assert(result.source_item_properties_bound > 0);
        assert((unsigned int)result.source_item_properties_bound ==
               (unsigned int)result.source_objects_materialized -
                   result.source_category_counts[THERON_CAT_CHEST]);
        assert(result.source_property_table_verified == 1);
        assert(result.source_property_table_offset == property_offsets[d]);
        assert(result.source_text_data_count == 0);
        assert(result.source_object_count ==
               (unsigned int)result.source_occurrences_decoded);
        for (int oi = 0; oi < world->object_count; ++oi) {
            const Theron_V1_Object *object = &world->objects[oi];
            const uint8_t *raw_name = NULL;
            size_t raw_name_size = 0u;
            uint8_t type_code = 0u;
            if (!object->source_property_valid) continue;
            assert(theron_v1_world_object_item_name_raw(
                       world, object, &raw_name, &raw_name_size) == 1);
            assert(raw_name != NULL);
            assert(raw_name_size < THERON_TRACK02_ITEM_NAME_SOURCE_CAPACITY);
            assert(theron_v1_world_object_item_type_code(
                       world, object, &type_code) == 1);
            assert(type_code ==
                   world->track02_item_names[d]
                       .raw_type_codes[object->source_item_type]);
        }
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        /* JP category-4 records must reach the same source-bound live pool as
         * US records.  This is static group materialization only; the
         * separate random-generator consumer remains fail-closed. */
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int ci = 0; ci < world->creature_count; ++ci) {
            const Theron_V1_Creature *creature = &world->creatures[ci];
            assert(creature->flags & THERON_CF_ACTIVE);
            assert(creature->source_ref != 0u);
            assert(creature->hp == creature->max_hp);
            assert(creature->primary_attack == THERON_ATTACK_NONE);
            assert(creature->ai == THERON_AI_UNAVAILABLE);
            assert(creature->secondary_attack == THERON_ATTACK_NONE);
        }
        carryable_not_first += census_real_carryable_not_first(world);
        nonfirst_take_roundtrips +=
            (unsigned int)assert_real_item_roundtrip(world);
        if (d == 0)
            assert_real_misc_roundtrip(world);
        assert_real_chests_are_not_itemrecords(
            world, result.source_category_counts[THERON_CAT_CHEST]);
        assert_real_doors_preserve_record_without_runtime_aliases(
            world, (unsigned int)result.doors_placed);
        {
            unsigned int chained = 0u;
            active_teleporters += assert_real_teleporters_preserve_map_state(
                world, (unsigned int)result.teleporters_placed, &chained);
            assert(assert_real_active_to_inactive_teleporter_links(world) ==
                   chained);
            active_to_inactive += chained;
        }
        assert_real_door_source_gate_survives_save(world);
        assert_real_teleporter_source_gate_survives_save(world);
        assert_real_pit_open_gate(world, &closed_pits, &open_pits);
        assert_real_control_records_have_no_fixture_mutation_aliases(
            world, (unsigned int)result.teleporters_placed,
            (unsigned int)result.actuators_placed);
        census_real_control_square_first_object_aliases(
            world, &door_aliases, &teleporter_aliases);
        census_real_actuator_types(
            world, floor_actuators, wall_actuators, actuator_effects,
            &local_effect_actuators, &reverted_actuators, &once_actuators,
            &high_actuator_values, &max_actuator_value,
            party_tiles, party_positions, party_effects,
            &party_once, &party_local_effects, &party_reverted,
            &party_with_teleporter, &party_with_open_teleporter,
            &party_reverted_set_shared_entry, &party_reverted_set_other);
        census_real_actuator_dispatch_context(
            world, local_multiples, remote_target_tiles,
            target_categories, targets_without_things,
            target_actuator_types);
        free(world);
    }
    printf("  JP first-object aliases: doors=%u teleporters=%u\n",
           door_aliases, teleporter_aliases);
    printf("  JP carryable records not first at square: %u\n",
           carryable_not_first);
    printf("  JP active teleporters=%u active-to-inactive targets=%u\n",
           active_teleporters, active_to_inactive);
    printf("  JP source pits: closed/passable=%u open/fail-closed=%u\n",
           closed_pits, open_pits);
    print_real_actuator_census(
        "JP", floor_actuators, wall_actuators, actuator_effects,
        local_effect_actuators, reverted_actuators, once_actuators,
        high_actuator_values, max_actuator_value);
    print_real_party_actuator_context(
        "JP", party_tiles, party_positions, party_effects,
        party_once, party_local_effects, party_reverted,
        party_with_teleporter, party_with_open_teleporter,
        party_reverted_set_shared_entry, party_reverted_set_other);
    print_real_actuator_dispatch_context(
        "JP", local_multiples, remote_target_tiles,
        target_categories, targets_without_things,
        target_actuator_types);
    assert(nonfirst_take_roundtrips > 0u);
    printf("  JP Track 02: all dungeon object records OK\n");
}

static void test_real_bank_reload_clears_stale_levels(
    const uint8_t *ud, size_t ud_size) {
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    Theron_DungeonData long_bank;
    Theron_DungeonData short_bank;

    assert(world);
    theron_v1_world_init(world);
    assert(theron_v1_track02_dungeon_map_load_for_variant(
               ud, ud_size, THERON_TRACK02_VARIANT_US_BIN, 1u,
               &long_bank));
    assert(long_bank.map_count == 8u);
    assert(theron_v1_world_load_track02_dungeon(world, 2, &long_bank) == 8);
    assert(world->level_loaded[1][7] == 1);

    assert(theron_v1_track02_dungeon_map_load_for_variant(
               ud, ud_size, THERON_TRACK02_VARIANT_US_BIN, 4u,
               &short_bank));
    assert(short_bank.map_count == 3u);
    assert(theron_v1_world_load_track02_dungeon(world, 2, &short_bank) == 3);
    for (unsigned int level = 0u; level < 3u; ++level)
        assert(world->level_loaded[1][level] == 1);
    for (unsigned int level = 3u; level < THERON_MAX_LEVELS_PER_DUNGEON;
         ++level)
        assert(world->level_loaded[1][level] == 0);

    free(world);
    printf("  US Track 02: real bank reload clears stale level records\n");
}

static unsigned int count_objects_in_dungeon(
    const Theron_V1_World *world, int dungeon_id) {
    unsigned int count = 0;
    for (int i = 0; i < world->object_count; ++i)
        if (world->objects[i].dungeon_id == dungeon_id) ++count;
    return count;
}

static void assert_object_ids_unique(const Theron_V1_World *world) {
    for (int i = 0; i < world->object_count; ++i) {
        assert(world->objects[i].id > 0);
        for (int j = i + 1; j < world->object_count; ++j)
            assert(world->objects[i].id != world->objects[j].id);
    }
}

static void test_real_source_ledgers_survive_other_dungeon_reload(
    const uint8_t *ud, size_t ud_size) {
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    Theron_DungeonLoadResult first;
    Theron_DungeonLoadResult second;
    Theron_DungeonLoadResult reloaded;
    unsigned int first_monsters;
    unsigned int first_generators;
    unsigned int first_source_objects;
    unsigned int first_placed_objects;
    unsigned int second_monsters;
    unsigned int second_generators;
    unsigned int second_source_objects;

    assert(world);
    theron_v1_world_init(world);
    world->current_dungeon = 1;
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 1, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &first) == 0);
    first_monsters = world->source_monster_count;
    first_generators = world->source_generator_count;
    first_source_objects = world->source_object_count;
    first_placed_objects = count_objects_in_dungeon(world, 1);
    assert(first_monsters > 0);
    assert(first_generators > 0);
    assert(first_source_objects > 0);
    assert(first_placed_objects == (unsigned int)first.total_things_placed);

    /* Loading dungeon 2 must replace only dungeon 2's records.  Dungeon 1
     * remains source-authenticated so a later Continue/return route does not
     * silently lose its monster, generator, or object consumers. */
    world->current_dungeon = 2;
    world->current_level = 0;
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 2, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &second) == 0);
    assert(world->source_monster_count > first_monsters);
    assert(world->source_generator_count > first_generators);
    assert(world->source_object_count > first_source_objects);
    assert(count_objects_in_dungeon(world, 1) == first_placed_objects);
    assert(count_objects_in_dungeon(world, 2) ==
           (unsigned int)second.total_things_placed);
    second_monsters = world->source_monster_count - first_monsters;
    second_generators = world->source_generator_count - first_generators;
    second_source_objects = world->source_object_count - first_source_objects;
    assert(second_monsters == second.source_category_counts[THERON_CAT_MONSTER]);
    assert(second_generators > 0);
    assert(second_source_objects == (unsigned int)second.source_object_count);
    for (unsigned int i = 0; i < world->source_monster_count; ++i)
        assert(world->source_monsters[i].dungeon_id == 1 ||
               world->source_monsters[i].dungeon_id == 2);
    for (unsigned int i = 0; i < world->source_generator_count; ++i)
        assert(world->source_generators[i].dungeon_id == 1 ||
               world->source_generators[i].dungeon_id == 2);
    for (unsigned int i = 0; i < world->source_object_count; ++i)
        assert(world->source_objects[i].dungeon_id == 1 ||
               world->source_objects[i].dungeon_id == 2);
    assert_object_ids_unique(world);

    /* A second load of the same dungeon must not duplicate its records or
     * objects, while the untouched dungeon-1 ledger remains unchanged. */
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 2, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &reloaded) == 0);
    assert(world->source_monster_count == first_monsters + second_monsters);
    assert(world->source_generator_count == first_generators + second_generators);
    assert(world->source_object_count == first_source_objects +
           second_source_objects);
    assert(count_objects_in_dungeon(world, 1) == first_placed_objects);
    assert(count_objects_in_dungeon(world, 2) ==
           (unsigned int)reloaded.total_things_placed);
    assert_object_ids_unique(world);

    free(world);
    printf("  US Track 02: dungeon-scoped source ledgers survive reload\n");
}

static void test_real_campaign_source_capacity(
    const uint8_t *ud, size_t ud_size) {
    static const unsigned int expected_generators[THERON_DUNGEON_COUNT] =
        {3u, 7u, 9u, 2u, 1u, 14u, 10u};
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    unsigned int expected_monsters = 0;
    unsigned int expected_generators_total = 0;
    unsigned int expected_source_objects = 0;
    unsigned int expected_placed_objects = 0;

    assert(world);
    theron_v1_world_init(world);
    for (int dungeon = 1; dungeon <= THERON_DUNGEON_COUNT; ++dungeon) {
        Theron_DungeonLoadResult result;
        unsigned int previous_monsters = world->source_monster_count;
        unsigned int previous_generators = world->source_generator_count;
        unsigned int previous_source_objects = world->source_object_count;

        world->current_dungeon = dungeon;
        world->current_level = 0;
        assert(theron_v1_track02_load_full_dungeon_for_variant(
                   world, dungeon, ud, ud_size,
                   THERON_TRACK02_VARIANT_US_BIN, &result) == 0);
        assert(world->source_monster_count > previous_monsters);
        assert(world->source_generator_count > previous_generators);
        assert(world->source_object_count > previous_source_objects);
        assert(world->source_monster_count - previous_monsters ==
               result.source_category_counts[THERON_CAT_MONSTER]);
        assert(world->source_generator_count - previous_generators ==
               expected_generators[dungeon - 1]);
        assert(world->source_object_count - previous_source_objects ==
               (unsigned int)result.source_object_count);
        assert(count_objects_in_dungeon(world, dungeon) ==
               (unsigned int)result.total_things_placed);

        expected_monsters += result.source_category_counts[THERON_CAT_MONSTER];
        expected_generators_total += expected_generators[dungeon - 1];
        expected_source_objects += (unsigned int)result.source_object_count;
        expected_placed_objects += (unsigned int)result.total_things_placed;
    }

    assert(world->source_monster_count == expected_monsters);
    assert(world->source_generator_count == expected_generators_total);
    assert(world->source_object_count == expected_source_objects);
    assert(world->object_count == (int)expected_placed_objects);
    assert(expected_monsters == 165u);
    assert(expected_generators_total == 46u);
    /* Category-4 records retain the generic item-list link before their
     * 14-byte dm_monster payload.  Three authentic follow-on objects depend
     * on those links in the US campaign. */
    assert(expected_source_objects == 2269u);
    assert(expected_placed_objects == 2189u);
    assert_object_ids_unique(world);
    /* Version 18 must carry the complete real seven-dungeon category-4
     * ledger, not merely fit the first dungeon used by the focused test. */
    assert_real_monster_ledger_roundtrip(world);
    {
        unsigned int ambiguous_source_identities = 0u;
        for (unsigned int i = 0u; i < world->source_object_count; ++i) {
            const Theron_V1_SourceObjectRecord *a = &world->source_objects[i];
            for (unsigned int j = i + 1u; j < world->source_object_count; ++j) {
                const Theron_V1_SourceObjectRecord *b = &world->source_objects[j];
                if (a->source_ref == b->source_ref &&
                    a->next_ref == b->next_ref &&
                    a->source_index == b->source_index &&
                    a->category == b->category &&
                    a->position == b->position &&
                    a->raw_size == b->raw_size &&
                    memcmp(a->raw, b->raw, a->raw_size) == 0 &&
                    (a->dungeon_id != b->dungeon_id || a->level != b->level ||
                     a->x != b->x || a->y != b->y)) {
                    ++ambiguous_source_identities;
                }
            }
        }
        printf("  US Track 02: %u source identities need origin coordinates\n",
               ambiguous_source_identities);
        assert(ambiguous_source_identities > 0u);
    }

    free(world);
    printf("  US Track 02: all seven dungeon ledgers fit and remain distinct\n");
}

static void test_generator_binding_rejects_non_source_records(void) {
    Theron_V1_World world;
    memset(&world, 0, sizeof(world));
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;

    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, 3, 4, TQ_ACT_FLOOR_MONSTER, 5,
               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == -1);
    assert(world.source_generator_count == 0);
    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, THERON_MAX_MAP_SIZE, 4,
               TQ_ACT_FLOOR_MONSTER_GEN, 5, 0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0) == -1);
    assert(world.source_generator_count == 0);
    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, 3, 4, TQ_ACT_FLOOR_MONSTER_GEN, 5,
               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 0);
    assert(world.source_generator_count == 1);
}

static void test_world_load_rejects_invalid_directory_envelope(void) {
    Theron_DungeonData invalid = {0};
    Theron_V1_World world;

    theron_v1_world_init(&world);
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;
    world.levels[0][0].width = 7;
    world.levels[0][0].height = 9;

    invalid.map_count = 1;
    invalid.maps[0].header.x_dim = THERON_MAX_MAP_SIZE;
    invalid.maps[0].header.y_dim = 0;
    assert(theron_v1_world_load_track02_dungeon(&world, 1, &invalid) == -1);
    assert(world.level_loaded[0][0] == 1);
    assert(world.levels[0][0].source_header_verified == 1);
    assert(world.levels[0][0].width == 7);
    assert(world.levels[0][0].height == 9);

    invalid.maps[0].header.x_dim = 0;
    invalid.map_count = THERON_MAX_LEVELS_PER_DUNGEON + 1u;
    assert(theron_v1_world_load_track02_dungeon(&world, 1, &invalid) == -1);
    assert(world.level_loaded[0][0] == 1);
}

static void test_object_binding_rejects_unverified_locations(void) {
    Theron_V1_World world;
    const uint8_t raw[2] = {0xFF, 0xFF};
    memset(&world, 0, sizeof(world));

    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0, 3, 4,
               raw, sizeof(raw)) == -1);
    assert(world.source_object_count == 0);
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;
    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0,
               THERON_MAX_MAP_SIZE, 4, raw, sizeof(raw)) == -1);
    assert(world.source_object_count == 0);
    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0, 3, 4,
               raw, sizeof(raw)) == 0);
    assert(world.source_object_count == 1);
}

static void test_authentic_coordinate_teleporter_without_endpoint(
    const uint8_t *ud, size_t ud_size,
    const uint8_t *track02, size_t track02_size) {
    Theron_V1_World world;
    Theron_DungeonLoadResult result;
    Theron_V1_Object *active;
    Theron_V1_Object *inactive;

    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.current_level = 0;
    assert(theron_v1_track02_load_full_dungeon(
               &world, 1, ud, ud_size, &result) == 0);
    bind_real_track02_party(
        &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);

    active = theron_v1_object_at_in_dungeon(&world, 1, 0, 0, 0);
    inactive = theron_v1_object_at_in_dungeon(&world, 1, 0, 2, 1);
    assert(active->type == THERON_OBJTYPE_TELEPORTER);
    assert(inactive->type == THERON_OBJTYPE_TELEPORTER);
    assert(world.levels[0][0].source_tiles[0][0] == 0xB8u);
    assert(world.levels[0][0].source_tiles[1][2] == 0xB4u);
    assert(active->state == 1u);
    assert(inactive->state == 0u);
    {
        static const uint8_t expected_actuator[8] = {
            0xfe, 0xff, 0x03, 0x00, 0xa4, 0x07, 0x80, 0x18
        };
        int found_actuator = 0;
        for (unsigned int i = 0; i < world.source_object_count; ++i) {
        const Theron_V1_SourceObjectRecord *source = &world.source_objects[i];
        if (source->level == 0 && source->x == 2 && source->y == 1 &&
            source->category == THERON_CAT_ACTUATOR) {
            assert(source->source_ref == 0x0c05u);
            assert(source->source_index == 5u);
            assert(source->raw_size == sizeof(expected_actuator));
            assert(memcmp(source->raw, expected_actuator,
                          sizeof(expected_actuator)) == 0);
            found_actuator = 1;
        }
        }
        assert(found_actuator);
    }

    /* Authentic US AKUTUBA M0 teleporter record 0 is at (0,0) and points to
     * (2,3) on M0.  (2,3) is a real floor square without a second object
     * record, so requiring an endpoint object would reject source data. */
    assert(theron_v1_teleporter_resolve(&world, 0, 0) == 0);
    assert(world.transition_pending == 1);
    assert(world.transition_target_level == 0);
    assert(world.transition_spawn_x == 2);
    assert(world.transition_spawn_y == 3);
    assert(theron_v1_transition_execute(&world) == 0);
    assert(world.current_level == 0);
    assert(world.party.leader_x == 2);
    assert(world.party.leader_y == 3);
    printf("  authentic Track 02 coordinate teleporter lands on floor OK\n");

    /* The original accepts a northward step from (2,2) onto the B4 pad but
     * leaves the party on (2,1), map 0.  Its clear OPEN bit therefore means
     * passable floor, not an active cross-map teleporter and not a wall. */
    world.party.leader_x = 2;
    world.party.leader_y = 2;
    world.party.leader_dir = 0;
    world.transition_pending = 0;
    assert(theron_v1_get_move_result(&world, 0) == THERON_MOVE_OK);
    assert(theron_v1_move_party(&world, 0) == THERON_MOVE_OK);
    assert(world.current_level == 0);
    assert(world.party.leader_x == 2);
    assert(world.party.leader_y == 1);
    assert(world.transition_pending == 0);
    assert(theron_v1_teleporter_resolve(&world, 2, 1) == -1);
    assert(world.source_actuator_event_count == 0u);
    {
        int exit_direction = -1;
        for (int direction = 0; direction < 4; ++direction) {
            if (theron_v1_get_move_result(&world, direction) ==
                    THERON_MOVE_OK) {
                exit_direction = direction;
                break;
            }
        }
        assert(exit_direction >= 0);
        assert(theron_v1_move_party(&world, exit_direction) == THERON_MOVE_OK);
    }
    assert(world.source_actuator_event_count == 0u);
    assert(world.party.leader_x != 2 || world.party.leader_y != 1);
    assert(theron_v1_world_queue_track02_party_events(
               &world, 0, 2, 1, 0, 1, 0) == 0);
    printf("  authentic Track 02 closed B4 teleporter remains passable floor OK\n");
    printf("  authentic reverted B4 party actuator remains silent on exit OK\n");

    {
        const Theron_V1_SourceObjectRecord *directional_source = NULL;
        Theron_Actuator directional_actuator;
        for (int dungeon = 1; dungeon <= 7 && !directional_source; ++dungeon) {
            if (dungeon != 1) {
                theron_v1_world_init(&world);
                world.current_dungeon = dungeon;
                world.current_level = 0;
                assert(theron_v1_track02_load_full_dungeon(
                           &world, dungeon, ud, ud_size, &result) == 0);
                bind_real_track02_party(
                    &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            }
            for (unsigned int i = 0; i < world.source_object_count; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_Actuator actuator;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &actuator) != 0 ||
                    actuator.type != TQ_ACT_FLOOR_PARTY ||
                    actuator.value < 1u || actuator.value > 4u ||
                    actuator.revert_effect ||
                    actuator.effect == TQ_ACT_EFFECT_HOLD ||
                    (world.levels[dungeon - 1][source->level]
                         .source_tiles[source->y][source->x] & 0xe0u) ==
                        THERON_TILE_WALL)
                    continue;
                directional_source = source;
                directional_actuator = actuator;
                break;
            }
        }
        assert(directional_source != NULL);
        {
            static const uint8_t expected_directional_raw[8] = {
                0xfe, 0xff, 0x83, 0x00, 0x80, 0x00, 0x80, 0x29
            };
            assert(directional_source->dungeon_id == 3 &&
                   directional_source->level == 2 &&
                   directional_source->x == 5 && directional_source->y == 0 &&
                   directional_source->source_ref == 0x0c5du &&
                   directional_source->source_index == 93u &&
                   memcmp(directional_source->raw, expected_directional_raw,
                          sizeof(expected_directional_raw)) == 0);
        }
        world.source_actuator_event_count = 0u;
        memset(world.source_actuator_events, 0,
               sizeof(world.source_actuator_events));
        world.current_level = directional_source->level;
        world.party.leader_x = directional_source->x;
        world.party.leader_y = directional_source->y;
        world.party.leader_dir =
            (int8_t)((directional_actuator.value + 2u) & 3u);
        const uint64_t directional_base_tick = world.world_tick;
        assert(theron_v1_turn_party(&world, 1) == 0);
        assert(world.party.leader_dir ==
               (int8_t)(directional_actuator.value - 1u));
        assert(world.source_actuator_event_count == 1u);
        assert(world.source_actuator_events[0].source_ref ==
               directional_source->source_ref);
        assert(world.source_actuator_events[0].source_index ==
               directional_source->source_index);
        assert(world.source_actuator_events[0].effect ==
               directional_actuator.effect);
        assert(world.source_actuator_events[0].due_tick ==
               directional_base_tick + 1u);
        assert(world.source_actuator_events[0].target_x == 6u &&
               world.source_actuator_events[0].target_y == 5u &&
               world.source_actuator_events[0].target_facing == 0u &&
               world.source_actuator_events[0].local_multiple == 0x0980u);
        printf("  authentic directional party actuator triggers after turn OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *stair_source = NULL;
        int approach_direction = -1;
        int approach_x = -1;
        int approach_y = -1;
        for (int dungeon = 1; dungeon <= 7 && !stair_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            world.current_level = 0;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !stair_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_Actuator actuator;
                Theron_ActuatorPartyEvent event;
                const Theron_V1_Level *level;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &actuator) != 0 ||
                    actuator.type != TQ_ACT_FLOOR_PARTY)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if ((level->source_tiles[source->y][source->x] >> 5) !=
                        THERON_TILE_STAIRS)
                    continue;
                for (int direction = 0; direction < 4; ++direction) {
                    int old_x = source->x - g_theron_dir_dx[direction];
                    int old_y = source->y - g_theron_dir_dy[direction];
                    int old_has_party_actuator = 0;
                    if (old_x < 0 || old_x >= level->width || old_y < 0 ||
                        old_y >= level->height)
                        continue;
                    for (unsigned int j = 0; j < world.source_object_count; ++j) {
                        const Theron_V1_SourceObjectRecord *old_source =
                            &world.source_objects[j];
                        Theron_Actuator old_actuator;
                        if (old_source->dungeon_id == dungeon &&
                            old_source->level == source->level &&
                            old_source->x == old_x && old_source->y == old_y &&
                            old_source->category == THERON_CAT_ACTUATOR &&
                            old_source->raw_size == 8u &&
                            theron_v1_track02_actuator_decode(
                                old_source->raw, &old_actuator) == 0 &&
                            old_actuator.type == TQ_ACT_FLOOR_PARTY) {
                            old_has_party_actuator = 1;
                            break;
                        }
                    }
                    if (old_has_party_actuator ||
                        theron_v1_track02_actuator_evaluate_party_event(
                            &actuator, 1, 0, world.party.champion_count,
                            (unsigned int)direction, &event) != 0 ||
                        !event.triggered)
                        continue;
                    world.current_level = source->level;
                    world.party.leader_x = old_x;
                    world.party.leader_y = old_y;
                    world.party.leader_dir = (int8_t)direction;
                    if (theron_v1_get_move_result(&world, direction) !=
                            THERON_MOVE_BLOCKED)
                        continue;
                    stair_source = source;
                    approach_direction = direction;
                    approach_x = old_x;
                    approach_y = old_y;
                    break;
                }
            }
        }
        assert(stair_source != NULL && approach_direction >= 0);
        {
            static const uint8_t expected_stair_raw[8] = {
                0x3a, 0x0c, 0x03, 0x00, 0x00, 0x00, 0xd0, 0x21
            };
            assert(stair_source->dungeon_id == 2 &&
                   stair_source->level == 4 && stair_source->x == 0 &&
                   stair_source->y == 6 &&
                   stair_source->source_ref == 0x0c0au &&
                   stair_source->source_index == 10u &&
                   memcmp(stair_source->raw, expected_stair_raw,
                          sizeof(expected_stair_raw)) == 0);
        }
        world.current_level = stair_source->level;
        world.party.leader_x = approach_x;
        world.party.leader_y = approach_y;
        world.party.leader_dir = (int8_t)approach_direction;
        world.source_actuator_event_count = 0u;
        memset(world.source_actuator_events, 0,
               sizeof(world.source_actuator_events));
        assert(theron_v1_get_move_result(&world, approach_direction) ==
               THERON_MOVE_BLOCKED);
        assert(theron_v1_move_party(&world, approach_direction) ==
               THERON_MOVE_BLOCKED);
        assert(world.current_level == stair_source->level &&
               world.party.leader_x == approach_x &&
               world.party.leader_y == approach_y &&
               world.transition_pending == 0u);
        assert(world.source_actuator_event_count == 0u);
        for (unsigned int i = 0; i < world.source_object_count; ++i) {
            const Theron_V1_SourceObjectRecord *target =
                &world.source_objects[i];
            assert(target->dungeon_id != stair_source->dungeon_id ||
                   target->level != stair_source->level || target->x != 8 ||
                   target->y != 1);
        }
        assert(world.source_square_state_count == 0u);
        {
            uint8_t runtime_tile_0 = 0u;
            uint8_t runtime_tile_1 = 0u;
            const Theron_V1_Level *source_level =
                &world.levels[stair_source->dungeon_id - 1]
                             [stair_source->level];
            assert((source_level->source_tiles[4][7] & 0x08u) != 0u &&
                   (source_level->source_tiles[1][9] & 0x08u) != 0u);
            assert(theron_v1_world_track02_runtime_tile(
                       &world, stair_source->dungeon_id,
                       stair_source->level, 7, 4, &runtime_tile_0) &&
                   theron_v1_world_track02_runtime_tile(
                       &world, stair_source->dungeon_id,
                       stair_source->level, 9, 1, &runtime_tile_1));
            assert((runtime_tile_0 & 0x08u) != 0u &&
                   (runtime_tile_1 & 0x08u) != 0u);
        }
        printf("  authentic stair party actuator is retained while "
               "unbound transition remains blocked: "
               "d=%d m=%d (%d,%d) ref=%04x index=%u raw=",
               stair_source->dungeon_id, stair_source->level,
               stair_source->x, stair_source->y, stair_source->source_ref,
               stair_source->source_index);
        for (unsigned int byte = 0; byte < stair_source->raw_size; ++byte)
            printf("%02x", stair_source->raw[byte]);
        printf(" OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        Theron_V1_Object *target = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        uint8_t initial_state = 0u;
        uint8_t expected_state = 0u;
        uint8_t source_tile_before = 0u;
        uint64_t due_tick = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate;
                const Theron_V1_Level *level;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect || source->level < 0 ||
                    source->level >= THERON_MAX_LEVELS_PER_DUNGEON)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height ||
                    (level->source_tiles[trigger.target_y]
                                        [trigger.target_x] >> 5) !=
                        THERON_TILE_TELEPORTER)
                    continue;
                target = NULL;
                for (int j = 0; j < world.object_count; ++j) {
                    Theron_V1_Object *candidate = &world.objects[j];
                    if (candidate->dungeon_id == dungeon &&
                        candidate->level == source->level &&
                        candidate->x == trigger.target_x &&
                        candidate->y == trigger.target_y &&
                        candidate->type == THERON_OBJTYPE_TELEPORTER &&
                        (candidate->flags & THERON_OBJ_F_TRACK02_COORD_LINK)) {
                        target = candidate;
                        break;
                    }
                }
                if (!target) continue;
                initial_state = target->state;
                expected_state = trigger.effect == TQ_ACT_EFFECT_SET ? 1u :
                    trigger.effect == TQ_ACT_EFFECT_CLEAR ? 0u :
                    trigger.effect == TQ_ACT_EFFECT_TOGGLE ?
                        (uint8_t)!initial_state : initial_state;
                if (expected_state == initial_state) continue;
                for (int direction = 0; direction < 4; ++direction) {
                    if (theron_v1_track02_actuator_evaluate_party_event(
                            &trigger, 1, 0, world.party.champion_count,
                            (unsigned int)direction, &gate) == 0 &&
                        gate.triggered) {
                        trigger_source = source;
                        trigger_direction = direction;
                        break;
                    }
                }
            }
        }
        assert(trigger_source != NULL && target != NULL &&
               trigger_direction >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xfe, 0xff, 0x03, 0x00, 0x40, 0x07, 0x40, 0x68
            };
            assert(trigger_source->dungeon_id == 4 &&
                   trigger_source->level == 1 && trigger_source->x == 0 &&
                   trigger_source->y == 21 &&
                   trigger_source->source_ref == 0x0cd6u &&
                   trigger_source->source_index == 214u &&
                   trigger.target_x == 1u && trigger.target_y == 13u &&
                   trigger.effect == TQ_ACT_EFFECT_SET &&
                   trigger.delay == 14u && initial_state == 0u &&
                   expected_state == 1u &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0);
        }
        source_tile_before = world.levels[trigger_source->dungeon_id - 1]
                                         [trigger_source->level]
                                         .source_tiles[trigger.target_y]
                                                      [trigger.target_x];
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, 1, 0,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        assert(due_tick >= world.world_tick);
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        assert(target->state == initial_state);
        theron_v1_world_tick(&world);
        assert(world.world_tick == due_tick);
        assert(target->state == expected_state);
        assert(world.levels[trigger_source->dungeon_id - 1]
                           [trigger_source->level]
                           .source_tiles[trigger.target_y][trigger.target_x] ==
               source_tile_before);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i)
            assert(world.source_actuator_events[i].source_ref !=
                       trigger_source->source_ref ||
                   world.source_actuator_events[i].source_index !=
                       trigger_source->source_index);
        printf("  authentic delayed teleporter SET dispatch preserves raw "
               "tile OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        Theron_V1_Object *door = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        uint64_t due_tick = 0u;
        uint8_t source_tile_before = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate;
                const Theron_V1_Level *level;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect ||
                    (trigger.effect != TQ_ACT_EFFECT_CLEAR &&
                     trigger.effect != TQ_ACT_EFFECT_TOGGLE))
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height ||
                    (level->source_tiles[trigger.target_y]
                                        [trigger.target_x] >> 5) !=
                        THERON_TILE_DOOR)
                    continue;
                door = NULL;
                for (int j = 0; j < world.object_count; ++j) {
                    Theron_V1_Object *candidate = &world.objects[j];
                    if (candidate->dungeon_id == dungeon &&
                        candidate->level == source->level &&
                        candidate->x == trigger.target_x &&
                        candidate->y == trigger.target_y &&
                        candidate->type == THERON_OBJTYPE_DOOR &&
                        candidate->source_category == THERON_CAT_DOOR) {
                        door = candidate;
                        break;
                    }
                }
                if (!door || door->state != THERON_DOOR_STATE_CLOSED)
                    continue;
                for (int direction = 0; direction < 4; ++direction) {
                    if (theron_v1_track02_actuator_evaluate_party_event(
                            &trigger, 1, 0, world.party.champion_count,
                            (unsigned int)direction, &gate) == 0 &&
                        gate.triggered) {
                        unsigned int triggered_at_source = 0u;
                        for (unsigned int k = 0;
                             k < world.source_object_count; ++k) {
                            const Theron_V1_SourceObjectRecord *peer =
                                &world.source_objects[k];
                            Theron_Actuator peer_actuator;
                            Theron_ActuatorPartyEvent peer_gate;
                            if (peer->dungeon_id != dungeon ||
                                peer->level != source->level ||
                                peer->x != source->x || peer->y != source->y ||
                                peer->category != THERON_CAT_ACTUATOR ||
                                peer->raw_size != 8u ||
                                theron_v1_track02_actuator_decode(
                                    peer->raw, &peer_actuator) != 0 ||
                                peer_actuator.type != TQ_ACT_FLOOR_PARTY ||
                                theron_v1_track02_actuator_evaluate_party_event(
                                    &peer_actuator, 1, 0,
                                    world.party.champion_count,
                                    (unsigned int)direction, &peer_gate) != 0)
                                continue;
                            triggered_at_source += peer_gate.triggered != 0u;
                        }
                        if (triggered_at_source != 1u) continue;
                        trigger_source = source;
                        trigger_direction = direction;
                        break;
                    }
                }
            }
        }
        assert(trigger_source != NULL && door != NULL &&
               trigger_direction >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xfe, 0xff, 0x03, 0x00, 0xd0, 0x00, 0x80, 0x18
            };
            assert(trigger_source->dungeon_id == 2 &&
                   trigger_source->level == 6 && trigger_source->x == 1 &&
                   trigger_source->y == 0 &&
                   trigger_source->source_ref == 0x0c04u &&
                   trigger_source->source_index == 4u &&
                   trigger.target_x == 2u && trigger.target_y == 3u &&
                   trigger.effect == TQ_ACT_EFFECT_TOGGLE &&
                   trigger.delay == 1u &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0);
        }
        source_tile_before = world.levels[trigger_source->dungeon_id - 1]
                                         [trigger_source->level]
                                         .source_tiles[trigger.target_y]
                                                      [trigger.target_x];
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, 1, 0,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        assert(door->state == THERON_DOOR_STATE_CLOSED);
        for (int frame = 1; frame <= 4; ++frame) {
            theron_v1_world_tick(&world);
            assert(door->state == frame);
        }
        assert(door->state == THERON_DOOR_STATE_OPEN);
        assert(world.levels[trigger_source->dungeon_id - 1]
                           [trigger_source->level]
                           .source_tiles[trigger.target_y][trigger.target_x] ==
               source_tile_before);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i)
            assert(world.source_actuator_events[i].source_ref !=
                       trigger_source->source_ref ||
                   world.source_actuator_events[i].source_index !=
                       trigger_source->source_index);
        printf("  authentic delayed door TOGGLE animates for four ticks "
               "and preserves raw tile OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        uint8_t source_tile = 0u;
        uint8_t expected_tile = 0u;
        uint8_t runtime_tile = 0u;
        uint64_t due_tick = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate;
                const Theron_V1_Level *level;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height)
                    continue;
                source_tile =
                    level->source_tiles[trigger.target_y][trigger.target_x];
                if ((source_tile >> 5) != THERON_TILE_FAKEWALL)
                    continue;
                expected_tile = source_tile;
                if (trigger.effect == TQ_ACT_EFFECT_SET)
                    expected_tile |= 0x04u;
                else if (trigger.effect == TQ_ACT_EFFECT_CLEAR)
                    expected_tile &= (uint8_t)~0x04u;
                else if (trigger.effect == TQ_ACT_EFFECT_TOGGLE)
                    expected_tile ^= 0x04u;
                if (expected_tile == source_tile) continue;
                for (int direction = 0; direction < 4; ++direction) {
                    if (theron_v1_track02_actuator_evaluate_party_event(
                            &trigger, 1, 0, world.party.champion_count,
                            (unsigned int)direction, &gate) == 0 &&
                        gate.triggered) {
                        trigger_source = source;
                        trigger_direction = direction;
                        break;
                    }
                }
            }
        }
        assert(trigger_source != NULL && trigger_direction >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xfe, 0xff, 0x03, 0x00, 0x90, 0x00, 0x40, 0x68
            };
            assert(trigger_source->dungeon_id == 4 &&
                   trigger_source->level == 2 && trigger_source->x == 0 &&
                   trigger_source->y == 1 &&
                   trigger_source->source_ref == 0x0c9bu &&
                   trigger_source->source_index == 155u &&
                   trigger.target_x == 1u && trigger.target_y == 13u &&
                   trigger.effect == TQ_ACT_EFFECT_TOGGLE &&
                   trigger.delay == 1u && source_tile == 0xc0u &&
                   expected_tile == 0xc4u &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0);
        }
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, 1, 0,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_tile(
                   &world, trigger_source->dungeon_id, trigger_source->level,
                   trigger.target_x, trigger.target_y, &runtime_tile) &&
               runtime_tile == source_tile);
        theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_tile(
                   &world, trigger_source->dungeon_id, trigger_source->level,
                   trigger.target_x, trigger.target_y, &runtime_tile) &&
               runtime_tile == expected_tile);
        assert(world.levels[trigger_source->dungeon_id - 1]
                           [trigger_source->level]
                           .source_tiles[trigger.target_y][trigger.target_x] ==
               source_tile);
        {
            const size_t save_size = theron_v1_world_serialize_size(&world);
            uint8_t *save = (uint8_t *)malloc(save_size);
            Theron_V1_World *restored =
                (Theron_V1_World *)malloc(sizeof(*restored));
            uint8_t restored_tile = 0u;
            assert(save != NULL && restored != NULL && save_size > 0u);
            assert(theron_v1_world_serialize(
                       &world, save, save_size) == save_size);
            *restored = world;
            restored->source_square_state_count = 0u;
            memset(restored->source_square_states, 0,
                   sizeof(restored->source_square_states));
            assert(theron_v1_world_deserialize(
                       restored, save, save_size) == 0);
            assert(theron_v1_world_track02_runtime_tile(
                       restored, trigger_source->dungeon_id,
                       trigger_source->level, trigger.target_x,
                       trigger.target_y, &restored_tile) &&
                   restored_tile == expected_tile);
            free(restored);
            free(save);
        }
        world.current_level = trigger_source->level;
        assert(theron_v1_world_get_square(
                   &world, trigger.target_x, trigger.target_y) ==
               ((expected_tile & 0x04u) ? THERON_SQUARE_FLOOR :
                                          THERON_SQUARE_SECRET));
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i)
            assert(world.source_actuator_events[i].source_ref !=
                       trigger_source->source_ref ||
                   world.source_actuator_events[i].source_index !=
                       trigger_source->source_index);
        assert(world.source_square_state_count == 1u);
        assert(theron_v1_track02_load_full_dungeon(
                   &world, 4, ud, ud_size, &result) == 0);
        assert(world.source_square_state_count == 0u);
        assert(theron_v1_world_track02_runtime_tile(
                   &world, 4, 2, 1, 13, &runtime_tile) &&
               runtime_tile == 0xc0u);
        printf("  authentic delayed fakewall TOGGLE opens passage and "
               "preserves raw tile across save/reload OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        uint8_t source_tile = 0u;
        uint8_t expected_tile = 0u;
        uint8_t runtime_tile = 0u;
        uint64_t due_tick = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate;
                const Theron_V1_Level *level;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height)
                    continue;
                source_tile =
                    level->source_tiles[trigger.target_y][trigger.target_x];
                if ((source_tile >> 5) != THERON_TILE_PIT)
                    continue;
                expected_tile = source_tile;
                if (trigger.effect == TQ_ACT_EFFECT_SET)
                    expected_tile |= 0x08u;
                else if (trigger.effect == TQ_ACT_EFFECT_CLEAR)
                    expected_tile &= (uint8_t)~0x08u;
                else if (trigger.effect == TQ_ACT_EFFECT_TOGGLE)
                    expected_tile ^= 0x08u;
                if (expected_tile == source_tile) continue;
                for (int direction = 0; direction < 4; ++direction) {
                    if (theron_v1_track02_actuator_evaluate_party_event(
                            &trigger, 1, 0, world.party.champion_count,
                            (unsigned int)direction, &gate) == 0 &&
                        gate.triggered) {
                        trigger_source = source;
                        trigger_direction = direction;
                        break;
                    }
                }
            }
        }
        assert(trigger_source != NULL && trigger_direction >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xeb, 0x0c, 0x03, 0x00, 0xc8, 0x07, 0x80, 0x80
            };
            assert(trigger_source->dungeon_id == 4 &&
                   trigger_source->level == 1 && trigger_source->x == 2 &&
                   trigger_source->y == 1 &&
                   trigger_source->source_ref == 0x0ceau &&
                   trigger_source->source_index == 234u &&
                   trigger.target_x == 2u && trigger.target_y == 16u &&
                   trigger.effect == TQ_ACT_EFFECT_CLEAR &&
                   trigger.delay == 15u && source_tile == 0x48u &&
                   expected_tile == 0x40u &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0);
        }
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, 1, 0,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_tile(
                   &world, trigger_source->dungeon_id, trigger_source->level,
                   trigger.target_x, trigger.target_y, &runtime_tile) &&
               runtime_tile == source_tile);
        theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_tile(
                   &world, trigger_source->dungeon_id, trigger_source->level,
                   trigger.target_x, trigger.target_y, &runtime_tile) &&
               runtime_tile == expected_tile);
        assert(world.levels[trigger_source->dungeon_id - 1]
                           [trigger_source->level]
                           .source_tiles[trigger.target_y][trigger.target_x] ==
               source_tile);
        world.current_level = trigger_source->level;
        {
            int approach_found = 0;
            const Theron_V1_Level *level =
                &world.levels[trigger_source->dungeon_id - 1]
                             [trigger_source->level];
            for (int direction = 0; direction < 4; ++direction) {
                const int old_x =
                    trigger.target_x - g_theron_dir_dx[direction];
                const int old_y =
                    trigger.target_y - g_theron_dir_dy[direction];
                if (old_x < 0 || old_x >= level->width || old_y < 0 ||
                    old_y >= level->height)
                    continue;
                world.party.leader_x = old_x;
                world.party.leader_y = old_y;
                if (theron_v1_get_move_result(&world, direction) ==
                        THERON_MOVE_OK) {
                    approach_found = 1;
                    break;
                }
            }
            assert(approach_found);
        }
        printf("  authentic delayed pit CLEAR closes pit, changes movement "
               "and preserves raw tile OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        const Theron_V1_SourceObjectRecord *text_target = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        int trigger_is_addition = -1;
        int trigger_party_square = -1;
        uint16_t source_word = 0u;
        uint16_t expected_word = 0u;
        uint16_t runtime_word = 0u;
        uint64_t due_tick = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate;
                const Theron_V1_Level *level;
                unsigned int text_count = 0u;
                int unsupported = 0;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height ||
                    (level->source_tiles[trigger.target_y]
                                        [trigger.target_x] >> 5) !=
                        THERON_TILE_OPEN)
                    continue;
                text_target = NULL;
                for (unsigned int j = 0;
                     j < world.source_object_count; ++j) {
                    const Theron_V1_SourceObjectRecord *target =
                        &world.source_objects[j];
                    Theron_Actuator target_actuator;
                    if (target->dungeon_id != dungeon ||
                        target->level != source->level ||
                        target->x != trigger.target_x ||
                        target->y != trigger.target_y)
                        continue;
                    if (target->category == THERON_CAT_TEXT) {
                        text_target = target;
                        ++text_count;
                    } else if (target->category == THERON_CAT_ACTUATOR &&
                               target->raw_size == 8u &&
                               theron_v1_track02_actuator_decode(
                                   target->raw, &target_actuator) == 0 &&
                               target_actuator.type ==
                                   TQ_ACT_FLOOR_MONSTER_GEN) {
                        unsupported = 1;
                    }
                }
                if (text_count != 1u || unsupported || !text_target ||
                    !theron_v1_world_track02_runtime_object_word(
                        &world, text_target, &source_word))
                    continue;
                for (int context = 0; context < 4 && !trigger_source;
                     ++context) {
                    const int is_addition = context == 0 || context == 3;
                    const int party_square = context >= 2;
                    for (int direction = 0; direction < 4; ++direction) {
                        if (theron_v1_track02_actuator_evaluate_party_event(
                                &trigger, is_addition, party_square,
                                world.party.champion_count,
                                (unsigned int)direction, &gate) != 0 ||
                            !gate.triggered)
                            continue;
                        expected_word = source_word;
                        if (gate.resolved_effect == TQ_ACT_EFFECT_SET)
                            expected_word |= 0x0001u;
                        else if (gate.resolved_effect == TQ_ACT_EFFECT_CLEAR)
                            expected_word &= (uint16_t)~0x0001u;
                        else if (gate.resolved_effect ==
                                 TQ_ACT_EFFECT_TOGGLE)
                            expected_word ^= 0x0001u;
                        trigger_source = source;
                        trigger_direction = direction;
                        trigger_is_addition = is_addition;
                        trigger_party_square = party_square;
                        break;
                    }
                }
            }
        }
        assert(trigger_source != NULL && text_target != NULL &&
               trigger_direction >= 0 && trigger_is_addition >= 0 &&
               trigger_party_square >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xfe, 0xff, 0x03, 0x00, 0x00, 0x00, 0x40, 0x48
            };
            static const uint8_t expected_text_raw[4] = {
                0xfe, 0xff, 0x09, 0x02
            };
            assert(trigger_source->dungeon_id == 4 &&
                   trigger_source->level == 1 && trigger_source->x == 6 &&
                   trigger_source->y == 11 &&
                   trigger_source->source_ref == 0x0c41u &&
                   trigger_source->source_index == 65u &&
                   trigger.target_x == 1u && trigger.target_y == 9u &&
                   trigger.effect == TQ_ACT_EFFECT_SET &&
                   trigger.delay == 0u && trigger_is_addition == 1 &&
                   trigger_party_square == 0 && source_word == 0x0209u &&
                   expected_word == source_word &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0 &&
                   text_target->source_ref == 0x0803u &&
                   text_target->source_index == 3u &&
                   memcmp(text_target->raw, expected_text_raw,
                          sizeof(expected_text_raw)) == 0);
        }
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, trigger_is_addition,
                   trigger_party_square,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_object_word(
                   &world, text_target, &runtime_word) &&
               runtime_word == source_word);
        theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_object_word(
                   &world, text_target, &runtime_word) &&
               runtime_word == expected_word);
        assert(((uint16_t)text_target->raw[2] |
                ((uint16_t)text_target->raw[3] << 8)) == source_word);
        assert(world.source_object_state_count == 0u);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i)
            assert(world.source_actuator_events[i].source_ref !=
                       trigger_source->source_ref ||
                   world.source_actuator_events[i].source_index !=
                       trigger_source->source_index);
        printf("  authentic corridor text SET is idempotent and preserves "
               "raw Visible bit OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *trigger_source = NULL;
        const Theron_V1_SourceObjectRecord *gate_target = NULL;
        Theron_Actuator trigger;
        int trigger_direction = -1;
        int trigger_is_addition = -1;
        int trigger_party_square = -1;
        uint16_t source_gate_word = 0u;
        uint16_t runtime_gate_word = 0u;
        uint64_t due_tick = 0u;
        for (int dungeon = 1; dungeon <= 7 && !trigger_source; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0;
                 i < world.source_object_count && !trigger_source; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_ActuatorPartyEvent gate_event;
                const Theron_V1_Level *level;
                unsigned int gate_count = 0u;
                int unsupported = 0;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &trigger) != 0 ||
                    trigger.type != TQ_ACT_FLOOR_PARTY ||
                    trigger.local_effect)
                    continue;
                level = &world.levels[dungeon - 1][source->level];
                if (trigger.target_x >= level->width ||
                    trigger.target_y >= level->height ||
                    (level->source_tiles[trigger.target_y]
                                        [trigger.target_x] >> 5) !=
                        THERON_TILE_WALL)
                    continue;
                gate_target = NULL;
                for (unsigned int j = 0;
                     j < world.source_object_count; ++j) {
                    const Theron_V1_SourceObjectRecord *target =
                        &world.source_objects[j];
                    Theron_Actuator target_actuator;
                    if (target->dungeon_id != dungeon ||
                        target->level != source->level ||
                        target->x != trigger.target_x ||
                        target->y != trigger.target_y)
                        continue;
                    if (target->category == THERON_CAT_TEXT &&
                        target->position == trigger.target_facing)
                        unsupported = 1;
                    if (target->category != THERON_CAT_ACTUATOR ||
                        target->raw_size != 8u ||
                        theron_v1_track02_actuator_decode(
                            target->raw, &target_actuator) != 0)
                        continue;
                    if (target_actuator.type == TQ_ACT_WALL_TRIGGER) {
                        gate_target = target;
                        ++gate_count;
                    } else if (target_actuator.type == 6u ||
                               target_actuator.type == 7u ||
                               target_actuator.type == 8u ||
                               target_actuator.type == 9u ||
                               target_actuator.type == 10u ||
                               target_actuator.type == 14u ||
                               target_actuator.type == 15u ||
                               target_actuator.type == 18u) {
                        unsupported = 1;
                    }
                }
                if (gate_count != 1u || unsupported || !gate_target ||
                    !theron_v1_world_track02_runtime_object_word(
                        &world, gate_target, &source_gate_word))
                    continue;
                for (int addition = 0; addition <= 1 && !trigger_source;
                     ++addition) {
                    for (int already = 0; already <= 1 && !trigger_source;
                         ++already) {
                        for (int direction = 0; direction < 4; ++direction) {
                            Theron_Actuator gate;
                            uint16_t data;
                            uint16_t bit;
                            int trigger_set;
                            int gate_triggers;
                            if (theron_v1_track02_actuator_evaluate_party_event(
                                    &trigger, addition, already,
                                    world.party.champion_count,
                                    (unsigned int)direction, &gate_event) != 0 ||
                                !gate_event.triggered ||
                                theron_v1_track02_actuator_decode(
                                    gate_target->raw, &gate) != 0)
                                continue;
                            data = (source_gate_word >> 7) & 0x01ffu;
                            bit = (uint16_t)(1u <<
                                (trigger.target_facing & 3u));
                            if (gate_event.resolved_effect ==
                                    TQ_ACT_EFFECT_TOGGLE)
                                data ^= bit;
                            else if (gate_event.resolved_effect ==
                                     TQ_ACT_EFFECT_SET)
                                data |= bit;
                            else if (gate_event.resolved_effect ==
                                     TQ_ACT_EFFECT_CLEAR)
                                data &= (uint16_t)~bit;
                            trigger_set =
                                ((data & 0x000fu) ==
                                 ((data & 0x00f0u) >> 4)) !=
                                gate.revert_effect;
                            gate_triggers = gate.effect ==
                                    TQ_ACT_EFFECT_HOLD || trigger_set;
                            if (!gate_triggers)
                                continue;
                            trigger_source = source;
                            trigger_direction = direction;
                            trigger_is_addition = addition;
                            trigger_party_square = already;
                            break;
                        }
                    }
                }
            }
        }
        assert(trigger_source != NULL && gate_target != NULL &&
               trigger_direction >= 0 && trigger_is_addition >= 0 &&
               trigger_party_square >= 0);
        {
            static const uint8_t expected_trigger_raw[8] = {
                0xa8, 0x0c, 0x03, 0x00, 0xd8, 0x10, 0x40, 0x19
            };
            static const uint8_t expected_gate_raw[8] = {
                0xfe, 0xff, 0x05, 0x00, 0x08, 0x00, 0x00, 0x32
            };
            assert(trigger_source->dungeon_id == 2 &&
                   trigger_source->level == 2 && trigger_source->x == 12 &&
                   trigger_source->y == 11 &&
                   trigger_source->source_ref == 0x0ca7u &&
                   trigger_source->source_index == 167u &&
                   trigger.target_x == 5u && trigger.target_y == 3u &&
                   trigger.target_facing == 0u &&
                   trigger.effect == TQ_ACT_EFFECT_HOLD &&
                   trigger.delay == 1u && trigger_is_addition == 0 &&
                   trigger_party_square == 0 && trigger_direction == 0 &&
                   source_gate_word == 0x0005u &&
                   memcmp(trigger_source->raw, expected_trigger_raw,
                          sizeof(expected_trigger_raw)) == 0 &&
                   gate_target->source_ref == 0xcc78u &&
                   gate_target->source_index == 120u &&
                   memcmp(gate_target->raw, expected_gate_raw,
                          sizeof(expected_gate_raw)) == 0);
        }
        assert(theron_v1_world_queue_track02_party_events(
                   &world, trigger_source->level, trigger_source->x,
                   trigger_source->y, trigger_is_addition,
                   trigger_party_square,
                   (unsigned int)trigger_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    trigger_source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    trigger_source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        theron_v1_world_tick(&world);
        assert(theron_v1_world_track02_runtime_object_word(
                   &world, gate_target, &runtime_gate_word));
        assert(runtime_gate_word == 0x0005u &&
               world.source_object_state_count == 0u &&
               world.source_actuator_event_count == 1u &&
               world.source_actuator_events[0].source_ref == 0xcc78u &&
               world.source_actuator_events[0].source_index == 120u &&
               world.source_actuator_events[0].target_x == 8u &&
               world.source_actuator_events[0].target_y == 6u &&
               world.source_actuator_events[0].target_facing == 0u &&
               world.source_actuator_events[0].effect ==
                   TQ_ACT_EFFECT_CLEAR &&
               world.source_actuator_events[0].delay == 0u &&
               world.source_actuator_events[0].due_tick == world.world_tick);
        {
            const size_t save_size = theron_v1_world_serialize_size(&world);
            uint8_t *save = (uint8_t *)malloc(save_size);
            Theron_V1_World *restored =
                (Theron_V1_World *)malloc(sizeof(*restored));
            uint16_t restored_word = 0u;
            assert(save != NULL && restored != NULL && save_size > 0u);
            assert(theron_v1_world_serialize(
                       &world, save, save_size) == save_size);
            *restored = world;
            restored->source_actuator_event_count = 0u;
            restored->source_object_state_count = 0u;
            assert(theron_v1_world_deserialize(
                       restored, save, save_size) == 0);
            assert(theron_v1_world_track02_runtime_object_word(
                       restored, gate_target, &restored_word) &&
                   restored_word == runtime_gate_word &&
                   restored->source_actuator_event_count == 1u &&
                   memcmp(&restored->source_actuator_events[0],
                          &world.source_actuator_events[0],
                          sizeof(world.source_actuator_events[0])) == 0);
            theron_v1_world_tick(&world);
            theron_v1_world_tick(restored);
            assert(restored->source_actuator_event_count ==
                       world.source_actuator_event_count &&
                   restored->source_actuator_event_count == 0u);
            free(restored);
            free(save);
        }
        printf("  authentic wall AND/OR gate dispatch and save/reload OK\n");
    }

    {
        const Theron_V1_SourceObjectRecord *source = NULL;
        const Theron_V1_SourceObjectRecord *launcher = NULL;
        Theron_Actuator actuator;
        int event_addition = -1;
        int event_already = -1;
        int event_direction = -1;
        uint64_t due_tick = 0u;
        static const uint8_t expected_source_raw[8] = {
            0xfe, 0xff, 0x03, 0x00, 0x04, 0x00, 0x80, 0x9a
        };
        static const uint8_t expected_launcher_raw[8] = {
            0x18, 0x98, 0x0f, 0x00, 0x84, 0x08, 0xf0, 0x0f
        };
        theron_v1_world_init(&world);
        world.current_dungeon = 5;
        assert(theron_v1_track02_load_full_dungeon(
                   &world, 5, ud, ud_size, &result) == 0);
        bind_real_track02_party(
            &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
        for (unsigned int i = 0; i < world.source_object_count; ++i) {
            const Theron_V1_SourceObjectRecord *record =
                &world.source_objects[i];
            if (record->source_ref == 0x0c59u &&
                record->source_index == 89u)
                source = record;
            if (record->source_ref == 0x4c65u &&
                record->source_index == 101u)
                launcher = record;
        }
        assert(source != NULL && launcher != NULL &&
               source->dungeon_id == 5 && source->level == 1 &&
               source->x == 15 && source->y == 11 &&
               launcher->x == 10 && launcher->y == 19 &&
               launcher->position == 1u &&
               memcmp(source->raw, expected_source_raw,
                      sizeof(expected_source_raw)) == 0 &&
               memcmp(launcher->raw, expected_launcher_raw,
                      sizeof(expected_launcher_raw)) == 0 &&
               theron_v1_track02_actuator_decode(
                   source->raw, &actuator) == 0 &&
               actuator.target_x == 10u && actuator.target_y == 19u &&
               actuator.target_facing == 0u);
        for (int addition = 0; addition <= 1 && event_addition < 0;
             ++addition) {
            for (int already = 0; already <= 1 && event_addition < 0;
                 ++already) {
                for (int direction = 0; direction < 4; ++direction) {
                    Theron_ActuatorPartyEvent event;
                    if (theron_v1_track02_actuator_evaluate_party_event(
                            &actuator, addition, already,
                            world.party.champion_count,
                            (unsigned int)direction, &event) == 0 &&
                        event.triggered) {
                        event_addition = addition;
                        event_already = already;
                        event_direction = direction;
                        break;
                    }
                }
            }
        }
        assert(event_addition >= 0 && event_already >= 0 &&
               event_direction >= 0);
        assert(theron_v1_world_queue_track02_party_events(
                   &world, source->level, source->x, source->y,
                   event_addition, event_already,
                   (unsigned int)event_direction) > 0);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i) {
            if (world.source_actuator_events[i].source_ref ==
                    source->source_ref &&
                world.source_actuator_events[i].source_index ==
                    source->source_index) {
                due_tick = world.source_actuator_events[i].due_tick;
                break;
            }
        }
        while (world.world_tick + 1u < due_tick)
            theron_v1_world_tick(&world);
        theron_v1_world_tick(&world);
        for (unsigned int i = 0;
             i < world.source_actuator_event_count; ++i)
            assert(world.source_actuator_events[i].source_ref !=
                       source->source_ref ||
                   world.source_actuator_events[i].source_index !=
                       source->source_index);
        assert(world.source_object_state_count == 0u);
        printf("  authentic mismatched-cell wall launcher is a no-op OK\n");
    }

    {
        unsigned int resolved_generator_events = 0u;
        for (int dungeon = 1; dungeon <= 7; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0; i < world.source_object_count; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_Actuator actuator;
                int targets_generator = 0;
                int queued = 0;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &actuator) != 0 ||
                    actuator.type != TQ_ACT_FLOOR_PARTY ||
                    actuator.local_effect)
                    continue;
                for (unsigned int g = 0; g < world.source_generator_count; ++g) {
                    const Theron_V1_SourceGeneratorRecord *generator =
                        &world.source_generators[g];
                    targets_generator |= generator->dungeon_id == dungeon &&
                        generator->level == source->level &&
                        generator->x == actuator.target_x &&
                        generator->y == actuator.target_y;
                }
                if (!targets_generator) continue;
                for (int addition = 0; addition <= 1 && !queued; ++addition) {
                    for (int already = 0; already <= 1 && !queued; ++already) {
                        for (int direction = 0; direction < 4; ++direction) {
                            Theron_ActuatorPartyEvent party_event;
                            if (theron_v1_track02_actuator_evaluate_party_event(
                                    &actuator, addition, already,
                                    world.party.champion_count,
                                    (unsigned int)direction,
                                    &party_event) == 0 &&
                                party_event.triggered) {
                                const unsigned int first_event =
                                    world.source_actuator_event_count;
                                assert(theron_v1_world_queue_track02_party_events(
                                           &world, source->level, source->x,
                                           source->y, addition, already,
                                           (unsigned int)direction) > 0);
                                for (unsigned int e = first_event;
                                     e < world.source_actuator_event_count; ++e) {
                                    unsigned int generator_index = 0u;
                                    const Theron_V1_SourceActuatorEvent *event =
                                        &world.source_actuator_events[e];
                                    const Theron_V1_SourceGeneratorRecord *generator;
                                    Theron_Actuator generator_actuator;
                                    Theron_ActuatorGeneratorPlan plan;
                                    Theron_V1_GeneratorExecutionWitness witness;
                                    Theron_V1_GeneratorMaterializationReceipt receipt;
                                    int plan_found = 0;
                                    if (event->source_ref != source->source_ref ||
                                        event->source_index != source->source_index)
                                        continue;
                                    assert(theron_v1_world_resolve_track02_generator_event(
                                               &world, event,
                                               &generator_index) == 1);
                                    generator =
                                        &world.source_generators[generator_index];
                                    assert(generator->dungeon_id == dungeon &&
                                           generator->level == source->level &&
                                           generator->x == actuator.target_x &&
                                           generator->y == actuator.target_y);
                                    for (unsigned int o = 0;
                                         o < world.source_object_count; ++o) {
                                        const Theron_V1_SourceObjectRecord *object =
                                            &world.source_objects[o];
                                        if (object->source_ref ==
                                                generator->source_ref &&
                                            object->source_index ==
                                                generator->source_index) {
                                            assert(theron_v1_track02_actuator_decode(
                                                       object->raw,
                                                       &generator_actuator) == 0);
                                            assert(theron_v1_track02_actuator_generator_plan(
                                                       &generator_actuator,
                                                       &plan) == 1);
                                            plan_found = 1;
                                            break;
                                        }
                                    }
                                    assert(plan_found &&
                                           plan.creature_type_value ==
                                               generator->value &&
                                           plan.toughness ==
                                               generator->generator_toughness &&
                                           plan.pause ==
                                               generator->generator_pause);
                                    memset(&witness, 0, sizeof(witness));
                                    memcpy(witness.event_raw, source->raw, 8u);
                                    for (unsigned int o = 0;
                                         o < world.source_object_count; ++o) {
                                        const Theron_V1_SourceObjectRecord *object =
                                            &world.source_objects[o];
                                        if (object->source_ref ==
                                                generator->source_ref &&
                                            object->source_index ==
                                                generator->source_index) {
                                            memcpy(witness.generator_raw,
                                                   object->raw, 8u);
                                            break;
                                        }
                                    }
                                    assert(theron_v1_world_bind_track02_generator_execution_witness(
                                               &world, event, &witness,
                                               &receipt) == 0);
                                    assert(receipt.event_identity_verified &&
                                           receipt.event_raw_verified &&
                                           receipt.generator_identity_verified &&
                                           receipt.generator_raw_verified &&
                                           !receipt.event_bound_rng_witness_verified &&
                                           !receipt.materialization_allowed &&
                                           receipt.generator_index == generator_index &&
                                           receipt.creature_type_value == plan.creature_type_value &&
                                           receipt.toughness == plan.toughness &&
                                           receipt.pause == plan.pause);
                                    if (source->dungeon_id == 2 &&
                                        source->source_ref == 0x0c81u) {
                                        static const uint8_t cc55_source[32] = {
                                            0xc4,0x46,0xd0,0x1e,0xda,0x20,0x67,0x46,
                                            0x29,0x02,0x18,0x69,0x00,0x8d,0x77,0x28,
                                            0xfa,0xa5,0x45,0x08,0xa9,0x03,0x28,0x30,
                                            0xe6,0xe4,0x45,0x90,0x02,0xd0,0xe0,0xa9
                                        };
                                        static const uint8_t before[3] =
                                            {0x2a,0x54,0x29};
                                        static const uint8_t after[3] =
                                            {0x98,0x8f,0x29};
                                        static const uint8_t runtime_record[10] = {
                                            0xa5,0x00,0x00,0x01,0x20,
                                            0xee,0x11,0x03,0x08,0x00
                                        };
                                        static const uint8_t position_consumer_source[27] = {
                                            0xad,0x3b,0x29,0x85,0xb5,0xad,0x3c,0x29,0x85,
                                            0xb6,0xad,0x3d,0x29,0x85,0xb4,0xa5,0xb4,0x18,
                                            0x69,0x02,0x29,0x03,0x85,0xbb,0x20,0xf8,0x51
                                        };
                                        static const uint32_t lifecycle_sequence[8] = {
                                            0u,2u,431u,467u,592u,795u,936u,966u
                                        };
                                        static const uint16_t lifecycle_pc[8] = {
                                            0x47b7u,0xc799u,0xca7eu,0xcaa3u,
                                            0xcbc9u,0xcbbeu,0xc9f3u,0xca78u
                                        };
                                        static const uint32_t lifecycle_physical_pc[8] = {
                                            0x000d07b7u,0x000de799u,0x000dea7eu,0x000deaa3u,
                                            0x000dabc9u,0x000dabbeu,0x000da9f3u,0x000daa78u
                                        };
                                        witness.authenticated_track02_execution = 1;
                                        witness.same_execution_window_verified = 1;
                                        witness.rng_return_boundary_verified = 1;
                                        witness.rng_caller_source_bytes_verified = 1;
                                        witness.generator_consumer_contract_verified = 1;
                                        witness.rng_entry_pc = 0x4667u;
                                        witness.rng_physical_entry_pc = 0x000d0667u;
                                        witness.rng_caller_pc = 0xcc55u;
                                        witness.rng_physical_caller_pc = 0x000d8c55u;
                                        witness.rng_return_value = 0x8fu;
                                        witness.rng_caller_source_size = 32u;
                                        memcpy(witness.rng_caller_source,
                                               cc55_source, sizeof(cc55_source));
                                        memcpy(witness.rng_state_before,
                                               before, sizeof(before));
                                        memcpy(witness.rng_state_after,
                                               after, sizeof(after));
                                        witness.successor_caller_pc = 0x4639u;
                                        witness.successor_physical_caller_pc =
                                            0x000d0639u;
                                        memcpy(witness.successor_state_before,
                                               after, sizeof(after));
                                        witness.runtime_materialization_verified = 1;
                                        witness.runtime_copy_pc = 0xcbceu;
                                        witness.runtime_physical_copy_pc =
                                            0x000dabceu;
                                        witness.runtime_slot = 0u;
                                        witness.runtime_record_address = 0x60ffu;
                                        memcpy(witness.runtime_record,
                                               runtime_record,
                                               sizeof(runtime_record));
                                        witness.runtime_first_consumer_verified = 1;
                                        witness.runtime_first_consumer_pc[0] = 0xc9f3u;
                                        witness.runtime_first_consumer_pc[1] = 0xc9fbu;
                                        witness.runtime_first_consumer_pc[2] = 0xca02u;
                                        witness.runtime_first_consumer_physical_pc[0] = 0x000da9f3u;
                                        witness.runtime_first_consumer_physical_pc[1] = 0x000da9fbu;
                                        witness.runtime_first_consumer_physical_pc[2] = 0x000daa02u;
                                        memcpy(witness.runtime_first_consumer_value,
                                               runtime_record, 3u);
                                        witness.runtime_unlink_verified = 1;
                                        witness.runtime_unlink_pc = 0xca78u;
                                        witness.runtime_unlink_physical_pc = 0x000daa78u;
                                        witness.runtime_lifecycle_window_verified = 1;
                                        memcpy(witness.runtime_lifecycle_sequence,
                                               lifecycle_sequence,
                                               sizeof(lifecycle_sequence));
                                        memcpy(witness.runtime_lifecycle_pc,
                                               lifecycle_pc, sizeof(lifecycle_pc));
                                        memcpy(witness.runtime_lifecycle_physical_pc,
                                               lifecycle_physical_pc,
                                               sizeof(lifecycle_physical_pc));
                                        witness.runtime_position_consumer_source_verified = 1;
                                        witness.runtime_position_consumer_pc = 0xc852u;
                                        witness.runtime_position_consumer_raw_offset = 0x000a1612u;
                                        witness.runtime_position_consumer_source_size = 27u;
                                        memcpy(witness.runtime_position_consumer_source,
                                               position_consumer_source,
                                               sizeof(position_consumer_source));
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 1);
                                        assert(receipt.materialization_allowed &&
                                               receipt.rng_return_value == 0x8fu &&
                                               receipt.creature_type_value == 12u &&
                                               !receipt.count_is_random &&
                                               receipt.fixed_count_minus_one == 0u &&
                                               receipt.toughness == 32u &&
                                               receipt.pause == 64u &&
                                               receipt.runtime_slot == 0u &&
                                               receipt.runtime_record_address == 0x60ffu &&
                                               memcmp(receipt.runtime_record,
                                                      runtime_record,
                                                      sizeof(runtime_record)) == 0 &&
                                               receipt.runtime_first_consumer_pc[0] == 0xc9f3u &&
                                               receipt.runtime_first_consumer_pc[1] == 0xc9fbu &&
                                               receipt.runtime_first_consumer_pc[2] == 0xca02u &&
                                               receipt.runtime_unlink_pc == 0xca78u &&
                                               receipt.runtime_unlink_physical_pc == 0x000daa78u &&
                                               receipt.runtime_lifecycle_window_verified &&
                                               memcmp(receipt.runtime_lifecycle_sequence,
                                                      lifecycle_sequence,
                                                      sizeof(lifecycle_sequence)) == 0 &&
                                               memcmp(receipt.runtime_lifecycle_pc,
                                                      lifecycle_pc,
                                                      sizeof(lifecycle_pc)) == 0 &&
                                               memcmp(receipt.runtime_lifecycle_physical_pc,
                                                      lifecycle_physical_pc,
                                                      sizeof(lifecycle_physical_pc)) == 0 &&
                                               receipt.runtime_position_fields_source_verified &&
                                               receipt.runtime_x == 17u &&
                                               receipt.runtime_y == 3u &&
                                               receipt.runtime_direction_raw == 8u &&
                                               receipt.runtime_direction == 0u);
                                        witness.successor_state_before[0] ^= 1u;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed);
                                        witness.successor_state_before[0] ^= 1u;
                                        witness.rng_caller_source[0] ^= 1u;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed);
                                        witness.rng_caller_source[0] ^= 1u;
                                        witness.runtime_record[4] ^= 1u;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed);
                                        witness.runtime_record[4] ^= 1u;
                                        witness.runtime_first_consumer_value[0] ^= 1u;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed);
                                        witness.runtime_first_consumer_value[0] ^= 1u;
                                        ++witness.runtime_lifecycle_sequence[4];
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed &&
                                               !receipt.runtime_lifecycle_window_verified);
                                        --witness.runtime_lifecycle_sequence[4];
                                        witness.runtime_position_consumer_source[0] ^= 1u;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed &&
                                               !receipt.runtime_position_fields_source_verified);
                                        witness.runtime_position_consumer_source[0] ^= 1u;
                                        ++witness.runtime_position_consumer_raw_offset;
                                        assert(theron_v1_world_bind_track02_generator_execution_witness(
                                                   &world, event, &witness,
                                                   &receipt) == 0);
                                        assert(!receipt.materialization_allowed);
                                        --witness.runtime_position_consumer_raw_offset;
                                    }
                                    witness.generator_raw[0] ^= 1u;
                                    assert(theron_v1_world_bind_track02_generator_execution_witness(
                                               &world, event, &witness,
                                               &receipt) == 0);
                                    assert(receipt.event_raw_verified &&
                                           !receipt.generator_raw_verified &&
                                           !receipt.materialization_allowed);
                                    ++resolved_generator_events;
                                    queued = 1;
                                    break;
                                }
                                break;
                            }
                        }
                    }
                }
                assert(queued);
            }
            for (unsigned int tick = 0; tick < 32u; ++tick)
                theron_v1_world_tick(&world);
            /* Identity and source plan are now resolved, but events remain
             * queued until their event-bound RNG witness is authenticated. */
            for (unsigned int e = 0;
                 e < world.source_actuator_event_count; ++e) {
                unsigned int generator_index = 0u;
                assert(theron_v1_world_resolve_track02_generator_event(
                           &world, &world.source_actuator_events[e],
                           &generator_index) == 1);
            }
            assert(world.creature_count == (int)expected_live_monsters(&world));
        }
        assert(resolved_generator_events == 7u);
        printf("  all 7 authentic generator events resolve to unique source plans OK\n");
    }

    {
        unsigned int local_events = 0u;
        for (int dungeon = 1; dungeon <= 7; ++dungeon) {
            theron_v1_world_init(&world);
            world.current_dungeon = dungeon;
            assert(theron_v1_track02_load_full_dungeon(
                       &world, dungeon, ud, ud_size, &result) == 0);
            bind_real_track02_party(
                &world, track02, track02_size, THERON_TRACK02_MD5_US_BIN);
            for (unsigned int i = 0; i < world.source_object_count; ++i) {
                const Theron_V1_SourceObjectRecord *source =
                    &world.source_objects[i];
                Theron_Actuator actuator;
                int already_queued = 0;
                if (source->category != THERON_CAT_ACTUATOR ||
                    source->raw_size != 8u ||
                    theron_v1_track02_actuator_decode(
                        source->raw, &actuator) != 0 ||
                    actuator.type != TQ_ACT_FLOOR_PARTY ||
                    !actuator.local_effect)
                    continue;
                assert(actuator.local_multiple == 176u);
                for (unsigned int j = 0;
                     j < world.source_actuator_event_count; ++j) {
                    if (world.source_actuator_events[j].source_ref ==
                            source->source_ref &&
                        world.source_actuator_events[j].source_index ==
                            source->source_index) {
                        already_queued = 1;
                        break;
                    }
                }
                if (!already_queued) {
                    int published = 0;
                    for (int addition = 0;
                         addition <= 1 && !published; ++addition) {
                        for (int already = 0;
                             already <= 1 && !published; ++already) {
                            for (int direction = 0; direction < 4;
                                 ++direction) {
                                Theron_ActuatorPartyEvent party_event;
                                if (theron_v1_track02_actuator_evaluate_party_event(
                                        &actuator, addition, already,
                                        world.party.champion_count,
                                        (unsigned int)direction,
                                        &party_event) == 0 &&
                                    party_event.triggered) {
                                    assert(theron_v1_world_queue_track02_party_events(
                                               &world, source->level,
                                               source->x, source->y,
                                               addition, already,
                                               (unsigned int)direction) > 0);
                                    published = 1;
                                    break;
                                }
                            }
                        }
                    }
                    assert(published);
                }
            }
            for (unsigned int i = 0;
                 i < world.source_actuator_event_count; ++i)
                local_events += world.source_actuator_events[i].local_effect;
            for (unsigned int tick = 0; tick < 16u; ++tick)
                theron_v1_world_tick(&world);
            for (unsigned int i = 0;
                 i < world.source_actuator_event_count; ++i)
                assert(!world.source_actuator_events[i].local_effect);
            assert(world.source_object_state_count == 0u);
        }
        assert(local_events == 12u);
        printf("  all 12 authentic local effect 176 events are no-ops OK\n");
    }
}

int main(void) {
    uint8_t *raw;
    size_t raw_size = 0u;
    printf("test_theron_v1_track02_dungeon_loader\n");

    test_generator_binding_rejects_non_source_records();
    test_world_load_rejects_invalid_directory_envelope();
    test_object_binding_rejects_unverified_locations();

    const char *path = find_track02();
    if (!path) {
        printf("  SKIP: Track 02 BIN not found\n");
        return 0;
    }
    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(path, &ud_size);
    if (!ud) {
        printf("  SKIP: could not load Track 02\n");
        return 0;
    }
    raw = load_raw_bytes(path, &raw_size);
    assert(raw != NULL);
    test_all_dungeons(ud, ud_size, raw, raw_size);
    test_real_item_name_sources(ud, ud_size, 2);
    test_real_sarmon_track19_mapping(ud, ud_size, 2);
    test_authentic_coordinate_teleporter_without_endpoint(
        ud, ud_size, raw, raw_size);
    test_real_bank_reload_clears_stale_levels(ud, ud_size);
    test_real_source_ledgers_survive_other_dungeon_reload(ud, ud_size);
    test_real_campaign_source_capacity(ud, ud_size);

    const char *jp_path = find_jp_track02();
    test_authenticated_world_spawn_binding(path, jp_path);
    free(raw);
    free(ud);
    if (jp_path) {
        size_t jp_ud_size = 0;
        uint8_t *jp_ud = load_track02_ud(jp_path, &jp_ud_size);
        if (jp_ud) {
            raw = load_raw_bytes(jp_path, &raw_size);
            assert(raw != NULL);
            test_all_jp_dungeons(jp_ud, jp_ud_size, raw, raw_size);
            test_real_item_name_sources(jp_ud, jp_ud_size, 1);
            test_real_sarmon_track19_mapping(jp_ud, jp_ud_size, 1);
            free(raw);
            free(jp_ud);
        }
    } else {
        printf("  SKIP: Japanese Track 02 BIN not found\n");
    }
    printf("PASS\n");
    return 0;
}
