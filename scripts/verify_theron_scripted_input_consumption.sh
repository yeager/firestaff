#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    printf 'usage: %s TRACE PLAN READ_LIMIT\n' "${0##*/}" >&2
    exit 2
fi

trace=$1
plan=$2
read_limit=$3
if [[ ! -s "$trace" ]]; then
    printf '%s\n' 'BLOCKED: scripted-input trace is missing or empty' >&2
    exit 1
fi
if [[ ! "$read_limit" =~ ^[1-9][0-9]*$ ]] ||
   (( read_limit > 1048576 )); then
    printf '%s\n' 'BLOCKED: scripted-input read limit is invalid' >&2
    exit 1
fi
if [[ -z "$plan" ]]; then
    printf '%s\n' 'BLOCKED: scripted-input plan is empty' >&2
    exit 1
fi

IFS=',' read -r -a plan_entries <<<"$plan"
expected_events=${#plan_entries[@]}
if (( expected_events == 0 )); then
    printf '%s\n' 'BLOCKED: scripted-input plan has no events' >&2
    exit 1
fi

awk -v expected_events="$expected_events" -v read_limit="$read_limit" '
    function hex_to_dec(text,    i, digit, value) {
        text = tolower(text)
        sub(/^0x/, "", text)
        value = 0
        for (i = 1; i <= length(text); i++) {
            digit = index("0123456789abcdef", substr(text, i, 1)) - 1
            if (digit < 0 || digit > 15)
                return -1
            value = value * 16 + digit
        }
        return value
    }
    function mask_union(left, right,    bit, result) {
        result = 0
        for (bit = 1; bit <= 32768; bit *= 2)
            if (int(left / bit) % 2 || int(right / bit) % 2)
                result += bit
        return result
    }
    function contains_mask(value, mask,    bit) {
        for (bit = 1; bit <= 32768; bit *= 2)
            if (int(mask / bit) % 2 && int(value / bit) % 2 == 0)
                return 0
        return mask != 0
    }
    # Mednafen 1.32.1 PCE Fast INPUT_Read selects the direction nibble with
    # SEL=1 and the button nibble with SEL=0; its visible value is active-low.
    # The raw pad word alone therefore cannot establish what the guest read.
    function selected_bank_matches(raw, value, sel, mask,    nibble) {
        if (raw < 0 || value < 0 || sel < 0 || !contains_mask(raw, mask))
            return 0
        if (sel == 1) {
            if (pending_direction_mask == 0)
                return 0
            nibble = int(raw / 16) % 16
        } else {
            if (pending_button_mask == 0)
                return 0
            nibble = raw % 16
        }
        return value % 16 == 15 - nibble
    }
    function accept_controller_read(raw_value, returned_value, read_sel,
                                    read_index, read_pc) {
        if (pending_frame == "" || !pending_apply ||
            read_index != 0 || read_pc !~ /^[0-9a-fA-F]+$/)
            return
        if (!pending_poll_classified) {
            pc = tolower(read_pc)
            if (pc == "e4b7" || pc == "e4c8" ||
                pc == "e4b4" || pc == "e4c5")
                system_card_poll_reads++
            else
                non_system_card_poll_reads++
            pending_poll_classified = 1
        }
        if (selected_bank_matches(raw_value, returned_value,
                                  read_sel, pending_mask)) {
            if (read_sel == 1)
                pending_direction_read = 1
            else
                pending_button_read = 1
        }
        if ((!pending_direction_mask || pending_direction_read) &&
            (!pending_button_mask || pending_button_read)) {
            pending_read = 1
            controller_read_witness_sequence = input_reads
            controller_read_witness_pc = read_pc
        }
    }
    pending_result && $0 !~ /^pce_input_result / {
        failure = "legacy PCE input result did not immediately follow its raw read"
        pending_result = 0
    }
    /^pce_input_read / {
        input_reads++
        if (input_reads > read_limit) {
            failure = "trace exceeds its declared controller-read limit"
            next
        }
        if (pending_frame != "" && pending_apply &&
            $0 ~ / register=1000([[:space:]]|$)/) {
            raw_value = -1
            returned_value = -1
            read_sel = -1
            read_index = -1
            read_pc = ""
            for (i = 1; i <= NF; i++) {
                if ($i ~ /^raw=[0-9a-fA-F]+$/)
                    raw_value = hex_to_dec(substr($i, 5))
                if ($i ~ /^value=[0-9a-fA-F]+$/)
                    returned_value = hex_to_dec(substr($i, 7))
                if ($i ~ /^sel=[01]$/)
                    read_sel = substr($i, 5)
                if ($i ~ /^index=[0-9]+$/)
                    read_index = substr($i, 7)
                if ($i ~ /^cpu_pc=[0-9a-fA-F]+$/)
                    read_pc = substr($i, 8)
            }
            if (returned_value >= 0) {
                accept_controller_read(raw_value, returned_value, read_sel,
                                       read_index, read_pc)
                pending_result = 0
            } else {
                # The original PCE core emits the raw port sample and the
                # returned active-low byte as adjacent trace records. Pair
                # them without counting one CPU poll twice; PCE Fast emits
                # both values on its single pce_input_read row.
                pending_result = 1
                pending_result_pc = read_pc
                pending_result_register = "1000"
                pending_result_raw = raw_value
                pending_result_sel = read_sel
                pending_result_index = read_index
            }
        }
        next
    }
    /^pce_input_result / {
        if (pending_result) {
            result_raw = result_value = result_sel = result_index = -1
            result_pc = result_register = ""
            for (i = 1; i <= NF; i++) {
                if ($i ~ /^raw=[0-9a-fA-F]+$/)
                    result_raw = hex_to_dec(substr($i, 5))
                if ($i ~ /^value=[0-9a-fA-F]+$/)
                    result_value = hex_to_dec(substr($i, 7))
                if ($i ~ /^sel=[01]$/)
                    result_sel = substr($i, 5)
                if ($i ~ /^index=[0-9]+$/)
                    result_index = substr($i, 7)
                if ($i ~ /^register=[0-9a-fA-F]+$/)
                    result_register = tolower(substr($i, 10))
                if ($i ~ /^cpu_pc=[0-9a-fA-F]+$/)
                    result_pc = substr($i, 8)
            }
            if (result_register == pending_result_register &&
                result_pc == pending_result_pc &&
                result_raw == pending_result_raw &&
                result_sel == pending_result_sel &&
                result_index == pending_result_index)
                accept_controller_read(result_raw, result_value,
                                       result_sel, result_index, result_pc)
            pending_result = 0
        }
        next
    }
    /^scripted_pce_input_apply / {
        apply_frame = ""
        scripted_mask = ""
        for (i = 1; i <= NF; i++) {
            if ($i ~ /^frame=[0-9]+$/)
                apply_frame = substr($i, 7)
            if ($i ~ /^scripted=[0-9a-fA-F]+$/)
                scripted_mask = substr($i, 10)
        }
        if (pending_frame != "" && apply_frame == pending_frame &&
            scripted_mask != "" && scripted_mask !~ /^0+$/)
            pending_apply = 1
        next
    }
    /^scripted_pce_input_event / {
        event_frame = ""
        event_mask = ""
        for (i = 1; i <= NF; i++)
            if ($i ~ /^frame=[0-9]+$/)
                event_frame = substr($i, 7)
            else if ($i ~ /^mask=[0-9a-fA-F]+$/)
                event_mask = substr($i, 6)
        if (event_frame == "") {
            failure = "a scripted event has no numeric frame"
            next
        }
        mask_value = hex_to_dec(event_mask)
        if (mask_value <= 0) {
            failure = "a scripted event has no valid nonzero controller mask"
            next
        }
        if (mask_value > 255) {
            failure = "a scripted event uses controller bits outside the supported two input banks"
            next
        }
        if (pending_frame != "" && event_frame != pending_frame) {
            if (!pending_apply)
                failure = "a scripted event frame has no nonzero apply receipt before the next frame"
            else if (!pending_read)
                failure = "a scripted event frame had no controller-port read exposing its scripted mask before the next frame"
            else {
                frames_with_apply++
                frames_with_controller_read++
                frames_with_mask_match++
            }
            pending_frame = ""
        }
        if (pending_frame == "") {
            pending_frame = event_frame
            pending_mask = mask_value
            pending_direction_mask = int(mask_value / 16) % 16
            pending_button_mask = mask_value % 16
        } else {
            pending_mask = mask_union(pending_mask, mask_value)
            pending_direction_mask = mask_union(pending_direction_mask, int(mask_value / 16) % 16)
            pending_button_mask = mask_union(pending_button_mask, mask_value % 16)
        }
        events++
        # Events scheduled on one frame combine before the CPU can poll them.
        # Require a controller read after the last event in that frame group.
        pending_apply = 0
        pending_read = 0
        pending_poll_classified = 0
        pending_direction_read = 0
        pending_button_read = 0
        next
    }
    END {
        if (failure == "") {
            if (events != expected_events)
                failure = "observed scripted-event count does not match the requested plan"
            else if (pending_result)
                failure = "final legacy PCE raw input read has no adjacent result record"
            else if (pending_frame != "" && !pending_apply)
                failure = "the final scripted event frame has no nonzero apply receipt"
            else if (pending_frame != "" && !pending_read)
                failure = "the final scripted event frame had no controller-port read exposing its scripted mask"
            else if (pending_frame != "") {
                frames_with_apply++
                frames_with_controller_read++
                frames_with_mask_match++
            }
        }
        if (failure != "") {
            printf "BLOCKED: %s (events=%d/%d input_reads=%d read_limit=%d)\n", \
                failure, events, expected_events, input_reads, read_limit > "/dev/stderr"
            exit 1
        }
        printf "source=mednafen-scripted-input-consumption-v1\n"
        printf "scripted_input_events=%d\n", events
        printf "event_frames_with_apply=%d\n", frames_with_apply
        printf "event_frames_followed_by_controller_read=%d\n", frames_with_controller_read
        printf "event_frames_with_scripted_mask_read=%d\n", frames_with_mask_match
        printf "controller_read_witness_sequence=%d\n", controller_read_witness_sequence
        printf "controller_read_witness_pc=%s\n", controller_read_witness_pc
        printf "system_card_poll_reads=%d\n", system_card_poll_reads
        printf "non_system_card_poll_reads=%d\n", non_system_card_poll_reads
        printf "observed_input_reads=%d\n", input_reads
        printf "input_read_limit=%d\n", read_limit
        print "controller_poll_boundary=verified"
        if (non_system_card_poll_reads > 0)
            print "game_or_non_system_card_poll_boundary=observed"
        else if (system_card_poll_reads > 0)
            print "game_or_non_system_card_poll_boundary=not_observed"
        else
            print "game_or_non_system_card_poll_boundary=unknown"
    }
' "$trace"
