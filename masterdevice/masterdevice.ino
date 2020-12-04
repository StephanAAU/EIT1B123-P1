// Master device software
// My MAC: 3C:71:BF:6A:4F:78

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include "rangefinder.h"
#include "stepper.h"

unsigned long startMillis;    // Start timestamp
unsigned long currentMillis;  // temp timestamp

//extern uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xD7, 0x5A, 0x28};
uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0xF9, 0xDC, 0x08};

typedef struct masterData {
  String cmd;
  float liftHeight;
  float kegleRadius;
  float beregnAfstandTilKegle;
  float laengdeAfvigelse;
  float stepLockGrader;
  float drejeKegleVinkel
  int funcNr;
  int arg1;
} masterData;

typedef struct slaveData {
  String status;
  float xpos;
  float ypos;
  float forhindring;
  int batPct;

} slaveData;

masterData sendData;
slaveData recvData; 
esp_now_peer_info_t peerInfo;

String serialData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("String: ");
  Serial.println(recvData.status);
  Serial.print("Float: ");
  Serial.println(recvData.xpos);
  Serial.print("Float: ");
  Serial.println(recvData.ypos);
  Serial.print("Float: ");
  Serial.println(recvData.forhindring);
  Serial.print("int: ");
  Serial.println(recvData.batPct);
  Serial.println();
}

void sendDataFunc() {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}

void espNowSetup() {
  WiFi.mode(WIFI_STA);                  // Opsætning af ESP's WIFI mode.
  Serial.println(WiFi.macAddress());
  Serial.println("Starting...");        // Informativ besked til seriel terminalen.
  if (esp_now_init() != ESP_OK) {       // Starter "ESP-NOW" funktionalitet.
    Serial.println("Fejl ved opstart af ESP-NOW.");
    return;
  }
  Serial.println("ESP-NOW opstartet."); // Informativ besked til seriel terminalen.

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

void setup () {
  Serial.begin(115200);

  Serial.println("Starting device...");
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  initRangefinders();
	
  espNowSetup();

  stepperSetup();

  startMillis = millis();
}

void execSlaveFunc(int funcNr) {
  sendData.cmd = "execFunc";
  sendData.funcNr = funcNr;

  sendDataFunc();
}

void checkSerial() {
  while (Serial.available()) {
    serialData = Serial.readString();// read the incoming data as string
  }

  if (serialData == "motorsRun" || serialData == "stopMotors" || serialData == "turn" || serialData == "drive") {
    Serial.print("Sending: ");
    Serial.println(serialData);
    
    sendData.cmd = serialData;
    if (serialData == "turn") {
      sendData.arg1 = 360;
    } else if (serialData == "drive") {
      sendData.arg1 = 100;
    }
    sendDataFunc();
  }

  if (serialData == "test") {
    Serial.println("\n\nSending: test data");
    sendData.cmd = "standby";
    sendData.radius = random(1, 100);
    sendData.xpos = 1.2;
    sendData.ypos = 2.1;

    sendDataFunc();
  }

  if (serialData == "find") {
    medUret();
    modUret();
    findAfvigendeLaengde(beregnAfstandTilKegle(afstandTilLift()));
    findDrejeVinkel();
    resetVinkel();
  }
  serialData = "";
}

void loop () {
  checkSerial();
  currentMillis = millis();
  
  if ((currentMillis - startMillis) >= 500)  //test whether the period has elapsed
  {
    float tempVar1 = afstandTilLift();
    Serial.printf("Lift: %.2f mm \t Kegle: %.2f \t Radius: %.2f \n", afstandTilKegle(), tempVar1, beregnAfstandTilKegle(tempVar1));
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED brightness
  }
}
