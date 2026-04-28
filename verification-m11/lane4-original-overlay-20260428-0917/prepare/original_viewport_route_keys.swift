import Foundation
import CoreGraphics
import ApplicationServices

if CommandLine.arguments.count != 4 {
    fputs("usage: original_viewport_route_keys.swift PID ROUTE_EVENTS SKIP_STARTUP_SELECTOR\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}
let route = CommandLine.arguments[2].split(separator: " ").map(String.init)
let skipStartupSelector = CommandLine.arguments[3] == "1"
let source = CGEventSource(stateID: .hidSystemState)

let keycodes: [String: CGKeyCode] = [
    "a": 0, "s": 1, "d": 2, "f": 3, "h": 4, "g": 5, "z": 6, "x": 7, "c": 8, "v": 9,
    "b": 11, "q": 12, "w": 13, "e": 14, "r": 15, "y": 16, "t": 17,
    "one": 18, "1": 18, "two": 19, "2": 19, "three": 20, "3": 20, "four": 21, "4": 21,
    "six": 22, "6": 22, "five": 23, "5": 23, "zero": 29, "0": 29,
    "o": 31, "u": 32, "i": 34, "p": 35, "l": 37, "j": 38, "k": 40,
    "n": 45, "m": 46,
    "enter": 36, "return": 36, "space": 49, "esc": 53, "escape": 53,
    "f1": 122, "f2": 120, "f3": 99, "f4": 118,
    "left": 123, "right": 124, "down": 125, "up": 126,
    "kp1": 83, "kp2": 84, "kp3": 85, "kp4": 86, "kp5": 87, "kp6": 88,
    "kp7": 89, "kp8": 91, "kp9": 92, "kp0": 82, "kpenter": 76
]

func post(_ key: CGKeyCode, _ down: Bool, flags: CGEventFlags = []) {
    guard let event = CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: down) else { return }
    event.flags = flags
    event.postToPid(pid)
}
func tap(_ key: CGKeyCode, _ delayUs: useconds_t = 120_000) {
    post(key, true)
    usleep(20_000)
    post(key, false)
    usleep(delayUs)
}
func cmdF5() {
    post(55, true, flags: .maskCommand)   // Command
    usleep(20_000)
    post(96, true, flags: .maskCommand)   // F5
    usleep(20_000)
    post(96, false, flags: .maskCommand)
    usleep(20_000)
    post(55, false)
    usleep(180_000)
}

func dosboxWindowBounds() -> CGRect? {
    let opts: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
    guard let windows = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else { return nil }
    for window in windows {
        guard let ownerPid = window[kCGWindowOwnerPID as String] as? pid_t, ownerPid == pid else { continue }
        guard let boundsDict = window[kCGWindowBounds as String] as? [String: Any] else { continue }
        guard
            let x = boundsDict["X"] as? CGFloat,
            let y = boundsDict["Y"] as? CGFloat,
            let w = boundsDict["Width"] as? CGFloat,
            let h = boundsDict["Height"] as? CGFloat,
            w > 0, h > 0
        else { continue }
        return CGRect(x: x, y: y, width: w, height: h)
    }
    return nil
}

func clickOriginalFrame(x: Int, y: Int) {
    guard let bounds = dosboxWindowBounds() else {
        fputs("could not find DOSBox window bounds for click:\(x),\(y)\n", stderr)
        exit(3)
    }
    // Map DM1's raw 320x200 coordinate space into the visible DOSBox content.
    // DOSBox Staging may letterbox/pillarbox depending on the current mode; use
    // a centered aspect-fit rectangle so clicks stay anchored to original pixels.
    let contentAspect = 320.0 / 200.0
    var contentW = Double(bounds.width)
    var contentH = contentW / contentAspect
    if contentH > Double(bounds.height) {
        contentH = Double(bounds.height)
        contentW = contentH * contentAspect
    }
    let left = Double(bounds.minX) + (Double(bounds.width) - contentW) / 2.0
    let top = Double(bounds.minY) + (Double(bounds.height) - contentH) / 2.0
    let px = left + ((Double(x) + 0.5) / 320.0) * contentW
    let py = top + ((Double(y) + 0.5) / 200.0) * contentH
    let point = CGPoint(x: px, y: py)
    guard let down = CGEvent(mouseEventSource: source, mouseType: .leftMouseDown, mouseCursorPosition: point, mouseButton: .left),
          let up = CGEvent(mouseEventSource: source, mouseType: .leftMouseUp, mouseCursorPosition: point, mouseButton: .left) else { return }
    down.postToPid(pid)
    usleep(45_000)
    up.postToPid(pid)
    print("click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height))")
    usleep(180_000)
}

// Original PC 3.4 startup selector: graphics=1, sound=1, input=1.
// The generated DOSBox config launches 'DM VGA' directly, which bypasses that
// selector.  In that state, posting the legacy selector keys would hit the
// title/game screens and shift the whole capture route.
if !skipStartupSelector {
    for _ in 0..<3 {
        tap(18) // '1'
        tap(36) // Return
    }
}

for token in route {
    let lowerToken = token.lowercased()
    print("route-token \(token)")
    if lowerToken == "shot" || lowerToken == "capture" || lowerToken == "screenshot" || lowerToken.hasPrefix("shot:") {
        cmdF5()
    } else if lowerToken.hasPrefix("wait:") {
        let msText = String(lowerToken.dropFirst("wait:".count))
        guard let ms = UInt32(msText) else {
            fputs("invalid wait token: \(token)\n", stderr)
            exit(2)
        }
        usleep(ms * 1000)
    } else if lowerToken.hasPrefix("click:") {
        let coords = lowerToken.dropFirst("click:".count).split(separator: ",")
        guard coords.count == 2, let x = Int(coords[0]), let y = Int(coords[1]), x >= 0, x < 320, y >= 0, y < 200 else {
            fputs("invalid click token: \(token)\n", stderr)
            exit(2)
        }
        clickOriginalFrame(x: x, y: y)
    } else if let key = keycodes[lowerToken] {
        tap(key)
    } else {
        fputs("unknown route token: \(token)\n", stderr)
        exit(2)
    }
}
