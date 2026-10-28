; Authentic System Card 3.0 controller-poll and candidate action branch.
; This is a disassembly excerpt, not a BIOS image.
;
; BIOS identity: MD5 ff1a674273fe3540ccef576376407d1d, 262656 bytes.
; The 512-byte file header is skipped by Mednafen's HuC_LoadCD() in
; src/pce_fast/huc.cpp:283-301. Therefore file offset $06B4 maps to physical
; ROM offset $04B4; with MPR7=00, the runtime logical address is $E4B4.
; Disassembler: MAME unidasm -arch h6280, run on trv2 from this bounded window.
;
; The code reads direction and button nibbles from $1000 for five pads, stores
; state/delta bytes in $2228/$222D, then tests a per-pad mask and two values
; before dispatching through ($2284). Static code does not prove which UI
; action that callback performs or that this path starts the retail game.

e4b4: ad 00 10    lda $1000
e4b7: 0a          asl a
e4b8: 0a          asl a
e4b9: 0a          asl a
e4ba: 0a          asl a
e4bb: 99 28 22    sta $2228,y
e4be: 9c 00 10    stz $1000
e4c1: 48          pha
e4c2: 68          pla
e4c3: ea          nop
e4c4: ea          nop
e4c5: ad 00 10    lda $1000
e4c8: 29 0f       and #$0F
e4ca: 19 28 22    ora $2228,y
e4cd: 49 ff       eor #$FF
e4cf: 99 28 22    sta $2228,y
e4d2: 59 32 22    eor $2232,y
e4d5: 39 28 22    and $2228,y
e4d8: 99 2d 22    sta $222D,y
e4db: c8          iny
e4dc: c0 05       cpy #$05
e4de: 90 c5       bcc $04A5
e4e0: c2          cly
e4e1: ad 27 22    lda $2227
e4e4: 39 04 e5    and $E504,y
e4e7: f0 15       beq $04FE
e4e9: b9 2d 22    lda $222D,y
e4ec: c9 04       cmp #$04
e4ee: d0 0e       bne $04FE
e4f0: b9 28 22    lda $2228,y
e4f3: c9 0c       cmp #$0C
e4f5: d0 07       bne $04FE
e4f7: a9 80       lda #$80
e4f9: 85 e7       sta $E7
e4fb: 6c 84 22    jmp ($2284)
e4fe: c8          iny
e4ff: c0 05       cpy #$05
e501: 90 de       bcc $04E1
