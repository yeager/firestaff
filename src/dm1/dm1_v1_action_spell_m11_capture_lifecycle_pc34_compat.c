#include "dm1_v1_action_spell_m11_capture_lifecycle_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int proof(int g, int z, int c, int n)
{
    return (g == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 && z == DM1_V1_ACTION_AREA_ZONE_ID_PC34 && c == 0 && n == 1) ||
           (g == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 && z == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
            c == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 && n == 2);
}

static int same(const DM1_V1_ActionSpellM11CaptureLifecycleStatePc34 *s,
                const DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *m)
{
    return s->graphicId == m->originalGraphicId && s->zoneId == m->originalZoneId &&
           s->companionGraphicId == m->companionGraphicId && s->assetCount == m->sourceAssetCount &&
           s->sourceTick == m->sourceTick && s->serial == m->serial &&
           s->commandFingerprint == m->commandFingerprint && s->orderingFingerprint == m->orderingFingerprint;
}

int dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11CaptureLifecycleStatePc34 *s,
    const DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *m,
    DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 *o)
{
    if (o) memset(o, 0, sizeof(*o));
    if (!s || !m || !m->accepted || !m->m11SourceFrameCurrent || !m->suppressSyntheticFallback ||
        m->sourceCommandCount <= 0 || m->frameTick == 0 || m->sourceTick == 0 || m->serial == 0 ||
        m->commandFingerprint == 0 || m->orderingFingerprint == 0 ||
        m->clearStaleSourceFrame != m->revokeStaleSourceFrame ||
        !proof(m->originalGraphicId, m->originalZoneId, m->companionGraphicId, m->sourceAssetCount)) return 0;
    if (m->revokeStaleSourceFrame && !proof(m->staleOriginalGraphicId, m->staleOriginalZoneId,
                                            m->staleCompanionGraphicId,
                                            m->staleOriginalGraphicId == 10 ? 1 : 2)) return 0;
    if (s->active && m->frameTick < s->frameTick) return 0;
    if (s->active && m->frameTick == s->frameTick && !same(s, m)) return 0;
    if (o) {
        o->accepted = 1; o->finalCaptureCurrent = 1;
        o->clearStaleCapture = s->active && m->frameTick > s->frameTick;
        o->revokeStaleCapture = o->clearStaleCapture;
        o->alreadyCurrent = s->active && m->frameTick == s->frameTick;
        o->presentationKind = m->presentationKind; o->graphicId = m->originalGraphicId;
        o->zoneId = m->originalZoneId; o->companionGraphicId = m->companionGraphicId;
        o->assetCount = m->sourceAssetCount; o->sourceCommandCount = m->sourceCommandCount;
        o->suppressSyntheticFallback = 1;
        if (o->clearStaleCapture) {
            o->staleGraphicId = s->graphicId; o->staleZoneId = s->zoneId;
            o->staleCompanionGraphicId = s->companionGraphicId;
        }
        o->frameTick = m->frameTick; o->sourceTick = m->sourceTick; o->serial = m->serial;
        o->commandFingerprint = m->commandFingerprint; o->orderingFingerprint = m->orderingFingerprint;
    }
    s->active = 1; s->graphicId = m->originalGraphicId; s->zoneId = m->originalZoneId;
    s->companionGraphicId = m->companionGraphicId; s->assetCount = m->sourceAssetCount;
    s->frameTick = m->frameTick; s->sourceTick = m->sourceTick; s->serial = m->serial;
    s->commandFingerprint = m->commandFingerprint; s->orderingFingerprint = m->orderingFingerprint;
    return 1;
}
