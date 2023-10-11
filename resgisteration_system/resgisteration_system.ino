#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WebServer.h>
#include <ArduinoJson.h>


// #define ON_Board_LED 2
#define Buzzer 12
#define LED_GREEN 13 /*Pin 8 for LED*/
#define LED_RED 14
#define SS_PIN 21  /*Slave Select Pin*/
#define RST_PIN 22 /*Reset Pin for RC522*/
#define reset_esp_btn 25
#define LED_YELLOW 26
#define reset_btn 27
#define LED_BLUE 33

String id_card = "";

// ------------------------------------------ SSID and Password of your WiFi router.
const char *ssid = "WE_0E9192";
const char *password = "adc015you";
// ------------------------------
WebServer server(80);
StaticJsonDocument<250> jsonDocument;
char buffer[250];

MFRC522 mfrc522(SS_PIN, RST_PIN); /*Create MFRC522 initialized*/
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

// ----------------------------- Functions signature
void start_connecting_to_WiFi();
void restart_ESP_if_not_connected();
void print_connected_successfully();
void handle_wifi_connection();
void fire_LED_green_and_buzzer();
void fire_LED_red_and_buzzer();
void fire_LED_red();
void on_LED_YELLOW();
void blue_LED(byte status);
void off_LED_YELLOW();
void fire_all_LED_on_reset();
void fire_leds_one_at_a_time(int delaytime);
void fire_all_LED(int delaytime);
void on_start_LED_sequence();
void on_press_reset();
void on_press_reset_esp();
void send_id_card_to_webserver();
void create_json(String id_card);
void setup_routing();


void setup() {
  Serial.begin(115200); /*Serial Communication begin*/
  SPI.begin();          /*SPI communication initialized*/
  mfrc522.PCD_Init();   /*RFID sensor initialized*/
  //----------------------------------------
  pinMode(LED_GREEN, OUTPUT); /*LED Pin set as output*/
  pinMode(LED_RED, OUTPUT);   /*LED Pin set as output*/
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //----------------------------------------
  pinMode(Buzzer, OUTPUT);
  //----------------------------------------
  pinMode(reset_btn, INPUT_PULLUP);
  //----------------------------------------
  pinMode(reset_esp_btn, INPUT_PULLUP);
  //----------------------------------------
  // pinMode(ON_Board_LED, OUTPUT);
  //----------------------------------------
  // digitalWrite(ON_Board_LED, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  delay(500);
  // digitalWrite(ON_Board_LED, LOW);
  digitalWrite(LED_BLUE, HIGH);

  start_connecting_to_WiFi();
  //----------------------------------------
  restart_ESP_if_not_connected();
  //----------------------------------------
  // digitalWrite(ON_Board_LED, LOW);  //--> Turn off the On Board LED when it is connected to the wifi router.
  //---------------------------------------- If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  print_connected_successfully();
  setup_routing();

  Serial.println("Put your card to the reader...");
  Serial.println();

  on_start_LED_sequence();
}

void loop() {
  on_press_reset();
  on_press_reset_esp();
  handle_wifi_connection();

  /*Look for the RFID Card*/
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /*Select Card*/
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("error read card serial");
    fire_LED_red_and_buzzer();
    return;
  }
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    fire_LED_red_and_buzzer();
    return;
  }

  if (mfrc522.uid.uidByte[0] != nuidPICC[0] || mfrc522.uid.uidByte[1] != nuidPICC[1] || mfrc522.uid.uidByte[2] != nuidPICC[2] || mfrc522.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println(F("A new card has been detected."));
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toLowerCase();
    id_card = content.substring(1);
    id_card.replace(" ", "");
    send_id_card_to_webserver();
    fire_LED_green_and_buzzer();

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = mfrc522.uid.uidByte[i];
    }
    off_LED_YELLOW();
  } else {
    Serial.println(F("Card read previously."));
    on_LED_YELLOW();
  }

  server.handleClient();
}

void start_connecting_to_WiFi() {
  //---------------------------------------- Make WiFi on ESP32 in "STA/Station" mode and start connecting to WiFi Router/Hotspot.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //----------------------------------------
  Serial.println("-------------");
  Serial.print("Connecting...");
}

void restart_ESP_if_not_connected() {
  //---------------------------------------- The process of connecting the WiFi on the ESP32 to the WiFi Router/Hotspot.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  int connecting_process_timed_out = 20;  //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //........................................ Make the On Board Flashing LED on the process of connecting to the wifi router.
    // digitalWrite(ON_Board_LED, HIGH);
    // blue_LED(1);
    digitalWrite(LED_BLUE, HIGH);
    delay(250);
    // digitalWrite(ON_Board_LED, LOW);
    // blue_LED(0);
    digitalWrite(LED_BLUE, LOW);
    delay(250);
    //........................................ Countdown "connecting_process_timed_out".
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }
}

void print_connected_successfully() {
  Serial.println();
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("-------------");
  // digitalWrite(ON_Board_LED, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  delay(1000);
  // digitalWrite(ON_Board_LED, LOW);
  digitalWrite(LED_BLUE, LOW);
}

void handle_wifi_connection() {
  if (WiFi.status() == WL_CONNECTED) {
    // blue_LED(1);
    digitalWrite(LED_BLUE, HIGH);
  } else {
    digitalWrite(Buzzer, HIGH);
    delay(300);
    digitalWrite(Buzzer, LOW);
    restart_ESP_if_not_connected();
  }
}

void fire_LED_green_and_buzzer() {
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(Buzzer, HIGH);
  delay(300);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(Buzzer, LOW);
}

void fire_LED_red_and_buzzer() {
  digitalWrite(LED_RED, HIGH);
  digitalWrite(Buzzer, HIGH);
  delay(300);
  digitalWrite(LED_RED, LOW);
  digitalWrite(Buzzer, LOW);
}

void on_LED_YELLOW() {
  digitalWrite(LED_YELLOW, HIGH);
  delay(600);
  digitalWrite(LED_YELLOW, LOW);
}

void off_LED_YELLOW() {
  digitalWrite(LED_YELLOW, LOW);
}

void blue_LED(byte status) {
  // 1 = led on
  // 0 = led off
  if (status == 1) {
    digitalWrite(LED_BLUE, HIGH);

  } else if (status == 0) {
    digitalWrite(LED_BLUE, LOW);
  }
}

void fire_LED_red() {
  digitalWrite(LED_RED, HIGH);
  delay(500);
  digitalWrite(LED_RED, LOW);
}

void setup_routing() {
  server.on("/sending-idcard-from-esp", send_id_card_to_webserver);
  server.begin();
}

void create_json(String id_card) {
  jsonDocument.clear();
  jsonDocument["id_card"] = id_card;
  serializeJson(jsonDocument, buffer);
}

void send_id_card_to_webserver() {
  Serial.println();
  Serial.println("Sending Id Card to the webserver ......");
  Serial.println("id card: " + id_card);
  create_json(id_card);
  server.send(200, "application/json", buffer);
}

void on_press_reset() {
  if (digitalRead(reset_btn) == 0) {
    Serial.println("Clear the array");
    // clear the array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = 0;
    }
    fire_all_LED_on_reset();
    on_start_LED_sequence();
    Serial.println("Put your card to the reader...");
    Serial.println();
  }
}

void on_press_reset_esp() {
  if (digitalRead(reset_esp_btn) == 0) {
    Serial.println("Clear the array");
    ESP.restart();
  }
}

void fire_all_LED_on_reset() {
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(Buzzer, HIGH);
  delay(1000);
  digitalWrite(Buzzer, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}

void fire_leds_one_at_a_time(int delaytime) {
  digitalWrite(LED_GREEN, HIGH);
  delay(delaytime);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  delay(delaytime);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  delay(delaytime);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  delay(delaytime);
  digitalWrite(LED_GREEN, LOW);
}

void fire_all_LED(int delaytime) {
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);
  delay(delaytime);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}
void on_start_LED_sequence() {
  fire_leds_one_at_a_time(500);
  delay(500);
  fire_all_LED(400);
  delay(200);
  fire_all_LED(400);
  delay(200);
  fire_all_LED(400);
  delay(200);
  fire_all_LED(1500);
}
