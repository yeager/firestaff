#include <stdio.h>
#include "save_rest_toggle_consequences_pc34_compat.h"

static void print_result(const char* name, SaveRestToggleResultPc34Compat result) {
    printf("case=%s handled=%u toggleInventoryChampionIndex=%u closeInventory=%u processSaveGame=%u partyIsResting=%u drawDisabledMenus=%u drawRestScreen=%u primaryMouseInputPartyResting=%u primaryKeyboardInputPartyResting=%u discardAllInput=%u wakeUp=%u\n",
           name,
           result.handled,
           result.toggleInventoryChampionIndex,
           result.closeInventory,
           result.processSaveGame,
           result.partyIsResting,
           result.drawDisabledMenus,
           result.drawRestScreen,
           result.primaryMouseInputPartyResting,
           result.primaryKeyboardInputPartyResting,
           result.discardAllInput,
           result.wakeUp);
}

int main(void) {
    const unsigned int ok = save_rest_toggle_consequences_GetInvariant();
    const SaveRestToggleInputPc34Compat restWithInventory = { SAVE_REST_TOGGLE_PC34_COMMAND_REST, 4u, 0u, 2u };
    const SaveRestToggleInputPc34Compat restWithCandidate = { SAVE_REST_TOGGLE_PC34_COMMAND_REST, 4u, 1u, 2u };
    const SaveRestToggleInputPc34Compat saveAllowed = { SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME, 1u, 0u, 0u };
    const SaveRestToggleInputPc34Compat saveNoParty = { SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME, 0u, 0u, 0u };
    const SaveRestToggleInputPc34Compat wakeUp = { SAVE_REST_TOGGLE_PC34_COMMAND_WAKE_UP, 4u, 0u, 0u };

    printf("probe=firestaff_save_rest_toggle_consequences\n");
    printf("sourceEvidence=%s\n", save_rest_toggle_consequences_GetEvidence());
    print_result("rest_with_inventory", save_rest_toggle_consequences_Evaluate(restWithInventory));
    print_result("rest_with_candidate", save_rest_toggle_consequences_Evaluate(restWithCandidate));
    print_result("save_allowed", save_rest_toggle_consequences_Evaluate(saveAllowed));
    print_result("save_no_party", save_rest_toggle_consequences_Evaluate(saveNoParty));
    print_result("wake_up", save_rest_toggle_consequences_Evaluate(wakeUp));
    printf("saveRestToggleConsequencesInvariantOk=%u\n", ok);
    return ok ? 0 : 1;
}
