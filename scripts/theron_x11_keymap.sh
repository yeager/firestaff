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
        7) printf '%s' d ;;
        10) printf '%s' g ;;
        22) printf '%s' s ;;
        26) printf '%s' w ;;
        40) printf '%s' Return ;;
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
