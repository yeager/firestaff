#ifndef FIRESTAFF_DM2_V1_SHOP_NPC_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SHOP_NPC_PC34_COMPAT_H

/*
 * dm2_v1_shop_npc_pc34_compat.h — DM2 shop panel and NPC merchant system.
 *
 * The DM2 shop system uses two actuator types:
 *
 * ACTUATOR_TYPE_SHOP_PANEL (0x3F): wall actuator for shop glass overlay.
 *   When an item is placed on the shop panel, ActiveStatus is set to 0
 *   and OnceOnlyActuator is set to 1, consuming the item.
 *   Source: skgame.cpp:2040-2046, 3564-3568
 *
 * ACTUATOR_FLOOR_TYPE__SHOP (0x30): floor actuator for shop exhibition.
 *   Controls item display on the shop floor tiles.
 *   Source: skfileop.cpp:208, skdungn.cpp:901
 *
 * Merchant NPCs (AI ref type 13) are creatures with special AI that
 * handle item exchange. The merchant AI table entry is at KSK37FC.h:1069
 * (tblAIRef13Merchant). Merchant guards (AI ref 17) protect the shop.
 *
 * Shop glass wall ornate type: GDAT_WALL_ORNATE__0A value 2
 *   (C2_WALL_ORNATE_OBJECT__SHOP_GLASS)
 *   Source: skdefine.h:804-808
 *
 * Shop overlay ornate: GDAT_WALL_ORNATE__OVERLAY (0x0F)
 *   Source: skdefine.h:822
 *
 * The shop panel actuator is already wired through the existing
 * dm2_v1_actuator_event module. This header documents the shop
 * constants and provides a classification API.
 *
 * Source: skproject/SKWINSPX/src/v4/skgame.cpp, skdefine.h, KSK37FC.h
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_ACTUATOR_TYPE_SHOP_PANEL       0x3F
#define DM2_ACTUATOR_FLOOR_TYPE_SHOP       0x30
#define DM2_WALL_ORNATE_SHOP_GLASS         0x02
#define DM2_WALL_ORNATE_OVERLAY            0x0F
#define DM2_AI_REF_MERCHANT                13
#define DM2_AI_REF_MERCHANT_GUARD          17

typedef struct {
    int is_shop_panel;
    int is_shop_floor;
    int is_merchant_npc;
    int is_merchant_guard;
} DM2_V1_ShopClassification;

int dm2_v1_classify_shop_element(
    int16_t actuator_type,
    int16_t creature_ai_ref,
    DM2_V1_ShopClassification *result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SHOP_NPC_PC34_COMPAT_H */
