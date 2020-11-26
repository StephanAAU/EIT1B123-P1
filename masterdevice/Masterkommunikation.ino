#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xD7, 0x5A, 0x28};

typedef struct masterData {
  String cmd;
  float radius;
  float xpos;
  float ypos;

} masterData;


masterData sendData;

typedef struct slaveData {
  String status; 
  float xpos; 
  float ypos; 
  float forhindring; 
  int batPct; 

} slaveData;

slaveData recvData; 


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
void setup () {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }


  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}


void loop () {
  sendData.cmd = "Dette er en kommando";
  sendData.radius = random(1, 100);
  sendData.xpos = 1.2;
  sendData.ypos = 2.1;

  recvData.status = "Status for slave";
  recvData.xpos = 2.1;
  recvData.ypos = 1.2;
  recvData.forhindring = 10;
  recvData.batPct = 5;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
}
