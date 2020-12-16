// Master device software
// My MAC: 3C:71:BF:6A:4F:78

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include "rangefinder.h"
#include "stepper.h"
#include "pinout.h"
#include "compass.h"

uint8_t progCnt = 0;

uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xD7, 0x5A, 0x28};
//uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0xF9, 0xDC, 0x08};

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

typedef struct slaveData {
  String status;
  float forhindring;
  float kegleVinkel;

} slaveData;

masterData sendData;
slaveData recvData;
esp_now_peer_info_t peerInfo;

float masterVinkel;

struct DistortionValues distortionValues;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  Serial.print("\n slave: ");
  Serial.print(recvData.status);
}

void sendDataFunc() {
  Serial.printf("\n Sending cmd:%s a:%i h:%.2f r:%.2f k:%.2f l:%.2f g:%.2f v:%.2f \n", sendData.cmd.c_str(), sendData.arg1, sendData.liftHeight, sendData.kegleRadius, sendData.beregnAfstandTilKegle, sendData.laengdeAfvigelse, sendData.stepLockGrader, sendData.drejeKegleVinkel);
  //Serial.printf("\n Sending cmd:%s a:%i \n", sendData.cmd.c_str(), sendData.arg1);
  //Serial.printf("\n Sending cmd:%s \t a:%i \t h:%.2f \t r:%.2f \n", sendData.cmd.c_str(), sendData.arg1, sendData.liftHeight, sendData.kegleRadius);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));

/*
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
*/
  // Nulstil data.
  sendData.cmd = "standby";
  sendData.arg1 = 0;
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

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Serial.println("Starting device...");
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  pinMode(KEY_BUILTIN, INPUT);

  espNowSetup();
  stepperSetup();

  initRangefinders();

  compassSetup(true);

  sendData.cmd = "standby";
  sendData.liftHeight = 1;
  sendData.kegleRadius = 2;
  sendData.beregnAfstandTilKegle = 3;
  sendData.laengdeAfvigelse = 4;
  sendData.stepLockGrader = 5;
  sendData.drejeKegleVinkel = 6;
  sendData.arg1 = 0;
}

void lokaliserKegle() {
  medUret();
  modUret();
  sendData.laengdeAfvigelse = findAfvigendeLaengde(sendData.kegleRadius);
  sendData.drejeKegleVinkel = findDrejeVinkel();
  resetVinkel();
}

void checkSerial() {
  String serialData;
  String cmd = "";
  String argument = "";

  // Håndering af data på seriel bussen.
  while (Serial.available()) {
    char str_array[serialData.length()];
    serialData = Serial.readString();
    serialData.concat(" 0");
    serialData.toCharArray(str_array, serialData.length());
    cmd = strtok(str_array, " ");
    argument = strtok(NULL, " ");
  }

  // Debug kommando'er som håndteres ens.
  if (cmd == "motorsRun" || cmd == "stopMotors" || cmd == "turn" || cmd == "drive" || cmd == "calibrate" || cmd == "abe" || cmd == "turnKegle" ) {
    sendData.cmd = cmd;
    sendData.arg1 = argument.toInt();
    sendDataFunc();
  }

  // Omvendt dreje
  if (cmd == "aturn") {
    sendData.cmd = "turn";
    sendData.arg1 = (argument.toInt()) * -1;
    sendDataFunc();
  }
  
  // Debug kommando til at sende data.
  if (cmd == "ultraDist") {
    sendData.cmd = "ultraDist";
    Serial.println("\n\nSending data from struct");
    sendDataFunc();
    delay(500);
    Serial.printf("ultra dist: %.2f \n", recvData.forhindring);
    delay(1000);
  }

  // Debug kommando til at sende data.
  if (cmd == "dataSend") {
    Serial.println("\n\nSending data from struct");
    sendDataFunc();
  }

  // Print Slave kompas kommando.
    if (cmd == "slaveCompas") {
    Serial.printf("\n Slave compass heading: %.2f", recvData.kegleVinkel);
  }

  // Debug kommando til kegle lokation.
  if (cmd == "find") {
    lokaliserKegle();
  }

  if (cmd == "calibrateMaster")  {
    Serial.println("master calibrating...");

    struct MinMaxVector mmv;

    int i = 0;
    while (i < 200) {
      sensors_event_t e = getCompassEvent();
      updateMinMaxVector(&mmv, &e);
      //getCompassPoint(&e); FOR BILAG 21
      delay(10); // 100 compass samples per second
      while (digitalRead(KEY_BUILTIN)) {  }
      i++;
      Serial.println("sampled...");
    }
    struct DistortionValues dv = getDistortionValues(&mmv);
    dv.set = true;

    distortionValues = dv;
    
    Serial.println("done master calibrating.");
  }

  if (cmd == "radiusTest") {
    sendData.liftHeight = afstandTilLift();
    sendData.kegleRadius = beregnAfstandTilKegle(sendData.liftHeight);
    Serial.printf("%.2f \t %.2f \n", sendData.liftHeight, sendData.kegleRadius);
    sendData.cmd = "drive";
    sendData.arg1 = sendData.kegleRadius;
    sendDataFunc();
  }

  if (cmd == "ok") {
    progCnt = 3;
  }
  
  // Kommando til at starte normal drift.
  if (cmd == "start") {
    Serial.println("Starter programmet...");
    delay(100);
    progCnt = 1;
  }
  
  serialData = "";
  cmd = "";
  argument = "";
}

void loop () {
  static unsigned long prevMillis200ms = 0;    // Timestamp til hvert 200ms
  static unsigned long prevMillis1000ms = 0;    // Timestamp til hvert 1s
  static unsigned long currMillis;  // Temp. timestamp
  static float sidsteLiftH = 0;
  
  checkSerial();
  currMillis = millis();

  // Opdater lift højde hvert 200ms.
  if ((currMillis - prevMillis200ms) >= 200)
  {
    sidsteLiftH = sendData.liftHeight;
    sendData.liftHeight = afstandTilLift();
    sendData.kegleRadius = beregnAfstandTilKegle(sendData.liftHeight);
    //masterVinkel = getCompassHeading(&distortionValues);
    prevMillis200ms = currMillis;
    //Serial.printf("l: %.2f \t k: %.2f \t %.2f \n", sendData.liftHeight, afstandTilKegle(), sendData.kegleRadius);
  }

  switch (progCnt)
  {
    case 1: // Step 1 er en scanning efter keglen.
      Serial.println("case 1 ");
      lokaliserKegle();
      progCnt = 2;
      break;

    case 2: // Step 2 er vurdering af om afstands afvigelsen er høj nok til at kræve justering.
      //Serial.print("case 2");
      // Hvis kegleposition er ok gå til case 4.
      if ((sendData.laengdeAfvigelse < (1 * minKegleAfvigelse)) && (sendData.laengdeAfvigelse > (-1 * minKegleAfvigelse))) {
        progCnt = 4;
        Serial.print("goto 4 \n");
        // Hvis kegle position ikke er ok og keglen ikke er igang med at dreje.
      } else if ((recvData.status != "turning") && (recvData.status != "done")) {
        Serial.print("turning \t");
        sendData.cmd = "turnKegle";
        recvData.status = "turning";
        sendDataFunc();
        delay(200);
      } else if (recvData.status == "turning") {

      }
      // Main loop køre denne case ind til standby status modtages.
      if (recvData.status == "done") {
        Serial.println("Juster hjul");

        while (progCnt == 2) {
          checkSerial();
        }
        
        progCnt = 3;
        recvData.status = "";
        delay(200);
      }
      break;

    case 3:
      // Sæt keglen igang med at køre fremad, hvis den ikke er igang med dette.
      //Serial.print("3, ");
      if ((recvData.status != "driving") && (recvData.status != "done")) {
        sendData.cmd = "driveKegle";
        sendDataFunc();
        delay(100);
      }
      // Main loop kører denne case ind til done status modtages.
      if (recvData.status == "done") {
        progCnt = 4;
      }
      break;

    case 4:
      //Serial.print("case 4");
      float forskel;
      forskel = abs(sendData.liftHeight - sidsteLiftH);

      if (forskel > minLiftForskel) {
        Serial.printf("Lifthøjde ændring over %.2f \n", forskel);
        while ((abs(afstandTilLift() - sidsteLiftH) > forskel)) {
          sidsteLiftH = afstandTilLift();
          Serial.printf("Ikke stabiliseret nok; %.2f \t", sidsteLiftH);
          delay(500);
        }
        Serial.println("Højde stabiliseret - starter sekvens...");
        progCnt = 1;
      } else {
        /*
        if ((currMillis - prevMillis1000ms) >= 200) {
          //Serial.println("Afventer lifthøjde ændring...");
          prevMillis1000ms = currMillis;
        }*/
      }
      break;

      case 5:
        
      break;

    default:
      break;
  }
}
