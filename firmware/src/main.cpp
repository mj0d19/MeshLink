/*
 * MeshLink Leader - WiFi Dashboard & Mesh Controller
 *
 * - BLE: Scans and connects to participants to relay sightings
 * - WiFi: Connects to AP and serves live dashboard
 * - Web: /api/status endpoint for JSON data
 */

#include <Arduino.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <math.h>


// ==========================================
// WIFI SETTINGS - CHANGE THESE
// ==========================================
#define WIFI_SSID "Faisal977_4G"
#define WIFI_PASS "63655144"

#define LED_LEADER_PIN 2       // LED 1: Leader status (built-in, always on)
#define LED_ROLLCALL_PIN 4     // LED 2: Roll call active (on while button held)
#define LED_ALLPRESENT_PIN 5   // LED 3: All participants present
#define BUTTON_ROLLCALL_PIN 15 // Momentary button for roll call

#define EXPECTED_PARTICIPANTS 2 // How many participants we expect
#define ALLPRESENT_TIMEOUT 3000 // LED stays on for 3 seconds after roll call

#define SCAN_TIME 5
#define MAX_DISTANCE 2.0
#define STALE_TIMEOUT 30000
#define MIN_RSSI_THRESHOLD -85 // Relaxed: allow devices up to ~5m away

// Service/Characteristic UUIDs
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID sightingsCharUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// RSSI to Distance
#define TX_POWER -59
#define PATH_LOSS_N 2.5

// Global Objects
AsyncWebServer server(80);
BLEScan *pBLEScan;

// Data Structures
#define MAX_FOUND 10
struct FoundParticipant {
  String addressStr;
  String name;
  int rssi;
  bool valid;
};
FoundParticipant foundParticipants[MAX_FOUND];
int foundCount = 0;

#define MAX_PARTICIPANTS 10
struct Participant {
  String address;
  String name;
  float distance;
  String sightings;
  unsigned long lastSeen;
  bool active;
};
Participant participants[MAX_PARTICIPANTS];
int participantCount = 0;

#define MAX_DEVICES 30
struct SeenDevice {
  String name;
  float distance;
  String seenBy;
  unsigned long lastSeen;
  bool active;
};
SeenDevice allDevices[MAX_DEVICES];
int deviceCount = 0;

// Roll Call State
bool rollCallActive = false;
bool allParticipantsFound = false;
unsigned long allPresentLedOffTime = 0; // When to turn off the all-present LED

// HTML Dashboard - Blueprint Style
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>MeshLink Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { 
      font-family: 'Courier New', monospace; 
      background: #0a0f0a; 
      color: #00ff88; 
      min-height: 100vh;
    }
    .header {
      background: #0d1a0d;
      border-bottom: 1px solid #00ff88;
      padding: 15px 30px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .logo { font-size: 1.4em; font-weight: bold; letter-spacing: 2px; }
    .status-dot { 
      width: 10px; height: 10px; 
      background: #00ff88; 
      border-radius: 50%; 
      display: inline-block;
      transition: transform 0.1s, box-shadow 0.1s;
    }
    .status-dot.flash { 
      transform: scale(1.5); 
      box-shadow: 0 0 10px #00ff88; 
    }
    @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.4; } }
    .container { display: flex; padding: 20px; gap: 20px; }
    .panel {
      background: #0d1a0d;
      border: 1px solid #1a3a1a;
      border-radius: 4px;
      padding: 20px;
    }
    .topology { flex: 2; min-height: 400px; }
    .devices { flex: 1; }
    h2 { 
      color: #00ff88; 
      font-size: 0.9em; 
      text-transform: uppercase; 
      letter-spacing: 1px;
      margin-bottom: 15px;
      padding-bottom: 10px;
      border-bottom: 1px solid #1a3a1a;
    }
    .node {
      fill: #0d1a0d;
      stroke: #00ff88;
      stroke-width: 1;
    }
    .node-label { fill: #00ff88; font-size: 10px; text-anchor: middle; }
    .link { stroke: #00ff88; stroke-width: 1; fill: none; }
    .link-data { stroke: #00ccff; stroke-dasharray: 5,5; animation: flow 1s linear infinite; }
    @keyframes flow { to { stroke-dashoffset: -10; } }
    .device-item {
      background: #0a150a;
      border: 1px solid #1a3a1a;
      border-left: 3px solid #00ff88;
      padding: 12px;
      margin-bottom: 10px;
      border-radius: 2px;
    }
    .device-item.relay { border-left-color: #00ccff; }
    .device-name { font-weight: bold; font-size: 0.9em; }
    .device-meta { color: #668866; font-size: 0.75em; margin-top: 5px; }
    .tag { 
      display: inline-block;
      padding: 2px 6px;
      background: #1a3a1a;
      border-radius: 2px;
      font-size: 0.7em;
      margin-left: 8px;
    }
    .tag.relay { background: #003344; color: #00ccff; }
    .legend { margin-top: 20px; font-size: 0.75em; color: #668866; }
    .legend-item { display: flex; align-items: center; gap: 8px; margin: 5px 0; }
    .legend-line { width: 30px; height: 2px; background: #00ff88; }
    .legend-line.data { background: #00ccff; }
  </style>
</head>
<body>
  <div class="header">
    <div class="logo">MESHLINK</div>
    <div><span class="status-dot"></span> LIVE</div>
  </div>
  
  <div class="container">
    <div class="panel topology">
      <h2>Network Topology</h2>
      <svg id="topology" width="100%" height="350" viewBox="0 0 500 300">
        <defs>
          <marker id="arrow" markerWidth="10" markerHeight="10" refX="9" refY="3" orient="auto">
            <path d="M0,0 L0,6 L9,3 z" fill="#00ff88"/>
          </marker>
        </defs>
        <!-- Dynamic topology will be built by JavaScript -->
        <g id="topologyContent"></g>
      </svg>
      <div class="legend">
        <div class="legend-item"><div class="legend-line"></div> Connection</div>
        <div class="legend-item"><div class="legend-line data"></div> Data Flow</div>
      </div>
    </div>
    
    <div class="panel devices">
      <h2>Device List</h2>
      <div id="deviceList">Loading...</div>
    </div>
  </div>

<script>
  function fetchStatus() {
    fetch('/api/status')
      .then(r => r.json())
      .then(data => {
        // Update device list
        let html = '';
        if(data.devices.length === 0) {
          html = '<div style="color:#668866">No devices detected</div>';
        }
        data.devices.forEach(d => {
          let cls = d.isRelay ? 'device-item relay' : 'device-item';
          let tag = d.isRelay ? '<span class="tag relay">RELAY</span>' : '<span class="tag">DEVICE</span>';
          html += `<div class="${cls}">
            <div class="device-name">${d.name}${tag}</div>
            <div class="device-meta">${d.distance.toFixed(1)}m away</div>
          </div>`;
        });
        document.getElementById('deviceList').innerHTML = html;
        
        // Build fully dynamic topology
        let svg = '';
        let relays = data.devices.filter(d => d.isRelay);
        let devices = data.devices.filter(d => !d.isRelay);
        
        // Leader node at top center
        svg += '<rect class="node" x="200" y="20" width="100" height="40"/>';
        svg += '<text class="node-label" x="250" y="45">LEADER</text>';
        
        // Draw all participants (relays) connected to Leader
        let relayCount = relays.length;
        let relayStartX = 250 - (relayCount * 60);
        
        relays.forEach((r, i) => {
          let x = relayStartX + i * 120;
          let label = r.name.replace('MESH_PARTICIPANT_', 'P_');
          svg += `<line class="link" x1="250" y1="60" x2="${x + 50}" y2="110" marker-end="url(#arrow)"/>`;
          svg += `<rect class="node" x="${x}" y="110" width="100" height="40" style="stroke:#00ccff"/>`;
          svg += `<text class="node-label" x="${x + 50}" y="135" style="fill:#00ccff">${label}</text>`;
        });
        
        // Draw detected devices below
        let deviceCount = devices.length;
        let deviceStartX = 250 - (deviceCount * 60);
        
        devices.forEach((d, i) => {
          let x = deviceStartX + i * 120;
          let label = d.name.replace('MESH_PARTICIPANT_', 'P_');
          svg += `<line class="link link-data" x1="250" y1="150" x2="${x + 50}" y2="200"/>`;
          svg += `<rect class="node" x="${x}" y="200" width="100" height="40"/>`;
          svg += `<text class="node-label" x="${x + 50}" y="225">${label}</text>`;
        });
        
        document.getElementById('topologyContent').innerHTML = svg;
        
        // Flash the status dot
        let dot = document.querySelector('.status-dot');
        dot.classList.add('flash');
        setTimeout(() => dot.classList.remove('flash'), 150);
      });
  }
  setInterval(fetchStatus, 500);
  fetchStatus();
</script>
</body>
</html>
)rawliteral";

// Helper Functions
float calculateDistance(int rssi) {
  if (rssi == 0)
    return -1.0;
  float ratio = (TX_POWER - rssi) / (10.0 * PATH_LOSS_N);
  return pow(10.0, ratio);
}

void addSeenDevice(String name, float distance, String seenBy) {
  for (int i = 0; i < deviceCount; i++) {
    if (allDevices[i].name == name) {
      allDevices[i].distance = distance;
      allDevices[i].seenBy = seenBy;
      allDevices[i].lastSeen = millis();
      allDevices[i].active = true;
      return;
    }
  }
  if (deviceCount < MAX_DEVICES) {
    allDevices[deviceCount].name = name;
    allDevices[deviceCount].distance = distance;
    allDevices[deviceCount].seenBy = seenBy;
    allDevices[deviceCount].lastSeen = millis();
    allDevices[deviceCount].active = true;
    deviceCount++;
  }
}

void parseSightings(String sightings, String reporterName) {
  if (sightings == "NONE" || sightings.length() == 0)
    return;
  int start = 0;
  while (start < (int)sightings.length()) {
    int pipePos = sightings.indexOf('|', start);
    if (pipePos < 0)
      pipePos = sightings.length();
    String entry = sightings.substring(start, pipePos);
    int colonPos = entry.indexOf(':');
    if (colonPos > 0) {
      String name = entry.substring(0, colonPos);
      float dist = entry.substring(colonPos + 1).toFloat();
      addSeenDevice(name, dist, reporterName);
    }
    start = pipePos + 1;
  }
}

int findOrAddParticipant(String address) {
  for (int i = 0; i < participantCount; i++) {
    if (participants[i].address == address)
      return i;
  }
  if (participantCount < MAX_PARTICIPANTS) {
    participants[participantCount].address = address;
    participants[participantCount].active = false;
    return participantCount++;
  }
  return -1;
}

String connectAndReadSightings(BLEAddress address, String participantName) {
  Serial.println("    Connecting to " + participantName + "...");

  BLEClient *pClient = BLEDevice::createClient();

  // Attempt connection
  if (!pClient->connect(address)) {
    Serial.println("    [FAILED] Connection failed");
    delete pClient;
    delay(500); // Allow BLE stack to recover
    return "";
  }

  delay(100); // Stabilize connection

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("    [FAILED] Service not found");
    pClient->disconnect();
    delay(200);
    delete pClient;
    delay(300);
    return "";
  }

  BLERemoteCharacteristic *pRemoteChar =
      pRemoteService->getCharacteristic(sightingsCharUUID);
  if (pRemoteChar == nullptr) {
    Serial.println("    [FAILED] Characteristic not found");
    pClient->disconnect();
    delay(200);
    delete pClient;
    delay(300);
    return "";
  }

  String value = pRemoteChar->readValue().c_str();
  Serial.println("    [OK] Got: " + value);

  // Clean disconnect
  pClient->disconnect();
  delay(200); // Wait for disconnect to complete
  delete pClient;
  delay(300); // Allow BLE stack to fully clean up

  return value;
}

// Scans callback - Connect to ALL MESH_PARTICIPANT devices within range
class MyScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String deviceName = advertisedDevice.getName().c_str();
    // Connect to any MESH_PARTICIPANT device (001, 002, 003, etc.)
    if (!deviceName.startsWith("MESH_PARTICIPANT_"))
      return;

    // Filter by distance - ignore if too far (weak signal)
    int rssi = advertisedDevice.getRSSI();
    if (rssi < MIN_RSSI_THRESHOLD) {
      Serial.println("[SKIP] " + deviceName +
                     " - too far (RSSI: " + String(rssi) + ")");
      return;
    }

    String addr = advertisedDevice.getAddress().toString().c_str();
    for (int i = 0; i < foundCount; i++) {
      if (foundParticipants[i].addressStr == addr)
        return;
    }

    if (foundCount < MAX_FOUND) {
      foundParticipants[foundCount].addressStr = addr;
      foundParticipants[foundCount].name = deviceName;
      foundParticipants[foundCount].rssi = rssi;
      foundParticipants[foundCount].valid = true;
      foundCount++;
      Serial.println("[FOUND] Participant: " + deviceName +
                     " (RSSI: " + String(rssi) + ")");
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize LEDs
  pinMode(LED_LEADER_PIN, OUTPUT);
  pinMode(LED_ROLLCALL_PIN, OUTPUT);
  pinMode(LED_ALLPRESENT_PIN, OUTPUT);

  // Initialize button with internal pullup
  pinMode(BUTTON_ROLLCALL_PIN, INPUT_PULLUP);

  // Turn on Leader LED (always on to show this is the leader)
  digitalWrite(LED_LEADER_PIN, HIGH);
  digitalWrite(LED_ROLLCALL_PIN, LOW);
  digitalWrite(LED_ALLPRESENT_PIN, LOW);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"devices\":[";
    bool first = true;
    unsigned long now = millis();

    // Track which names we've already added
    String addedNames[20];
    int addedCount = 0;

    // Add participants (Relays) first
    for (int i = 0; i < participantCount; i++) {
      if (participants[i].active &&
          (now - participants[i].lastSeen) < STALE_TIMEOUT) {
        if (!first)
          json += ",";
        json += "{\"name\":\"" + participants[i].name +
                "\",\"distance\":" + String(participants[i].distance) +
                ",\"isRelay\":true}";
        first = false;
        if (addedCount < 20)
          addedNames[addedCount++] = participants[i].name;
      }
    }

    // Add detected devices (only if not already added)
    for (int i = 0; i < deviceCount; i++) {
      if (allDevices[i].active &&
          (now - allDevices[i].lastSeen) < STALE_TIMEOUT) {
        // Check if already added
        bool alreadyAdded = false;
        for (int j = 0; j < addedCount; j++) {
          if (addedNames[j] == allDevices[i].name) {
            alreadyAdded = true;
            break;
          }
        }
        if (alreadyAdded)
          continue;

        if (!first)
          json += ",";
        json += "{\"name\":\"" + allDevices[i].name +
                "\",\"distance\":" + String(allDevices[i].distance) +
                ",\"isRelay\":false}";
        first = false;
        if (addedCount < 20)
          addedNames[addedCount++] = allDevices[i].name;
      }
    }

    json += "]}";
    request->send(200, "application/json", json);
  });

  server.begin();

  // BLE Init
  BLEDevice::init("MESH_LEADER");

  // Lower BLE transmit power (ESP_PWR_LVL_N12 = -12dBm, lowest)
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyScanCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  // ============================================
  // ROLL CALL BUTTON AND LED LOGIC
  // ============================================

  // Check if roll call button is pressed (LOW because of INPUT_PULLUP)
  bool buttonPressed = (digitalRead(BUTTON_ROLLCALL_PIN) == LOW);

  if (buttonPressed) {
    // Button is held - roll call is active
    rollCallActive = true;
    digitalWrite(LED_ROLLCALL_PIN, HIGH);

    // Count ALL detected MESH_PARTICIPANT devices (RELAYs + DEVICEs)
    int activeCount = 0;
    unsigned long now = millis();
    String countedNames[20];
    int countedCount = 0;

    // Count from direct participants (RELAYs)
    for (int i = 0; i < participantCount; i++) {
      if (participants[i].active &&
          (now - participants[i].lastSeen) < STALE_TIMEOUT) {
        if (participants[i].name.startsWith("MESH_PARTICIPANT_")) {
          countedNames[countedCount++] = participants[i].name;
          activeCount++;
        }
      }
    }

    // Count from reported devices (seen by participants)
    for (int i = 0; i < deviceCount; i++) {
      if (allDevices[i].active &&
          (now - allDevices[i].lastSeen) < STALE_TIMEOUT) {
        if (allDevices[i].name.startsWith("MESH_PARTICIPANT_")) {
          // Check if already counted
          bool alreadyCounted = false;
          for (int j = 0; j < countedCount; j++) {
            if (countedNames[j] == allDevices[i].name) {
              alreadyCounted = true;
              break;
            }
          }
          if (!alreadyCounted && countedCount < 20) {
            countedNames[countedCount++] = allDevices[i].name;
            activeCount++;
          }
        }
      }
    }

    // Check if all expected participants are present
    if (activeCount >= EXPECTED_PARTICIPANTS) {
      allParticipantsFound = true;
      digitalWrite(LED_ALLPRESENT_PIN, HIGH); // Solid ON
      allPresentLedOffTime = millis() + ALLPRESENT_TIMEOUT;
      Serial.println("[ROLL CALL] ALL PRESENT! " + String(activeCount) + "/" +
                     String(EXPECTED_PARTICIPANTS));
    } else {
      // Not all present - BLINK the LED
      allParticipantsFound = false;
      if ((millis() / 200) % 2 == 0) {
        digitalWrite(LED_ALLPRESENT_PIN, HIGH);
      } else {
        digitalWrite(LED_ALLPRESENT_PIN, LOW);
      }
      Serial.println("[ROLL CALL] Missing: Found " + String(activeCount) + "/" +
                     String(EXPECTED_PARTICIPANTS));
    }
  } else {
    // Button released
    rollCallActive = false;
    digitalWrite(LED_ROLLCALL_PIN, LOW);

    // If not all found, turn off the blinking LED immediately
    if (!allParticipantsFound) {
      digitalWrite(LED_ALLPRESENT_PIN, LOW);
    }
  }

  // Turn off all-present LED after timeout (only if it was solid ON from
  // finding all)
  if (allPresentLedOffTime > 0 && millis() > allPresentLedOffTime) {
    digitalWrite(LED_ALLPRESENT_PIN, LOW);
    allParticipantsFound = false;
    allPresentLedOffTime = 0;
  }

  // ============================================
  // BLE SCANNING AND CONNECTING
  // ============================================

  // 1. Scan
  foundCount = 0;
  pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();

  // 2. Connect to found participants
  for (int i = 0; i < foundCount; i++) {
    String name = foundParticipants[i].name;
    float distance = calculateDistance(foundParticipants[i].rssi);
    String addr = foundParticipants[i].addressStr;

    int idx = findOrAddParticipant(addr);
    if (idx >= 0) {
      participants[idx].name = name;
      participants[idx].distance = distance;
      participants[idx].lastSeen = millis();
      participants[idx].active = true;
    }

    BLEAddress bleAddr(addr.c_str());
    String sightings = connectAndReadSightings(bleAddr, name);
    if (sightings.length() > 0 && idx >= 0) {
      participants[idx].sightings = sightings;
      parseSightings(sightings, name);
    }
    delay(100);
  }

  // Small delay for stability
  delay(100);
}
