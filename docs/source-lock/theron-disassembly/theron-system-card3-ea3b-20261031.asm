; Authentic System Card 3.0 device-status/data-port routine excerpt.
; BIOS image MD5: ff1a674273fe3540ccef576376407d1d (262,656 bytes).
; Disassembled on TRV2 with MAME unidasm -arch h6280.
; Command: unidasm syscard3.pce -arch h6280 -skip 3131 -basepc 0xEA3B -count 100
; The identical unidasm listing SHA-256 was observed in three runs:
;   622a606e096c59e8db76676475089860e45024eea3eac036522d62e4a8238e6f
; This is a bounded 100-instruction excerpt, not a BIOS image.
;
; The listing shows $EA3B loading and masking $1800, then comparing it with
; $C8. The matching branch reaches $EA4D, which reads $1801; $EA50 stores
; the byte through ($FA). The authentic r5 trace joins $1800=$C8 at event
; 10510, $1801=$04 at 10517, and the $2259=$04 write at 10519.
; See theron-jp-system-card-run-path-20261030.md for capture limits and hashes.
; The 2026-10-31 r5 trace later observes $E162 reading $2259=$04, then
; $E166->$E176. It does not observe $E174/$E11D/$E130/$E209 or game handoff.

0ea3b: ad 00 18  lda  $1800
0ea3e: 29 f8     and  #$F8
0ea40: 8d 7a 22  sta  $227A
0ea43: c9 c8     cmp  #$C8
0ea45: f0 06     beq  $EA4D
0ea47: c9 d8     cmp  #$D8
0ea49: f0 1c     beq  $EA67
0ea4b: 80 ee     bra  $EA3B
0ea4d: ad 01 18  lda  $1801
0ea50: 92 fa     sta  ($FA)
0ea52: 44 d3     bsr  $EA27
0ea54: e6 fa     inc  $FA
0ea56: d0 02     bne  $EA5A
0ea58: e6 fb     inc  $FB
0ea5a: a5 f8     lda  $F8
0ea5c: d0 02     bne  $EA60
0ea5e: c6 f9     dec  $F9
0ea60: 3a        dea
0ea61: 85 f8     sta  $F8
0ea63: 05 f9     ora  $F9
0ea65: d0 d4     bne  $EA3B
0ea67: ad 00 18  lda  $1800
0ea6a: 29 f8     and  #$F8
0ea6c: 8d 7a 22  sta  $227A
0ea6f: 29 b8     and  #$B8
0ea71: c9 88     cmp  #$88
0ea73: d0 03     bne  $EA78
0ea75: 4c 5e eb  jmp  $EB5E
0ea78: 60        rts
0ea79: ad 00 18  lda  $1800
0ea7c: 29 f8     and  #$F8
0ea7e: 8d 7a 22  sta  $227A
0ea81: c9 c8     cmp  #$C8
0ea83: f0 06     beq  $EA8B
0ea85: c9 d8     cmp  #$D8
0ea87: f0 34     beq  $EABD
0ea89: 80 ee     bra  $EA79
0ea8b: a5 f8     lda  $F8
0ea8d: 05 f9     ora  $F9
0ea8f: f0 24     beq  $EAB5
0ea91: a2 03     ldx  #$03
0ea93: ca        dex
0ea94: d0 fd     bne  $EA93
0ea96: c2        cly
0ea97: a2 08     ldx  #$08
0ea99: ad 08 18  lda  $1808
0ea9c: 91 fa     sta  ($FA),y
0ea9e: a5 f8     lda  $F8
