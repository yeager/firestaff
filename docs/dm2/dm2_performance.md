# DM2 V1 — Performance

## Hardware Requirements

### Minimum (Official)
- **CPU**: Intel 386 or faster (486 recommended for流畅 play)
- **RAM**: 4 MB minimum, 8 MB recommended
- **Display**: EGA/VGA compatible graphics
- **Sound**: AdLib, Sound Blaster, or compatible
- **Storage**: ~20 MB hard drive space
- **Operating System**: DOS 3.3+

### Modern Play (DOSBox/emulation)
- Any modern system running DOSBox or DOSBox-X at 3000–15000 cycles provides smooth play
- 486-equivalent instruction speed is the practical bottleneck; 3D pipeline is not the limiting factor

### Context: DM/CSB Performance Background
The original DM/CSB ran at a target ~14–15 FPS on 386/486 hardware, with rendering time taking the bulk of each frame. DM2 is a more complex game (more objects, larger maps, more creatures) but uses the same fundamental raycasting 3D engine.

## Frame Rate

### Expected Performance
- On period-accurate 486 hardware: approximately **10–15 FPS** depending on scene complexity
- On modern machines with DOSBox at moderate cycle count (e.g., 5000–10000 cycles): **near 60 FPS** most of the time
- Some areas (dense creature fights, complex map geometry) cause frame drops even on modern hardware — Lunever reported going below 10 FPS in some cases on a real 486
- The DM/CSB engine on RTC (Return to Chaos) runs acceptably on Win95 with a 200 MHz processor

### Engine Notes
- The rendering pipeline is a **software rasterizer** using raycasting (similar to Wolfenstein 3D style)
- No hardware acceleration or FPU required — all math is integer/fixed-point
- The SKULL.EXE disassembly shows no use of 8087 FPU instructions; all math is pure integer
- Performance scales with single-threaded integer throughput — heavily CPU-bound

## Memory Usage

### Runtime Memory Footprint
- **Code segment**: ~64 KB (SKULL.EXE entry stub); full game loaded in memory
- **Data segments**: Dungeon data, graphics, sound loaded as needed
- **EMS/XMS**: The game can use EMS memory for graphics data on machines with an EMS board
- Typical peak memory usage: **under 640 KB** conventional RAM (with EMS for graphics)
- No protected-mode version exists — the game is pure real-mode DOS

### Graphics Data
- `graphics.dat`: Contains all sprites, tiles, UI elements — loaded as needed
- No texture streaming; all data is pre-loaded from disk
- Level/map data (`dungeon.dat`) loaded at game start

## Rendering Pipeline Characteristics

### What Is Rendered Each Frame
- **View frustum**: The 3D viewport (first-person view into the dungeon)
- **Raycasting**: For each vertical stripe of the screen, cast a ray to find wall distance — pure integer math
- **Sprites**: Creatures and items are billboard sprites (always facing the player)
- **UI overlay**: Champions panel, stats, message area, spell selection — rendered on top of 3D view
- **Transparency**: Some UI elements (champion portraits) use color-keyed transparency

### Sprite Sorting
- Sprites (creatures/items) are sorted by distance each frame for correct overdraw order
- No depth buffer — painter's algorithm used

## Performance on Modern Systems

| Platform | Expected FPS | Notes |
|----------|-------------|-------|
| DOSBox ~3000 cycles | 15–30 | Period-accurate feel |
| DOSBox ~10000 cycles | 50–60 | Comfortable modern play |
| DOSBox-X / hardware | 60 | Fully smooth |
| Real 486/66MHz | 10–15 | Original experience |
| Real 386/33MHz | 5–8 | Very sluggish, playable |

## Disk Space

- Base game: ~15–20 MB
- With custom dungeons and saved games: varies
- No install program needed; can run from a single directory

## Audio Performance Impact

- Sound playback (AdLib/Sound Blaster) has minimal CPU impact in DM2
- Music (HMP format) is handled by the sound card, not the CPU
- Sound effects are short samples; no streaming complexity
