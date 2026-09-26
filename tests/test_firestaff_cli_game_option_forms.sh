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
check_form --duration -1 --version
check_form --duration 0 --version
check_form --boot-probe-frames 0 --version

check_invalid_integer_value() {
    option=$1
    value=$2
    diagnostic=$3
    output=$("$firestaff_bin" "$option" "$value" --version 2>&1) && {
        printf 'fail: invalid %s value was accepted: %s\n%s\n' "$option" "$value" "$output" >&2
        exit 1
    }
    case "$output" in
        *"$diagnostic"*) ;;
        *)
            printf 'fail: invalid %s value did not produce the expected diagnostic: %s\n%s\n' "$option" "$value" "$output" >&2
            exit 1
            ;;
    esac
}

check_missing_integer_option() {
    option=$1
    diagnostic=$2
    output=$("$firestaff_bin" "$option" --version 2>&1) && {
        printf 'fail: missing %s value was accepted\n%s\n' "$option" "$output" >&2
        exit 1
    }
    case "$output" in
        *"$diagnostic"*) ;;
        *)
            printf 'fail: missing %s value did not produce the expected diagnostic\n%s\n' "$option" "$output" >&2
            exit 1
            ;;
    esac
}

for value in '' abc 1.5 2147483648; do
    check_invalid_integer_value --duration "$value" '--duration must be an integer'
    check_invalid_integer_value --boot-probe-frames "$value" '--boot-probe-frames must be an integer'
done
check_invalid_integer_value --duration -2 '--duration must be an integer'
check_invalid_integer_value --boot-probe-frames -1 '--boot-probe-frames must be an integer'
check_missing_integer_option --duration '--duration requires an integer'
check_missing_integer_option --boot-probe-frames '--boot-probe-frames requires an integer'

for option in \
    --boot-probe-expect-champions \
    --boot-probe-expect-level-loaded \
    --boot-probe-expect-map \
    --boot-probe-expect-runtime-tick-min \
    --boot-probe-expect-runtime-tick-max \
    --boot-probe-expect-startup-active \
    --boot-probe-expect-startup-frame-min \
    --boot-probe-expect-startup-frame-max \
    --boot-probe-expect-startup-animation-active \
    --boot-probe-expect-title-frame-min \
    --boot-probe-expect-title-frame-max \
    --boot-probe-expect-title-frame-boundary \
    --boot-probe-expect-title-ready \
    --ra-hardcore; do
    case "$option" in
        --boot-probe-expect-level-loaded|--boot-probe-expect-startup-active|\
        --boot-probe-expect-startup-animation-active|\
        --boot-probe-expect-title-ready|--ra-hardcore)
            check_invalid_integer_value "$option" 2 'requires an integer from 0 through 1'
            check_invalid_integer_value "$option" abc 'requires an integer from 0 through 1'
            check_form "$option" 1 --version
            ;;
        *)
            check_invalid_integer_value "$option" abc 'requires an integer from 0 through'
            check_invalid_integer_value "$option" 2147483648 'requires an integer from 0 through'
            ;;
    esac
    check_missing_integer_option "$option" 'requires an integer from 0 through'
done
check_invalid_integer_value --boot-probe-expect-champions -1 'requires an integer from 0 through'
check_invalid_integer_value --boot-probe-expect-map -1 'requires an integer from 0 through'
check_invalid_integer_value --duration --version '--duration requires an integer'
check_invalid_integer_value --boot-probe-frames --version '--boot-probe-frames requires an integer'

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
