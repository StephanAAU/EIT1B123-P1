
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

struct DistortionValues distortionValues;

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
  if (status == DONE) { sendData.status = "done"; recvData.cmd = "standby"; }
  if (status == DRIVE) { sendData.status = "driving"; recvData.cmd = "busy"; }
  if (status == TURN) { sendData.status = "turning"; recvData.cmd = "busy"; }
  if (status == BUSY) { sendData.status = "busy"; recvData.cmd = "busy"; }
  if (status == READY) { sendData.status = "standby"; recvData.cmd = "standby"; }
  if (status == CALIBRATE) { sendData.status = "calibrate"; recvData.cmd = "calibrate"; }

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));
/*
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
*/
}

void setup() {
	Serial.begin(115200); // Opsætning af seriel forbindelse.
  delay(100);
  Serial.println(WiFi.macAddress());

	espNowSetup(); // Funktion der konfigurere og starter ESP-NOW protokollen.

  motorSetup();

	initUltra();
	if (compassSetup(0)) {
		Serial.println("Compass found!");
	}
	// Initial variabel værdier.
  sendData.status = "standby";
  sendData.forhindring = 0;
  sendData.kegleVinkel = 0;
}

int i = 0;
struct MinMaxVector minMaxVector;

/* FOR BILAG 21
struct Vector pts[2000];
int compassPoints = 0;
void printCompassPoints() {
  if (compassPoints > 0) {
    for(int j = compassPoints; j < n; j++) {
      Serial.printf("%.02f,%.02f,%.02f", pts[j].x, pts[j].y, pts[j].z);
      Serial.println("");
    }

    struct DistortionValues dv = distortionValues;
    Serial.printf("oX=%.02f;oY=%.02f;oZ=%.02f;sX=%.02f;sY=%.02f;sZ=%.02f;", dv.oX, dv.oY, dv.oZ, dv.sX, dv.sY, dv.sZ);
    Serial.println("");

    delay(10000);
  }
}
void getCompassPoint(sensors_event_t *e) {
  pts[n].x = (*e).magnetic.x;
  pts[n].y = (*e).magnetic.y;
  pts[n].z = (*e).magnetic.z;
}
*/

void loop() {
  //printCompassPoints(); FOR BILAG 21

  static unsigned long curMillis = 0;
  static unsigned long prevMillis = 0;
  verifyTurn();
  verifyDrive();
  sendData.kegleVinkel = getCompassHeading(&distortionValues);
  curMillis = millis();

  // Statemachine based on data from master over the radio.

  // Funktion som klaibrer kompasset
  if (recvData.cmd == "calibrate")  {
    Serial.println("calibrating...");
    sendDataFunc(CALIBRATE);

    struct MinMaxVector mmv;

    // turn around 5 times
    turn(360*10);

    int i = 0;
    while (!verifyTurn()) {
      sensors_event_t e = getCompassEvent();
      updateMinMaxVector(&mmv, &e);
      //getCompassPoint(&e); FOR BILAG 21
      delay(10); // 100 compass samples per second
    }
    struct DistortionValues dv = getDistortionValues(&mmv);
    dv.set = true;

    distortionValues = dv;
    
    sendData.forhindring = 10000;
    sendData.status = "calibrate";
    sendDataFunc(READY);
    Serial.println("done calibrating.");
  }
  // Funktion der sender en enkelt ultralydsmåling
  if (recvData.cmd == "ultraDist")  {
    sendDataFunc(BUSY);
    sendData.forhindring = ultraGetDist();
    Serial.printf("Distance measurement: %.2f \n", sendData.forhindring);
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
    sendDataFunc(DONE);
  }

  // Blokerende funktion der drejer baseret på kompas
  if (recvData.cmd == "turnCompass")  {   
    sendDataFunc(TURN);
    turn(recvData.arg1);
    float startPos = getCompassHeading(&distortionValues);
    delay(100);
    while (abs(getCompassHeading(&distortionValues) - startPos) < recvData.arg1) { };
    stopMotors();       
    sendDataFunc(DONE);
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
    sendDataFunc(DONE);
  }

	if ((curMillis - prevMillis) > 1000)  //test whether the period has elapsed
	{
  	sendDataFunc(READY);
    Serial.println("Idling..");
  	prevMillis = curMillis;
	}
}
