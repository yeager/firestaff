#include "dm2_v1_dungeon_input_owner.h"

#include <string.h>

#define DM2_V1_DUNGEON_INPUT_FMTOWNS_GRAPHICS_MD5 \
    "027ff3b8ddc2c4c4cdda7ada0b0bc46c"
#define DM2_V1_DUNGEON_INPUT_FMTOWNS_SKULL_MD5 \
    "0f4b44d286cbee35924a95e7d75ad7e5"
#define DM2_V1_DUNGEON_INPUT_FMTOWNS_MOUSE_TABLE_HASH 0x1500c4c9u

static uint32_t dm2_v1_fmtowns_mouse_table_hash(const uint8_t *bytes)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes) return 0u;
    for (i = 0u; i < DM2_V1_FMTOWNS_MOUSE_INPUT_TABLE_BYTES; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

/* The native SKULL.EXP contains the relocated MOUSE_INPUT records.  This
 * three-record run is the disassembled action-panel branch at file offset
 * 0x0c80: event 0x70/rect 0x003b, event 0x71/rect 0x003f, and
 * event 0x72/rect 0x0040.  Keep the check content-based because Phar Lap
 * extraction can move the executable inside a disc image. */
static int dm2_v1_fmtowns_skull_mouse_table_admitted(
    const DM2_V1_BootProfile *profile)
{
    if (!profile ||
        !profile->fmtowns_skull_mouse_input_anchor_verified ||
        profile->fmtowns_skull_mouse_input_table_hash !=
            DM2_V1_DUNGEON_INPUT_FMTOWNS_MOUSE_TABLE_HASH ||
        strcmp(profile->fmtowns_skull_md5,
               DM2_V1_DUNGEON_INPUT_FMTOWNS_SKULL_MD5) != 0) {
        return 0;
    }
    return dm2_v1_fmtowns_mouse_table_hash(
               profile->fmtowns_skull_mouse_input_table) ==
           DM2_V1_DUNGEON_INPUT_FMTOWNS_MOUSE_TABLE_HASH;
}

static uint16_t dm2_v1_fmtowns_read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

typedef struct {
    int16_t event_index;
    uint16_t rect_id;
} DM2_V1_FmtownsDungeonEventRect;

/* COMMAND.C/SKWIN's source MOUSE_INPUT entries.  The event numbers and
 * rect IDs are source facts; the coordinates are deliberately not copied
 * from the PC table. */
static const DM2_V1_FmtownsDungeonEventRect
    dm2_v1_fmtowns_event_rects[] = {
        { 80, 0x0007u },
        { 1, 0x0028u }, { 2, 0x0029u }, { 3, 0x002Au },
        { 4, 0x002Bu }, { 5, 0x002Cu }, { 6, 0x002Du },
        { 112, 0x003Bu },
        { 113, 0x003Fu }, { 114, 0x0040u }, { 115, 0x0041u },
        { 116, 0x004Au }, { 117, 0x0046u },
        { 118, 0x004Bu }, { 119, 0x0047u },
        { 120, 0x004Cu }, { 121, 0x0048u },
        { 122, 0x004Du }, { 123, 0x0049u }
    };

static uint32_t dm2_v1_fmtowns_table_hash(void)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < sizeof(dm2_v1_fmtowns_event_rects) /
                         sizeof(dm2_v1_fmtowns_event_rects[0]); ++i) {
        hash ^= (uint16_t)dm2_v1_fmtowns_event_rects[i].event_index;
        hash *= 16777619u;
        hash ^= dm2_v1_fmtowns_event_rects[i].rect_id;
        hash *= 16777619u;
    }
    return hash;
}

static int dm2_v1_rect_contains(const DM2_V1_InterfaceRect *rect,
                                int16_t x, int16_t y)
{
    return rect && rect->w > 0 && rect->h > 0 &&
           x >= rect->x && y >= rect->y &&
           x < rect->x + rect->w && y < rect->y + rect->h;
}

/* The relocated Towns table is shared by dungeon, champion, inventory,
 * status and dialogue branches.  A rectangle hit alone is not enough to
 * select a dungeon command: the same native rectangle may be reused by a
 * different branch.  Keep the runtime route on the source context owner and
 * leave records that have no dungeon context available only to a future
 * context-specific consumer. */
static int dm2_v1_fmtowns_candidate_has_dungeon_context(
    const DM2_V1_DungeonInputOwner *owner,
    const DM2_V1_FmtownsMouseInputCandidate *candidate)
{
    unsigned int context_count;
    unsigned int context_index;

    if (!owner || !candidate) return 0;
    context_count = DM2_TOUCHCLICK_Compat_GetSourceRecordContextCount(
        candidate->event_index, candidate->rect_id, candidate->button_mask);
    for (context_index = 0u; context_index < context_count; ++context_index) {
        Dm2TouchClickZonePc34Compat context;
        if (DM2_TOUCHCLICK_Compat_GetSourceRecordContextAt(
                candidate->event_index, candidate->rect_id,
                candidate->button_mask, context_index, &context) &&
            context.view == DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT)
            return 1;
    }
    return 0;
}

static Dm2TouchClickViewPc34Compat
dm2_v1_fmtowns_pc_context(DM2_V1_FmtownsUiContext context)
{
    switch (context) {
        case DM2_V1_FMTOWNS_UI_DUNGEON:
            return DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT;
        case DM2_V1_FMTOWNS_UI_INVENTORY:
            return DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT;
        case DM2_V1_FMTOWNS_UI_STATUS:
            return DM2_TOUCH_CLICK_VIEW_CHAMPION_PC34_COMPAT;
        case DM2_V1_FMTOWNS_UI_DIALOGUE:
            return DM2_TOUCH_CLICK_VIEW_DIALOG_PC34_COMPAT;
        default:
            return 0;
    }
}

static uint32_t dm2_v1_dungeon_input_table_hash(void)
{
    uint32_t hash = 2166136261u;
    Dm2TouchClickZonePc34Compat zone;
    unsigned int i;

    for (i = 0; i < DM2_TOUCHCLICK_Compat_GetViewZoneCount(
                    DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT); ++i) {
        if (!DM2_TOUCHCLICK_Compat_GetViewZone(
                DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, i, &zone))
            return 0u;
        const uint32_t values[] = {
            zone.commandId, zone.zoneIndex, zone.buttonMask,
            (uint16_t)zone.x, (uint16_t)zone.y,
            (uint16_t)zone.w, (uint16_t)zone.h
        };
        for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); ++j) {
            const uint8_t *bytes = (const uint8_t *)&values[j];
            for (size_t k = 0; k < sizeof(values[j]); ++k) {
                hash ^= bytes[k];
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

int dm2_v1_dungeon_input_owner_init(DM2_V1_DungeonInputOwner *owner,
                                    const char *graphics_md5)
{
    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    if (!graphics_md5 ||
        strcmp(graphics_md5, DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5) != 0)
        return 0;

    owner->source_table_hash = dm2_v1_dungeon_input_table_hash();
    if (owner->source_table_hash == 0u) return 0;
    memcpy(owner->graphics_md5, graphics_md5, sizeof(owner->graphics_md5));
    owner->active = 1;
    return 1;
}

int dm2_v1_dungeon_input_owner_init_fmtowns(
    DM2_V1_DungeonInputOwner *owner,
    const DM2_V1_BootProfile *profile)
{
    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    if (!profile || strcmp(profile->graphics_md5,
                           DM2_V1_DUNGEON_INPUT_FMTOWNS_GRAPHICS_MD5) != 0 ||
        !profile->graphics_dat ||
        !dm2_v1_fmtowns_skull_mouse_table_admitted(profile)) return 0;
    owner->active = 1;
    owner->fmtowns = 1;
    owner->boot_profile = profile;
    owner->source_table_hash = dm2_v1_fmtowns_table_hash();
    memcpy(owner->graphics_md5, profile->graphics_md5,
           sizeof(owner->graphics_md5));
    return owner->source_table_hash != 0u;
}

unsigned int dm2_v1_dungeon_input_owner_fmtowns_candidate_count(
    const DM2_V1_DungeonInputOwner *owner)
{
    if (!owner || !owner->active || !owner->fmtowns ||
        !dm2_v1_fmtowns_skull_mouse_table_admitted(owner->boot_profile))
        return 0u;
    return DM2_V1_FMTOWNS_MOUSE_INPUT_RECORD_COUNT;
}

int dm2_v1_dungeon_input_owner_fmtowns_candidate(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    DM2_V1_FmtownsMouseInputCandidate *out_candidate)
{
    const uint8_t *record;
    uint16_t event_word;

    if (!out_candidate ||
        ordinal >= dm2_v1_dungeon_input_owner_fmtowns_candidate_count(owner))
        return 0;
    record = owner->boot_profile->fmtowns_skull_mouse_input_table +
             ordinal * DM2_V1_FMTOWNS_MOUSE_INPUT_RECORD_BYTES;
    event_word = dm2_v1_fmtowns_read_le16(record);
    out_candidate->source_record_index = (uint16_t)ordinal;
    out_candidate->event_index = (uint16_t)(event_word & 0x7fffu);
    out_candidate->event_flags = (uint16_t)(event_word & 0x8000u);
    out_candidate->rect_id = dm2_v1_fmtowns_read_le16(record + 2u);
    out_candidate->button_mask = dm2_v1_fmtowns_read_le16(record + 4u);
    return 1;
}

int dm2_v1_dungeon_input_owner_fmtowns_candidate_context(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    Dm2TouchClickZonePc34Compat *out_context)
{
    DM2_V1_FmtownsMouseInputCandidate candidate;

    if (!out_context ||
        !dm2_v1_dungeon_input_owner_fmtowns_candidate(
            owner, ordinal, &candidate))
        return 0;
    return DM2_TOUCHCLICK_Compat_GetZoneBySourceRecord(
        candidate.event_index, candidate.rect_id, candidate.button_mask,
        0u, out_context);
}

unsigned int dm2_v1_dungeon_input_owner_fmtowns_candidate_context_count(
    const DM2_V1_DungeonInputOwner *owner, unsigned int ordinal)
{
    DM2_V1_FmtownsMouseInputCandidate candidate;
    if (!dm2_v1_dungeon_input_owner_fmtowns_candidate(
            owner, ordinal, &candidate))
        return 0u;
    return DM2_TOUCHCLICK_Compat_GetSourceRecordContextCount(
        candidate.event_index, candidate.rect_id, candidate.button_mask);
}

int dm2_v1_dungeon_input_owner_fmtowns_candidate_context_at(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    unsigned int context_ordinal,
    Dm2TouchClickZonePc34Compat *out_context)
{
    DM2_V1_FmtownsMouseInputCandidate candidate;
    if (!dm2_v1_dungeon_input_owner_fmtowns_candidate(
            owner, ordinal, &candidate))
        return 0;
    return DM2_TOUCHCLICK_Compat_GetSourceRecordContextAt(
        candidate.event_index, candidate.rect_id, candidate.button_mask,
        context_ordinal, out_context);
}

int dm2_v1_dungeon_input_owner_fmtowns_candidate_native_rect(
    const DM2_V1_DungeonInputOwner *owner,
    unsigned int ordinal,
    DM2_V1_BootExpandedRectReceipt *out_rect)
{
    DM2_V1_FmtownsMouseInputCandidate candidate;

    if (!out_rect) return 0;
    memset(out_rect, 0, sizeof(*out_rect));
    if (!dm2_v1_dungeon_input_owner_fmtowns_candidate(
            owner, ordinal, &candidate))
        return 0;
    return dm2_v1_boot_query_expanded_rect_receipt(
        owner->boot_profile, candidate.rect_id, out_rect);
}

int dm2_v1_dungeon_input_owner_fmtowns_route_context(
    const DM2_V1_DungeonInputOwner *owner,
    DM2_V1_FmtownsUiContext context,
    int16_t screen_x, int16_t screen_y, unsigned int button_mask,
    DM2_V1_FmtownsUiRouteReceipt *out_receipt)
{
    Dm2TouchClickViewPc34Compat source_view;
    unsigned int ordinal;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    source_view = dm2_v1_fmtowns_pc_context(context);
    if (!owner || !owner->active || !owner->fmtowns || !source_view ||
        button_mask == 0u ||
        !dm2_v1_fmtowns_skull_mouse_table_admitted(owner->boot_profile))
        return 0;

    for (ordinal = 0u;
         ordinal < dm2_v1_dungeon_input_owner_fmtowns_candidate_count(owner);
         ++ordinal) {
        DM2_V1_FmtownsMouseInputCandidate candidate;
        DM2_V1_BootExpandedRectReceipt native_rect;
        unsigned int context_count;
        unsigned int context_ordinal;

        if (!dm2_v1_dungeon_input_owner_fmtowns_candidate(
                owner, ordinal, &candidate) ||
            (candidate.button_mask & button_mask) == 0u)
            continue;
        context_count = dm2_v1_dungeon_input_owner_fmtowns_candidate_context_count(
            owner, ordinal);
        for (context_ordinal = 0u; context_ordinal < context_count;
             ++context_ordinal) {
            Dm2TouchClickZonePc34Compat source_context;
            memset(&source_context, 0, sizeof(source_context));
            if (!dm2_v1_dungeon_input_owner_fmtowns_candidate_context_at(
                    owner, ordinal, context_ordinal, &source_context) ||
                source_context.view != source_view)
                continue;
            memset(&native_rect, 0, sizeof(native_rect));
            if (!dm2_v1_boot_query_expanded_rect_receipt(
                    owner->boot_profile, candidate.rect_id, &native_rect) ||
                !native_rect.valid || !dm2_v1_rect_contains(
                    &native_rect.rect, (int16_t)(screen_x * 2),
                    (int16_t)(screen_y * 2)))
                continue;
            out_receipt->accepted = 1;
            out_receipt->context = context;
            out_receipt->candidate = candidate;
            out_receipt->source_context = source_context;
            out_receipt->native_rect = native_rect;
            out_receipt->source_table_hash = owner->source_table_hash;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_dungeon_input_owner_route(
    const DM2_V1_DungeonInputOwner *owner,
    int16_t screen_x,
    int16_t screen_y,
    unsigned int button_mask,
    DM2_V1_DungeonInputEventSink sink,
    void *sink_ctx,
    DM2_V1_DungeonInputReceipt *out_receipt)
{
    Dm2TouchClickZonePc34Compat zone;
    DM2_V1_DungeonInputReceipt receipt;
    DM2_V1_BootExpandedRectReceipt fmtowns_rect;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!owner || !owner->active ||
        (!owner->fmtowns && strcmp(owner->graphics_md5,
                                   DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5) != 0) ||
        (owner->fmtowns && strcmp(owner->graphics_md5,
                                  DM2_V1_DUNGEON_INPUT_FMTOWNS_GRAPHICS_MD5) != 0)) {
        receipt.blocked_unverified_graphics = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_table_hash = owner->source_table_hash;
    if (owner->fmtowns) {
        unsigned int ordinal;
        if (button_mask == 0u ||
            !dm2_v1_fmtowns_skull_mouse_table_admitted(owner->boot_profile)) {
            receipt.blocked_no_source_zone = 1;
            *out_receipt = receipt;
            return 0;
        }
        if ((button_mask & 0x0002u) != 0u) {
            size_t i;
            for (i = 0u; i < sizeof(dm2_v1_fmtowns_event_rects) /
                               sizeof(dm2_v1_fmtowns_event_rects[0]); ++i) {
                const DM2_V1_FmtownsDungeonEventRect *entry =
                    &dm2_v1_fmtowns_event_rects[i];
                /* The Towns INTERFACE_GENERAL table is 640x400 while M11's
                 * original presentation surface is 320x200. */
                if (!dm2_v1_boot_query_expanded_rect_receipt(
                        owner->boot_profile, entry->rect_id, &fmtowns_rect) ||
                    !fmtowns_rect.valid ||
                    !dm2_v1_rect_contains(&fmtowns_rect.rect,
                                          (int16_t)(screen_x * 2),
                                          (int16_t)(screen_y * 2)))
                    continue;
                receipt.accepted = 1;
                receipt.event_index = (uint16_t)entry->event_index;
                receipt.source_zone_index = (uint16_t)i;
                receipt.source_x = (int16_t)fmtowns_rect.rect.x;
                receipt.source_y = (int16_t)fmtowns_rect.rect.y;
                receipt.source_w = (int16_t)fmtowns_rect.rect.w;
                receipt.source_h = (int16_t)fmtowns_rect.rect.h;
                if (sink) sink(sink_ctx, entry->event_index,
                               screen_x, screen_y);
                *out_receipt = receipt;
                return 1;
            }
        }
        if ((button_mask & 0x0001u) == 0u) {
            receipt.blocked_no_source_zone = 1;
            *out_receipt = receipt;
            return 0;
        }
        /* The relocated SKULL.EXP table is the source hit-test order.  The
         * older static movement/action subset remains a source hash witness,
         * but must not be the runtime route: it omitted authentic right-button
         * and inventory/panel records. */
        for (ordinal = 0u;
             ordinal < DM2_V1_FMTOWNS_MOUSE_INPUT_RECORD_COUNT; ++ordinal) {
            DM2_V1_FmtownsMouseInputCandidate candidate;
            uint16_t source_button_mask;
            if (!dm2_v1_dungeon_input_owner_fmtowns_candidate(
                    owner, ordinal, &candidate))
                continue;
            if (!dm2_v1_fmtowns_candidate_has_dungeon_context(
                    owner, &candidate))
                continue;
            source_button_mask = candidate.button_mask;
            if ((source_button_mask & button_mask) == 0u)
                continue;
            /* The Towns INTERFACE_GENERAL table is 640x400 while M11's
             * original presentation surface is 320x200.  The source
             * renderer's 2:1 presentation contract converts host input into
             * the authenticated native coordinate space. */
            if (!dm2_v1_boot_query_expanded_rect_receipt(
                    owner->boot_profile, candidate.rect_id, &fmtowns_rect) ||
                !fmtowns_rect.valid ||
                !dm2_v1_rect_contains(&fmtowns_rect.rect,
                                      (int16_t)(screen_x * 2),
                                      (int16_t)(screen_y * 2)))
                continue;
            receipt.accepted = 1;
            receipt.event_index = candidate.event_index;
            receipt.source_zone_index = candidate.source_record_index;
            receipt.source_x = (int16_t)fmtowns_rect.rect.x;
            receipt.source_y = (int16_t)fmtowns_rect.rect.y;
            receipt.source_w = (int16_t)fmtowns_rect.rect.w;
            receipt.source_h = (int16_t)fmtowns_rect.rect.h;
            if (sink) sink(sink_ctx, candidate.event_index, screen_x, screen_y);
            *out_receipt = receipt;
            return 1;
        }
        receipt.blocked_no_source_zone = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (!DM2_TOUCHCLICK_Compat_HitTestInView(
            DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
            screen_x, screen_y, button_mask, &zone)) {
        receipt.blocked_no_source_zone = 1;
        *out_receipt = receipt;
        return 0;
    }

    receipt.accepted = 1;
    receipt.event_index = (uint16_t)zone.commandId;
    receipt.source_zone_index = (uint16_t)zone.zoneIndex;
    receipt.source_x = (int16_t)zone.x;
    receipt.source_y = (int16_t)zone.y;
    receipt.source_w = (int16_t)zone.w;
    receipt.source_h = (int16_t)zone.h;
    if (sink) sink(sink_ctx, (int16_t)zone.commandId, screen_x, screen_y);
    *out_receipt = receipt;
    return 1;
}
