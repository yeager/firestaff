#include "dm1_v1_champion_live_runtime_composition_evidence_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int append_evidence(Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *receipt,
                           Dm1V1ChampionLiveRuntimeCompositionKindPc34 kind,
                           int champion, int zone, const uint8_t *pixels,
                           const uint8_t *portrait, const uint8_t *palette,
                           uint8_t *surface)
{
    Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 *evidence;
    if (!receipt || receipt->evidenceCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    evidence = &receipt->evidence[receipt->evidenceCount++];
    evidence->kind = kind;
    evidence->championIndex = champion;
    evidence->zoneId = zone;
    evidence->originalPixels = pixels;
    evidence->portraitPixels = portrait;
    evidence->originalPalette = palette;
    evidence->originalSurface = surface;
    return 1;
}

static int assets_and_bridge_proof_match(
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    const Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *bridge)
{
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof;
    if (!assets || !bridge || !assets->c008Accepted || !assets->c028Accepted ||
        !assets->assets.deadStatusBox.pixels || !assets->assets.championIcons.pixels ||
        assets->assets.deadStatusBox.graphicIndex != DM1_GFX_DEAD_CHAMPION ||
        assets->assets.deadStatusBox.width != 67 || assets->assets.deadStatusBox.height != 29 ||
        assets->assets.championIcons.graphicIndex != DM1_GFX_CHAMPION_ICONS ||
        assets->assets.championIcons.width != 76 || assets->assets.championIcons.height != 14)
        return 0;
    proof = &bridge->originalMaterials;
    return proof->c008Original && proof->c008Pixels == assets->assets.deadStatusBox.pixels &&
           proof->c008GraphicIndex == DM1_GFX_DEAD_CHAMPION && proof->c008Width == 67 &&
           proof->c008Height == 29 && proof->c028Original &&
           proof->c028Pixels == assets->assets.championIcons.pixels &&
           proof->c028GraphicIndex == DM1_GFX_CHAMPION_ICONS && proof->c028Width == 76 &&
           proof->c028Height == 14 && proof->indexedPaletteOriginal &&
           proof->indexedPalette && proof->indexedPaletteEntryCount >= 16 &&
           proof->indexedSurfaceOriginal && proof->indexedSurface;
}

static int status_bar_is_retained(
    const Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *statusBars,
    const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command)
{
    int i;
    if (!statusBars || !statusBars->valid || !statusBars->atomicPublish) return 0;
    for (i = 0; i < statusBars->operationCount; ++i) {
        const Dm1V1ChampionStatusBarFrameOpPc34 *bar = &statusBars->operations[i];
        if (bar->championIndex == command->source.statusBar.championIndex &&
            bar->zoneId == command->source.statusBar.zoneId &&
            bar->originalPalette == command->source.statusBar.originalPalette &&
            bar->originalIndexedSurface == command->source.statusBar.originalIndexedSurface) {
            return 1;
        }
    }
    return 0;
}

const char *dm1_v1_champion_live_runtime_composition_evidence_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292 selects C008 dead status or C028 live "
           "icon together with Champion portrait data; F0287 emits retained "
           "statusbar palette/surface writes. The live runtime receipt admits "
           "only pointer-identical original material, never fallback art.";
}

int dm1_v1_champion_live_runtime_composition_evidence_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *topRowAssets,
    const Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *statusBars,
    const Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *runtimeBridge,
    Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *outReceipt)
{
    Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 pending;
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!party || !runtimeBridge || !outReceipt || !runtimeBridge->valid ||
        runtimeBridge->clearOnly || runtimeBridge->action !=
        DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34 ||
        party->championCount < 0 || party->championCount > CHAMPION_MAX_PARTY ||
        !assets_and_bridge_proof_match(topRowAssets, runtimeBridge)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));

    for (i = 0; i < runtimeBridge->commandCount; ++i) {
        const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command =
            &runtimeBridge->commands[i];
        int champion = command->source.championIndex;
        if (champion < 0 || champion >= party->championCount ||
            !party->champions[champion].present) return 0;
        if (command->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34) {
            if (party->champions[champion].hp.current > 0 ||
                command->source.sourcePixels != topRowAssets->assets.deadStatusBox.pixels ||
                !append_evidence(&pending, DM1_V1_CHAMPION_LIVE_RUNTIME_C008_PC34,
                    champion, command->source.zoneId, command->source.sourcePixels,
                    NULL, NULL, NULL)) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34) {
            if (party->champions[champion].hp.current == 0 ||
                !party->champions[champion].portraitBitmapValid ||
                command->source.sourcePixels != topRowAssets->assets.championIcons.pixels ||
                command->source.portraitPixels != party->champions[champion].portraitBitmap ||
                !append_evidence(&pending, DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34,
                    champion, command->source.zoneId, command->source.sourcePixels,
                    command->source.portraitPixels, NULL, NULL)) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34) {
            if (!status_bar_is_retained(statusBars, command) ||
                !append_evidence(&pending, DM1_V1_CHAMPION_LIVE_RUNTIME_STATUS_BAR_PC34,
                    champion, command->source.statusBar.zoneId, NULL, NULL,
                    command->source.statusBar.originalPalette,
                    command->source.statusBar.originalIndexedSurface)) return 0;
            ++barCount;
        } else {
            return 0;
        }
    }
    if (sourceCount == 0 || barCount == 0) return 0;
    pending.tick = runtimeBridge->tick;
    pending.generation = runtimeBridge->generation;
    pending.originalMaterials = runtimeBridge->originalMaterials;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
