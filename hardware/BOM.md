# Bill of Materials — GarminBridge DIY Kit

No soldering required. Just one component and a cable.

## Component List

| Qty | Part | Sourcing Options | Approx. Cost |
|-----|------|-----------------|--------------|
| 1 | **Seeed XIAO nRF52840** | [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) / Amazon / AliExpress | $6–$13 |
| 1 | **USB-A to USB-C cable** (data-capable) | Any Android phone/tablet charging cable | $0 (use existing) |
| 1 | USB power source | Treadmill's USB-A port (recommended) / wall adapter / power bank | $0 |
| 1 | 3D-printed case (optional) | Print yourself — see [hardware/case/](case/) | ~$0.50 filament |

**Total DIY cost: ~$10–$15** (board only; you likely already have a USB cable)

## Sourcing Notes

### Seeed XIAO nRF52840

| Source | Price | Lead Time | Notes |
|--------|-------|-----------|-------|
| [Seeed Studio (official)](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) | ~$9.90 | 5–10 days | Most reliable; worldwide shipping |
| Amazon | ~$12–15 | 1–2 days (Prime) | Convenient; higher price |
| AliExpress | ~$5–7 | 2–4 weeks | Cheapest option; slower shipping |
| Mouser / DigiKey | ~$11 | 2–5 days | Good for bulk; reliable stock |

> **Important:** Make sure to buy the **nRF52840** version (Bluetooth 5.0), not the XIAO nRF52832 or any Wi-Fi variant. The correct board has "BLE" in its name and uses the nRF52840 chip.

### USB Power Requirements

GarminBridge draws approximately 15–25 mA in normal operation (Bluetooth active, LEDs on).
Any USB-A or USB-C 5V source works:
- **Treadmill's built-in USB port** — most convenient; keeps everything on one power switch
- **USB wall adapter** — any 5V adapter, even a phone charger
- **USB power bank** — great for testing or if the treadmill has no USB port

### Cable Note

The XIAO has a **USB-C port**. Treadmill USB ports are **USB-A** (the standard rectangular type). You need a **USB-A to USB-C cable** — the same cable used to charge most Android phones.

Use a **data-capable** cable, not a charge-only cable. Charge-only cables lack the data lines needed for firmware updates via UF2. Any cable that syncs your Android phone will work fine.

## What You Don't Need

- No soldering iron
- No breadboard or jumper wires
- No resistors, capacitors, or other passive components
- No antenna (the XIAO nRF52840 has a built-in PCB antenna)
- No phone app or computer during operation (only needed for firmware updates)

## Firmware

Flash the pre-compiled UF2 firmware from the [Releases page](https://github.com/boge12/Garmin_bridge/releases). See the [Updater page](https://boge12.github.io/Garmin_bridge/web/updater.html) for step-by-step instructions.

To compile from source, install the [Adafruit nRF52 Arduino BSP](https://learn.adafruit.com/introducing-the-adafruit-nrf52840-feather/arduino-bsp-setup) (version ≥ 1.5.0) and open `firmware/garmin_bridge/garmin_bridge.ino` in Arduino IDE. Select board: **Seeed nRF52840 XIAO**.
