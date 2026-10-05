# DM2 FM Towns P3 entry disassembly

## Scope

This is a static disassembly of the authenticated HME-242 `SKULL.EXP` load
image. It does not claim that Firestaff executes this program or that the
menu continuation has been recovered. The selected original file is
`~/.firestaff/data/dm2/fmtowns_iso/SKULL.EXP` (374,416 bytes, SHA-256
`068218bb31a7ed3700974394cbf11cb51c9a9da694511e748114b5dd7ee07671`).
Its P3 envelope and load-image bounds match
`dm2_v1_boot_parse_fmtowns_p3` in `src/dm2/dm2_v1_boot.c:254-303` and the
existing startup evidence in
`parity-evidence/dm2_fmtowns_startup_p3_gdat_boundary.md`.

The P3 header reports level 1, header size `0x180`, load image at file offset
`0x200` with size `0x5b490`, initial EIP `0x5741c`, and no symbol table. P3
virtual addresses in the listing therefore map to file offsets as
`0x200 + address`.

## Entry window

The listing was generated from the local original file with:

```sh
python3 tools/disassemble_fmtowns_p3.py \
  ~/.firestaff/data/dm2/fmtowns_iso/SKULL.EXP \
  --sha256 068218bb31a7ed3700974394cbf11cb51c9a9da694511e748114b5dd7ee07671 \
  --address 0x57474 --length 0x60
```

```text
00057474: 3c cd                         cmp       al, 0xcd
00057476: 75 1b                         jne       0x57493
00057478: 80 fc ab                      cmp       ah, 0xab
0005747b: 75 16                         jne       0x57493
0005747d: 3d cd ab cd ab                cmp       eax, 0xabcdabcd
00057482: 75 0f                         jne       0x57493
00057484: c6 05 bc 43 01 00 01          mov       byte ptr [0x143bc], 1
0005748b: 89 15 90 43 01 00             mov       dword ptr [0x14390], edx
00057491: eb 07                         jmp       0x5749a
00057493: c6 05 bc 43 01 00 02          mov       byte ptr [0x143bc], 2
0005749a: 2b ed                         sub       ebp, ebp
0005749c: 83 e4 fc                      and       esp, 0xfffffffc
0005749f: c7 05 b8 14 00 00 00 02 00 00 mov       dword ptr [0x14b8], 0x200
000574a9: c7 05 bc 14 00 00 00 01 00 00 mov       dword ptr [0x14bc], 0x100
000574b3: 81 05 b8 14 00 00 90 b4 05 00 add       dword ptr [0x14b8], 0x5b490
000574bd: 81 05 bc 14 00 00 90 b4 05 00 add       dword ptr [0x14bc], 0x5b490
000574c7: 80 3d bc 43 01 00 01          cmp       byte ptr [0x143bc], 1
000574ce: 74 20                         je        0x574f0
```

At the declared entry address, the first instruction is `0005741c: eb 56
jmp 0x57474`.

To follow reachable direct branches without linearly decoding the intervening
copyright/data bytes, run the bounded control-flow walker:

```sh
python3 tools/disassemble_fmtowns_p3.py \
  ~/.firestaff/data/dm2/fmtowns_iso/SKULL.EXP \
  --sha256 068218bb31a7ed3700974394cbf11cb51c9a9da694511e748114b5dd7ee07671 \
  --address 0x5741c --length 0x700 --flow --max-instructions 512
```

This hash-pinned walk shows `0x5741c -> 0x57474`, the distinct error route
`0x57900 -> 0x57423`, and the ordinary route `0x57939 -> 0x5741e ->
0x1dfd4`. Direct calls or branches outside the selected address window and
indirect targets are reported but not followed. The walk is navigation
evidence; it does not emulate Phar Lap relocation, resolve indirect calls, or
assign source-level names.

The entry jumps into a startup stub. The stub branches on the `0xcdab` /
`0xabcdabcd` register signature, writes a mode byte, aligns the stack, and
builds two address bounds using the load-image base/size constants. The data
addresses and branch state are still unlabeled; this listing does not assign
them application-level meanings.

This window confirms that the initial P3 entry is startup machinery. It does
not reach or identify the New Game/Resume menu code in the captured range.

## Startup handoff trace

The boot stub continues through `0x5787d` and prepares runtime state. Its later
validation has two distinct outcomes. Error-reporting paths call DOS `int 21h`
with `AH=9` and then reach `0x57900`, which jumps to `0x57423` and terminates
through DOS `int 21h` with `AH=0`. The ordinary continuation reaches `0x57939`,
which jumps to `0x5741e`. That address calls `0x1dfd4`, pushes its return value,
calls `0x5903c`, and finally terminates through the same `AH=0` sequence.
Consequently `0x1dfd4` is the post-runtime routine on the ordinary path;
`0x57900` is an error exit, not the normal handoff.

The route split is directly visible in the selected original at `0x57905`:

```text
0x57905: cmp byte [0x143af], 1
         je  0x57939
0x5790e: cmp word [0x143d6], 0xabcd
         jne 0x57939
         ... print diagnostic with DOS AH=9 ...
         jmp 0x578fc
0x578fc: int 21h                 ; AH=9, diagnostic string
0x57900: jmp 0x57423             ; termination path
0x57939: jmp 0x5741e             ; ordinary continuation
```

The bytes at `0x5792f..0x57934` are the DOS `$`-terminated text `80387$`; the
dword at `0x57935..0x57938` points back to that string. Starting a linear x86
decode at `0x57938` crosses the pointer and produces a false instruction. The
continuation must be decoded at its actual branch target, `0x57939`. The
listing in the prior subsection is therefore address-level control-flow
evidence, not a contiguous instruction stream.

At `0x1dfd4`, the routine first calls `0x57350` with `0x684` and returns when
the result is nonzero. If it returns zero, the routine calls `0x4adbc`,
`0x4534c`, then loops:
`0x1dd24` -> load word `[0x614]` -> call `0x19cdc` with that word. The loop
returns to `0x1dd24`. These are address-level call-graph facts, not recovered
source names. In particular, the loop must not be labeled as the title/menu
loop without an address-to-symbol or runtime trace.

The adjacent routine boundaries sharpen that description. In `0x1dd24`,
`0x1de87` tests dword `[0x614]`; a nonzero value branches to `0x1dfcd`, the
epilogue that returns at `0x1dfd0`. The caller at `0x1dfd4` then reads the low
word of `[0x614]` and passes it as the argument to `0x19cdc`. At `0x19cdc`, the
first argument is copied to `DI` and tested for zero before the two main
paths diverge. This is evidence for a frame routine that yields to an
argument-dispatch routine when `[0x614]` is nonzero. It does not identify the
event values, their producers, or the visible menu action they select.

```text
0x1de87: cmp dword [0x614], 0
         jne 0x1dfcd
...
0x1dfcd: pop edi
         pop esi
         leave
0x1dfd0: ret
0x1dfd4: call 0x57350(0x684)
         if eax != 0: return
         call 0x4adbc
         call 0x4534c
loop:    call 0x1dd24
         mov ax, word [0x614]
         push eax
         call 0x19cdc
         goto loop
0x19cdc: mov di, word [ebp + 8]
         call 0x4b394
         test di, di
         je zero-argument branch
```

The ordinary route's core control flow is:

```text
0x57939: jmp 0x5741e
0x5741e: call 0x1dfd4
         push eax
         call 0x5903c
         terminate (DOS int 21h, AH=0)
0x1dfd4: call 0x57350(0x684)
         if return != 0: return
         call 0x4adbc
         call 0x4534c
loop:    call 0x1dd24
         call 0x19cdc(word [0x614])
         goto loop
```

The start of the second routine writes `0x000c` to `[0x65a]`, clears
`[0x2e0]`, and calls `0x2b620`; it then repeatedly checks `[0x664]`, with a
conditional call to `0x1dcc4` and a call to `0x3c518`, before proceeding to
`0x498b0` and `0x14bd4`. The routine at `0x19cdc` has a separate input argument
and conditionally enters the longer branch summarized below. These memory
locations and routines remain numeric pending a proven symbol map.

The `0x19cdc` listing contains a conditional path that calls `0x44804` with a
large argument set, then `0x19bb8` with `0xf0`; the common path calls
`0x51ce4`, `0x36de0`, and can continue through calls at `0x4d450`, `0x18490`,
`0x18250`, `0x54460`, `0x45204`, `0x4b3c0`, and `0x1a7ec`. This is useful
navigation evidence, but without symbols or state observations it does not
establish which branch renders or accepts the title menu.

The trace was generated from the same hash-checked executable with the bounded
disassembler, using P3 virtual addresses:

```sh
python3 tools/disassemble_fmtowns_p3.py \
  ~/.firestaff/data/dm2/fmtowns_iso/SKULL.EXP \
  --sha256 068218bb31a7ed3700974394cbf11cb51c9a9da694511e748114b5dd7ee07671 \
  --address 0x1dfd4 --length 0x100
```

The next useful evidence is an address bridge from a symbolized matching build
or a trace from the original FM Towns runtime. This trace corrects the normal
versus error continuation but does not identify the title/menu loop. The
no-menu-after-40-seconds report is still not explained by the static trace,
and this evidence does not justify changing the menu/input route.
