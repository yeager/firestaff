#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def lines(text: str, start: int, end: int) -> str:
    return "\n".join(text.splitlines()[start - 1:end])


def require(segment: str, needle: str, cite: str) -> None:
    if needle not in segment:
        raise AssertionError(f"{cite} missing token: {needle}")


def require_all(text: str, start: int, end: int, needles: tuple[str, ...], cite: str) -> None:
    segment = lines(text, start, end)
    for needle in needles:
        require(segment, needle, cite)


def main() -> None:
    command_c = read(SOURCE_ROOT / "COMMAND.C")
    coord_c = read(SOURCE_ROOT / "COORD.C")
    input_c = read(SOURCE_ROOT / "INPUT.C")
    clikmenu_c = read(SOURCE_ROOT / "CLIKMENU.C")
    clikview_c = read(SOURCE_ROOT / "CLIKVIEW.C")

    fire_touch_matrix = read(ROOT / "src/shared/touch_click_zone_matrix_pc34_compat.c")
    fire_touch_pointer = read(ROOT / "src/shared/touch_pointer_input_pc34_compat.c")
    fire_test = read(ROOT / "tests/test_dm1_v1_dungeon_view_touch_route_pc34_compat.c")

    source_anchors = {
        "COMMAND.C:375-405": (
            command_c,
            375,
            405,
            (
                "G0447_as_Graphic561_PrimaryMouseInput_Interface",
                "G0448_as_Graphic561_SecondaryMouseInput_Movement",
                "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
                "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
                "MASK0x0002_MOUSE_LEFT_BUTTON",
                "MASK0x0001_MOUSE_RIGHT_BUTTON",
            ),
        ),
        "COMMAND.C:2787-2828 F0358_COMMAND_GetCommandFromMouseInput_CPSC": (
            command_c,
            2787,
            2828,
            (
                "F0358_COMMAND_GetCommandFromMouseInput_CPSC",
                "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button",
                "return L1107_Command",
            ),
        ),
        "COMMAND.C:2831-2928 F0359_COMMAND_ProcessClick_CPSC": (
            command_c,
            2831,
            2928,
            (
                "F0359_COMMAND_ProcessClick_CPSC",
                "G0436_B_PendingClickPresent",
                "G2247_CommandQueue",
                "F0358_COMMAND_GetCommandFromMouseInput_CPSC",
            ),
        ),
        "COMMAND.C:2931-3069 F0361_COMMAND_ProcessKeyPress": (
            command_c,
            2931,
            3069,
            (
                "F0361_COMMAND_ProcessKeyPress",
                "L1112_ps_KeyboardInput",
                "G2247_CommandQueue",
                "F0360_COMMAND_ProcessPendingClick",
            ),
        ),
        "COMMAND.C:2045-2156 F0380 turn/move dispatch": (
            command_c,
            2045,
            2156,
            (
                "F0380_COMMAND_ProcessQueue_CPSC",
                "F0365_COMMAND_ProcessTypes1To2_TurnParty",
                "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            ),
        ),
        "COMMAND.C:2296-2324 UI/viewport click dispatch": (
            command_c,
            2296,
            2324,
            (
                "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
                "C111_COMMAND_CLICK_IN_ACTION_AREA",
                "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
                "F0377_COMMAND_ProcessType80_ClickInDungeonView",
            ),
        ),
        "COORD.C:1915-1920 F0798_COMMAND_IsPointInZone": (
            coord_c,
            1915,
            1920,
            (
                "F0798_COMMAND_IsPointInZone",
                "P0750_i_X >= M704_ZONE_LEFT",
                "P0751_i_Y <= M707_ZONE_BOTTOM",
            ),
        ),
        "INPUT.C:574-664 mouse button forwarding": (
            input_c,
            574,
            664,
            (
                "IECLASS_RAWMOUSE",
                "MASK0x0002_MOUSE_LEFT_BUTTON",
                "MASK0x0001_MOUSE_RIGHT_BUTTON",
                "F0359_COMMAND_ProcessClick_CPSC",
            ),
        ),
        "CLIKMENU.C:142-330 turn/move consumers": (
            clikmenu_c,
            142,
            330,
            (
                "F0365_COMMAND_ProcessTypes1To2_TurnParty",
                "F0366_COMMAND_ProcessTypes3To6_MoveParty",
                "F0357_COMMAND_DiscardAllInput",
            ),
        ),
        "CLIKVIEW.C:311-510 C080 viewport click consumer": (
            clikview_c,
            311,
            510,
            (
                "F0377_COMMAND_ProcessType80_ClickInDungeonView",
                "P0752_i_X -= G2067_i_ViewportScreenX",
                "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor",
                "F0373_COMMAND_ProcessType80_ClickInDungeonView_GrabLeaderHandObject",
                "F0374_COMMAND_ProcessType80_ClickInDungeonView_DropLeaderHandObject",
            ),
        ),
    }

    for cite, (text, start, end, needles) in source_anchors.items():
        require_all(text, start, end, needles, cite)

    for token in (
        '"viewport.dungeon"',
        "COMMAND.C:403 maps C080 to C007_ZONE_VIEWPORT",
        "kPrimaryInterfaceSourceOrder",
        "kSecondaryMovementSourceOrder",
        "TOUCHCLICK_Compat_MapDungeonViewportLocalPointToDispatch",
    ):
        require(fire_touch_matrix, token, "src/shared/touch_click_zone_matrix_pc34_compat.c")

    for token in (
        "TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT",
        "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
        "touch bridge enqueues resolved mouse commands through DM1_V1_InputCommandQueue without changing keyboard routes",
        "N2 current line audit:",
    ):
        require(fire_touch_pointer, token, "src/shared/touch_pointer_input_pc34_compat.c")

    for token in (
        "TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT",
        "viewport.not_keyboard_move_gate",
        "keyboard.forward.movement_gate",
        "main_ui.action_parent.command",
        "viewport.right.command",
    ):
        require(fire_test, token, "tests/test_dm1_v1_dungeon_view_touch_route_pc34_compat.c")

    print("PASS_DM1_V1_DUNGEON_VIEW_TOUCH_ROUTE_SOURCE_LOCK")
    print("source_root=" + str(SOURCE_ROOT))
    for cite in source_anchors:
        print("source_anchor=" + cite)


if __name__ == "__main__":
    main()
