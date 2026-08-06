; Theron's Quest US Track 02 — bank $1f static consumer support fragment
;
; Source: canonical retail ISO projection
;   TQUS19.iso + TQUS02End.iso
;   MD5 ceb02343868f80cec899e9b239aff2da
; Extraction: file offset $1f0000, 64 KiB, HuC6280 bank $1f
; Disassembler: da65 V2.18, --cpu huc6280 --start-addr 0
;
; This is a byte-backed fragment, not a guessed stage-3 listing.  The
; `$2450` entry is reached by the adjacent bitstream helper at `$2448`.
; The `$2600` RAM consumer seen in the live capture is intentionally not
; represented here: its bytes are loaded after a CD read and are absent from
; the static bank image.

        .setcpu  "huc6280"

; The caller and entry prologue below are byte-backed by the same
; hash-locked US/JP bank-$1f span. The caller measures the output length
; through $3b7c/$3b7d; it does not identify a level or object record.

        .org    $2386

L2386:  lda     $30
        sta     $3B7C
        lda     $31
        sta     $3B7D
        bsr     L23AD
        sec
        lda     $30
        sbc     $3B7C
        sta     $3B7C
        lda     $31
        sbc     $3B7D
        sta     $3B7D
        rts

        lda     #$01
        sta     $0F
        lda     #$09
        sta     $14
        rts

        .org    $23AD

L23AD:  lda     $2E
        sta     $32
        sta     $34
        lda     $2F
        sta     $33
        sta     $35
        ldy     #$02
        lda     ($2E),y
        sta     $00
        iny
        lda     ($2E),y
        sta     $01
        sec
        lda     $00
        sbc     #$05
        sta     $12
        lda     $01
        sbc     #$00
        sta     $13
        clc
        lda     $2E
        adc     #$06
        sta     $2E
        bcc     L23DC
        inc     $2F

L23DC:  bsr     L23AD
L23DE:  cly
        lda     $30
        sta     ($34),y
        iny
        lda     $31
        sta     ($34),y
        clc
        lda     $34
        adc     #$02
        sta     $34
        bcc     L23F3
        inc     $35
L23F3:  bsr     L242A
        lda     $10
        bne     L2403
        lda     $11
        cmp     #$01
        bne     L2403
        inc     $14
        bra     L23F3
L2403:  lda     $12
        ora     $13
        beq     L2429
        bsr     L2483
        bsr     L2459
L240D:  bsr     L242A
        lda     $10
        bne     L241D
        lda     $11
        cmp     #$01
        bne     L241D
        inc     $14
        bra     L240D
L241D:  lda     $12
        ora     $13
        beq     L2429
        bsr     L2483
        bsr     L2459
        bra     L23DE
L2429:  rts

L242A:  stz     $10
        ldy     $14
        ldx     #$08
L2430:  dec     $0F
        beq     L243E
L2434:  rol     $0E
        rol     $10
        rol     $11
        dey
        bne     L2430
        rts

        .org    $243e

L243E:  lda     ($2E)
L2440:  sta     $0E
        inc     $2E
        bne     L2448
        inc     $2F
L2448:  stx     $0F
        lda     $12
        bne     L2450
        dec     $13
L2450:  dec     $12
        bne     L2434
        lda     $13
        bne     L2434
        rts

L2459:  lda     $3B7E
        tam     #$08
        lda     $3B7F
        tam     #$10
        lda     $3B80
        tam     #$20
        lda     $3B81
        tam     #$40
        rts

L246E:  lda     $3B82
        tam     #$08
        lda     $3B83
        tam     #$10
        lda     $3B84
        tam     #$20
        lda     $3B85
        tam     #$40
        rts

L2483:  lda     $11
        beq     L2489
        bpl     L2496
L2489:  bsr     L246E
        lda     $10
        sta     ($30)
        inc     $30
        bne     L2495
        inc     $31
L2495:  rts

L2496:  sec
        lda     $10
        sbc     #$01
        sta     $10
        lda     $11
        sbc     #$01
        sta     $11
        asl     $10
        rol     $11
        clc
        lda     $10
        adc     $32
        sta     $10
        lda     $11
        adc     $33
        sta     $11
        sta     $37
        lda     $10
        sta     $36
        bsr     L2459
        cly
        lda     ($36),y
        sta     $02
        brk
