; Authentic JP Track 02 candidate bytes at the observed $44fb call target.
; Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; The six raw candidate offsets for the $489f window are documented in
; theron-3879-runtime-window-candidates-20261007.md.
; This candidate-relative alignment subtracts $3a4 in MODE1 user-data space
; from each candidate window before converting back to raw-sector offsets.
; Resulting raw offsets: 610459, 911515, 1212571, 1513627, 1814683, 2115739.
; Each candidate-relative 256-byte span matches; SHA-256:
; 533fbdf66380e79795e31433d96a71bbf391cdb142368ac860bd7b43f9fd012b
; MAME 0.285 unidasm -arch h6280 -basepc 0x44fb -skip 610459 -count 255.
; The 255-byte listing ends at an instruction boundary ($45f9); the following
; byte is not decoded. Listing SHA-256:
; 7c92199784a4a0e35f1b231ed9b8db041742d56ab1d03dabddfb3a8956cf8965
; This static candidate does not prove a loader transfer, CD-RAM contents,
; execution of this body, or any routine/gameplay semantics.
; BEGIN MAME OUTPUT
0044fb: 08                    php
0044fc: 48                    pha
0044fd: 18                    clc
0044fe: 65 00                 adc  $00
004500: 53 20                 tam  #$20
004502: 68                    pla
004503: 28                    plp
004504: 60                    rts
004505: 08                    php
004506: 48                    pha
004507: 18                    clc
004508: 65 00                 adc  $00
00450a: 53 40                 tam  #$40
00450c: 68                    pla
00450d: 28                    plp
00450e: 60                    rts
00450f: 08                    php
004510: 43 08                 tma  #$08
004512: 80 0d                 bra  $4521
004514: 08                    php
004515: 43 10                 tma  #$10
004517: 80 08                 bra  $4521
004519: 08                    php
00451a: 43 20                 tma  #$20
00451c: 80 03                 bra  $4521
00451e: 08                    php
00451f: 43 40                 tma  #$40
004521: 38                    sec
004522: e5 00                 sbc  $00
004524: 28                    plp
004525: 60                    rts
004526: 08                    php
004527: 58                    cli
004528: ad b7 28              lda  $28B7
00452b: cd b7 28              cmp  $28B7
00452e: f0 fb                 beq  $452B
004530: 28                    plp
004531: 60                    rts
004532: f0 05                 beq  $4539
004534: 44 f0                 bsr  $4526
004536: ca                    dex
004537: d0 f9                 bne  $4532
004539: 60                    rts
00453a: 85 d8                 sta  $D8
00453c: 84 d9                 sty  $D9
00453e: 68                    pla
00453f: 18                    clc
004540: 69 01                 adc  #$01
004542: 85 d4                 sta  $D4
004544: 68                    pla
004545: 69 00                 adc  #$00
004547: 85 d5                 sta  $D5
004549: a9 45                 lda  #$45
00454b: 48                    pha
00454c: a9 77                 lda  #$77
00454e: 48                    pha
00454f: a5 d5                 lda  $D5
004551: 48                    pha
004552: a5 d4                 lda  $D4
004554: 48                    pha
004555: c2                    cly
004556: b1 d4                 lda  ($D4),y
004558: 8d 71 45              sta  $4571
00455b: 38                    sec
00455c: 65 d6                 adc  $D6
00455e: 85 d6                 sta  $D6
004560: a8                    tay
004561: e9 01                 sbc  #$01
004563: 8d 6f 45              sta  $456F
004566: ad 71 45              lda  $4571
004569: 99 ff 22              sta  $22FF,y
00456c: c3 bb 20 00 23 00 00  tdd  $20BB $2300 $0000
004573: a4 d9                 ldy  $D9
004575: a5 d8                 lda  $D8
004577: 60                    rts
004578: 85 d8                 sta  $D8
00457a: 84 d9                 sty  $D9
00457c: a4 d6                 ldy  $D6
00457e: b9 ff 22              lda  $22FF,y
004581: 8d 98 45              sta  $4598
004584: 98                    tya
004585: 38                    sec
004586: e9 02                 sbc  #$02
004588: 8d 94 45              sta  $4594
00458b: a5 d6                 lda  $D6
00458d: 18                    clc
00458e: ed 98 45              sbc  $4598
004591: 85 d6                 sta  $D6
004593: c3 00 23 bb 20 00 00  tdd  $2300 $20BB $0000
00459a: a4 d9                 ldy  $D9
00459c: a5 d8                 lda  $D8
00459e: 60                    rts
00459f: 85 d8                 sta  $D8
0045a1: 68                    pla
0045a2: 85 d4                 sta  $D4
0045a4: 68                    pla
0045a5: 85 d5                 sta  $D5
0045a7: a9 45                 lda  #$45
0045a9: 48                    pha
0045aa: a9 c6                 lda  #$C6
0045ac: 48                    pha
0045ad: a5 d5                 lda  $D5
0045af: 48                    pha
0045b0: a5 d4                 lda  $D4
0045b2: 48                    pha
0045b3: a5 d7                 lda  $D7
0045b5: 8d c0 45              sta  $45C0
0045b8: 18                    clc
0045b9: 69 0a                 adc  #$0A
0045bb: 85 d7                 sta  $D7
0045bd: 73 34 29 00 24 0a 00  tii  $2934 $2400 $000A
0045c4: a5 d8                 lda  $D8
0045c6: 60                    rts
0045c7: 85 d8                 sta  $D8
0045c9: a5 d7                 lda  $D7
0045cb: 38                    sec
0045cc: e9 0a                 sbc  #$0A
0045ce: 8d d4 45              sta  $45D4
0045d1: 85 d7                 sta  $D7
0045d3: 73 00 24 34 29 0a 00  tii  $2400 $2934 $000A
0045da: a5 d8                 lda  $D8
0045dc: 60                    rts
0045dd: 85 d8                 sta  $D8
0045df: 84 d9                 sty  $D9
0045e1: 68                    pla
0045e2: 7a                    ply
0045e3: 85 d4                 sta  $D4
0045e5: 84 d5                 sty  $D5
0045e7: 18                    clc
0045e8: 69 02                 adc  #$02
0045ea: 42                    say
0045eb: 69 00                 adc  #$00
0045ed: 48                    pha
0045ee: 5a                    phy
0045ef: 43 40                 tma  #$40
0045f1: 48                    pha
0045f2: a0 01                 ldy  #$01
0045f4: b1 d4                 lda  ($D4),y
0045f6: 8d 20 46              sta  $4620
0045f9: c8                    iny
; END MAME OUTPUT
