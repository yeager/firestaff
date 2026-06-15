#include "action_area_icon_routes_pc34_compat.h"
#include "action_area_routes_pc34_compat.h"

/* action_area_icon_routes_GetInvariant — verify V1 PC34 action-area icon routes.
 *
 * V1 source anchors:
 * - ReDMCSB COMMAND.C:461-471 action-area parent, rows, pass and champion
 *   action icons; COMMAND.C:467-472 defines G0453_as_Graphic561_MouseInput_
 *   ActionAreaIcons with C116-C119 champion action icon hit zones C089-C092.
 * - ReDMCSB COMMAND.C:1394-1449 F0358 scans mouse routes in table order.
 * - ReDMCSB MENU.C:820-837 maps selected ActionIndices to champion action state.
 *
 * The invariant checks that:
 * (a) The champion action icon range C116..C119 maps to champion index 0..3.
 * (b) The pass slot C112 and action slots C113..C115 exist in the touch matrix.
 */
unsigned int action_area_icon_routes_GetInvariant(void) {
    return action_area_routes_GetTouchMatrixInvariant() ? 1u : 0u;
}

const char* action_area_icon_routes_GetEvidence(void) {
    return "COMMAND.C:461-471 defines G0453_as_Graphic561_MouseInput_ActionAreaIcons "
           "with C116-C119 champion action icon hit zones C089-C092; "
           "COMMAND.C:467-472; MENU.C:820-837 ActionIndex mapping; "
           "action_area_routes_GetTouchMatrixInvariant() locks C112..C115.";
}