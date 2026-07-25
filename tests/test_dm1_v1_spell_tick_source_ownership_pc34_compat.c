/*
 * A legacy Firestaff SPELL_TICK must never become a generic host spell.
 * ReDMCSB TIMELINE.C F0261 dispatches C71/C73/C74/C77/C78/C79 directly;
 * this regression proves that the compatibility queue accepts only those
 * authenticated source forms and gives them the same ownership as C13.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

struct SourceStatusCase { const char* label; int eventType; int defense; };

static void init_world(struct GameWorld_Compat* world)
{
    memset(world, 0, sizeof(*world));
    world->newPartyMapIndex = -1;
    world->gameTick = 100;
    world->partyMapIndex = 3;
    world->party.mapIndex = 3;
}

static void seed_case(struct GameWorld_Compat* world,
                      const struct SourceStatusCase* source)
{
    switch (source->eventType) {
    case DM1_EVENT_INVISIBILITY:
        world->magic.event71CountInvisibility = 1;
        world->lifecycle.status.invisibilityCount = 1;
        break;
    case DM1_EVENT_THIEVES_EYE:
        world->magic.event73CountThievesEye = 1;
        world->lifecycle.status.thievesEyeCount = 1;
        break;
    case DM1_EVENT_PARTY_SHIELD:
        world->magic.partyShieldDefense = source->defense;
        world->lifecycle.status.partyShieldDefense = source->defense;
        break;
    case DM1_EVENT_SPELLSHIELD:
        world->magic.spellShieldDefense = source->defense;
        world->lifecycle.status.partySpellShieldDefense = source->defense;
        break;
    case DM1_EVENT_FIRESHIELD:
        world->magic.fireShieldDefense = source->defense;
        world->lifecycle.status.partyFireShieldDefense = source->defense;
        break;
    case DM1_EVENT_FOOTPRINTS:
        world->magic.event79CountFootprints = 1;
        world->magic.magicFootprintsActive = 1;
        world->lifecycle.status.footprintsCount = 1;
        break;
    default:
        assert(!"unsupported source status case");
    }
}

static void assert_consumed(const struct GameWorld_Compat* world,
                            const struct SourceStatusCase* source)
{
    (void)world;
    switch (source->eventType) {
    case DM1_EVENT_INVISIBILITY:
        assert(world->magic.event71CountInvisibility == 0);
        assert(world->lifecycle.status.invisibilityCount == 0);
        break;
    case DM1_EVENT_THIEVES_EYE:
        assert(world->magic.event73CountThievesEye == 0);
        assert(world->lifecycle.status.thievesEyeCount == 0);
        break;
    case DM1_EVENT_PARTY_SHIELD:
        assert(world->magic.partyShieldDefense == 0);
        assert(world->lifecycle.status.partyShieldDefense == 0);
        break;
    case DM1_EVENT_SPELLSHIELD:
        assert(world->magic.spellShieldDefense == 0);
        assert(world->lifecycle.status.partySpellShieldDefense == 0);
        break;
    case DM1_EVENT_FIRESHIELD:
        assert(world->magic.fireShieldDefense == 0);
        assert(world->lifecycle.status.partyFireShieldDefense == 0);
        break;
    case DM1_EVENT_FOOTPRINTS:
        assert(world->magic.event79CountFootprints == 0);
        assert(!world->magic.magicFootprintsActive);
        assert(world->lifecycle.status.footprintsCount == 0);
        break;
    default:
        assert(!"unsupported source status case");
    }
}

static void dispatch_source_case(int kind, const struct SourceStatusCase* source)
{
    struct GameWorld_Compat world;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;

    init_world(&world);
    seed_case(&world, source);
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = kind;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 1; /* Original cast map differs from the party map. */
    event.aux0 = source->eventType;
    event.aux1 = source->defense;
    event.aux2 = source->eventType;

    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert(world.timeline.count == 0);
    assert_consumed(&world, source);
}

int main(void)
{
    static const struct SourceStatusCase cases[] = {
        { "C71 invisibility", DM1_EVENT_INVISIBILITY, 0 },
        { "C73 thieves eye", DM1_EVENT_THIEVES_EYE, 0 },
        { "C74 party shield", DM1_EVENT_PARTY_SHIELD, 12 },
        { "C77 spell shield", DM1_EVENT_SPELLSHIELD, 13 },
        { "C78 fire shield", DM1_EVENT_FIRESHIELD, 14 },
        { "C79 footprints", DM1_EVENT_FOOTPRINTS, 0 }
    };
    struct GameWorld_Compat world;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int i;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        dispatch_source_case(TIMELINE_EVENT_STATUS_TIMEOUT, &cases[i]);
        dispatch_source_case(TIMELINE_EVENT_SPELL_TICK, &cases[i]);
    }

    /* Unauthenticated legacy bytes are consumed without a host mutation. */
    init_world(&world);
    world.magic.fireShieldDefense = 17;
    world.lifecycle.status.partyFireShieldDefense = 17;
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SPELL_TICK;
    event.fireAtTick = world.gameTick;
    event.aux0 = 0x5a5a;
    event.aux1 = 99;
    event.aux2 = 0x5a5a;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert(world.magic.fireShieldDefense == 17);
    assert(world.lifecycle.status.partyFireShieldDefense == 17);

    puts("PASS dm1_v1_spell_tick_source_ownership_pc34_compat");
    return 0;
}
