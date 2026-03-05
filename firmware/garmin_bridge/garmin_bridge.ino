/**
 * Garmin Bridge
 * =============
 * BLE FTMS Treadmill → Garmin RSC Foot Pod Bridge
 *
 * Hardware : Seeed XIAO nRF52840
 * Library  : Adafruit nRF52 Arduino (BSP ≥ 1.5.0)
 *
 * Behaviour
 * ---------
 *  Central  : connects to a Bluetooth FTMS treadmill (UUID 0x1826)
 *             reads Treadmill Data notifications (UUID 0x2ACD)
 *  Peripheral: advertises as a standard BLE RSC foot pod (UUID 0x1814)
 *             notifies the watch every 500 ms
 *
 * Cadence is deliberately set to 0 so the Garmin watch uses its own
 * wrist-based cadence measurement instead of a value from this device.
 *
 * LED states
 * ----------
 *  Blue             : booting / searching for treadmill
 *  Yellow (R+G)     : treadmill connected, waiting for watch
 *  Red              : watch connected, no treadmill
 *  Green            : fully operational
 */

#include <bluefruit.h>

// ----------------------------------------------------------------
// BLE UUIDs
// ----------------------------------------------------------------
#define FTMS_SERVICE_UUID        0x1826
#define FTMS_TREADMILL_DATA_UUID 0x2ACD

#define RSC_SERVICE_UUID         0x1814
#define RSC_MEASUREMENT_UUID     0x2A53
#define RSC_FEATURE_UUID         0x2A54
#define RSC_SENSOR_LOCATION_UUID 0x2A5D

// ----------------------------------------------------------------
// FTMS Treadmill Data flags  (16-bit, little-endian)
// ----------------------------------------------------------------
#define FTMS_FLAG_MORE_DATA       (1u << 0)  // 0 = instant speed present
#define FTMS_FLAG_AVG_SPEED       (1u << 1)
#define FTMS_FLAG_TOTAL_DISTANCE  (1u << 2)
#define FTMS_FLAG_INCLINATION     (1u << 3)
#define FTMS_FLAG_ELEVATION       (1u << 4)
#define FTMS_FLAG_INSTANT_PACE    (1u << 5)
#define FTMS_FLAG_AVG_PACE        (1u << 6)
#define FTMS_FLAG_EXP_ENERGY      (1u << 7)
#define FTMS_FLAG_HEART_RATE      (1u << 8)
#define FTMS_FLAG_MET             (1u << 9)
#define FTMS_FLAG_ELAPSED_TIME    (1u << 10)
#define FTMS_FLAG_REMAINING_TIME  (1u << 11)
#define FTMS_FLAG_FORCE_BELT      (1u << 12)

// ----------------------------------------------------------------
// RSC Measurement flags  (8-bit)
// ----------------------------------------------------------------
#define RSC_FLAG_STRIDE_LENGTH   (1u << 0)
#define RSC_FLAG_TOTAL_DISTANCE  (1u << 1)
#define RSC_FLAG_RUNNING         (1u << 2)

// ----------------------------------------------------------------
// RSC Feature bits  (16-bit)
// ----------------------------------------------------------------
// Bit 1 = Total Distance Measurement Supported
// All other bits clear (including no stride-length / cadence feature bits)
#define RSC_FEATURE_VALUE 0x0002u

// ----------------------------------------------------------------
// Peripheral (RSC foot pod)
// ----------------------------------------------------------------
BLEService        rscService(RSC_SERVICE_UUID);
BLECharacteristic rscMeasurement(RSC_MEASUREMENT_UUID, BLERead | BLENotify, 8);
BLECharacteristic rscFeature(RSC_FEATURE_UUID,         BLERead,             2);
BLECharacteristic rscSensorLocation(RSC_SENSOR_LOCATION_UUID, BLERead,      1);

// ----------------------------------------------------------------
// Central (FTMS treadmill)
// ----------------------------------------------------------------
BLEClientService        ftmsService(FTMS_SERVICE_UUID);
BLEClientCharacteristic ftmsTreadmillData(FTMS_TREADMILL_DATA_UUID);

// ----------------------------------------------------------------
// Shared state  (written from BLE notify ISR, read from main loop)
// ----------------------------------------------------------------
volatile uint16_t g_speed_kmh100 = 0;   // units: 0.01 km/h
volatile uint32_t g_distance_m   = 0;   // units: 1 m
volatile bool     g_treadmill_connected = false;
volatile bool     g_watch_connected     = false;

// ----------------------------------------------------------------
// LED helpers  (XIAO: active-low)
// ----------------------------------------------------------------
inline void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED,   r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE,  b ? LOW : HIGH);
}

void updateLED() {
  if      ( g_treadmill_connected &&  g_watch_connected) setLED(false, true,  false); // green
  else if ( g_treadmill_connected && !g_watch_connected) setLED(true,  true,  false); // yellow
  else if (!g_treadmill_connected &&  g_watch_connected) setLED(true,  false, false); // red
  else                                                   setLED(false, false, true);  // blue
}

// ----------------------------------------------------------------
// FTMS Treadmill Data parser
//
// Characteristic format (Bluetooth SIG, FTMS spec):
//   [0..1]  Flags           uint16
//   [2..3]  Instant Speed   uint16  0.01 km/h  (when MORE_DATA bit = 0)
//   [4..5]  Average Speed   uint16  0.01 km/h  (when AVG_SPEED bit = 1)
//   [6..8]  Total Distance  uint24  1 m         (when TOTAL_DISTANCE bit = 1)
//   …further optional fields skipped
// ----------------------------------------------------------------
void parseFTMSTreadmill(const uint8_t* data, uint16_t len) {
  if (len < 4) return;

  uint16_t flags = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  uint8_t  offset = 2;

  // Instantaneous Speed (present when MORE_DATA flag is NOT set)
  if (!(flags & FTMS_FLAG_MORE_DATA)) {
    if (offset + 2 > len) return;
    g_speed_kmh100 = (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
    offset += 2;
  }

  // Average Speed (skip)
  if (flags & FTMS_FLAG_AVG_SPEED) {
    offset += 2;
  }

  // Total Distance (uint24, meters)
  if ((flags & FTMS_FLAG_TOTAL_DISTANCE) && (offset + 3 <= len)) {
    g_distance_m = (uint32_t)data[offset]
                 | ((uint32_t)data[offset + 1] << 8)
                 | ((uint32_t)data[offset + 2] << 16);
  }
}

// ----------------------------------------------------------------
// Build and notify RSC Measurement
//
// Packet layout (flags = RSC_FLAG_TOTAL_DISTANCE set):
//   [0]     Flags           uint8   = 0x02
//   [1..2]  Instant Speed   uint16  0.01 m/s
//   [3]     Cadence         uint8   = 0  (watch uses its own sensor)
//   [4..7]  Total Distance  uint32  0.1 m
// ----------------------------------------------------------------
void sendRSCMeasurement() {
  // Convert speed: 0.01 km/h → 0.01 m/s
  //   speed_ms   = speed_kmh / 3.6
  //   rsc_units  = speed_kmh100 * 100 / 360   (keeps integer arithmetic)
  uint16_t speed_rsc      = (uint32_t)g_speed_kmh100 * 100 / 360;
  uint32_t distance_rsc   = g_distance_m * 10;   // m → 0.1 m

  uint8_t flags = RSC_FLAG_TOTAL_DISTANCE;
  if (speed_rsc > 0) flags |= RSC_FLAG_RUNNING;

  uint8_t buf[8];
  buf[0] = flags;
  buf[1] = speed_rsc & 0xFF;
  buf[2] = (speed_rsc >> 8) & 0xFF;
  buf[3] = 0;                              // cadence = 0
  buf[4] = distance_rsc & 0xFF;
  buf[5] = (distance_rsc >>  8) & 0xFF;
  buf[6] = (distance_rsc >> 16) & 0xFF;
  buf[7] = (distance_rsc >> 24) & 0xFF;

  rscMeasurement.notify(buf, sizeof(buf));
}

// ----------------------------------------------------------------
// Central callbacks
// ----------------------------------------------------------------
void onCentralConnect(uint16_t conn_handle) {
  if (!ftmsService.discover(conn_handle)) {
    Bluefruit.disconnect(conn_handle);
    return;
  }
  if (!ftmsTreadmillData.discover()) {
    Bluefruit.disconnect(conn_handle);
    return;
  }
  ftmsTreadmillData.enableNotify();
  g_treadmill_connected = true;
  updateLED();
}

void onCentralDisconnect(uint16_t conn_handle, uint8_t /*reason*/) {
  g_treadmill_connected = false;
  g_speed_kmh100        = 0;
  updateLED();
  Bluefruit.Scanner.start(0);   // resume scanning
}

void onFTMSNotify(BLEClientCharacteristic* /*chr*/, uint8_t* data, uint16_t len) {
  parseFTMSTreadmill(data, len);
}

// ----------------------------------------------------------------
// Scanner callback
// ----------------------------------------------------------------
void scanCallback(ble_gap_evt_adv_report_t* report) {
  if (Bluefruit.Scanner.checkReportForUuid(report, BLEUuid(FTMS_SERVICE_UUID))) {
    Bluefruit.Central.connect(report);
  }
}

// ----------------------------------------------------------------
// Peripheral callbacks
// ----------------------------------------------------------------
void onPeripheralConnect(uint16_t /*conn_handle*/) {
  g_watch_connected = true;
  updateLED();
}

void onPeripheralDisconnect(uint16_t /*conn_handle*/, uint8_t /*reason*/) {
  g_watch_connected = false;
  updateLED();
  Bluefruit.Advertising.start(0);   // resume advertising
}

// ----------------------------------------------------------------
// setup()
// ----------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);
  setLED(false, false, true);   // blue: starting

  // 1 peripheral slot + 1 central slot
  Bluefruit.begin(1, 1);
  Bluefruit.setName("GarminBridge");
  Bluefruit.setTxPower(4);

  // ---- Peripheral (RSC foot pod) ----
  Bluefruit.Periph.setConnectCallback(onPeripheralConnect);
  Bluefruit.Periph.setDisconnectCallback(onPeripheralDisconnect);

  rscService.begin();

  // Feature: only Total Distance supported; cadence NOT advertised as a feature
  // so the watch knows to rely on its own cadence sensor
  rscFeature.begin();
  uint16_t feat = RSC_FEATURE_VALUE;
  rscFeature.write(&feat, sizeof(feat));

  // Sensor location: foot (value 3 per BT SIG)
  rscSensorLocation.begin();
  uint8_t loc = 3;
  rscSensorLocation.write(&loc, sizeof(loc));

  rscMeasurement.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(rscService);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.setInterval(32, 244);   // 20 ms / 152.5 ms
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);

  // ---- Central (FTMS treadmill) ----
  ftmsTreadmillData.setNotifyCallback(onFTMSNotify);

  Bluefruit.Central.setConnectCallback(onCentralConnect);
  Bluefruit.Central.setDisconnectCallback(onCentralDisconnect);

  Bluefruit.Scanner.setRxCallback(scanCallback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterUuid(BLEUuid(FTMS_SERVICE_UUID));
  Bluefruit.Scanner.setInterval(160, 80);   // 100 ms / 50 ms
  Bluefruit.Scanner.useActiveScan(true);
  Bluefruit.Scanner.start(0);
}

// ----------------------------------------------------------------
// loop()   – notify every 500 ms
// ----------------------------------------------------------------
void loop() {
  static uint32_t lastSend = 0;

  if (g_watch_connected && (millis() - lastSend >= 500)) {
    lastSend = millis();
    sendRSCMeasurement();
  }

  updateLED();
  delay(10);
}
