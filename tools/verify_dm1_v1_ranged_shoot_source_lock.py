#!/usr/bin/env python3
"""Verify DM1 V1 ranged SHOOT action remains source-locked.

This is deliberately a narrow gate for MENU.C C032_ACTION_SHOOT: source
class/ammo validation, kinetic/attack/step-energy derivation, champion-cell
launch placement, ammunition removal, and the PC34/I34E absence of the newer
projectile movement lock.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MENU = RED / "MENU.C"
CHAMPION = RED / "CHAMPION.C"
DUNGEON = RED / "DUNGEON.C"
GAME_VIEW = ROOT / "src/engine/m11_game_view.c"
PROBE = ROOT / "probes/m11/firestaff_m11_game_view_probe.c"


def lines(path: Path, start: int, end: int) -> str:
    return "\n".join(path.read_text(encoding="latin-1").splitlines()[start - 1:end])


def compact(s: str) -> str:
    return " ".join(s.split())


def require(cite: str, hay: str, needles: list[str]) -> None:
    h = compact(hay)
    for needle in needles:
        if compact(needle) not in h:
            raise AssertionError(f"{cite}: missing {needle!r}")


def require_re(cite: str, hay: str, pattern: str) -> None:
    if not re.search(pattern, hay, re.S):
        raise AssertionError(f"{cite}: missing pattern {pattern!r}")


def main() -> int:
    citations: list[str] = []

    require("MENU.C:1363-1396", lines(MENU, 1363, 1396), [
        "case C032_ACTION_SHOOT",
        "M012_TYPE(L1247_ps_Champion->Slots[C00_SLOT_READY_HAND]) != C05_THING_TYPE_WEAPON",
        "L1256_ps_WeaponInfoActionHand = &G0238_as_Graphic559_WeaponInfo[L1248_ps_Weapon->Type]",
        "L1257_ps_WeaponInfoReadyHand = F0158_DUNGEON_GetWeaponInfo(L1247_ps_Champion->Slots[C00_SLOT_READY_HAND])",
        "AL1246_i_ActionHandWeaponClass = L1256_ps_WeaponInfoActionHand->Class",
        "AL1250_i_ReadyHandWeaponClass = L1257_ps_WeaponInfoReadyHand->Class",
        "C016_CLASS_FIRST_BOW",
        "C010_CLASS_BOW_AMMUNITION",
        "C032_CLASS_FIRST_SLING",
        "C011_CLASS_SLING_AMMUNITION",
        "G0513_i_ActionDamage = CM2_DAMAGE_NO_AMMUNITION",
        "F0300_CHAMPION_GetObjectRemovedFromSlot(P0787_ui_ChampionIndex, C00_SLOT_READY_HAND)",
        "F0326_CHAMPION_ShootProjectile",
        "L1256_ps_WeaponInfoActionHand->KineticEnergy + L1257_ps_WeaponInfoReadyHand->KineticEnergy",
        "M065_SHOOT_ATTACK(L1256_ps_WeaponInfoActionHand->Attributes) + F0303_CHAMPION_GetSkillLevel(P0787_ui_ChampionIndex, C11_SKILL_SHOOT)",
        "AL1246_i_StepEnergy",
    ])
    citations.append("MENU.C:1363-1396 C032_ACTION_SHOOT validates bow/sling ammo, removes ready-hand ammo, and calls F0326")

    require("CHAMPION.C:2051-2071", lines(CHAMPION, 2051, 2071), [
        "void F0326_CHAMPION_ShootProjectile",
        "L0990_ui_Direction = P0674_ps_Champion->Direction",
        "F0212_PROJECTILE_Create(P0675_T_Thing, G0306_i_PartyMapX, G0307_i_PartyMapY",
        "M021_NORMALIZE((((P0674_ps_Champion->Cell - L0990_ui_Direction + 1) & 0x0002) >> 1) + L0990_ui_Direction)",
        "BUG0_46 You can run into a projectile shot by a champion",
        "G0311_i_ProjectileDisabledMovementTicks = 4",
    ])
    citations.append("CHAMPION.C:2051-2071 F0326 launch cell/direction and BUG0_46 PC34 no-lock note")

    require("DUNGEON.C:261-308", lines(DUNGEON, 261, 308), [
        "WEAPON_INFO G0238_as_Graphic559_WeaponInfo[46]",
        "{  10,  20,   1,  50, 0x2032 },   /* BOW */",
        "{  28,  30,   1, 180, 0x2078 },   /* CROSSBOW */",
        "{   2,  10,   2,  10, 0x0100 },   /* ARROW */",
        "{   2,  10,   2,  28, 0x0500 },   /* SLAYER */",
        "{  19,  39,   5,  20, 0x2032 },   /* SLING */",
        "{  10,  11,   6,  18, 0x2000 },   /* ROCK */",
        "{  30,  26,   1, 220, 0x207D },   /* SPEEDBOW */",
    ])
    citations.append("DUNGEON.C:261-308 G0238 weapon class/kinetic/attribute table")

    require("MENU.C action tables", lines(MENU, 157, 465), [
        "14,  /* SHOOT */",
        "3,   /* SHOOT */",
        "11,  /* SHOOT */",
        "20,  /* SHOOT Atari ST Versions",
    ])
    citations.append("MENU.C:157-465 SHOOT disabled ticks/stamina/skill/experience table entries")

    game = GAME_VIEW.read_text(encoding="utf-8")
    probe = PROBE.read_text(encoding="utf-8")

    require("m11_game_view.c source table", game, [
        "ReDMCSB DUNGEON.C:261-308 G0238_as_Graphic559_WeaponInfo",
        "{ 20,  50,  50}, /* BOW */",
        "{ 30, 180, 120}, /* CROSSBOW */",
        "{ 10,  10,   0}, /* ARROW */",
        "{ 39,  20,  50}, /* SLING */",
        "{ 11,  18,   0}, /* ROCK */",
        "{ 26, 220, 125}, /* SPEEDBOW */",
    ])
    require("m11_game_view.c SHOOT path", game, [
        "case 32: { /* SHOOT */",
        "ReDMCSB MENU.C:1363-1396 validates C01 action-hand",
        "m11_dm1_weapon_info_for_thing(state, actionThing)",
        "m11_dm1_weapon_info_for_thing(state, readyThing)",
        "readyClass != 10",
        "readyClass != 11",
        "m11_dm1_shoot_step_energy(actionClass, &stepEnergy)",
        "m11_dm1_shoot_skill_level(state, championIndex)",
        "shootAttack = ((int)actionInfo->shootAttack + skillShoot) << 1",
        "kineticEnergy = (int)actionInfo->kineticEnergy +",
        "m11_dm1_projectile_launch_cell(championIndex & 3,",
        "m11_spawn_action_projectile_ex(",
        "champ->inventory[CHAMPION_SLOT_HAND_LEFT] = THING_NONE",
        "HAS NO AMMUNITION",
    ])
    require_re("m11_game_view.c PC34 no projectile movement lock", game,
               r"case 32: \{ /\* SHOOT \*/.*?return spawned;.*?shoot_no_ammunition:")
    shoot_block = game[game.index("case 32: { /* SHOOT */"):game.index("case 20:   /* FIREBALL */")]
    if "projectileDisabledMovementTicks" in shoot_block:
        raise AssertionError("SHOOT block must not set projectileDisabledMovementTicks for PC34/I34E")
    citations.append("src/engine/m11_game_view.c C032 SHOOT source table/path")

    require("m11 probe SHOOT invariants", probe, [
        "SHOOT rejects mismatched",
        "bow/rock ammunition class",
        "SHOOT bow/arrow uses source",
        "kinetic attack cell direction step-energy and consumes ammo",
        "p->kineticEnergy == 60",
        "p->attack == 106",
        "p->stepEnergy == 4",
        "p->direction == DIR_EAST",
        "p->cell == 1",
        "projectileDisabledMovementTicks == 0",
    ])
    citations.append("probes/m11/firestaff_m11_game_view_probe.c INV_GV_338/339A/339 SHOOT invariants")

    print("DM1_V1_RANGED_SHOOT_SOURCE_LOCK_VERIFIED")
    for c in citations:
        print(f"- {c}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
