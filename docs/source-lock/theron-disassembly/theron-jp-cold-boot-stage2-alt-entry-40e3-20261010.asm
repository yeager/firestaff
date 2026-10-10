; Static alternate-entry listing from authentic JP Rev. 1 Track 02.
; The preceding candidate at $4002 calls $40E3 when CPX #$03 is not lower.
; This entry overlaps the linear TIA decode ending at byte $40E3.
; JP Track 02 raw offset 0x297483; full-image MD5
; b7afb338ad31be1025b53f9aff12d73a; span SHA-256
; 2c41de75cc88b527a3ed8606c24bfe1cfc4e11f39a781714601890b638e5a573.
; MAME 0.285: unidasm TQJP02.bin -arch h6280 -basepc 0x40e3
;   -skip 2716803 -count 201
; Static bytes only; not a runtime source receipt or function identification.

0040e3: 20 43 41  jsr  $4143
0040e6: b0 01     bcs  $40E9
0040e8: 60        rts
0040e9: a9 01     lda  #$01
0040eb: 20 69 e0  jsr  $E069
0040ee: 20 cd 44  jsr  $44CD
0040f1: 20 7c 44  jsr  $447C
0040f4: 20 38 44  jsr  $4438
0040f7: 20 b2 4b  jsr  $4BB2
0040fa: 9c c0 4e  stz  $4EC0
0040fd: 9c 5b 47  stz  $475B
004100: a9 00     lda  #$00
004102: 8d 6c 47  sta  $476C
004105: a9 30     lda  #$30
004107: 8d 6d 47  sta  $476D
00410a: a9 ca     lda  #$CA
00410c: 8d 6a 47  sta  $476A
00410f: a9 50     lda  #$50
004111: 8d 6b 47  sta  $476B
004114: a9 f0     lda  #$F0
004116: 8d 57 47  sta  $4757
004119: a9 01     lda  #$01
00411b: 8d be 4e  sta  $4EBE
00411e: 20 d1 42  jsr  $42D1
004121: b0 1c     bcs  $413F
004123: f0 1a     beq  $413F
004125: 20 12 45  jsr  $4512
004128: 10 15     bpl  $413F
00412a: 20 c4 45  jsr  $45C4
00412d: 20 c7 42  jsr  $42C7
004130: 20 b9 41  jsr  $41B9
004133: 20 6f 47  jsr  $476F
004136: 20 a0 4b  jsr  $4BA0
004139: 20 94 4b  jsr  $4B94
00413c: 20 cd 44  jsr  $44CD
00413f: 20 79 41  jsr  $4179
004142: 60        rts
004143: a9 ac     lda  #$AC
004145: 85 f8     sta  $F8
004147: a9 41     lda  #$41
004149: 85 f9     sta  $F9
00414b: a9 5e     lda  #$5E
00414d: 85 fa     sta  $FA
00414f: a9 50     lda  #$50
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
00417e: 9c c5 4e  stz  $4EC5
004181: 73 c5 4e c6 4e 98 01  tii  $4EC5 $4EC6 $0198
004188: a9 ac     lda  #$AC
00418a: 85 f8     sta  $F8
00418c: a9 41     lda  #$41
00418e: 85 f9     sta  $F9
004190: a9 c5     lda  #$C5
004192: 85 fa     sta  $FA
004194: a9 4e     lda  #$4E
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
