# 🌱 Automated Greenhouse Monitoring & Control System

An Arduino-based automated greenhouse system that monitors **soil moisture**, **temperature**, **humidity**, and **light levels** — and automatically controls a **water pump** to keep plants healthy. All readings are displayed in real time on the **Blynk mobile app**, with an emergency manual override button.

---

## 📸 Demo

> Project built and demonstrated as a hardware prototype.  
> Mobile app (Blynk) shows live sensor readings and pump control.

---

## ✨ Features

| Feature | Details |
|---|---|
| 🌡️ Temperature | DHT11/DHT22 sensor – displayed in °C |
| 💧 Humidity | DHT11/DHT22 sensor – displayed in % |
| 🌱 Soil Moisture | Capacitive/resistive moisture sensor – auto pump trigger |
| ☀️ Light Level | LDR sensor – ambient light percentage |
| ⚙️ Auto Pump Control | Pump turns ON when soil is dry, OFF when wet |
| 🚨 Emergency Override | Manually turn pump ON/OFF from Blynk app anytime |
| 📱 Blynk Dashboard | Real-time monitoring from anywhere via WiFi |

---

## 🧰 Hardware Required

- Arduino Uno / Nano *(or NodeMCU / ESP8266 / ESP32 for built-in WiFi)*
- DHT11 or DHT22 Temperature & Humidity Sensor
- Soil Moisture Sensor (resistive or capacitive)
- LDR (Light Dependent Resistor) + 10kΩ resistor
- 5V Relay Module
- Mini Water Pump (5V DC)
- WiFi Module (ESP8266 / WiFiNINA shield) *(skip if using ESP32/NodeMCU)*
- Jumper Wires, Breadboard, Power Supply

---

## 📐 Circuit / Pin Connections

| Component | Arduino Pin |
|---|---|
| Soil Moisture Sensor (AO) | A0 |
| LDR (via voltage divider) | A1 |
| DHT11/DHT22 Data Pin | D7 |
| Relay Module IN | D8 |
| Relay VCC | 5V |
| Relay GND | GND |

> ⚠️ **Note:** Most relay modules are **active-low** (LOW = pump ON). The code handles this correctly. If your relay is active-high, swap `LOW` / `HIGH` in `setPump()`.

---

## 📱 Blynk App Setup

### Step 1 – Create a Blynk Project
1. Download the **Blynk** app (iOS / Android)
2. Create a new project → select **Arduino** as device
3. Copy the **Auth Token** sent to your email

### Step 2 – Add Widgets

| Widget | Virtual Pin | Type |
|---|---|---|
| Gauge / Label | V0 | Soil Moisture % |
| Gauge / Label | V1 | Temperature °C |
| Gauge / Label | V2 | Humidity % |
| Gauge / Label | V3 | Light Level % |
| LED / Label | V4 | Pump Status |
| **Button (Switch)** | **V5** | **Emergency Pump ON/OFF** |

### Step 3 – Fill Credentials in Code

```cpp
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

---

## 🔧 Software / Libraries

Install these via **Arduino Library Manager** (`Sketch → Include Library → Manage Libraries`):

| Library | Install Name |
|---|---|
| Blynk | `Blynk` |
| DHT sensor library | `DHT sensor library` by Adafruit |
| Adafruit Unified Sensor | `Adafruit Unified Sensor` |
| WiFiNINA *(if using WiFiNINA shield)* | `WiFiNINA` |
| ESP8266WiFi *(if using NodeMCU)* | Built-in with ESP8266 board package |

---

## ⚙️ Configuration

Open `greenhouse.ino` and adjust these constants:

```cpp
// Soil thresholds (percentage)
const int SOIL_DRY_THRESHOLD = 40;  // Pump ON  when moisture < 40%
const int SOIL_WET_THRESHOLD = 60;  // Pump OFF when moisture > 60%

// Sensor read interval
const unsigned long SENSOR_INTERVAL = 2000; // Every 2 seconds
```

**Calibrate soil sensor:**
```cpp
// In readSoilMoisture() — adjust these raw ADC values for your sensor:
int pct = map(raw, 1023, 300, 0, 100);
//              ^^^^  ^^^
//              DRY   WET  (measure with your sensor in air vs water)
```

---

## 🚨 Emergency Override

In the Blynk app, the **V5 button** acts as an emergency manual control:

- **Press ON →** Pump runs immediately regardless of soil moisture
- **Press OFF →** Returns to automatic soil-moisture-based control

This is useful when plants need immediate watering or the soil sensor reading seems off.

---

## 🔄 How It Works

```
Every 2 seconds:
  1. Read soil moisture (A0)
  2. Read temperature & humidity (DHT on D7)
  3. Read light level (LDR on A1)
  4. Send all values to Blynk (V0–V3)
  5. If NO emergency override:
       - Soil < 40% → Turn pump ON
       - Soil > 60% → Turn pump OFF
  6. Emergency override (V5 button):
       - Ignores soil reading
       - Directly controls pump
```

---

## 📁 Project Structure

```
greenhouse/
├── greenhouse.ino     ← Main Arduino sketch
└── README.md          ← This file
```

---

## 🛠️ Troubleshooting

| Problem | Solution |
|---|---|
| DHT read failed | Check wiring; add 4.7kΩ pull-up on data pin |
| Pump doesn't turn on | Check relay wiring; verify `RELAY_PIN` number |
| Blynk not connecting | Verify Auth Token, SSID, password; check WiFi signal |
| Soil reading always 0 or 100 | Recalibrate raw ADC values in `readSoilMoisture()` |
| Relay clicks but pump doesn't run | Check pump power supply (separate 5V recommended) |

---

## 🧑‍💻 Author

Built as an academic/personal project demonstrating IoT-based smart agriculture.

---

## 📜 License

MIT License – free to use, modify, and distribute.
