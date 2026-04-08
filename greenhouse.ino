/*
 * ============================================================
 *   Automated Greenhouse Monitoring & Control System
 * ============================================================
 *  Sensors  : Soil Moisture | DHT11 (Temp & Humidity) | LDR
 *  Actuator : Water Pump via Relay
 *  Platform : Arduino Uno / Nano
 *  App      : Blynk (Legacy / Blynk IoT)
 * ============================================================
 *  Blynk Virtual Pins:
 *    V0  - Soil Moisture (%)
 *    V1  - Temperature (°C)
 *    V2  - Humidity (%)
 *    V3  - Light Level (%)
 *    V4  - Pump Status (0=OFF / 1=ON)
 *    V5  - Emergency Manual Pump Button (from app)
 * ============================================================
 */

// ──────────────────────────────────────────────
//  Library Includes
// ──────────────────────────────────────────────
#include <SPI.h>
#include <WiFiNINA.h>           // Use <ESP8266WiFi.h> for NodeMCU/ESP8266
                                // Use <WiFi.h>       for ESP32
#include <BlynkSimpleWiFiNINA.h>// Match to your WiFi library
#include <DHT.h>

// ──────────────────────────────────────────────
//  Blynk Credentials  ← FILL THESE IN
// ──────────────────────────────────────────────
char auth[]     = "YOUR_BLYNK_AUTH_TOKEN";   // From Blynk app / console
char ssid[]     = "YOUR_WIFI_SSID";
char pass[]     = "YOUR_WIFI_PASSWORD";

// ──────────────────────────────────────────────
//  Pin Definitions
// ──────────────────────────────────────────────
#define SOIL_MOISTURE_PIN   A0   // Analog pin – soil moisture sensor
#define LDR_PIN             A1   // Analog pin – LDR light sensor
#define DHT_PIN             7    // Digital pin – DHT11/DHT22 data
#define RELAY_PIN           8    // Digital pin – relay module IN

// ──────────────────────────────────────────────
//  Sensor Configuration
// ──────────────────────────────────────────────
#define DHT_TYPE            DHT11  // Change to DHT22 if you use DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// ──────────────────────────────────────────────
//  Threshold Settings
// ──────────────────────────────────────────────
const int SOIL_DRY_THRESHOLD  = 40;  // % – pump turns ON  below this
const int SOIL_WET_THRESHOLD  = 60;  // % – pump turns OFF above this

// ──────────────────────────────────────────────
//  Timing
// ──────────────────────────────────────────────
const unsigned long SENSOR_INTERVAL = 2000; // Read sensors every 2 s
unsigned long lastSensorRead = 0;

// ──────────────────────────────────────────────
//  State Variables
// ──────────────────────────────────────────────
bool pumpRunning      = false;
bool emergencyOverride = false;  // True when user manually triggers pump

BlynkTimer timer;

// ──────────────────────────────────────────────
//  Helper: map raw ADC → percentage (0–100 %)
// ──────────────────────────────────────────────

/**
 * Soil moisture sensor outputs HIGH voltage when DRY, LOW when WET.
 * Typical dry  value ≈ 1023  →  0 % moisture
 * Typical wet  value ≈  300  → 100 % moisture
 * Calibrate the two boundary values for your specific sensor.
 */
int readSoilMoisture() {
  int raw = analogRead(SOIL_MOISTURE_PIN);
  int pct = map(raw, 1023, 300, 0, 100);
  return constrain(pct, 0, 100);
}

/**
 * LDR: lower resistance (lower analog value) = more light.
 * Invert so 100 % = very bright, 0 % = very dark.
 */
int readLightLevel() {
  int raw = analogRead(LDR_PIN);
  int pct = map(raw, 1023, 0, 0, 100);
  return constrain(pct, 0, 100);
}

// ──────────────────────────────────────────────
//  Pump Control
// ──────────────────────────────────────────────
void setPump(bool on) {
  pumpRunning = on;
  // Many relay modules are ACTIVE-LOW (LOW = ON). Adjust if needed.
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);

  // Update Blynk button widget to reflect real pump state
  Blynk.virtualWrite(V4, on ? 1 : 0);

  Serial.print("[PUMP] ");
  Serial.println(on ? "ON" : "OFF");
}

// ──────────────────────────────────────────────
//  Blynk: Emergency / Manual Pump Button (V5)
//  Widget type: Button (Switch mode) in Blynk app
// ──────────────────────────────────────────────
BLYNK_WRITE(V5) {
  int val = param.asInt();

  if (val == 1) {
    emergencyOverride = true;
    setPump(true);
    Serial.println("[BLYNK] Emergency pump ON triggered.");
  } else {
    emergencyOverride = false;
    // Let automatic logic decide pump state on next cycle
    Serial.println("[BLYNK] Emergency override released.");
  }
}

// ──────────────────────────────────────────────
//  Main sensor read + control logic
// ──────────────────────────────────────────────
void readAndControl() {
  // ── Soil moisture ──────────────────────────
  int soilPct = readSoilMoisture();

  // ── DHT (Temperature & Humidity) ──────────
  float temperature = dht.readTemperature();   // Celsius
  float humidity    = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("[DHT] Read failed – check wiring.");
    temperature = 0;
    humidity    = 0;
  }

  // ── Light level ───────────────────────────
  int lightPct = readLightLevel();

  // ── Serial debug ──────────────────────────
  Serial.print("[SENSOR] Soil: "); Serial.print(soilPct);    Serial.print("% | ");
  Serial.print("Temp: ");          Serial.print(temperature); Serial.print("°C | ");
  Serial.print("Humidity: ");      Serial.print(humidity);    Serial.print("% | ");
  Serial.print("Light: ");         Serial.print(lightPct);    Serial.println("%");

  // ── Send to Blynk ─────────────────────────
  Blynk.virtualWrite(V0, soilPct);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, lightPct);

  // ── Automatic pump logic ───────────────────
  // Only run automatic control if emergency override is NOT active
  if (!emergencyOverride) {
    if (soilPct < SOIL_DRY_THRESHOLD && !pumpRunning) {
      Serial.println("[CONTROL] Soil dry – auto pump ON.");
      setPump(true);
    } else if (soilPct >= SOIL_WET_THRESHOLD && pumpRunning) {
      Serial.println("[CONTROL] Soil moist enough – auto pump OFF.");
      setPump(false);
    }
  }
}

// ──────────────────────────────────────────────
//  Setup
// ──────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // Pin modes
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF at startup (active-low module)

  // Init DHT
  dht.begin();

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);

  // Schedule sensor reads every 2 seconds via BlynkTimer
  timer.setInterval(SENSOR_INTERVAL, readAndControl);

  Serial.println("=== Automated Greenhouse System Started ===");
}

// ──────────────────────────────────────────────
//  Loop
// ──────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
}
