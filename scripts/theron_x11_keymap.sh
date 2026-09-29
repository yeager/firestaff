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
