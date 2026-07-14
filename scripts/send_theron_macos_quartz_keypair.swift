#!/usr/bin/env swift
import CoreGraphics
import Foundation

let arguments = CommandLine.arguments
guard arguments.count == 3,
      let keyCode = UInt16(arguments[1]),
      let holdSeconds = UInt32(arguments[2]),
      let source = CGEventSource(stateID: .hidSystemState),
      let down = CGEvent(keyboardEventSource: source,
                         virtualKey: CGKeyCode(keyCode), keyDown: true),
      let up = CGEvent(keyboardEventSource: source,
                       virtualKey: CGKeyCode(keyCode), keyDown: false) else {
    fputs("usage: send_theron_macos_quartz_keypair.swift KEY_CODE HOLD_SECONDS\\n",
          stderr)
    exit(2)
}

down.post(tap: .cghidEventTap)
sleep(holdSeconds)
up.post(tap: .cghidEventTap)
