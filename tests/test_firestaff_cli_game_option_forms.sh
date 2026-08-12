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

printf '%s\n' 'test_firestaff_cli_game_option_forms: PASS'
