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
