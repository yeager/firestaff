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
    /^pce_input_read / {
        input_reads++
        if (input_reads > read_limit) {
            failure = "trace exceeds its declared controller-read limit"
            next
        }
        if (pending_frame != "" && pending_apply && !pending_read &&
            $0 ~ / register=1000([[:space:]]|$)/) {
            pending_read = 1
            controller_read_witness_sequence = input_reads
            for (i = 1; i <= NF; i++)
                if ($i ~ /^cpu_pc=[0-9a-fA-F]+$/)
                    controller_read_witness_pc = substr($i, 8)
            if (controller_read_witness_pc ~ /^[0-9a-fA-F]+$/) {
                pc = tolower(controller_read_witness_pc)
                if (pc == "e4b7" || pc == "e4c8" ||
                    pc == "e4b4" || pc == "e4c5")
                    system_card_poll_reads++
                else
                    non_system_card_poll_reads++
            }
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
        for (i = 1; i <= NF; i++)
            if ($i ~ /^frame=[0-9]+$/)
                event_frame = substr($i, 7)
        if (event_frame == "") {
            failure = "a scripted event has no numeric frame"
            next
        }
        if (pending_frame != "" && event_frame != pending_frame) {
            if (!pending_apply)
                failure = "a scripted event frame has no nonzero apply receipt before the next frame"
            else if (!pending_read)
                failure = "a scripted event frame was not followed by a controller-port read before the next frame"
            else {
                frames_with_apply++
                frames_with_controller_read++
            }
            pending_frame = ""
        }
        if (pending_frame == "")
            pending_frame = event_frame
        events++
        # Events scheduled on one frame combine before the CPU can poll them.
        # Require a controller read after the last event in that frame group.
        pending_apply = 0
        pending_read = 0
        next
    }
    END {
        if (events != expected_events)
            failure = "observed scripted-event count does not match the requested plan"
        else if (pending_frame != "" && !pending_apply)
            failure = "the final scripted event frame has no nonzero apply receipt"
        else if (pending_frame != "" && !pending_read)
            failure = "the final scripted event frame has no subsequent controller-port read"
        else if (pending_frame != "") {
            frames_with_apply++
            frames_with_controller_read++
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
