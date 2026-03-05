# Garmin Bridge

Bluetooth FTMS Treadmill → Garmin RSC Foot Pod Bridge.

Reads speed and distance from a Bluetooth FTMS treadmill and re-broadcasts
them to a Garmin watch as a standard BLE Running Speed & Cadence (RSC) foot
pod.  **Cadence is set to 0** so the watch falls back to its own wrist-based
cadence sensor.

---

## Hardware

| Part | Notes |
|------|-------|
| [Seeed XIAO nRF52840](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) | ~$10, USB-C powered |
| USB-C cable / power bank | Any 5 V source |
| Optional: 3D-printed case | See `hardware/` |

---

## Build & Flash

### 1. Install Arduino IDE 2.x

### 2. Add Adafruit nRF52 board support

In *File → Preferences → Additional Boards Manager URLs* add:

```
https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
```

Then *Tools → Board → Boards Manager* → install **Adafruit nRF52**.

> The Seeed XIAO nRF52840 uses the same nRF52840 SoC.  Select
> **Adafruit Feather nRF52840 Express** as the board target — pin
> assignments for the RGB LED are identical.

### 3. Open the sketch

```
firmware/garmin_bridge/garmin_bridge.ino
```

### 4. Compile and upload

Connect the XIAO via USB-C, select the correct COM port, click **Upload**.

To enter the UF2 bootloader manually: double-press the reset button quickly
— the XIAO will appear as a USB mass-storage drive.

---

## Pairing with your Garmin watch

1. On the watch: **Settings → Sensors & Accessories → Add New → Foot Pod**
2. The device appears as **GarminBridge**.
3. Select it and confirm.

Once paired, speed and distance come from the treadmill; cadence is measured
by the watch itself.

If you have another foot pod already paired, disable it so the watch uses
GarminBridge as the primary speed/distance source.

---

## Treadmill compatibility

Any treadmill that broadcasts the Bluetooth FTMS profile (UUID `0x1826`) is
compatible.  Common brands: NordicTrack, Sole, Horizon, Bowflex, ProForm.

If the treadmill connects to iFit, Kinomap, Zwift, or a similar app over
Bluetooth, it almost certainly supports FTMS.

Not compatible: Peloton, older belt-drive treadmills without Bluetooth, and
closed-ecosystem machines with proprietary protocols.

---

## LED states

| Colour | Meaning |
|--------|---------|
| Blue | Booting / scanning for treadmill |
| Yellow | Treadmill connected, waiting for watch |
| Red | Watch connected, no treadmill |
| Green | Fully operational |

---

## Data conversion

| Field | FTMS unit | RSC unit | Conversion |
|-------|-----------|----------|------------|
| Speed | 0.01 km/h | 0.01 m/s | × 100 / 360 |
| Distance | 1 m | 0.1 m | × 10 |
| Cadence | — | rpm | **0** (watch measures) |

---

## License

MIT
