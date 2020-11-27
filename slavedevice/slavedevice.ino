// Slave device software
// My MAC: 24:62:AB:D7:5A:28

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "pinout.h"
#include "motor.h"


uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0x6A, 0x4F, 0x78};

// Slave data structure - Sending from this device.
typedef struct slaveData {
    String status;
    float xpos;
    float ypos;
    float forhindring;
    int batPct;
} slaveData;

typedef struct masterData {
    String cmd;
    float radius;
    float xpos;
    float ypos;
} masterData;

// Create a slaveData called sendData to handle outgoing data.
slaveData sendData;
// Create a masterdData called recvData to handle incomming data.
masterData recvData;


// Callback funktion der bruges til at sende data.
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback funktion der bruges til at modtage data.
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.printf("Cmd: %s", recvData.cmd);
  Serial.println();
}

// Funktion der konfigurere og starter ESP-NOW protokollen. 
void espNowSetup() {
  WiFi.mode(WIFI_STA);                  // Opsætning af ESP's WIFI mode.
  Serial.println("Starting...");        // Informativ besked til seriel terminalen.
  if (esp_now_init() != ESP_OK) {       // Starter "ESP-NOW" funktionalitet.
    Serial.println("Fejl ved opstart af ESP-NOW.");
    return;
  }
  Serial.println("ESP-NOW opstartet."); // Informativ besked til seriel terminalen.

  esp_now_register_send_cb(OnDataSent); // Register funktionen "OnDataSent" som callback når der skal sendes trådløst. 

  // Opret en peer struct - dvs. masteren som skal modtage kommunikation og gem relevant data.
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Tilføj peer til kommunikation.
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer added...");

  // Brug "OnDataRecv" funktionen når der modtages data.
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Recv callback.");
}

// Send data function.
void sendDataFunc() {
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

  pinMode(MOTOR_INA1, OUTPUT);
  pinMode(MOTOR_INB1, OUTPUT);

  pinMode(MOTOR_INA2, OUTPUT);
  pinMode(MOTOR_INB2, OUTPUT);

  PwmSetup();
  updatePWMValues();

  espNowSetup(); // Funktion der konfigurere og starter ESP-NOW protokollen.
}

void loop() {
  // Send message via ESP-NOW
    sendData.status = "Standby"; 
    sendData.xpos = 107.3; 
    sendData.ypos = 60.3;
    sendData.forhindring = 9.3;
    sendData.batPct = 40;
  sendDataFunc();
  delay(2500);
}
