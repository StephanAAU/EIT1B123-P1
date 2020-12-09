
// Slave device software
// My MAC: 24:62:AB:D7:5A:28

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "pinout.h"
#include "motor.h"
#include "ultra.h"
#include "compass.h"

uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0xF9, 0xF2, 0xEC};
//uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0x6A, 0x4F, 0x78};
//uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xD7, 0x5A, 0x28}; // BRUNO's ESP32

// Slave data structure - Sending from this device.
typedef struct slaveData {
  String status;
  float forhindring;
  float kegleVinkel;
} slaveData;

typedef struct masterData {
  String cmd;
  float liftHeight;
  float kegleRadius;
  float beregnAfstandTilKegle;
  float laengdeAfvigelse;
  float stepLockGrader;
  float drejeKegleVinkel;
  int arg1;
} masterData;

slaveData sendData;
masterData recvData;
esp_now_peer_info_t peerInfo;

// Callback funktion der bruges til at sende data.
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback funktion der bruges til at modtage data.
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.printf("Cmd: %s", recvData.cmd.c_str());
  Serial.println();
}

// Funktion der konfigurere og starter ESP-NOW protokollen.
void espNowSetup() {
  WiFi.mode(WIFI_STA);                  // Opsætning af ESP's WIFI mode.
  Serial.println(WiFi.macAddress());
  Serial.println("Starting...");        // Informativ besked til seriel terminalen.
  if (esp_now_init() != ESP_OK) {       // Starter "ESP-NOW" funktionalitet.
    Serial.println("Fejl ved opstart af ESP-NOW.");
    return;
  }
  Serial.println("ESP-NOW opstartet."); // Informativ besked til seriel terminalen.
  delay(1000);
  esp_now_register_send_cb(OnDataSent); // Register funktionen "OnDataSent" som callback når der skal sendes trådløst.

  // Opret en peer struct - dvs. masteren som skal modtage kommunikation og gem relevant data.
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Tilføj peer til kommunikation.
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer added...");

  // Brug "OnDataRecv" funktionen når der modtages data.
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Recv callback.");
}

// Send data function.
void sendDataFunc(int status) {
  if (status == DRIVE) { sendData.status = "driving"; recvData.cmd = "busy"; }
  if (status == TURN) { sendData.status = "turning"; recvData.cmd = "busy"; }
  if (status == BUSY) { sendData.status = "busy"; recvData.cmd = "busy"; }
  if (status == READY) { sendData.status = "standby"; recvData.cmd = "standby"; }

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}

void setup() {
	Serial.begin(115200); // Opsætning af seriel forbindelse.
  delay(100);
  Serial.println(WiFi.macAddress());

	espNowSetup(); // Funktion der konfigurere og starter ESP-NOW protokollen.

  motorSetup();

	initUltra();
	
	if (compassSetup()) {
		Serial.println("Compass found!");
	}

	// Initial variabel værdier.
  sendData.status = "standby";
  sendData.forhindring = 0;
}

void loop() {
  static unsigned long curMillis = 0;
  static unsigned long prevMillis = 0;
  verifyTurn();
  verifyDrive();
  sendData.kegleVinkel = getCompassHeading();
  curMillis = millis();

  // Statemachine based on data from master over the radio.

  // Funktion der sender en enkelt ultralydsmåling
  if (recvData.cmd == "ultraDist")  {
    sendDataFunc(BUSY);
    sendData.forhindring = ultraGetDist();
    sendData.status = "standby";
    sendDataFunc(READY);
  }
  // Ikke blokerende funktion kører uendeligt
  if (recvData.cmd == "motorsRun")  {
    sendDataFunc(BUSY);
    motorsRun();
    sendDataFunc(READY);
  }
  // Ikke blokerende funktion der stopper motorer
  if (recvData.cmd == "stopMotors")  {
    sendDataFunc(BUSY);
    stopMotors();
    sendDataFunc(READY);
  }
  // Ikke blokerende funktion drejer baseret på argument 1 fra radio.
  if (recvData.cmd == "turn")  {
    sendDataFunc(BUSY);
    turn(recvData.arg1);
    sendDataFunc(READY);
  }
  // Blokerende funktion der drejer baseret på lift data.
  if (recvData.cmd == "turnKegle")  {   
    sendDataFunc(TURN);
    turn(recvData.drejeKegleVinkel);
    while (!verifyTurn()) {};       
    sendDataFunc(READY);
  }
  // Ikke blokerende funktion kører baseret på argument 1 fra radio.
  if (recvData.cmd == "drive")  {
    sendDataFunc(BUSY);
    drive(recvData.arg1);
    sendDataFunc(READY);
  }
  // Blokerende funktion der kører baseret på lift data.
  if (recvData.cmd == "driveKegle")  {
    sendDataFunc(DRIVE);
    drive(recvData.laengdeAfvigelse);
    while (!verifyDrive()) {};
    sendDataFunc(READY);
  }

	if ((curMillis - prevMillis) > 200)  //test whether the period has elapsed
	{
  	sendDataFunc(READY);
    Serial.println("Idling..");
  	prevMillis = curMillis;
	}
}
