; Authentic System Card 3.0 CD status polling and data dispatch.
; This bounded disassembly excerpt is not a BIOS image.
;
; BIOS MD5: ff1a674273fe3540ccef576376407d1d (262144-byte payload).
; The .pce file is 262656 bytes: HuC_LoadCD strips its 512-byte header,
; then reads the payload into ROMSpace. With MPR7=$00, logical $E95E maps
; to ROMSpace offset $095E and file offset 512+$095E = 2910.
; MAME 0.285 command on TRV2:
;   unidasm syscard3.pce -arch h6280 -skip 2910 -basepc 0xE95E -count 80
; At $E976 in the v21 capture, MPR6=$01; logical $CC17 therefore maps to
; ROMSpace offset $2C17 and file offset 512+$2C17 = 11799.
;   unidasm syscard3.pce -arch h6280 -skip 11799 -basepc 0xCC17 -count 80
;
; $1800 status bits are reported by the reference core. This listing records
; the authentic BIOS control flow; it does not assign higher-level CD meaning.

e95e: ad 00 18  lda  $1800
e961: 29 f8     and  #$F8
e963: 8d 7a 22  sta  $227A
e966: c9 d0     cmp  #$D0
e968: f0 10     beq  $E97A
e96a: 29 b8     and  #$B8
e96c: c9 98     cmp  #$98
e96e: f0 1a     beq  $E98A
e970: c9 88     cmp  #$88
e972: f0 16     beq  $E98A
e974: c9 80     cmp  #$80
e976: f0 12     beq  $E98A
e978: 80 e4     bra  $E95E
e97a: bd 4c 22  lda  $224C,x
e97d: e8        inx
e97e: 8d 01 18  sta  $1801
e981: ea        nop
e982: ea        nop
e983: ea        nop
e984: ea        nop
e985: 20 27 ea  jsr  $EA27
e988: 80 d4     bra  $E95E
e98a: ad a4 22  lda  $22A4
e98d: 0d 48 22  ora  $2248
e990: d0 32     bne  $E9C4
e992: ad 4c 22  lda  $224C
e995: c9 d9     cmp  #$D9
e997: d0 0b     bne  $E9A4
e999: ad 4d 22  lda  $224D
e99c: c9 01     cmp  #$01
e99e: f0 24     beq  $E9C4
e9a0: c9 02     cmp  #$02
e9a2: f0 20     beq  $E9C4
e9a4: ae 41 22  ldx  $2241
e9a7: ad 00 18  lda  $1800
e9aa: 29 f8     and  #$F8
e9ac: 8d 7a 22  sta  $227A

cc17: ad 56 22  lda  $2256
cc1a: 20 00 e6  jsr  $E600
cc1d: 8d 56 22  sta  $2256
cc20: ad 57 22  lda  $2257
cc23: 20 00 e6  jsr  $E600
cc26: 8d 57 22  sta  $2257
cc29: ad 58 22  lda  $2258
cc2c: 20 00 e6  jsr  $E600
cc2f: 8d 58 22  sta  $2258
cc32: 64 f8     stz  $F8
cc34: 64 f9     stz  $F9
cc36: 64 fa     stz  $FA
cc38: ad 56 22  lda  $2256
cc3b: 44 5a     bsr  $CC97
cc3d: 44 3b     bsr  $CC7A
cc3f: 44 40     bsr  $CC81
cc41: 44 3e     bsr  $CC81
cc43: 44 3c     bsr  $CC81
cc45: 44 3a     bsr  $CC81
cc47: ad 57 22  lda  $2257
cc4a: 44 4b     bsr  $CC97
cc4c: 44 35     bsr  $CC83
cc4e: a5 f8     lda  $F8
cc50: 85 ec     sta  $EC
cc52: a5 f9     lda  $F9
cc54: 85 ed     sta  $ED
cc56: a5 fa     lda  $FA
cc58: 85 ee     sta  $EE
cc5a: 44 25     bsr  $CC81
cc5c: 44 1c     bsr  $CC7A
cc5e: 44 21     bsr  $CC81
cc60: 44 18     bsr  $CC7A
cc62: 44 16     bsr  $CC7A
cc64: 44 1b     bsr  $CC81
cc66: ad 58 22  lda  $2258
