#!/usr/bin/env swift
import CoreGraphics
import Foundation

let arguments = CommandLine.arguments
guard arguments.count == 4,
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

down.postToPid(targetPid)
sleep(holdSeconds)
up.postToPid(targetPid)
print("quartz_event_access=granted")
print("quartz_keypair=posted_to_pid")
print("quartz_target_pid=\(targetPid)")
