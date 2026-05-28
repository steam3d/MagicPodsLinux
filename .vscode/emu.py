#!/usr/bin/env python3
import asyncio
import json
import signal
import sys
import threading
from copy import deepcopy

try:
    import websockets
except ImportError:  # pragma: no cover - dependency hint
    print("Missing dependency: websockets. Install with: pip install websockets")
    sys.exit(1)


HOST = "0.0.0.0"
PORT = 2020

API = {"init": {"api": 0, "version": "2.0.7"}}

STATE = {
    "defaultbluetooth": {"enabled": True},
    "headphones": [
        {
            "name": "AirPods Pro 3 (magicpods)",
            "address": "AA:BB:CC:DD:EE:FF",
            "connected": True,
        },
        {"name": "Galaxy Buds", "address": "CC:BB:CC:DD:EE:FF", "connected": False},
    ],
    "info": {
        "vendor": 0x4c,
        "model": 0x200a,
        "color": 0x0f,
        "address": "AA:BB:CC:DD:EE:FF",
        "capabilities": {
            "adaptiveAudioNoise": {"readonly": False, "selected": 50},
            "anc": {"options": 23, "readonly": False, "selected": 1},
            "ancOneAirPod": {"readonly": False, "selected": False},
            "battery": {
                "case": {"battery": 90, "charging": False, "status": 3},
                "left": {"battery": 50, "charging": False, "status": 2},
                "readonly": True,
                "right": {"battery": 25, "charging": True, "status": 2},
                "single": {"battery": 0, "charging": False, "status": 0},
            },
            "conversationAwareness": {"readonly": False, "selected": True},
            "endCall": {"readonly": False, "selected": 2},
            "personalizedVolume": {"readonly": False, "selected": False},
            "pressAndHoldDuration": {"readonly": False, "selected": 0},
            "pressSpeed": {"readonly": False, "selected": 0},
            "toneVolume": {"readonly": False, "selected": 29},
            "volumeSwipe": {"readonly": False, "selected": True},
            "volumeSwipeLength": {"readonly": False, "selected": 0},
            "bluetoothCodec": {
                "options": [
                    ["off", "Off"],
                    ["a2dp-sink-sbc", "High Fidelity Playback (A2DP Sink, codec SBC)"],
                    [
                        "a2dp-sink-sbc_xq",
                        "High Fidelity Playback (A2DP Sink, codec SBC-XQ)",
                    ],
                    ["a2dp-sink", "High Fidelity Playback (A2DP Sink, codec AAC)"],
                    [
                        "headset-head-unit-cvsd",
                        "Headset Head Unit (HSP/HFP, codec CVSD)",
                    ],
                    ["headset-head-unit", "Headset Head Unit (HSP/HFP, codec mSBC)"],
                ],
                "readonly": False,
                "selected": "a2dp-sink",
            },
        },
        "connected": True,
        "name": "AirPods Pro 3 (magicpods)",
    },
}


PRESETS = [
    ("Empty", {}),
    ("Info empty", {"info": {}}),
    (
        "Info all",
        {
            "defaultbluetooth": {"enabled": True},
            "headphones": [
                {
                    "address": "CC:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods ",
                    "vendor": "4c00",
                    "model": "0a20",
                    "color": "0f",
                },
                {
                    "address": "AA:BB:CC:DD:EE:FF",
                    "connected": True,
                    "name": "AirPods Pro 3 (magicpods)",
                    "vendor": "4c00",
                    "model": "0a20",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
            ],
            "info": {
                "vendor": 0x4c,
                "model": 0x200a,
                "color": 0x0f,
                "address": "AA:BB:CC:DD:EE:FF",
                "capabilities": {
                    "adaptiveAudioNoise": {"readonly": False, "selected": 50},
                    "anc": {"options": 23, "readonly": False, "selected": 1},
                    "ancOneAirPod": {"readonly": False, "selected": False},
                    "battery": {
                        "case": {"battery": 90, "charging": False, "status": 3},
                        "left": {"battery": 50, "charging": False, "status": 2},
                        "readonly": True,
                        "right": {"battery": 25, "charging": True, "status": 2},
                        "single": {"battery": 0, "charging": False, "status": 0},
                    },
                    "conversationAwareness": {"readonly": False, "selected": True},
                    "endCall": {"readonly": False, "selected": 2},
                    "personalizedVolume": {"readonly": False, "selected": False},
                    "pressAndHoldDuration": {"readonly": False, "selected": 0},
                    "pressSpeed": {"readonly": False, "selected": 0},
                    "toneVolume": {"readonly": False, "selected": 29},
                    "volumeSwipe": {"readonly": False, "selected": True},
                    "volumeSwipeLength": {"readonly": False, "selected": 0},
                    "bluetoothCodec": {
                        "options": [
                            ["off", "Off"],
                            [
                                "a2dp-sink-sbc",
                                "High Fidelity Playback (A2DP Sink, codec SBC)",
                            ],
                            [
                                "a2dp-sink-sbc_xq",
                                "High Fidelity Playback (A2DP Sink, codec SBC-XQ)",
                            ],
                            [
                                "a2dp-sink",
                                "High Fidelity Playback (A2DP Sink, codec AAC)",
                            ],
                            [
                                "headset-head-unit-cvsd",
                                "Headset Head Unit (HSP/HFP, codec CVSD)",
                            ],
                            [
                                "headset-head-unit",
                                "Headset Head Unit (HSP/HFP, codec mSBC)",
                            ],
                        ],
                        "readonly": False,
                        "selected": "a2dp-sink",
                    },
                },
                "connected": True,
                "name": "AirPods Pro 3 (magicpods)",
            },
        },
    ),
    (
        "Info common",
        {
            "info": {
                "vendor": 0,
                "model": 0,
                "color": 0,
                "address": "AA:BB:CC:DD:EE:FF",
                "capabilities": {
                    "battery": {
                        "case": {"battery": 0, "charging": False, "status": 0},
                        "left": {"battery": 0, "charging": False, "status": 0},
                        "readonly": True,
                        "right": {"battery": 0, "charging": False, "status": 0},
                        "single": {"battery": 50, "charging": False, "status": 2},
                    },
                    "bluetoothCodec": {
                        "options": [
                            ["off", "Off"],
                            [
                                "a2dp-sink-sbc",
                                "High Fidelity Playback (A2DP Sink, codec SBC)",
                            ],
                            [
                                "a2dp-sink-sbc_xq",
                                "High Fidelity Playback (A2DP Sink, codec SBC-XQ)",
                            ],
                            [
                                "a2dp-sink",
                                "High Fidelity Playback (A2DP Sink, codec AAC)",
                            ],
                            [
                                "headset-head-unit-cvsd",
                                "Headset Head Unit (HSP/HFP, codec CVSD)",
                            ],
                            [
                                "headset-head-unit",
                                "Headset Head Unit (HSP/HFP, codec mSBC)",
                            ],
                        ],
                        "readonly": False,
                        "selected": "a2dp-sink",
                    },
                },
                "connected": True,
                "name": "HUAWEI FreeBuds 5 (magicpods)",
            }
        },
    ),
    (
        "Info min",
        {
            "info": {
                "vendor": 0,
                "model": 0,
                "color": 0,
                "address": "AA:BB:CC:DD:EE:FF",
                "capabilities": {
                    "battery": {
                        "case": {"battery": 0, "charging": False, "status": 0},
                        "left": {"battery": 0, "charging": False, "status": 0},
                        "readonly": True,
                        "right": {"battery": 0, "charging": False, "status": 0},
                        "single": {"battery": 50, "charging": False, "status": 2},
                    },
                },
                "connected": True,
                "name": "SBH20",
            }
        },
    ),
    (
        "headphones empty",
        {"headphones": []},
    ),
    (
        "headphones",
        {
            "headphones": [
                {
                    "address": "СС:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods ",
                    "vendor": "4c00",
                    "model": "0a20",
                    "color": "0f",
                },
                {
                    "address": "AA:BB:CC:DD:EE:FF",
                    "connected": True,
                    "name": "AirPods Pro 3 (magicpods)",
                    "vendor": "4c00",
                    "model": "0a20",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
                {
                    "address": "DD:BB:CC:DD:EE:FF",
                    "connected": False,
                    "name": "AirPods Pro (magicpods)",
                    "vendor": "0",
                    "model": "0",
                    "color": "0",
                },
            ]
        },
    ),
    (
        "Bt Off",
        {"defaultbluetooth": {"enabled": False}},
    ),
    (
        "Bt On",
        {"defaultbluetooth": {"enabled": True}},
    ),
    (
        "Animation air2 show",
        {
        "animation": {
        "address": "CC:BB:CC:DD:EE:FF",
        "name": "AirPods 2",
        "connected": False,
        "vendor": 0x4c,
        "model": 0x2027,
        "color": 0x0,
        "show": True,
        "battery": {
            "case": {
                "battery": 1,
                "charging": False,
                "status": 3
            },
            "left": {
                "battery": 1,
                "charging": False,
                "status": 2
            },
            "readonly": True,
            "right": {
                "battery": 1,
                "charging": True,
                "status": 2
            },
            "single": {
                "battery": 0,
                "charging": False,
                "status": 0
            }
        }
    }
    }    
    ),
    (
        "Animation max show",
        {
        "animation": {
        "address": "CC:BB:CC:DD:EE:FC",
        "name": "AirPods Max",
        "connected": False,
        "vendor": 0x4c,
        "model": 0x200a,
        "color": 0x11,
        "show": True,
        "battery": {
            "case": {
                "battery": 0,
                "charging": False,
                "status": 0
            },
            "left": {
                "battery": 0,
                "charging": False,
                "status": 0
            },
            "readonly": True,
            "right": {
                "battery": 0,
                "charging": True,
                "status": 0
            },
            "single": {
                "battery": 50,
                "charging": True,
                "status": 2
            }
        }
    }
    }    
    ),
       (
        "Animation air2 hide",
        {
        "animation": {
        "address": "CC:BB:CC:DD:EE:FF",
        "name": "AirPods Pro",
        "connected": False,
        "vendor": 0x4c,
        "model": 0x200f,
        "color": 0x0,
        "show": False,
        "battery": {
            "case": {
                "battery": 90,
                "charging": False,
                "status": 3
            },
            "left": {
                "battery": 50,
                "charging": False,
                "status": 2
            },
            "readonly": True,
            "right": {
                "battery": 25,
                "charging": True,
                "status": 2
            },
            "single": {
                "battery": 0,
                "charging": False,
                "status": 0
            }
        }
    }
    }    
    ),
]


CLIENTS = set()


def _merge_info_capabilities(update_caps):
    info = STATE.get("info") or {}
    caps = info.get("capabilities") or {}
    for name, update in update_caps.items():
        current = caps.get(name)
        if not isinstance(current, dict):
            continue
        if current.get("readonly"):
            continue
        if "options" in current:
            options = current["options"]
            selected = update.get("selected")
            if isinstance(options, list):
                valid = any(
                    item[0] == selected for item in options if isinstance(item, list)
                )
                if not valid:
                    continue
            elif isinstance(options, int) and isinstance(selected, int):
                if selected & options != selected:
                    continue
        if "selected" in update:
            current["selected"] = update["selected"]
    info["capabilities"] = caps
    STATE["info"] = info


async def _broadcast(payload):
    if not CLIENTS:
        return
    message = json.dumps(payload)
    dead = []
    for ws in CLIENTS:
        try:
            await ws.send(message)
        except Exception:
            dead.append(ws)
    for ws in dead:
        CLIENTS.discard(ws)


async def _send(ws, payload):
    await ws.send(json.dumps(payload))


def _parse_message(raw):
    try:
        data = json.loads(raw)
    except Exception:
        return None
    if not isinstance(data, dict):
        return None
    return data


def _set_connected(address, connected):
    changed = False
    for device in STATE["headphones"]:
        if device["address"] == address:
            if device["connected"] != connected:
                device["connected"] = connected
                changed = True
    return changed


async def _handle_method(ws, data):
    method = data.get("method")
    if not method:
        await ws.send("")
        return

    if method == "GetDefaultBluetoothAdapter":
        await _send(ws, {"defaultbluetooth": STATE["defaultbluetooth"]})
        return

    if method == "EnableDefaultBluetoothAdapter":
        STATE["defaultbluetooth"]["enabled"] = True
        await _send(ws, {"defaultbluetooth": STATE["defaultbluetooth"]})
        await _broadcast({"defaultbluetooth": STATE["defaultbluetooth"]})
        return

    if method == "DisableDefaultBluetoothAdapter":
        STATE["defaultbluetooth"]["enabled"] = False
        await _send(ws, {"defaultbluetooth": STATE["defaultbluetooth"]})
        await _broadcast({"defaultbluetooth": STATE["defaultbluetooth"]})
        return

    if method == "GetDevices":
        await _send(ws, {"headphones": STATE["headphones"]})
        return

    if method == "ConnectDevice":
        args = data.get("arguments") or {}
        address = args.get("address")
        if not address:
            await ws.send("")
            return
        _set_connected(address, True)
        await _send(ws, {"headphones": STATE["headphones"]})
        await _broadcast({"headphones": STATE["headphones"]})
        return

    if method == "DisconnectDevice":
        args = data.get("arguments") or {}
        address = args.get("address")
        if not address:
            await ws.send("")
            return
        _set_connected(address, False)
        await _send(ws, {"headphones": STATE["headphones"]})
        await _broadcast({"headphones": STATE["headphones"]})        
        return

    if method == "GetActiveDeviceInfo":
        await _send(ws, {"info": STATE["info"]})
        return

    if method == "GetAll":
        await _send(
            ws,
            {
                "headphones": STATE["headphones"],
                "defaultbluetooth": STATE["defaultbluetooth"],
                "info": STATE["info"],
            },
        )
        return

    if method == "SetCapabilities":
        args = data.get("arguments") or {}
        address = args.get("address")
        capabilities = args.get("capabilities")
        if not address or not isinstance(capabilities, dict):
            return
        info = STATE.get("info") or {}
        if info.get("address") != address:
            return
        _merge_info_capabilities(capabilities)
        await _broadcast({"info": STATE["info"]})
        return

    await ws.send("")


async def _ws_handler(ws):
    CLIENTS.add(ws)
    print(f"Client connected: {getattr(ws, 'remote_address', None)}")
    await _broadcast(API)
    try:
        async for raw in ws:
            data = _parse_message(raw)
            if data is None:
                await ws.send("")
                continue
            await _handle_method(ws, data)
    finally:
        CLIENTS.discard(ws)
        print(f"Client disconnected: {getattr(ws, 'remote_address', None)}")


def _apply_preset_payload(payload):
    if "defaultbluetooth" in payload:
        STATE["defaultbluetooth"].update(payload["defaultbluetooth"])
    if "headphones" in payload:
        STATE["headphones"] = payload["headphones"]        
    if "info" in payload:
        info = STATE.get("info") or {}
        update = payload["info"]
        if "capabilities" in update:
            caps = update["capabilities"]
            if info.get("capabilities") and isinstance(caps, dict):
                _merge_info_capabilities(caps)
            else:
                info["capabilities"] = caps
        for key in ("name", "address", "connected"):
            if key in update:
                info[key] = update[key]
        STATE["info"] = info


def _console_menu():
    print("\nPresets:")
    for idx, (label, _) in enumerate(PRESETS, start=1):
        print(f"{idx} - {label}")
    print("r - refresh menu, q - quit\n")


def _console_loop(loop):
    _console_menu()
    while True:
        try:
            choice = input("Send: ").strip().lower()
        except EOFError:
            break
        if choice in ("q", "quit", "exit"):
            loop.call_soon_threadsafe(loop.stop)
            break
        if choice in ("r", "refresh", "menu"):
            _console_menu()
            continue
        if not choice.isdigit():
            print("Unknown option. Use a number, r, or q.")
            continue
        idx = int(choice) - 1
        if idx < 0 or idx >= len(PRESETS):
            print("Unknown preset.")
            continue
        label, payload = PRESETS[idx]
        _apply_preset_payload(deepcopy(payload))
        future = asyncio.run_coroutine_threadsafe(_broadcast(payload), loop)
        try:
            future.result(timeout=5)
        except Exception as exc:
            print(f"Broadcast failed: {exc}")
        else:
            print(f"Sent: {label}")


async def _run_server():
    async with websockets.serve(_ws_handler, HOST, PORT):
        await asyncio.Future()


def main():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    stop_event = threading.Event()

    def _handle_sigint(_sig, _frame):
        if stop_event.is_set():
            return
        stop_event.set()
        loop.call_soon_threadsafe(loop.stop)

    signal.signal(signal.SIGINT, _handle_sigint)

    server_task = loop.create_task(_run_server())
    console_thread = threading.Thread(target=_console_loop, args=(loop,), daemon=True)
    console_thread.start()

    print(f"WebSocket emulator listening on ws://{HOST}:{PORT}/")
    try:
        loop.run_forever()
    finally:
        server_task.cancel()
        with contextlib.suppress(asyncio.CancelledError):
            loop.run_until_complete(server_task)
        loop.close()


if __name__ == "__main__":
    import contextlib

    main()
