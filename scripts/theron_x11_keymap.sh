#!/usr/bin/env bash

theron_x11_key_for_sdl_mapping() {
    local key=$1
    local scancode=$2

    case "$key:$scancode" in
        i:91) printf '%s' KP_3 ;;
        ii:90) printf '%s' KP_2 ;;
        up:26) printf '%s' w ;;
        down:22) printf '%s' s ;;
        left:4) printf '%s' a ;;
        right:7) printf '%s' d ;;
        i:32) printf '%s' 3 ;;
        ii:31) printf '%s' 2 ;;
        up:82) printf '%s' Up ;;
        down:81) printf '%s' Down ;;
        left:80) printf '%s' Left ;;
        right:79) printf '%s' Right ;;
        i:12) printf '%s' i ;;
        i:29) printf '%s' z ;;
        ii:27) printf '%s' x ;;
        i:54) printf '%s' comma ;;
        i:55) printf '%s' period ;;
        ii:54) printf '%s' comma ;;
        ii:55) printf '%s' period ;;
        *) return 1 ;;
    esac
}

theron_x11_key_for_sdl_scancode() {
    case "$1" in
        4) printf '%s' a ;;
        5) printf '%s' b ;;
        6) printf '%s' c ;;
        7) printf '%s' d ;;
        8) printf '%s' e ;;
        9) printf '%s' f ;;
        10) printf '%s' g ;;
        11) printf '%s' h ;;
        12) printf '%s' i ;;
        13) printf '%s' j ;;
        14) printf '%s' k ;;
        15) printf '%s' l ;;
        16) printf '%s' m ;;
        17) printf '%s' n ;;
        18) printf '%s' o ;;
        19) printf '%s' p ;;
        20) printf '%s' q ;;
        21) printf '%s' r ;;
        22) printf '%s' s ;;
        23) printf '%s' t ;;
        24) printf '%s' u ;;
        25) printf '%s' v ;;
        26) printf '%s' w ;;
        27) printf '%s' x ;;
        28) printf '%s' y ;;
        29) printf '%s' z ;;
        30) printf '%s' 1 ;;
        31) printf '%s' 2 ;;
        32) printf '%s' 3 ;;
        33) printf '%s' 4 ;;
        34) printf '%s' 5 ;;
        35) printf '%s' 6 ;;
        36) printf '%s' 7 ;;
        37) printf '%s' 8 ;;
        38) printf '%s' 9 ;;
        39) printf '%s' 0 ;;
        40) printf '%s' Return ;;
        41) printf '%s' Escape ;;
        42) printf '%s' BackSpace ;;
        43) printf '%s' Tab ;;
        44) printf '%s' space ;;
        45) printf '%s' minus ;;
        46) printf '%s' equal ;;
        47) printf '%s' bracketleft ;;
        48) printf '%s' bracketright ;;
        49) printf '%s' backslash ;;
        50) printf '%s' numbersign ;;
        51) printf '%s' semicolon ;;
        52) printf '%s' apostrophe ;;
        53) printf '%s' grave ;;
        54) printf '%s' comma ;;
        55) printf '%s' period ;;
        56) printf '%s' slash ;;
        57) printf '%s' Caps_Lock ;;
        58) printf '%s' F1 ;;
        59) printf '%s' F2 ;;
        60) printf '%s' F3 ;;
        61) printf '%s' F4 ;;
        62) printf '%s' F5 ;;
        63) printf '%s' F6 ;;
        64) printf '%s' F7 ;;
        65) printf '%s' F8 ;;
        66) printf '%s' F9 ;;
        67) printf '%s' F10 ;;
        68) printf '%s' F11 ;;
        69) printf '%s' F12 ;;
        70) printf '%s' Print ;;
        71) printf '%s' Scroll_Lock ;;
        72) printf '%s' Pause ;;
        73) printf '%s' Insert ;;
        74) printf '%s' Home ;;
        75) printf '%s' Page_Up ;;
        76) printf '%s' Delete ;;
        77) printf '%s' End ;;
        78) printf '%s' Page_Down ;;
        79) printf '%s' Right ;;
        80) printf '%s' Left ;;
        81) printf '%s' Down ;;
        82) printf '%s' Up ;;
        89) printf '%s' KP_1 ;;
        90) printf '%s' KP_2 ;;
        91) printf '%s' KP_3 ;;
        92) printf '%s' KP_4 ;;
        93) printf '%s' KP_5 ;;
        94) printf '%s' KP_6 ;;
        95) printf '%s' KP_7 ;;
        96) printf '%s' KP_8 ;;
        97) printf '%s' KP_9 ;;
        98) printf '%s' KP_0 ;;
        101) printf '%s' Menu ;;
        224) printf '%s' Control_L ;;
        225) printf '%s' Shift_L ;;
        226) printf '%s' Alt_L ;;
        227) printf '%s' Super_L ;;
        228) printf '%s' Control_R ;;
        229) printf '%s' Shift_R ;;
        230) printf '%s' Alt_R ;;
        231) printf '%s' Super_R ;;
        *) return 1 ;;
    esac
}

theron_x11_chord_for_sdl_binding() {
    local binding=$1
    local -a parts
    local component key chord=

    IFS='+' read -r -a parts <<<"$binding"
    if (( ${#parts[@]} < 2 )) || [[ ! "${parts[0]}" =~ ^[0-9]+$ ]]; then
        return 1
    fi
    key=$(theron_x11_key_for_sdl_scancode "${parts[0]}" || true)
    [[ -n "$key" ]] || return 1
    for component in "${parts[@]:1}"; do
        case "$component" in
            ctrl) component=ctrl ;;
            shift) component=shift ;;
            alt) component=alt ;;
            meta|super) component=super ;;
            *) return 1 ;;
        esac
        if [[ -n "$chord" ]]; then chord+="+"; fi
        chord+="$component"
    done
    if [[ -n "$chord" ]]; then chord+="+"; fi
    printf '%s%s' "$chord" "$key"
}
