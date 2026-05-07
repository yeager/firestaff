# Pass244 — DM1 V1 QEMU/FreeDOS DUNVIEW path

Status: `BLOCKED_QEMU_FREEDOS_DUNVIEW_COMPLETED_ZERO_OR_MISSING_OBJ`

## Result

QEMU+FreeDOS now has a reproducible partitioned hard-disk route that starts the authentic TCPP101 DUNVIEW.C compile outside DOSBox. The observed N2 run completed the TCPP101 invocation outside DOSBox, but TCC reported out-of-memory at DUNVIEW.C line 3938 and emitted no DUNVIEW.OBJ; FIRES.MAP therefore remains blocked before TLINK.

## Reproducible non-DOSBox path

- `disk`: dd 256M; sfdisk DOS partition at sector 2048 type 6; mkfs.fat -F 16 --offset=2048; mtools copy source/toolchain into image partition
- `boot`: FreeDOS 1.3 floppy x86BOOT.img with FDAUTO.BAT
- `qemu`: qemu-system-i386 -drive file=c.img,format=raw,if=ide,index=0 -fda boot.img -boot a -m 64 -display none -serial none -monitor none -no-reboot -no-shutdown
- `compile`: TCC.EXE -c -ml -O -Z -DEXEID=74 -n\OBJECT\I34E\FIRES DUNVIEW.C >> C:\QEMU_DUN.LOG

## Source anchors

- `MKII.BAT` TCPP101 PATH line: `10`.
- `MKII.BAT` authentic DUNVIEW compile line: `473`.
- `MKII.BAT` FIRES.MAP TLINK line: `510`.
- `DUNVIEW.C` repeated diagnostic source anchor: `4791`.

## N2-local run metadata only

- Run dir: `$HOME/.openclaw/data/firestaff-qemu-dunview-20260506g`.
- Disk image exists/size: `True` / `268435456` bytes.
- Host start/end: `2026-05-06T13:34:50+00:00` / `2026-05-06T13:42:57+00:00`.
- QEMU log present: `True`; lines: `22`; chars: `1220`.
- First compiler error: `Error dunview.c 3938: Out of memory in function F0107_DUNGEONVIEW_IsDrawnWallOrn`.
- `DUNVIEW.OBJ` mdir present: `False`.
- `FIRES.MAP` mdir present: `False`.

Evidence manifest: `parity-evidence/verification/pass244_dm1_v1_qemu_freedos_dunview_path/manifest.json`.
