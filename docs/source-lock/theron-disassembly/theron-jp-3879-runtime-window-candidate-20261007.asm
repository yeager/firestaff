; Authentic JP Track 02 candidate context; not a loader receipt.
; Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Raw context offset: 611631 (141-byte context; context SHA-256:
; 1f6df3c02976c33d01f8e15cd088ce186e46a7b7405c3bc6ef865c2cd2a94b10)
; Candidate 13-byte runtime window begins at raw offset 611695. Its
; provisional CPU alignment is $489f, placing the observed LDA at $48a8.
; This candidate-relative alignment does not bind the media offset to CD RAM.
; MAME 0.285 unidasm, h6280, base PC $485f, 139-byte decode prefix.
; Listing SHA-256: 526a0e4061e81a18786ac3cccb15879b5dfc25ec0f03e697c266e6e0c980e955
; The final two context bytes are $bd $1b; the next 3-byte instruction is
; left undecoded because it would cross the authenticated context boundary.
; BEGIN MAME OUTPUT
00485f: 14 09     trb  $09
004861: 00        brk
004862: 09 04     ora  #$04
004864: 40        rti
004865: 4a        lsr  a
004866: c8        iny
004867: 14 12     trb  $12
004869: 00        brk
00486a: 09 04     ora  #$04
00486c: 80 4c     bra  $48BA
00486e: ec 14 1b  cpx  $1B14
004871: 00        brk
004872: 09 04     ora  #$04
004874: c0 4e     cpy  #$4E
004876: 10 15     bpl  $488D
004878: 24 00     bit  $00
00487a: 04 04     tsb  $04
00487c: 00        brk
00487d: 51 20     eor  ($20),y
00487f: 15 1c     ora  $1C,x
004881: 04 0c     tsb  $0C
004883: 04 00     tsb  $00
004885: 52 50     eor  ($50)
004887: 35 1c     and  $1C,x
004889: 08        php
00488a: 0c 09 00  tsb  $0009
00488d: 55 bc     eor  $BC,x
00488f: 15 1c     ora  $1C,x
004891: 11 0c     ora  ($0C),y
004893: 06 c0     asl  $C0
004895: 5b        ill  $5B
004896: 04 16     tsb  $16
004898: 00        brk
004899: 17 28     rmb1 $28
00489b: 02        sxy
00489c: 40        rti
00489d: 60        rts
00489e: 58        cli
00489f: 16 00     asl  $00,x
0048a1: 04 1c     tsb  $1C
0048a3: 02        sxy
0048a4: 80 65     bra  $490B
0048a6: 00        brk
0048a7: 00        brk
0048a8: a9 08     lda  #$08
0048aa: 20 fb 44  jsr  $44FB
0048ad: 4c 20 b9  jmp  $B920
0048b0: 85 8a     sta  $8A
0048b2: 20 26 45  jsr  $4526
0048b5: a9 07     lda  #$07
0048b7: c0 80     cpy  #$80
0048b9: 90 02     bcc  $48BD
0048bb: a9 01     lda  #$01
0048bd: 85 07     sta  $07
0048bf: da        phx
0048c0: a9 01     lda  #$01
0048c2: 20 05 45  jsr  $4505
0048c5: 8a        txa
0048c6: 10 04     bpl  $48CC
0048c8: a9 00     lda  #$00
0048ca: 80 03     bra  $48CF
0048cc: bd 3c 4f  lda  $4F3C,x
0048cf: 8d ff 48  sta  $48FF
0048d2: 98        tya
0048d3: 10 01     bpl  $48D6
0048d5: c2        cly
0048d6: b9 13 49  lda  $4913,y
0048d9: 8d 00 49  sta  $4900
0048dc: 8d 0f 49  sta  $490F
0048df: b9 17 49  lda  $4917,y
0048e2: 8d 01 49  sta  $4901
0048e5: 8d 10 49  sta  $4910
0048e8: a6 8a     ldx  $8A
; END MAME OUTPUT
