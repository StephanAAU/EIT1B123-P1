// Slave device software
// My MAC: 24:62:AB:D7:5A:28

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0x6A, 0x4F, 0x78};

unsigned long startMillis;    // Start timestamp
unsigned long currentMillis;  // temp timestamp

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
  int funcNr;
} masterData;

slaveData sendData;
masterData recvData;
esp_now_peer_info_t peerInfo;

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

void run() {
  Serial.println("Funktion udført");
}

void setup() {
  startMillis = millis();
  Serial.begin(115200); // Opsætning af seriel forbindelse.

  espNowSetup(); // Funktion der konfigurere og starter ESP-NOW protokollen.
}

void loop() {
  currentMillis = millis();
  // Send message via ESP-NOW
  if (recvData.cmd == "standby")
  {

  }

  if (recvData.cmd == "execFunc")
  {
    if (recvData.funcNr == 1) {
      run();
    }
    recvData.cmd = "standby";
    recvData.funcNr = 0;
  }

  if ((currentMillis - startMillis) >= 5000)  //test whether the period has elapsed
  {
    sendData.status = "standby";
    sendData.xpos = 107.3;
    sendData.ypos = 60.3;
    sendData.forhindring = 9.3;
    sendData.batPct = 40;
    sendDataFunc();
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED brightness
  }
}
