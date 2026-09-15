/*
 * MeshLink Participant - Advertise + Scan + Report
 *
 * - Advertises as a participant
 * - Scans for other MESH_ devices (including phones)
 * - Exposes sightings via BLE characteristic for leader to read
 */

#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <math.h>

#define LED_ROLE_PIN 25  // LED 1: Role indicator (OFF = not a leader)
#define LED_FOUND_PIN 26 // LED 2: Found/Acknowledged (ON when Leader connected)
#define LED_NOTFOUND_PIN 27 // LED 3: Not Found (ON when no Leader connection)

#define CONNECTION_TIMEOUT                                                     \
  30000 // 30 seconds - switch to "not found" after this

// Unique participant ID - CHANGE THIS FOR EACH DEVICE!
#define PARTICIPANT_ID "MESH_PARTICIPANT_001"

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SIGHTINGS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define FOUNDLIST_CHAR_UUID                                                    \
  "beb5483e-36e1-4688-b7f5-ea07361b26a9" // Writable: Leader sends found list

// RSSI to Distance calibration
#define TX_POWER -59
#define PATH_LOSS_N 2.5
#define MIN_RSSI_FOR_SIGHTING -75 // Detect other participants within ~5m

// Timing
#define ADVERTISE_TIME 3000
#define SCAN_TIME 3

// Storage for sightings
#define MAX_SIGHTINGS 10
struct Sighting {
  String deviceId;
  String name;
  float distance;
  unsigned long timestamp;
  bool active;
};
Sighting sightings[MAX_SIGHTINGS];
int sightingCount = 0;

BLEScan *pBLEScan = nullptr;
BLECharacteristic *pSightingsCharacteristic = nullptr;
BLECharacteristic *pFoundListCharacteristic = nullptr;
bool deviceConnected = false;
bool foundViaNetwork = false; // True if our name is in the Leader's found list
unsigned long lastLeaderConnection = 0; // Track when Leader last connected

// Calculate distance from RSSI
float calculateDistance(int rssi) {
  if (rssi == 0)
    return -1.0;
  float ratio = (TX_POWER - rssi) / (10.0 * PATH_LOSS_N);
  return pow(10.0, ratio);
}

// Build sightings report string for BLE characteristic
String buildSightingsReport() {
  String report = "";
  int count = 0;
  unsigned long now = millis();

  for (int i = 0; i < sightingCount; i++) {
    if (sightings[i].active && (now - sightings[i].timestamp) < 10000) {
      if (count > 0)
        report += "|";
      report += sightings[i].name + ":" + String(sightings[i].distance, 1);
      count++;
    }
  }

  if (count == 0)
    return "NONE";
  return report;
}

// Store or update a sighting
void recordSighting(String id, String name, int rssi, float distance) {
  for (int i = 0; i < sightingCount; i++) {
    if (sightings[i].deviceId == id) {
      sightings[i].distance = distance;
      sightings[i].timestamp = millis();
      sightings[i].active = true;
      return;
    }
  }

  if (sightingCount < MAX_SIGHTINGS) {
    sightings[sightingCount].deviceId = id;
    sightings[sightingCount].name = name;
    sightings[sightingCount].distance = distance;
    sightings[sightingCount].timestamp = millis();
    sightings[sightingCount].active = true;
    sightingCount++;
  }
}

// Callback for scan results
class MyScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String deviceName = advertisedDevice.getName().c_str();

    // Only track MESH_ devices (but not ourselves or the leader)
    if (!deviceName.startsWith("MESH_") || deviceName == PARTICIPANT_ID ||
        deviceName == "MESH_LEADER") {
      return;
    }

    int rssi = advertisedDevice.getRSSI();

    // Filter by distance - only detect very close devices
    if (rssi < MIN_RSSI_FOR_SIGHTING) {
      return; // Too far, ignore
    }

    String address = advertisedDevice.getAddress().toString().c_str();
    float distance = calculateDistance(rssi);

    Serial.println(">>> Detected: " + deviceName + " at ~" +
                   String(distance, 1) + "m (RSSI: " + String(rssi) + ")");
    recordSighting(address, deviceName, rssi, distance);

    // Update BLE characteristic with sightings
    String report = buildSightingsReport();
    pSightingsCharacteristic->setValue(report.c_str());

    // Note: LED is controlled in main loop() based on connection status
    // No flash here to avoid confusing rapid blinking
  }
};

// Callback for when Leader writes the found list
class FoundListCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String foundList = pCharacteristic->getValue().c_str();
    Serial.println(">>> Received found list: " + foundList);

    // Check if our name is in the found list
    if (foundList.indexOf(PARTICIPANT_ID) >= 0) {
      foundViaNetwork = true;
      Serial.println(">>> WE ARE IN THE FOUND LIST!");
    } else {
      foundViaNetwork = false;
    }
  }
};

// Server callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    lastLeaderConnection = millis(); // Track connection time
    Serial.println(">>> Leader connected!");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println(">>> Leader disconnected");
    // Restart advertising
    BLEDevice::startAdvertising();
  }
};

void setup() {
  // Startup delay to allow power to stabilize
  delay(2000);

  // Initialize all LED pins
  pinMode(LED_ROLE_PIN, OUTPUT);
  pinMode(LED_FOUND_PIN, OUTPUT);
  pinMode(LED_NOTFOUND_PIN, OUTPUT);

  // Initial state: Role OFF (not a leader), NotFound ON (waiting for Leader)
  digitalWrite(LED_ROLE_PIN, LOW);      // OFF = Not a Leader
  digitalWrite(LED_FOUND_PIN, LOW);     // OFF initially
  digitalWrite(LED_NOTFOUND_PIN, HIGH); // ON = Not yet acknowledged

  // Fast blink to indicate boot start
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_FOUND_PIN, HIGH);
    delay(50);
    digitalWrite(LED_FOUND_PIN, LOW);
    delay(50);
  }

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║   MeshLink PARTICIPANT (Relay Mode)   ║");
  Serial.print("║   ID: ");
  Serial.print(PARTICIPANT_ID);
  Serial.println("          ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println();
  Serial.println("Scanning for devices and reporting to leader...");
  Serial.println();

  // Initialize BLE
  BLEDevice::init(PARTICIPANT_ID);

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create Sightings Characteristic (readable by leader)
  pSightingsCharacteristic = pService->createCharacteristic(
      SIGHTINGS_CHAR_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSightingsCharacteristic->addDescriptor(new BLE2902());
  pSightingsCharacteristic->setValue("NONE");

  // Create Found List Characteristic (writable by leader)
  pFoundListCharacteristic = pService->createCharacteristic(
      FOUNDLIST_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);
  pFoundListCharacteristic->setCallbacks(new FoundListCallback());

  // Start the service
  pService->start();

  // Setup scanner
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyScanCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();

  Serial.println("BLE Server started, advertising...");

  // Startup LED
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_FOUND_PIN, HIGH);
    delay(100);
    digitalWrite(LED_FOUND_PIN, LOW);
    delay(100);
  }
}

void loop() {
  // ============================================
  // LED STATUS LOGIC
  // ============================================

  // Check if Leader has connected recently OR if we're in the found list
  bool isFound = false;

  // Direct connection from Leader
  if (lastLeaderConnection > 0 &&
      (millis() - lastLeaderConnection) < CONNECTION_TIMEOUT) {
    isFound = true;
  }

  // Indirect discovery: our name is in the Leader's found list
  if (foundViaNetwork) {
    isFound = true;
  }

  if (isFound) {
    // FOUND (directly or via network)
    digitalWrite(LED_FOUND_PIN, HIGH);
    digitalWrite(LED_NOTFOUND_PIN, LOW);
  } else {
    // Not connected recently - NOT FOUND
    digitalWrite(LED_FOUND_PIN, LOW);
    digitalWrite(LED_NOTFOUND_PIN, HIGH);
  }

  // ============================================
  // BLE SCANNING
  // ============================================

  // Scan for devices
  Serial.println("\n[SCANNING for MESH_ devices...]");
  pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();

  // Update characteristic with current sightings
  String report = buildSightingsReport();
  pSightingsCharacteristic->setValue(report.c_str());

  // Print current sightings
  Serial.println("--- My Sightings (for leader) ---");
  Serial.println("  " + report);
  Serial.println("---------------------------------");

  delay(2000);
}
