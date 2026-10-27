# DM1 V1 Champion Equipment

Source: DEFS.H:773-811, CHAMPION.C:301-656

## Equipment Slots (32 total)

Combat Slots (0-5):
C00_SLOT_READY_HAND: left hand (shield/secondary)
C01_SLOT_ACTION_HAND: right hand (primary weapon)
C02_SLOT_HEAD: helmet
C03_SLOT_TORSO: body armor
C04_SLOT_LEGS: leg armor
C05_SLOT_FEET: foot armor

Utility Slots (6-12):
C06_SLOT_POUCH_2: secondary pouch
C07_SLOT_QUIVER_LINE2_1: quiver row 2 slot 1
C08_SLOT_QUIVER_LINE1_2: quiver row 1 slot 2
C09_SLOT_QUIVER_LINE2_2: quiver row 2 slot 2
C10_SLOT_NECK: neck amulet/pendant
C11_SLOT_POUCH_1: primary pouch
C12_SLOT_QUIVER_LINE1_1: quiver row 1 slot 1

Backpack Slots (13-29): C13-C29
Chest Slots (30-31): C30_SLOT_CHEST_1, C31_SLOT_CHEST_2

## Thing Types
C05: WEAPON, C06: ARMOUR, C07: SCROLL, C08: POTION
C09: CONTAINER, C10: JUNK

## Rules
One weapon in C01_SLOT_ACTION_HAND only.
Armor has AllowedSlots bitmask per piece.
Auto-equip on creation: weapon->action hand, armor->appropriate slot.
Scrolls/potions->C11 then C06. Junk->neck or backpack.

## Object Modifiers
F0299 applies stat modifiers on equip/unequip.
BUG0_38: 4+ cursed objects wraps luck min to 254/251/248.

## Combat
Defense = ready hand + body/leg/foot armor + wounds
Attack = weapon kinetic energy + strength + skill + dexterity
