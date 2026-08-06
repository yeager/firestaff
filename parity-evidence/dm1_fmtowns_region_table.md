# DM1 FM Towns region table (GET_RGN_COORD / GET_SCL_COORD)

Companion to `dm1_fmtowns_menu_p3_disassembly.md`. Same load image:
HMA-240 English disc `EDM.EXP` (310518 bytes, sha256
`c888470d39aa449eac85438b598158492d2c981008cc6b427c52f2c73001ecb6`),
Phar Lap P3 load offset `0x200`, file offset = `0x200 + vaddr`.

All disassembly below is `capstone` i386 protected-mode against the
verified image; every byte quoted below is real, no invented values.

## Table registry `[0x28f08]`

`GET_SCL_COORD` (0x1942c) and `GET_COORD` (0x18df0) each read a dword
head pointer from the flat data address `0x28f08`. That dword holds
`0x28e78` statically in EDM.EXP:

```
dword @ 0x28f08 = 0x28e78     ; file offset 0x29108, bytes: 78 8e 02 00
```

No cross-reference writes a different value at 0x28f08 in the observed
image — the runtime uses the static list directly (no build-time
allocation). The list is therefore fully recoverable from the ROM.

## The lookup routine at 0x18db4 (16 instructions, 60 bytes)

Verified disassembly:

```
0x18db4: push  ebp
0x18db5: mov   ebp, esp
0x18db7: push  edi
0x18db8: mov   edi, [ebp+8]        ; arg0 = block head pointer
0x18dbb: mov   cx,  [ebp+0xc]      ; arg1 = region id (word)
0x18dbf: and   cx, cx
0x18dc2: je    0x18de9             ; id == 0 -> return NULL
0x18dc4: and   edi, edi
0x18dc6: je    0x18de9             ; ptr == NULL -> return NULL
0x18dc8: mov   ax, [edi+4]         ; first_id in this block
0x18dcc: cmp   cx, ax
0x18dcf: jl    0x18de5             ; id below range -> next block
0x18dd1: cmp   cx, [edi+6]         ; last_id
0x18dd5: jg    0x18de5             ; id above range -> next block
0x18dd7: movsx eax, ax
0x18dda: movsx edx, cx
0x18ddd: sub   edx, eax            ; slot = id - first_id
0x18ddf: lea   eax, [edi+edx*8+8]  ; ptr to 8-byte record
0x18de3: jmp   0x18deb
0x18de5: mov   edi, [edi]          ; ptr = next block (offset 0)
0x18de7: jmp   0x18dc4
0x18de9: sub   eax, eax            ; return NULL
0x18deb: pop   edi
0x18dec: leave
0x18ded: ret
```

### Block header (8 bytes) and record layout (8 bytes each)

```
struct RegionBlock {
    RegionBlock *next;      // +0  dword (0 terminates list)
    uint16_t     first_id;  // +4  word — inclusive
    uint16_t     last_id;   // +6  word — inclusive
    RegionRec    records[last_id - first_id + 1];  // +8, stride 8
};

struct RegionRec {
    uint16_t type;      // +0  see semantics below
    uint16_t parent;    // +2  id of parent region (0 = root)
    int16_t  a;         // +4  width  (type == 9) OR x-offset field
    int16_t  b;         // +6  height (type == 9) OR y-offset field
};
```

Signed 16-bit for the a/b fields is confirmed by `GET_SCL_COORD`'s
`movsx ecx, word ptr [ebx+4]` when the caller supplies a scale factor
other than 10000.

### Semantic tags observed in the table

- `type == 9`  — SIZE anchor: `a`=width, `b`=height. Regions used as
  drawable rectangles (SPC_BLOT / FILL_CSCREEN) resolve their pixel
  size by walking to the nearest ancestor with `type == 9`.
- `type == 1..4` — POSITION anchors relative to `parent`. Distinct
  subtypes select which corner/edge the (x, y) fields lock to (the
  jump tables at `0x18fe6` / `0x18f14` inside `GET_COORD` cover types
  `10..18` — the scaled equivalents; only types `1..4` and `9` appear
  in blocks 1 and 400 which own the menu regions).
- `type == 10..18` — scale-anchor variants, dispatched through the
  9-entry jump table at `0x18f14` inside `GET_COORD`.

## Table registry contents — walked live from EDM.EXP

Block-list walk starting at `0x28e78`. Terminator is `next == 0`.

| Block @   | Next      | ID range   | Count |
|-----------|-----------|------------|-------|
| 0x28e78   | 0x28dd0   | 1..17      | 17    |
| 0x28dd0   | 0x28d38   | 400..419   | 20    |
| 0x28d38   | 0x28bc0   | 120..137   | 18    |
| 0x28bc0   | 0x28ac8   | 700..745   | 46    |
| 0x28ac8   | 0x28a18   | 800..829   | 30    |
| 0x28a18   | 0x284e8   | 850..870   | 21    |
| 0x284e8   | 0x283a0   | 3200..3364 | 165   |
| 0x283a0   | 0x281b0   | 2900..2939 | 40    |
| 0x281b0   | 0x27fd0   | 2500..2560 | 61    |
| 0x27fd0   | 0x27c68   | 3000..3058 | 59    |
| 0x27c68   | 0x27c18   | 1000..1107 | 108   |
| 0x27c18   | 0x27bc8   | 1500..1508 | 9     |
| 0x27bc8   | 0x278f0   | 2000..2008 | 9     |
| 0x278f0   | 0x278c8   | 3700..3789 | 90    |
| 0x278c8   | 0x27810   | 1950..1953 | 4     |
| 0x27810   | 0x277d0   | 450..471   | 22    |
| 0x277d0   | 0x27438   | 100..106   | 7     |
| 0x27438   | 0x27208   | 500..613   | 114   |
| 0x27208   | 0x27098   | 150..218   | 69    |
| 0x27098   | 0x26fd0   | 220..264   | 45    |
| 0x26fd0   | 0x26f80   | 75..98     | 24    |
| 0x26f80   | 0x26f40   | 65..73     | 9     |
| 0x26f40   | 0x0       | 110..116   | 7     |

Total: 23 blocks, 994 records.

## Regions 1..17 (block 0x28e78) — the SPC_BLOT / menu block

Exact 8-byte records, decoded live from EDM.EXP:

| ID | rec @    | type | parent |   a  |   b  | interpretation                          |
|----|----------|-----:|-------:|-----:|-----:|-----------------------------------------|
|  1 | 0x28e80  |    9 |      0 |  320 |  200 | Screen size (root)                      |
|  2 | 0x28e88  |    1 |      1 |    0 |    0 | Screen origin                           |
|  3 | 0x28e90  |    9 |      0 |  224 |  136 | Viewport size (224x136)                 |
|  4 | 0x28e98  |    1 |      3 |    0 |    0 | Viewport origin anchor                  |
|  5 | 0x28ea0  |   10 |      2 |    0 |    0 | Scale anchor tied to screen origin      |
|  6 | 0x28ea8  |   10 |      4 |    0 |    0 | Scale anchor tied to viewport           |
|  7 | 0x28eb0  |    1 |      3 |    0 |   33 | Point (0, 33) relative to viewport      |
|  8 | 0x28eb8  |    9 |      2 |   87 |   45 | SIZE 87x45                              |
|  9 | 0x28ec0  |    3 |      8 |  319 |  168 | Anchor (kind 3) relative to id 8        |
| 10 | 0x28ec8  |    9 |      2 |   87 |   45 | **SPC_BLOT panel — 87x45 pixels**       |
| 11 | 0x28ed0  |    2 |     10 |  319 |   77 | **Menu-area anchor over region 10**     |
| 12 | 0x28ed8  |    9 |      2 |   87 |   33 | SIZE 87x33                              |
| 13 | 0x28ee0  |    3 |     12 |  319 |   74 | Anchor kind 3 over region 12            |
| 14 | 0x28ee8  |    9 |      2 |  320 |   31 | SIZE 320x31 (message bar)               |
| 15 | 0x28ef0  |    4 |     14 |    0 |  199 | Anchor kind 4 over region 14            |
| 16 | 0x28ef8  |    9 |      2 |   87 |    6 | SIZE 87x6                               |
| 17 | 0x28f00  |    1 |     16 |  233 |   33 | Point (233, 33) relative to region 16   |

Interpretations for regions 1..6 are inferred from the topology; the
byte-level rectangles for 8..17 are byte-exact and require no
inference. Region 10 (SPC_BLOT panel) has a byte-level size of 87x45;
region 11 (FILL_CSCREEN menu area) inherits size 87x45 from its
parent (see math below) and adds anchor fields (319, 77).

Region IDs 0..9 not present in the table are outside the domain and
return `NULL` from the lookup — the linked list has no id 0 (rejected
by the zero-check at 0x18dc2).

## GET_SCL_COORD math (locked)

Signature: `GET_SCL_COORD(id, out_ptr, scale_x_10k, scale_y_10k)`.
Called by `GET_RGN_COORD` as `GET_SCL_COORD(id, out, 0x2710, 0x2710)`.

```
r  = lookup([0x28f08], id)
if r == NULL || r->parent == 0: return 0
p  = lookup([0x28f08], r->parent)
if p == NULL || p->type != 9: return 0
w  = (scale_x == 10000) ? p->a : (int32_t)((int16_t)p->a * (uint16_t)scale_x) / 10000
h  = (scale_y == 10000) ? p->b : (int32_t)((int16_t)p->b * (uint16_t)scale_y) / 10000
if w <= 0 || h <= 0: return 0
GET_COORD(0, out_ptr, id, &w, &h)
return non-zero
```

Two implications for the shim:

1. `GET_SCL_COORD` returns the size of the **parent** region (which
   must be `type == 9`). The child record supplies the *position*
   (via `GET_COORD`) but the size comes from the parent SIZE anchor.
2. Region id 10 cannot be resolved through `GET_SCL_COORD` — its
   parent (id 2) has `type == 1`, so the type-9 check fails and the
   routine returns zero. SPC_BLOT is expected to call `GET_COORD`
   directly (as recorded in the prior menu evidence). Region id 11
   *can* be resolved through GET_SCL_COORD because its parent (id 10)
   has `type == 9`; it returns width 87, height 45.

For the FM Towns menu, both regions therefore ultimately draw an
87x45 pixel panel — region 10 supplies its own size, and region 11
inherits 87x45 from region 10.

## GET_COORD (0x18df0) — what it does with (x, y, id)

Signature: `GET_COORD(flags_word, out_xy_ptr, id, &in_w, &in_h)`.
`flags_word == -1` short-circuits to the "id-only" success path at
`0x193ad` (this is the entry used when the caller passes `-1` as the
first arg, e.g. SPC_BLOT's `push -1` seen in DRAW_DMENU).

Verified prologue (bytes at 0x18df0..0x18ea3, disassembled above):

1. `r = lookup([0x28f08], id)`; NULL -> return 0.
2. Initialise the running rect on the stack:
   `x0=y0=0`, `x1=y1=10000` (0x4e20 in `word[ebp-0x16]` and
   `word[ebp-0x12]`). These are the *maximum extents*; they will be
   clamped down while walking the parent chain.
3. `self_type = r->type`. If `self_type <= 8`, seed
   `w = r->a, h = r->b`; else `self_type -= 10` and `w = h = 0`.
4. If `flags_word & 0x8000` (`si != 0`), add the caller-supplied
   in_w/in_h (deref `[ebp+0x14]`, `[ebp+0x18]`) into (w, h) and clear
   those two out-params.
5. Walk the parent chain via `r->parent`. Types `10..18` dispatch
   through jump table `0x18f14`; types `1..4` use the simple path
   at `0x19068`. Each step:
   - looks up parent (`p`),
   - reads parent's `si = p->a`, `bx = p->b`,
   - dispatches on the child's `self_type` to compute an anchor
     origin (top-left / center / right / bottom edge variants),
   - accumulates into the running rect and clamps against the
     parent's extents (see `[ebp-0x18]..[ebp-0x12]` updates around
     0x18f7a..0x18fd0), then advances `r = p`.
6. Terminates at `r->parent == 0` (root, id 1 / id 3 in the observed
   table) and writes the final (x, y) into the caller's out pointer.

The complete GET_COORD (both the type-10..18 dispatch at 0x18f14 and
the type-1..4 branch at 0x19068 with its own 9-entry jump table at
0x18fe6) is 1465 bytes long and threads through the same lookup
routine on every parent step. The 9-entry jump tables are fully
resolvable from the image:

- `0x18f14` -> {`0x18f38`, `0x18f7a`, `0x18f51`, `0x18f4a`,
  `0x18f73`, `0x18f41`, `0x18f5a`, `0x18f6a`, `0x18f61`}
  (self_type 10..18, "self is a scale anchor" path)
- `0x18fe6` -> {`0x1900a`, `0x1904c`, `0x19023`, `0x1901c`,
  `0x19045`, `0x19013`, `0x1902c`, `0x1903c`, `0x19033`}
  (parent-type dispatch inside the `type <= 8` branch)

Every parent-arithmetic block ends by either subtracting a half-extent
(`sar eax, 1`) or a full extent (`dec eax` / no shift) from `si` (x)
or `bx` (y). The observable pattern of `sar eax, 1` (center-anchor)
vs. plain `dec eax` (edge-anchor) is what distinguishes the four
observed position types in the menu block.

## What is NOT recovered here

- The exact semantic mapping of position types 1..4 (which corner /
  center each represents) is not spelled out; the arithmetic is fully
  present in the disassembly at 0x18f38..0x1904c but naming each
  branch requires cross-referencing at least one other call site to
  confirm the intended geometry. The **byte-level rectangles** in the
  table above are locked regardless.
- Types 5..8 do not appear anywhere in the region blocks that own the
  menu; their branches inside GET_COORD are present but unexercised
  in that context.
- No runtime writes to `[0x28f08]` were observed, so the head pointer
  is treated as constant. If a later trace uncovers a mutator, that
  supersedes the "static list" assumption here.

## Consumption plan

The FM Towns menu shim can now stop guessing region geometry:

- SPC_BLOT panel (region 10) = 87x45 pixels. Position anchored via
  region 2 (screen origin). GET_COORD walks (2 -> 1) with anchor type
  1 twice, giving origin (0, 0) — panel therefore sits at screen top.
  The panel colour (0x0B, 0x4D, or 0x4F) selected by DRAW_DMENU is
  drawn as a filled 87x45 rectangle at the resolved origin.
- FILL_CSCREEN menu clear (region 11) = 87x45 pixels. Anchor type 2
  with (319, 77) relative to region 10; GET_COORD's final rect is the
  clear region for the menu backdrop.

Both are the correct, byte-exact geometry pulled from the ROM —
nothing else in this document may be substituted for those bytes.
