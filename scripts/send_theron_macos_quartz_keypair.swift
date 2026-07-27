#!/usr/bin/env swift
import CoreGraphics
import Foundation
import AppKit

let arguments = CommandLine.arguments
let globalHid = arguments.count == 5 && arguments[4] == "--global-hid"
guard (arguments.count == 4 || globalHid),
      let keyCode = UInt16(arguments[1]),
      let holdSeconds = UInt32(arguments[2]),
      let targetPid = pid_t(arguments[3]),
      targetPid > 0,
      let source = CGEventSource(stateID: .hidSystemState),
      let down = CGEvent(keyboardEventSource: source,
                         virtualKey: CGKeyCode(keyCode), keyDown: true),
      let up = CGEvent(keyboardEventSource: source,
                       virtualKey: CGKeyCode(keyCode), keyDown: false) else {
    fputs("usage: send_theron_macos_quartz_keypair.swift KEY_CODE HOLD_SECONDS TARGET_PID\\n",
          stderr)
    exit(2)
}

if !CGPreflightPostEventAccess() {
    fputs("quartz_event_access=denied\\n", stderr)
    exit(1)
}

guard let targetApplication = NSRunningApplication(processIdentifier: targetPid) else {
    fputs("quartz_target_missing\\n", stderr)
    exit(1)
}
let activationAccepted = targetApplication.activate()
Thread.sleep(forTimeInterval: 0.2)
let observedPid = NSWorkspace.shared.frontmostApplication?.processIdentifier ?? 0
let targetIsFrontmost = observedPid == targetPid
if globalHid && !targetIsFrontmost {
    fputs("quartz_target_not_frontmost activation=\(activationAccepted ? 1 : 0) expected=\(targetPid) observed=\(observedPid)\\n", stderr)
    exit(1)
}

if globalHid {
    down.post(tap: .cghidEventTap)
} else {
    down.postToPid(targetPid)
}
sleep(holdSeconds)
if globalHid {
    up.post(tap: .cghidEventTap)
} else {
    up.postToPid(targetPid)
}
print("quartz_event_access=granted")
print(globalHid ? "quartz_keypair=posted_to_global_hid" : "quartz_keypair=posted_to_pid")
print("quartz_target_pid=\(targetPid)")
print("quartz_activation=\(activationAccepted ? "accepted" : "rejected")")
print("quartz_frontmost_pid=\(observedPid)")
print(targetIsFrontmost ? "quartz_target_focus=frontmost" : "quartz_target_focus=background_pid_delivery")
