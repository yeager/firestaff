# Theron's Original Turn Commands

## Sources and Shared Starting Point

The captures come from the authentic US disc with Track 02 MD5
`ceb02343868f80cec899e9b239aff2da`, System Card 3.0-MD5
`ff1a674273fe3540ccef576376407d1d` and the same Mednafen save state with MD5
`f17f377df210b4a3ae904a13fb85a7f0`. No generated map, RAM image, palette or
button event is included.

Button I is observed in the original input buffer as `$28B8=$01`, written from
logical HuC6280 PC `$44E5` and physical PC `$0D04E5`. The command trace begins
at this edge and contains exactly 65,536 ordered writes to main RAM.

## Left Turn

The left movement-panel button queues command type `$01`. The verified click
has internal coordinate `$7B/$8F`; its X value corresponds to the nine-bit
screen coordinate through the original's doubling in `$D56A..$D578`.

The original routine `$D900..$D92E` changes global direction `$203F` from `1`
to `0`. Group fields `$2944` and `$2948` simultaneously change from `1` to
`0`. The before and after images of 8 KiB main RAM have MD5
`2449d5b14c41565a9d6c71c7c61f481d` and, respectively,
`4072c735edbe4d60182870876a8ceb79`.

An extended click/control pair from the same starting point differs at 27,226
presented pixels within `(0,0)..(271,175)`. The click's VRAM MD5 is
`01ec4386a553b0382c737c593f8dc04d`; the control's is
`a44656d752b2910f48944831eaf23d61`.

## Right Turn

The right movement-panel button queues command type `$02` at internal
coordinate `$98/$8F`. The same original routine changes `$203F`, `$2944` and
`$2948` from `1` to `2`. The earlier visible click/control capture differs at
27,430 pixels.

## Production Boundary

Firestaff maps only original commands `$01` and `$02` to left and right
quarter-turns, respectively. Other command types are rejected. Captures at
2,097,152 VDC records are used as presentation evidence but are not admitted
to the atomic VRAM production list because the CPU-port trace crosses the
HuC6270's internal DMA. The earlier clean 65,536-record boundary and
command-RAM evidence remain separate gates.
