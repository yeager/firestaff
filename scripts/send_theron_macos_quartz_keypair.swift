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

if globalHid {
    guard let targetApplication = NSRunningApplication(processIdentifier: targetPid) else {
        fputs("quartz_global_target_missing\\n", stderr)
        exit(1)
    }
    _ = targetApplication.activate()
    Thread.sleep(forTimeInterval: 0.2)
    let observedPid = NSWorkspace.shared.frontmostApplication?.processIdentifier ?? 0
    guard observedPid == targetPid else {
        fputs("quartz_global_target_not_frontmost expected=\(targetPid) observed=\(observedPid)\\n", stderr)
        exit(1)
    }
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
