# CSB V1 - New Creatures Audit

## Source Paths
- CSB: Objects.h:153-183, Statistics.cpp:14-44, Attack.cpp
- DM1: DUNGEON.C:439-500, DEFS.H:2435 (C027_CREATURE_TYPE_COUNT=27)

## Monster Type Table
Both DM1 and CSB use C027_CREATURE_TYPE_COUNT = 27 (0x00-0x1a).

### Shared Types (0x00-0x18, indices 0-24)
CSB and DM1 share exactly the same 25 creature types in identical enum order:
Scorpion, SlimeDevil, Giggler, FlyingEye, PainRat, Ruster, Screamer,
RockPile, Rive, StoneGolem, Mummy, BlackFlame, Skeleton, Couatl, Vexirk,
Worm, Trolin, Wasp, AnimatedArmour (DethKnight in DM1 naming), Zytaz,
WaterElemental, Oitu, Demon, LordChaos, Dragon

### CSB-Only Types (0x19, 0x1a)
- mon_25 (0x19): Used in Attack.cpp:2374 (monsterType assignment).
  Statistics.cpp names it "25". No named combat role found.
  DM1 has no creature at this index. Appears inactive/placeholder.
  
- mon_GreyLord (0x1a): Named in Objects.h:181, Statistics.cpp:44 as "GreyLord",
  Attack.cpp:2423. Referenced in Chaos.cpp as attack data bytes.
  DEFS.H:1679: Grey Lord is a source of C5_ATTACK_MAGIC (magical attacks),
  alongside Lord Chaos, Lord Order, Zytaz, Vexirk, FlyingEye.
  NEW boss-tier magical creature in CSB.

## Key Findings
1. Grey Lord (0x1a) is the only genuinely new named creature type in CSB.
2. mon_25 (0x19) exists but appears to be an unused/placeholder entry.
3. The 25 shared creature types are identical between DM1 and CSB.
4. DethKnight = AnimatedArmour - same entity at index 18, naming only.
5. No new fixed possessions for 0x19/0x1a in CSB source.

## Conclusion
CSB adds exactly 1 new named creature: Grey Lord (mon_GreyLord, 0x1a),
a magic-attacking boss creature. One placeholder slot (mon_25, 0x19) also exists.
