#!/usr/bin/env swift
import CoreGraphics
import Foundation

let arguments = CommandLine.arguments
let globalHid = arguments.count == 3 && arguments[2] == "--global-hid"
guard (arguments.count == 2 || globalHid),
      let targetPid = pid_t(arguments[1]),
      targetPid > 0,
      let source = CGEventSource(stateID: .hidSystemState) else {
    fputs("usage: send_theron_macos_quartz_chord.swift TARGET_PID [--global-hid]\n", stderr)
    exit(2)
}

if !CGPreflightPostEventAccess() {
    fputs("quartz_event_access=denied\n", stderr)
    exit(1)
}

func post(_ event: CGEvent) {
    if globalHid {
        event.post(tap: .cghidEventTap)
    } else {
        event.postToPid(targetPid)
    }
    Thread.sleep(forTimeInterval: 0.03)
}

/* The checked-in Mednafen profile binds command.toggle_grab to Ctrl+Shift+G.
 * Quartz virtual keycodes are Control=59, Shift=56, G=5. Emit modifier
 * transitions explicitly so SDL receives a real chord, not a bare G event. */
let control = CGKeyCode(59)
let shift = CGKeyCode(56)
let g = CGKeyCode(5)
let controlDown = CGEvent(keyboardEventSource: source, virtualKey: control, keyDown: true)!
let shiftDown = CGEvent(keyboardEventSource: source, virtualKey: shift, keyDown: true)!
let gDown = CGEvent(keyboardEventSource: source, virtualKey: g, keyDown: true)!
let gUp = CGEvent(keyboardEventSource: source, virtualKey: g, keyDown: false)!
let shiftUp = CGEvent(keyboardEventSource: source, virtualKey: shift, keyDown: false)!
let controlUp = CGEvent(keyboardEventSource: source, virtualKey: control, keyDown: false)!

controlDown.flags = .maskControl
shiftDown.flags = [.maskControl, .maskShift]
gDown.flags = [.maskControl, .maskShift]
gUp.flags = [.maskControl, .maskShift]
shiftUp.flags = .maskControl
controlUp.flags = []

post(controlDown)
post(shiftDown)
post(gDown)
post(gUp)
post(shiftUp)
post(controlUp)

print("quartz_event_access=granted")
print(globalHid ? "quartz_chord=posted_to_global_hid" : "quartz_chord=posted_to_pid")
print("quartz_target_pid=\(targetPid)")
print("quartz_chord_keys=ctrl+shift+g")
