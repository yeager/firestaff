; Authentic JP Stage-2 user-byte window [0x1879, 0x1888).
; MAME 0.285: unidasm -arch h6280 -basepc 0x5879 -skip 2883689 -count 15
; JP Track 02 SHA-256 d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Span SHA-256 bbee9339b6547617c75393de4a712a818dbc5619a756f56f01e70d57d3522aab
; Span FNV-1a-64 2647c26e2d10c559. This branch target is JP-specific.

005879: 8c 4f c8  sty  $C84F
00587c: b1 18     lda  ($18),y
00587e: 8d 8d 4f  sta  $4F8D
005881: c8        iny
005882: b1 18     lda  ($18),y
005884: 8d 8e 4f  sta  $4F8E
005887: 60        rts
