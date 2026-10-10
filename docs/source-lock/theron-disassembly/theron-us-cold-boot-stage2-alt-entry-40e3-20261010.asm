; Static alternate-entry listing from authentic US Rev. 1 Track 02.
; The preceding candidate at $4002 calls $40E3 when CPX #$03 is not lower.
; This entry overlaps the linear TIA decode ending at byte $40E3.
; US Track 02 raw offset 0x297db3; full-image MD5
; f23601102138f87c33025877767ebf76; span SHA-256
; fa686ced729a6563d0f4d00c6d9e6bdf2f9c3f547d8a21afbd51ee6c26c0cd05.
; MAME 0.285: unidasm TQUS02.bin -arch h6280 -basepc 0x40e3
;   -skip 2719155 -count 201
; Static bytes only; not a runtime source receipt or function identification.

0040e3: 20 43 41  jsr  $4143
0040e6: b0 01     bcs  $40E9
0040e8: 60        rts
0040e9: a9 01     lda  #$01
0040eb: 20 69 e0  jsr  $E069
0040ee: 20 cd 44  jsr  $44CD
0040f1: 20 7c 44  jsr  $447C
0040f4: 20 38 44  jsr  $4438
0040f7: 20 4d 4b  jsr  $4B4D
0040fa: 9c 7b 51  stz  $517B
0040fd: 9c 5b 47  stz  $475B
004100: a9 00     lda  #$00
004102: 8d 6c 47  sta  $476C
004105: a9 30     lda  #$30
004107: 8d 6d 47  sta  $476D
00410a: a9 85     lda  #$85
00410c: 8d 6a 47  sta  $476A
00410f: a9 53     lda  #$53
004111: 8d 6b 47  sta  $476B
004114: a9 00     lda  #$00
004116: 8d 57 47  sta  $4757
004119: a9 01     lda  #$01
00411b: 8d 79 51  sta  $5179
00411e: 20 d1 42  jsr  $42D1
004121: b0 1c     bcs  $413F
004123: f0 1a     beq  $413F
004125: 20 12 45  jsr  $4512
004128: 10 15     bpl  $413F
00412a: 20 c4 45  jsr  $45C4
00412d: 20 c7 42  jsr  $42C7
004130: 20 b9 41  jsr  $41B9
004133: 20 6f 47  jsr  $476F
004136: 20 3b 4b  jsr  $4B3B
004139: 20 2f 4b  jsr  $4B2F
00413c: 20 cd 44  jsr  $44CD
00413f: 20 79 41  jsr  $4179
004142: 60        rts
004143: a9 ac     lda  #$AC
004145: 85 f8     sta  $F8
004147: a9 41     lda  #$41
004149: 85 f9     sta  $F9
00414b: a9 19     lda  #$19
00414d: 85 fa     sta  $FA
00414f: a9 53     lda  #$53
004151: 85 fb     sta  $FB
004153: a9 99     lda  #$99
004155: 85 fc     sta  $FC
004157: a9 01     lda  #$01
004159: 85 fd     sta  $FD
00415b: a9 00     lda  #$00
00415d: 85 fe     sta  $FE
00415f: a9 00     lda  #$00
004161: 85 ff     sta  $FF
004163: 20 4e e0  jsr  $E04E
004166: c9 00     cmp  #$00
004168: f0 0d     beq  $4177
00416a: a9 ac     lda  #$AC
00416c: 85 f8     sta  $F8
00416e: a9 41     lda  #$41
004170: 85 f9     sta  $F9
004172: 20 54 e0  jsr  $E054
004175: 38        sec
004176: 60        rts
004177: 18        clc
004178: 60        rts
004179: 20 12 45  jsr  $4512
00417c: 30 2d     bmi  $41AB
00417e: 9c 80 51  stz  $5180
004181: 73 80 51 81 51 98 01  tii  $5180 $5181 $0198
004188: a9 ac     lda  #$AC
00418a: 85 f8     sta  $F8
00418c: a9 41     lda  #$41
00418e: 85 f9     sta  $F9
004190: a9 80     lda  #$80
004192: 85 fa     sta  $FA
004194: a9 51     lda  #$51
004196: 85 fb     sta  $FB
004198: a9 99     lda  #$99
00419a: 85 fc     sta  $FC
00419c: a9 01     lda  #$01
00419e: 85 fd     sta  $FD
0041a0: a9 00     lda  #$00
0041a2: 85 fe     sta  $FE
0041a4: a9 00     lda  #$00
0041a6: 85 ff     sta  $FF
0041a8: 20 51 e0  jsr  $E051
0041ab: 60        rts
