# GarminBridge

**Bluetooth FTMS Treadmill → Garmin RSC Foot Pod Bridge**

Reads speed and distance from any Bluetooth FTMS treadmill and re-broadcasts them to a Garmin watch (or any RSC-capable watch) as a standard BLE Running Speed & Cadence (RSC) foot pod. No apps, no cloud, no subscription.

**Cadence is set to 0** so the watch falls back to its own wrist-based cadence sensor — this is intentional (the XIAO has no accelerometer).

---

## Web Tools

| Tool | Description |
|------|-------------|
| [Landing Page](web/index.html) | Product overview, compatibility, FAQ |
| [Live Analyzer](web/analyzer.html) | Web Bluetooth dashboard — inspect live RSC data in Chrome |
| [Firmware Updater](web/updater.html) | UF2 drag-and-drop instructions + WebUSB bootloader trigger |
| [Documentation](web/docs.html) | Full docs: setup, troubleshooting, compatibility |

---

## Hardware

| Part | Notes |
|------|-------|
| [Seeed XIAO nRF52840](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) | ~$10, USB-C powered |
| USB-C cable (data + power) | Any data-capable USB-C cable |
| Optional: 3D-printed case | See [hardware/case/](hardware/case/) |

Full BOM with sourcing options: [hardware/BOM.md](hardware/BOM.md)

---

## Build & Flash

### Quick method (pre-compiled UF2)

1. Download the latest `garmin_bridge_vX.X.X.uf2` from the [Releases page](https://github.com/boge12/Garmin_bridge/releases).
2. Double-tap the reset button on the XIAO — a USB drive named **XIAO-BOOT** appears.
3. Drag the `.uf2` file onto the drive. Done.

See [web/updater.html](web/updater.html) for guided instructions.

### Build from source

1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software).
2. Add Adafruit nRF52 board support:
   - *File → Preferences → Additional Boards Manager URLs*: add `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
   - *Tools → Board → Boards Manager* → install **Adafruit nRF52** (BSP ≥ 1.5.0)
3. Open `firmware/garmin_bridge/garmin_bridge.ino`.
4. Select board: **Seeed nRF52840 XIAO** (or Adafruit Feather nRF52840 Express — LED pins are identical).
5. Connect XIAO via USB-C, select the COM port, click **Upload**.

---

## Usage

### 1. Power on

Plug GarminBridge into any USB-C 5V source. The LED turns **blue** while scanning.

### 2. Connect treadmill

Turn on your FTMS treadmill. GarminBridge connects automatically. LED turns **yellow**.

> If iFit, Kinomap, or Zwift is connected to the treadmill, disconnect those apps first. FTMS allows only one central connection at a time.

### 3. Pair Garmin watch

On your watch: **Settings → Sensors & Accessories → Add New → Foot Pod**. Select **GarminBridge**. LED turns **green**.

Start a Run activity — speed and distance come directly from the treadmill.

---

## LED States

| Colour | Meaning |
|--------|---------|
| 🔵 Blue | Booting / scanning for treadmill |
| 🟡 Yellow | Treadmill connected, waiting for watch |
| 🔴 Red | Watch connected, no treadmill |
| 🟢 Green | Fully operational |

---

## Data Conversion

| Field | FTMS Unit | RSC Unit | Conversion |
|-------|-----------|----------|------------|
| Speed | 0.01 km/h | 1/256 m/s | `× 256 / 360` |
| Distance | 1 m | 1/10 m | `× 10` |
| Cadence | — | spm | **0** (watch measures itself) |
| Inclination | 0.1 % | — | Read from FTMS, not forwarded (RSC has no inclination field) |

> **Note on RSC speed unit:** The BT SIG GATT specification defines the RSC Instantaneous Speed field in units of **1/256 m/s**, not 0.01 m/s. Firmware v1.1.0+ uses the correct `× 256 / 360` conversion. Older firmware used `× 100 / 360`, causing watches to display ~39% of actual speed.

---

## Treadmill Compatibility

Any treadmill that broadcasts the Bluetooth **FTMS** profile (UUID `0x1826`) is compatible. Common brands: NordicTrack, ProForm, Sole, Horizon, Bowflex, Technogym.

If your treadmill connects to Zwift, Kinomap, or a fitness app over Bluetooth, it almost certainly supports FTMS.

**Not compatible:** Peloton, LifeFitness commercial machines, dumb/manual treadmills.

Full list: [web/docs.html#treadmills](web/docs.html)

---

## Watch Compatibility

Any watch supporting BLE RSC sensors (Garmin, COROS, Polar, Suunto). Apple Watch is not compatible (requires proprietary GymKit protocol).

Full list: [web/docs.html#watches](web/docs.html)

---

## Serial Debug

With `#define DEBUG 1` (default), GarminBridge prints status messages to Serial at **115200 baud**:

```
=== GarminBridge v1.1.0 ===
[INIT] Scanning for FTMS treadmill...
[BLE] Treadmill connected
[FTMS] Speed: 10.00 km/h
[FTMS] Distance: 142 m
[RSC] notify speed_raw=711 dist_raw=1420
[STATUS] treadmill=yes watch=yes speed=10.00 km/h dist=142 m incl=1.5 %
```

Set `#define DEBUG 0` to disable and save flash space.

---

## Project Structure

```
Garmin_bridge/
├── README.md
├── .gitignore
├── firmware/
│   └── garmin_bridge/
│       └── garmin_bridge.ino     ← Arduino sketch
├── hardware/
│   ├── BOM.md                    ← Bill of Materials
│   └── case/
│       └── README.md             ← 3D-printed case guide
└── web/
    ├── style.css                 ← Shared CSS design system
    ├── index.html                ← Landing page
    ├── analyzer.html             ← Web Bluetooth live dashboard
    ├── updater.html              ← Firmware updater
    └── docs.html                 ← Full documentation
```

---

## License

MIT — see [LICENSE](LICENSE).
