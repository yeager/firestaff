#!/bin/sh
# Keep pasted macOS rich-text commands and conventional --option=value
# invocations on the same parser path as the documented --game <id> form.

set -eu

firestaff_bin=${FIRESTAFF_BIN:?FIRESTAFF_BIN must name the firestaff executable}

check_form() {
    output=$("$firestaff_bin" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        exit 1
    }
    case "$output" in
        'Firestaff v'*) ;;
        *)
            printf 'fail: game option form was not accepted:' >&2
            printf ' %s' "$@" >&2
            printf '\n%s\n' "$output" >&2
            exit 1
            ;;
    esac
}

check_form --game dm1 --version
check_form --game=dm1 --version
check_form '—game' dm1 --version
check_form '–game=dm1' --version
check_form --width 1 --height 4096 --version

check_invalid_game() {
    output=$("$firestaff_bin" "$@" --version 2>&1) && {
        printf 'fail: invalid --game value was accepted:' >&2
        printf ' %s' "$@" >&2
        printf '\n%s\n' "$output" >&2
        exit 1
    }
    case "$output" in
        *'--game requires dm1, csb, dm2, nexus, or theron'*) ;;
        *)
            printf 'fail: invalid --game value did not produce the documented diagnostic:' >&2
            printf ' %s' "$@" >&2
            printf '\n%s\n' "$output" >&2
            exit 1
            ;;
    esac
}

check_invalid_game --game nonsense
check_invalid_game --game=unknown
check_invalid_game --game --version

for scale in 0 1 2 3 4 5 1x 2x 3x 4x fit stretch; do
    check_form --scale-mode "$scale" --version
done

check_invalid_scale() {
    scale=$1
    output=$("$firestaff_bin" --scale-mode "$scale" --version 2>&1) && {
        printf 'fail: invalid scale mode was accepted: %s\n%s\n' "$scale" "$output" >&2
        exit 1
    }
    case "$output" in
        *'--scale-mode must be 0..5'*) ;;
        *)
            printf 'fail: invalid scale mode did not produce the documented diagnostic: %s\n%s\n' "$scale" "$output" >&2
            exit 1
            ;;
    esac
}

check_invalid_scale ''
check_invalid_scale abc
check_invalid_scale 6
check_invalid_scale -1
check_invalid_scale 2X
check_invalid_scale +2
check_invalid_scale ' 2'
check_invalid_scale '2 '

check_invalid_dimension() {
    option=$1
    value=$2
    output=$("$firestaff_bin" "$option" "$value" --version 2>&1) && {
        printf 'fail: invalid %s was accepted: %s\n%s\n' "$option" "$value" "$output" >&2
        exit 1
    }
    case "$output" in
        *"$option must be an integer from 1 through 4096"*) ;;
        *)
            printf 'fail: invalid %s did not produce the documented diagnostic: %s\n%s\n' "$option" "$value" "$output" >&2
            exit 1
            ;;
    esac
}

check_missing_dimension() {
    option=$1
    output=$("$firestaff_bin" "$option" --version 2>&1) && {
        printf 'fail: missing %s value was accepted\n%s\n' "$option" "$output" >&2
        exit 1
    }
    case "$output" in
        *"$option must be an integer from 1 through 4096"*) ;;
        *)
            printf 'fail: missing %s value did not produce the documented diagnostic\n%s\n' "$option" "$output" >&2
            exit 1
            ;;
    esac
}

for option in --width --height; do
    for dimension in '' abc 0 -1 4097 2147483648 ' 960' '960x'; do
        check_invalid_dimension "$option" "$dimension"
    done
    check_missing_dimension "$option"
done

printf '%s\n' 'test_firestaff_cli_game_option_forms: PASS'
