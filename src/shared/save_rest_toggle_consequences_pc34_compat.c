#include "save_rest_toggle_consequences_pc34_compat.h"

const char* save_rest_toggle_consequences_GetEvidence(void) {
    return "DEFS.H:332-335 defines save/rest/wake command ordinals; COMMAND.C:412-415 keeps save/rest/close-inventory active in inventory secondary mouse input; COMMAND.C:578-625 defines interface keyboard routes for inventory toggles/save/freeze; COMMAND.C:2180-2184 dispatches toggle/close inventory only when no candidate champion is open; COMMAND.C:2336-2359 handles rest only when no candidate champion is open, closes inventory first, draws disabled menus/rest screen, sets G0300_B_PartyIsResting, redraws viewport, switches primary input to party-resting tables, clears secondary input, and discards queued input; COMMAND.C:2361-2363 dispatches wake-up; COMMAND.C:2366-2370 saves only when party count > 0 and no candidate champion is open.";
}

SaveRestToggleResultPc34Compat save_rest_toggle_consequences_Evaluate(SaveRestToggleInputPc34Compat input) {
    SaveRestToggleResultPc34Compat result = {0};

    if ((input.command >= SAVE_REST_TOGGLE_PC34_COMMAND_TOGGLE_INVENTORY_CHAMPION_0) &&
        (input.command <= SAVE_REST_TOGGLE_PC34_COMMAND_CLOSE_INVENTORY)) {
        const unsigned int championIndex = input.command - SAVE_REST_TOGGLE_PC34_COMMAND_TOGGLE_INVENTORY_CHAMPION_0;
        result.handled = 1u;
        if (((championIndex == SAVE_REST_TOGGLE_PC34_CHAMPION_CLOSE_INVENTORY) ||
             (championIndex < input.partyChampionCount)) &&
            !input.candidateChampionOrdinal) {
            result.toggleInventoryChampionIndex = championIndex + 1u;
            result.closeInventory = (championIndex == SAVE_REST_TOGGLE_PC34_CHAMPION_CLOSE_INVENTORY) ? 1u : 0u;
        }
        return result;
    }

    if (input.command == SAVE_REST_TOGGLE_PC34_COMMAND_REST) {
        result.handled = 1u;
        if (!input.candidateChampionOrdinal) {
            if (input.inventoryChampionOrdinal) {
                result.closeInventory = 1u;
            }
            result.drawDisabledMenus = 1u;
            result.partyIsResting = 1u;
            result.drawRestScreen = 1u;
            result.drawViewportBeforeRestOrFreeze = 1u;
            result.waitForInputMaxVbl = 0u;
            result.primaryMouseInputPartyResting = 1u;
            result.primaryKeyboardInputPartyResting = 1u;
            result.discardAllInput = 1u;
        }
        return result;
    }

    if (input.command == SAVE_REST_TOGGLE_PC34_COMMAND_WAKE_UP) {
        result.handled = 1u;
        result.wakeUp = 1u;
        return result;
    }

    if (input.command == SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME) {
        result.handled = 1u;
        if ((input.partyChampionCount > 0u) && !input.candidateChampionOrdinal) {
            result.processSaveGame = 1u;
        }
        return result;
    }

    return result;
}

unsigned int save_rest_toggle_consequences_GetInvariant(void) {
    const SaveRestToggleInputPc34Compat restWithInventory = {
        SAVE_REST_TOGGLE_PC34_COMMAND_REST, 4u, 0u, 2u
    };
    const SaveRestToggleResultPc34Compat restWithInventoryResult =
        save_rest_toggle_consequences_Evaluate(restWithInventory);
    if (!restWithInventoryResult.handled || !restWithInventoryResult.closeInventory ||
        !restWithInventoryResult.drawDisabledMenus || !restWithInventoryResult.partyIsResting ||
        !restWithInventoryResult.drawRestScreen || !restWithInventoryResult.drawViewportBeforeRestOrFreeze ||
        !restWithInventoryResult.primaryMouseInputPartyResting ||
        !restWithInventoryResult.primaryKeyboardInputPartyResting || !restWithInventoryResult.discardAllInput) {
        return 0u;
    }

    {
        const SaveRestToggleInputPc34Compat restWithCandidate = {
            SAVE_REST_TOGGLE_PC34_COMMAND_REST, 4u, 1u, 2u
        };
        const SaveRestToggleResultPc34Compat restWithCandidateResult =
            save_rest_toggle_consequences_Evaluate(restWithCandidate);
        if (!restWithCandidateResult.handled || restWithCandidateResult.partyIsResting ||
            restWithCandidateResult.closeInventory || restWithCandidateResult.discardAllInput) {
            return 0u;
        }
    }

    {
        const SaveRestToggleInputPc34Compat saveAllowed = {
            SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME, 1u, 0u, 0u
        };
        const SaveRestToggleInputPc34Compat saveNoParty = {
            SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME, 0u, 0u, 0u
        };
        const SaveRestToggleInputPc34Compat saveCandidate = {
            SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME, 1u, 1u, 0u
        };
        if (!save_rest_toggle_consequences_Evaluate(saveAllowed).processSaveGame ||
            save_rest_toggle_consequences_Evaluate(saveNoParty).processSaveGame ||
            save_rest_toggle_consequences_Evaluate(saveCandidate).processSaveGame) {
            return 0u;
        }
    }

    {
        const SaveRestToggleInputPc34Compat toggleChampion1 = {
            SAVE_REST_TOGGLE_PC34_COMMAND_TOGGLE_INVENTORY_CHAMPION_0 + 1u, 2u, 0u, 0u
        };
        const SaveRestToggleInputPc34Compat toggleMissingChampion = {
            SAVE_REST_TOGGLE_PC34_COMMAND_TOGGLE_INVENTORY_CHAMPION_0 + 3u, 2u, 0u, 0u
        };
        const SaveRestToggleInputPc34Compat closeInventory = {
            SAVE_REST_TOGGLE_PC34_COMMAND_CLOSE_INVENTORY, 0u, 0u, 0u
        };
        if (save_rest_toggle_consequences_Evaluate(toggleChampion1).toggleInventoryChampionIndex != 2u ||
            save_rest_toggle_consequences_Evaluate(toggleMissingChampion).toggleInventoryChampionIndex != 0u ||
            !save_rest_toggle_consequences_Evaluate(closeInventory).closeInventory) {
            return 0u;
        }
    }

    return 1u;
}
