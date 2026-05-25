# DM2 V1 — Math Subsystem

## Overview

DM2's math subsystem is entirely **integer-based** — no floating-point operations are used anywhere in the game. This is consistent with the original DM and CSB engines, which were written for the 8088/80286 CPU without an FPU coprocessor. All trigonometric calculations, random number generation, geometry, and arithmetic use fixed-point integer techniques and lookup tables.

The SKULL.EXE disassembly (522,128 lines) shows no 8087 FPU instructions whatsoever.

## Random Number Generation

The disassembly shows a 64-byte buffer area pointed to by DS:SI at the entry point — this is likely the random number generator's working memory, possibly a Linear Feedback Shift Register (LFSR) or similar fast integer PRNG.

### Observed Characteristics
- **No BIOS timer seed read** at startup visible in the entry stub — the PRNG may be seeded from the game clock at first use rather than at program launch
- **Type**: Likely a fast tabular or LFSR PRNG, not a complex LCG — DM/CSB tradition used fast generators suitable for real-time game loops
- **No Mersenne Twister or similar** — the era (1992) and the real-time rendering constraint imply a simple, fast PRNG with acceptable periodicity for game purposes

### Where RNG Is Used
- **Creature AI decisions**: random element in attack/retreat choices
- **Item drops**: whether a creature drops an item and which item
- **Spell targeting**: some spells have random target selection within an area
- **Damage rolls**: physical attacks use a random component in damage calculation
- **Treasure generation**: random item placement and treasure content
- **Champion stat generation**: initial champion stats likely use the RNG

## Fixed-Point Arithmetic

### Angle Representation
- Angles are almost certainly stored as **integer degrees or binary degrees** (360/256 divisions) in a lookup table
- The rendering engine casts rays for each vertical screen stripe; the angle step per stripe is computed from a precomputed sine/cosine table

### Lookup Tables
- **Sine/Cosine tables**: Precomputed tables for 0–360 degrees at some resolution (likely 1-degree steps or finer, stored as integers representing sin(x) on a fixed scale)
- **Distance correction**: For diagonal walls, a fixed multiplier is applied to correct the fish-eye distortion inherent in simple raycasting

### No Trigonometric Computation at Runtime
- **No tan(), sin(), cos() function calls** in the disassembly — all trig is lookup-only
- The raycasting view uses the relationship: `distance = side_offset / sin(angle)` type correction via a cosine lookup for perpendicular distance (avoiding the fisheye effect)

## Geometry Calculations

### Raycasting (Primary Rendering Math)
The 3D rendering is classic raycasting:
1. For each vertical stripe of the screen, compute a ray direction
2. Step through the dungeon grid (DDA-style integer stepping) until a wall is hit
3. Calculate perpendicular distance to avoid fisheye distortion
4. Compute wall slice height from distance: `slice_height = screen_height / distance`
5. Apply a shading factor based on whether the hit was on a N/S or E/W wall face (darker for E/W)

### Wall Height Calculation
- Uses integer division: `wall_height = (screen_height * constant) / distance`
- No division by zero protection visible for distance=0 (player cannot be inside a wall in normal play, but invalid dungeon data could trigger it)

### Sprite Projection
- Sprites (creatures, items) are billboard-rendered (always facing player)
- Distance sorting via integer comparison
- Screen X position computed from angle difference between sprite and player facing direction
- Sprite clipping at screen edges via integer comparison

### Collision Detection
- All collision uses **integer grid coordinates** (map tiles)
- Player movement checked against wall bits in the dungeon data
- No sub-tile collision — all positions snap to the 1-tile grid

## Integer Arithmetic Details

### Damage Calculation
- Attack damage = base damage + random roll + strength bonus
- All integer arithmetic; damage is a small integer (typically 1–20 range)
- No negative numbers in final damage output (clamped at 0 minimum)

### Speed/Movement
- Movement cooldown timers are integer tick counters
- Turn timing is managed in game ticks (not tied to display frame rate)
- No velocity vectors — all movement is tile-to-tile discrete steps

### Spell Effects
- Spell duration/effect magnitudes stored as integer ticks
- Area-of-effect radius stored as integer grid distance

## No Protection Against Invalid Data

### Div-by-Zero
- Distance = 0 (player inside wall, invalid dungeon data, or corrupted save) would cause a divide-by-zero exception in the raycasting code — the game has no software protection against this; it would crash

### Out-of-Range Values
- Map coordinates out of range could address beyond the dungeon data array
- Sprite distances beyond expected range could produce negative or overflow results
- No range checking on champion stat modifications

## Comparison to DM/CSB

DM2's math subsystem is structurally identical to DM and CSB's — the same integer lookup-table trigonometry, same raycasting renderer, same fixed-point geometry. The primary differences are in data content (different dungeons, creatures, spells) and potentially different random seeds/timing, not in the underlying math approach.

## Source Reference

- SKULL.ASM disassembly (522,128 lines) at canonical path on N2: no FPU instructions, no floating-point data constants, no trig function calls — purely integer math throughout
