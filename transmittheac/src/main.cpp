#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL666ID2JBc"
#define BLYNK_TEMPLATE_NAME "AcProjectRX"
#define BLYNK_AUTH_TOKEN "TqbzckrmD745wPAWQRoAWhKvsYgJyUWL"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

char auth[] = BLYNK_AUTH_TOKEN;

const char* ssid = "iPhone 12 Pro";
const char* password = "irisjuara";

// Firebase project credentials
#define API_KEY "AIzaSyD2CcJJ6MCuHj6EU2168b9l10whznBxmWE"
#define DATABASE_URL "https://noumicapstone-default-rtdb.asia-southeast1.firebasedatabase.app/"

unsigned long previousMillis = 0;
unsigned long interval = 30000;

String InfoFirebase = "Connecting...";

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth authFirebase;         // Rename for Firebase
FirebaseConfig configFirebase;

Adafruit_AHTX0 aht;
BlynkTimer timer;

const uint16_t kIrLed1 = 32;
const uint16_t kIrLed2 = 33;  

IRsend irsend1(kIrLed1);
IRsend irsend2(kIrLed2);

unsigned long firebaseReconnectMillis = 0; // Timestamp untuk mencoba koneksi ulang Firebase
const unsigned long firebaseReconnectInterval = 10000; // Interval 10 detik untuk mencoba ulang
bool isFirebaseConnected = false; // Status koneksi Firebase

// Data COOLIX untuk suhu tertentu
uint64_t coolixData30C = 0xB29FB0;  // 30°C (0)
uint64_t coolixData29C = 0xB29FA0;  // 29°C (1)
uint64_t coolixData28C = 0xB29F80;  // 28°C (2)
uint64_t coolixData27C = 0xB29F90;  // 27°C (3)
uint64_t coolixData26C = 0xB29FD0;  // 26°C (4)
uint64_t coolixData25C = 0xB29FC0;  // 25°C (5)

// Variabel global untuk menyimpan nilai suhu dan kelembapan
float AHT_SUHU = 0.0;
float AHT_KELEMBAPAN = 0.0;

// Variable Firebase
unsigned long lastReadMillis = 0;
const unsigned long readInterval = 30000; // Read data every xx seconds
int jumlah_orang = 0;
int jumlahOrang = 0;
int jumlahOrangSebelumnya = -1;

int LastStateIRnum = 0;
String LastStateIRdat;

unsigned long sendDataPrevMillis = 0; // Deklarasi variabel untuk pengiriman data Firebase

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void SendToBlynk(){
  Blynk.virtualWrite(V0, LastStateIRnum);
  Blynk.virtualWrite(V1, AHT_SUHU);
  Blynk.virtualWrite(V2, AHT_KELEMBAPAN);
  Blynk.virtualWrite(V3, jumlahOrang);
  Blynk.virtualWrite(V4, InfoFirebase);
}

void reconnectFirebase() {
  if (!isFirebaseConnected && millis() - firebaseReconnectMillis >= firebaseReconnectInterval) {
    firebaseReconnectMillis = millis();
    Serial.println("Attempting to reconnect to Firebase...");

    if (Firebase.signUp(&configFirebase, &authFirebase, "", "")) {
      Serial.println("Firebase Reconnected Successfully");
      isFirebaseConnected = true;
    } else {
      Serial.printf("Firebase Reconnect Failed: %s\n", configFirebase.signer.signupError.message.c_str());
    }
  }
}

void setup() {
  irsend1.begin();
  irsend2.begin();
  Serial.begin(115200);
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Blynk.begin(auth, ssid, password);
  Serial.println("IR Transmitter siap.");
  Serial.println("Adafruit AHT10/AHT20 initialize!");

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  timer.setInterval(15000L, SendToBlynk);

  // Setup Firebase
  configFirebase.api_key = API_KEY;
  configFirebase.database_url = DATABASE_URL;

  if (Firebase.signUp(&configFirebase, &authFirebase, "", "")) {
    Serial.println("Firebase Signup Successful");
    isFirebaseConnected = true;
  } else {
    Serial.printf("Firebase Signup Failed: %s\n", configFirebase.signer.signupError.message.c_str());
    isFirebaseConnected = false;
  }

  Firebase.begin(&configFirebase, &authFirebase);
  Firebase.reconnectWiFi(true);
}

void ReadAHT10(){
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // Perbarui nilai variabel global AHT
  AHT_SUHU = temp.temperature;
  AHT_KELEMBAPAN = humidity.relative_humidity;

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
}

void IRLogic(){
  if (jumlahOrang == 0) {
    Serial.println("Mengirimkan sinyal untuk suhu 30°C...");
    irsend1.sendCOOLIX(coolixData30C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData30C, 24);
    LastStateIRnum = 30;
    LastStateIRdat = "30C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 1) {
    Serial.println("Mengirimkan sinyal untuk suhu 29°C...");
    irsend1.sendCOOLIX(coolixData29C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData29C, 24);
    LastStateIRnum = 29;
    LastStateIRdat = "29C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 2) {
    Serial.println("Mengirimkan sinyal untuk suhu 29°C...");
    irsend1.sendCOOLIX(coolixData29C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData29C, 24);
    LastStateIRnum = 29;
    LastStateIRdat = "29C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 3) {
    Serial.println("Mengirimkan sinyal untuk suhu 28°C...");
    irsend1.sendCOOLIX(coolixData28C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData28C, 24);
    LastStateIRnum = 28;
    LastStateIRdat = "28C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 4) {
    Serial.println("Mengirimkan sinyal untuk suhu 28°C...");
    irsend1.sendCOOLIX(coolixData28C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData28C, 24);
    LastStateIRnum = 28;
    LastStateIRdat = "28C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 5) {
    Serial.println("Mengirimkan sinyal untuk suhu 27°C...");
    irsend1.sendCOOLIX(coolixData27C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData27C, 24);
    LastStateIRnum = 27;
    LastStateIRdat = "27C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 6) {
    Serial.println("Mengirimkan sinyal untuk suhu 27°C...");
    irsend1.sendCOOLIX(coolixData27C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData27C, 24);
    LastStateIRnum = 27;
    LastStateIRdat = "27C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 7) {
    Serial.println("Mengirimkan sinyal untuk suhu 26°C...");
    irsend1.sendCOOLIX(coolixData26C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData26C, 24);
    LastStateIRnum = 26;
    LastStateIRdat = "26C/FAN1/NOSWING/NORMAL";
  } else if (jumlahOrang == 8) {
    Serial.println("Mengirimkan sinyal untuk suhu 25°C...");
    irsend1.sendCOOLIX(coolixData25C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData25C, 24);
    LastStateIRnum = 25;
    LastStateIRdat = "25C/FAN1/NOSWING/NORMAL";
  } else {
    Serial.println("Jumlah orang tidak sesuai dengan kondisi yang diatur.");
    irsend1.sendCOOLIX(coolixData30C, 24);
    delay(1000);
    irsend2.sendCOOLIX(coolixData30C, 24);
    LastStateIRnum = 00;
    LastStateIRdat = "30C/FAN1/NOSWING/NORMAL";
  }
  // Cetak status terakhir
  Serial.print("SENDING STATE: ");
  Serial.print(LastStateIRdat);
  Serial.println(LastStateIRnum);
}

void ReceiveFirebase() {
  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) { // Pastikan WiFi terhubung
    if (Firebase.RTDB.getInt(&fbdo, "/jumlah_orang")) {
      if (fbdo.dataType() == "int") {
        int jumlahOrangBaru = fbdo.intData();

        // Hanya kirim IR jika nilai berubah
        if (jumlahOrangBaru != jumlahOrang) {
          jumlahOrangSebelumnya = jumlahOrang;
          jumlahOrang = jumlahOrangBaru;
          Serial.print("Jumlah orang dari Firebase berubah: ");
          Serial.println(jumlahOrang);

          IRLogic();
        }

        InfoFirebase = "Connected!";
      }
    } else {
      Serial.print("Error reading Firebase: ");
      Serial.println(fbdo.errorReason());
      InfoFirebase = "Disconnected!";
      isFirebaseConnected = false; // Tandai Firebase sebagai terputus
    }
  } else {
    InfoFirebase = "Disconnected!";
    Serial.println("Firebase not ready or WiFi disconnected!");
    isFirebaseConnected = false;
  }
}


void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  Blynk.run();
  timer.run();
  ReadAHT10();
  SendToBlynk();

    //Terima data dari Firebase tiap 30 detik
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    ReceiveFirebase();
  }
  reconnectFirebase();
}