; Authentic System Card 3.0 bootstrap and controller/CD launch check.
; This bounded disassembly excerpt is not a BIOS image.
;
; BIOS identity: MD5 ff1a674273fe3540ccef576376407d1d, 262656 bytes.
; MAME unidasm was run on TRV2. Its offsets below are file offsets with the
; 512-byte header skipped, so physical offset $00F3 is logical CPU address
; $E0F3 when MPR7 maps ROM bank $00.
;
; The bootstrap calls $C860, handles its nonzero result through $CF78, then
; invokes $F0A9 and an internal controller/CD-status routine. A clear carry
; configures MPR6 and jumps to $4000; a set carry invokes $CB03/$CB54 and
; retries $C860. This excerpt does not establish what code is at $4000 or
; whether it is Theron gameplay.

e0f3: 78        sei
e0f4: d4        csh
e0f5: d8        cld
e0f6: a2 ff     ldx #$FF
e0f8: 9a        txs
e0f9: 8a        txa
e0fa: 53 01     tam #$01
e0fc: a9 f8     lda #$F8
e0fe: 53 02     tam #$02
e100: 44 38     bsr $E13A
e102: a9 01     lda #$01
e104: 53 40     tam #$40
e106: 20 0f cb  jsr $CB0F
e109: 20 60 c8  jsr $C860
e10c: aa        tax
e10d: f0 09     beq $E118
e10f: 44 29     bsr $E13A
e111: a9 03     lda #$03
e113: 53 40     tam #$40
e115: 4c 78 cf  jmp $CF78
e118: 20 a9 f0  jsr $F0A9
e11b: 44 2f     bsr $E14C
e11d: 90 0d     bcc $E12C
e11f: a9 01     lda #$01
e121: 53 40     tam #$40
e123: 20 03 cb  jsr $CB03
e126: 20 54 cb  jsr $CB54
e129: 4c 09 e1  jmp $E109
e12c: a9 04     lda #$04
e12e: 44 0d     bsr $E13D
e130: 4c 00 40  jmp $4000
e133: a9 17     lda #$17
e135: 44 06     bsr $E13D
e137: 4c 00 40  jmp $4000

; $E14C loads a reference value from $226A, scans $2256 onward through the
; $EFF1 callback while comparing $2259/$226B control data, then either updates
; $2269 and enters common memory setup at $CC17, or returns clear carry.
; This data/return behavior is recorded literally; no higher-level meaning
; such as “CD ready” is assigned without a runtime witness.
e14c: ad 6a 22  lda $226A
e14f: 48        pha
e150: 85 f9     sta $F9
e152: a9 02     lda #$02
e154: 85 f8     sta $F8
e156: a9 56     lda #$56
e158: 85 fa     sta $FA
e15a: a9 22     lda #$22
e15c: 85 fb     sta $FB
e15e: 20 f1 ef  jsr $EFF1
e161: 68        pla
e162: 93 04 59 22 tst #$04 $2259
e166: d0 0e     bne $E176
e168: cd 6b 22  cmp $226B
e16b: b0 07     bcs $E174
e16d: f8        sed
e16e: 18        clc
e16f: 69 01     adc #$01
e171: d8        cld
e172: 80 db     bra $E14F
e174: 18        clc
e175: 60        rts
e176: 8d 69 22  sta $2269
e179: 20 17 cc  jsr $CC17
e17c: 73 f8 20 74 22 03 00  tii $20F8 $2274 $0003
e183: 73 f8 20 77 22 03 00  tii $20F8 $2277 $0003
e18a: 64 fc     stz $FC
e18c: 64 fd     stz $FD
e18e: 64 fe     stz $FE
e190: a9 00     lda #$00
e192: 85 fa     sta $FA
e194: a9 28     lda #$28
e196: 85 fb     sta $FB
e198: a9 01     lda #$01
e19a: 85 ff     sta $FF
