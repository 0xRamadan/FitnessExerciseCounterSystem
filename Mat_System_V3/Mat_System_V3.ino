// ------------------------------------------ Start libraries.
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
// ------------------------------------------ OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// ------------------------------------------ RFID
#include <SPI.h>
#include <MFRC522.h>
// ------------------------------------------ Keypad
#include <Keypad.h>
//------------------------------------------- End Libraries.

// ----------------------------------------- Define Pins
#define UltraSonic_One 14        // trig and echo on the same pin - d1
#define UltraSonic_Two 12        // trig and echo on the same pin - d2
#define UltraSonic_Three 27      // trig and echo on the same pin - d3
#define Ultrasonic_Four_trig 15  // output pin - d4
#define Ultrasonic_Four_echo 39  // input pin only - d4
// #define Ultrasonic_Five_trig 2   // output pin -- optional case to add another Ultrasonic sensor.
// #define Ultrasonic_Five_echo 35  // input pin only
#define Buzzer 13
#define SS_PIN 5                 // Slave Select Pin
#define RST_PIN 0                // Reset Pin for RC522
#define Push_Up_Pressure_PIN 36  // P1
#define Pull_Up_Pressure_PIN 34  // P2
#define Burpee_Pressure_PIN 35   // P3

// ------------------------------------------ OLED SCREEN
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// ------------------------------------------ RFID Module

/*
    OUTPUT ARRANGEMENT
    ------------------
    PIN NAME |   PINs ON ESP32
    _________________________________________
    SDA      |     D5
    SCK      |     D18
    MOSI     |     D23
    MISO     |     D19
    GND      |     GND
    RST      |     EMPTY (Defined 0 in code)
    3.3V     |     3.3v 
*/

MFRC522 mfrc522(SS_PIN, RST_PIN); /*Create MFRC522 initialized*/
MFRC522::MIFARE_Key key;
byte nuidPICC[4];  // Init array that will store new NUID

// ------------------------------------------ Keypad

#define ROW_NUM 4
#define COLUMN_NUM 3

/*
    OUTPUT ARRANGEMENT
    ------------------
    KEYPAD PIN NO. |   SYMBOL | PINs ON ESP32
    _________________________________________
            1      |   COL 2  |     D26
            2      |   ROW 1  |     D25
            3      |   COL 1  |     D33
            4      |   ROW 4  |     D32
            5      |   COL 3  |     D4
            6      |   ROW 3  |     RX2 (GPIO 16)
            7      |   ROW 2  |     TX2 (GPIO 17)
*/

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte pin_rows[ROW_NUM] = { 25, 17, 16, 32 };  // ROW 1, ROW 2, ROW 3, ROW 4
byte pin_column[COLUMN_NUM] = { 33, 26, 4 };  // COL 1, COL 2, COL 3

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// ------------------------------------------


// ------------------------------------------ SSID and Password of your WiFi router.
const char *ssid = "WE_0E9192";
const char *password = "adc015you";
// ------------------------------------------ Define Constant

#define SPEED_SOUND 0.0343
#define ARM_RATIO 0.22
#define ARM_RATIO_IN_PULL_UP 0.1875
#define WAIST_RATIO 0.17

// ------------------------------------------ Declare variables
bool read_id_card_flag = false;
bool exercise_flag = true;
bool pull_up_exercise_flag = true;
bool holding_bar = false;
int distance_ultra_one;
int distance_ultra_two;
int distance_ultra_three;
int distance_ultra_four;
// int distance_ultra_five;
int stand_down_distance;
int exercise_counter;
int pull_up_counter;
int temp_exercise_counter;
int push_up_pressure_reading;
int pull_up_pressure_reading;
int burpee_pressure_reading;
char input_key;
unsigned long start_countdown_timer;
unsigned long stay_steady_countdown_timer;
// ---------------------------------------------
unsigned long start_time;
unsigned long elapsed_time = 0;
unsigned long exercise_time = 120 * 1000;  // 2 minutes in milliseconds
const unsigned long interval = 1000;       // Check elapsed time every 1 second
// ---------------------------------------------
int hour, secLsd, secMsd, minLsd, minMsd;
// ---------------------------------------------

int height;
String id_card = "";
String gender = "";
String service_number = "";
String player_age = "";

// -----------------------------  Variables for HTTP POST request data.
String HOST_NAME = "http://192.168.8.100";  // change to your PC's IP address, New Dell G15 IPv4: 192.168.8.100
String PATH_NAME = "/esp32-get-and-post-to-database/";
// String PATH_NAME = "/php_script_files/";
String GET_SCRIPT_FILE = "get-data-from-database.php";
String UPDATE_SCRIPT_FILE = "update-data-in-database.php";
String post_data = "";
String payload = "";
// -----------------------------


// ------------------------------------------ Functions signature

// -------------------------------- ESP32 wifi connection
void start_connecting_to_WiFi();
void restart_ESP_if_not_connected();
void print_connected_successfully();
// -------------------------------- sensors
void read_id_card();
void fire_Buzzer();
void calculate_distance_one();
void calculate_distance_two();
void calculate_distance_three();
void calculate_distance_four();
long microsecondsToCentimeters(long microseconds);
// ------------------------------- logic for push up
void push_up_exercise(bool with_timer);
void stop_exercise_with_pressure(String exercise_name);
void stop_push_up_when_stand_up();
void stop_exercise_with_keypad(String exercise_name);
void stop_exercise_with_timer(String exercise_name, int countdown_timer);
void stop_exercise_with_stay_steady_timer(int stay_steady_timeout, String exercise_name);
// ------------------------------- logic for pull up
void pull_up_exercise();
void stop_pull_up_when_not_holding_bar();
// ------------------------------- logic for abs
void abs_exercise(bool with_timer);
// ------------------------------- logic for burpee exercise
void burpee_exercise(bool with_timer);
// -------------------------------- Work with time
void count_time(unsigned long Time);

// ------------------------------- logic for reading another id card.
void prepare_to_take_another_player();
// -------------------------------

// ------------------------------- OLED display
void initialize_oled();
template<class T>
void display_something_on_OLED(T something, int textSize, int delay_in_milli_second);
void display_in_test_mode(String text_one, String text_two, int delay_in_milli_second);
void display_in_test_mode_single_text(String text_one, int delay_in_milli_second);
void display_in_center_double_text(String text_one, String text_two, int delay_in_milli_second, int text_size);
void display_reseting_on_oled_screen();
void display_in_center(String text, int delay_in_milli_second, int text_size);
void display_Ready_squence_with_Buzzer();
void display_starting_sequence();
void display_options(String title, String op1, String op2, String op3, String op4, int delay_in_milli_second, int text_size);
// ------------------------------- HTTP client mehtod
void post_id_card_and_get_data();
void update_exercise_counter(String exercise_name);

void setup() {
  //----------------------------------------
  Serial.begin(115200);
  //----------------------------------------
  pinMode(Buzzer, OUTPUT);
  //----------------------------------------
  pinMode(Ultrasonic_Four_trig, OUTPUT);
  pinMode(Ultrasonic_Four_echo, INPUT);
  //----------------------------------------
  SPI.begin();
  mfrc522.PCD_Init();
  //----------------------------------------
  initialize_oled();
  display_starting_sequence();
  // ----------------------------------------
  start_connecting_to_WiFi();
  // ----------------------------------------
  restart_ESP_if_not_connected();
  // ----------------------------------------
  print_connected_successfully();
  // ----------------------------------------
  display_in_center("Put Card", 0, 2);

  while (!read_id_card_flag) {
    read_id_card();
  }
  display_in_center(id_card, 2000, 2);
  start_time = millis();
  while (!height) {
    if (millis() - start_time >= elapsed_time + interval) {
      elapsed_time += interval;
      post_id_card_and_get_data();
      if (!height) {
        display_in_center("Solving...", 0, 2);
      }
      if (elapsed_time >= 10000) {
        display_in_center("Restarting", 1000, 2);
        ESP.restart();
      }
    }
  }
  elapsed_time = 0;
  display_Ready_squence_with_Buzzer();
  display_in_center(service_number, 2000, 2);
}

void loop() {
  input_key = keypad.getKey();
  display_in_center("Choose Exercise", 0, 2);

  /* 
  ___________________________
      KEY | Functionality  
 ___________________________
  |   1   | push up exercise.
  |   2   | pull up exercise.
  |   3   | burpee exercise.
  |   4   | abs exercise.
  |   5   | Prepare to take another player.
  |   6   | nothing.
  |   7   | nothing.
  |   8   | nothing.
  |   9   | nothing.
  |   0   | nothing.
  |   *   | check Sensors, press 0 on keypad to exit.
  |   #   | Reset ESP.
  ___________________________

  */

  if (input_key == '1') {
    fire_Buzzer();
    // choose timer or not.
    bool with_timer;
    while (true) {
      input_key = keypad.getKey();
      display_options("Timer or not", "1. with timer.", "2. no timer.", "", "", 0, 1);

      // with timer
      if (input_key == '1') {
        with_timer = true;
        push_up_exercise(with_timer);
        break;
      }
      // with no timer
      else if (input_key == '2') {
        with_timer = false;
        push_up_exercise(with_timer);
        break;
      }
      // exit exercise
      else if (input_key == '0') {
        break;
      }
    }
  }
  if (input_key == '2') {
    fire_Buzzer();
    pull_up_exercise();
  }
  if (input_key == '3') {
    fire_Buzzer();
    // choose timer or not.
    bool with_timer;
    while (true) {
      input_key = keypad.getKey();

      display_options("Timer or not", "1. with timer.", "2. no timer.", "", "", 0, 1);

      // with timer
      if (input_key == '1') {
        with_timer = true;
        burpee_exercise(with_timer);
        break;
      }
      // with no timer
      else if (input_key == '2') {
        with_timer = false;
        burpee_exercise(with_timer);
        break;
      }
      // exit exercise
      else if (input_key == '0') {
        break;
      }
    }
  }
  if (input_key == '4') {
    fire_Buzzer();
    // choose timer or not.
    bool with_timer;
    while (true) {
      input_key = keypad.getKey();
      display_options("Timer or not", "1. with timer.", "2. no timer.", "", "", 0, 1);

      // with timer
      if (input_key == '1') {
        with_timer = true;
        abs_exercise(with_timer);
        break;
      }
      // with no timer
      else if (input_key == '2') {
        with_timer = false;
        abs_exercise(with_timer);
        break;
      }
      // exit exercise
      else if (input_key == '0') {
        break;
      }
    }
  }
  if (input_key == '5') {
    fire_Buzzer();
    prepare_to_take_another_player();
  }
  if (input_key == '6') {
    fire_Buzzer();
  }
  if (input_key == '7') {
    fire_Buzzer();
  }
  if (input_key == '8') {
    fire_Buzzer();
  }
  if (input_key == '9') {
    fire_Buzzer();
  }
  if (input_key == '0') {
    fire_Buzzer();
  }
  if (input_key == '*') {
    fire_Buzzer();

    /*
        __________________________________________________
            faulty check    |     sensors     
        __________________________________________________
              test one      |   ultrasonic one and two
              test two      |   ultrasonic three and four
              test three    |   Push up pressure - pressure 1
              test four     |   Pull up pressure - pressure 2
              test five     |   Burpee Pressure - pressure 3
    */

    // add fault check sequence.
    // test wifi, if okay go on, if not try to connect, if failed, restart esp

    // test one - ultrasonic one and two -- push up
    while (true) {
      calculate_distance_one();
      delay(10);
      calculate_distance_two();
      Serial.println("------------------------------------------------");
      Serial.println("distance one: " + String(distance_ultra_one));
      Serial.println("distance two: " + String(distance_ultra_two));
      Serial.println("------------------------------------------------");

      display_in_center_double_text("D1: " + String(distance_ultra_one), "D2: " + String(distance_ultra_two), 0, 2);

      input_key = keypad.getKey();
      if (input_key == '0') {
        fire_Buzzer();
        break;
      }
    }

    // test two - ultrasonic three and four  -- pull up
    while (true) {
      calculate_distance_three();
      delay(10);
      calculate_distance_four();
      Serial.println("------------------------------------------------");
      Serial.println("distance three: " + String(distance_ultra_three));
      Serial.println("distance four: " + String(distance_ultra_four));
      Serial.println("------------------------------------------------");

      display_in_center_double_text("D3: " + String(distance_ultra_three), "D4: " + String(distance_ultra_four), 0, 2);

      input_key = keypad.getKey();
      if (input_key == '0') {
        fire_Buzzer();
        break;
      }
    }

    // Test three - Pressure 1 - Push up pressure
    while (true) {
      push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);
      Serial.println("push_up_pressure_reading: " + String(push_up_pressure_reading));
      // display_in_test_mode_single_text("P1: " + String(push_up_pressure_reading), 0);
      display_in_center("P1: " + String(push_up_pressure_reading), 0, 2);

      // to get out of loop.
      input_key = keypad.getKey();
      if (input_key == '0') {
        fire_Buzzer();
        break;
      }
    }

    // Test four Pressure 2 - Pull up pressure - square pressure
    while (true) {
      pull_up_pressure_reading = analogRead(Pull_Up_Pressure_PIN);
      Serial.println("pull_up_pressure_reading: " + String(pull_up_pressure_reading));
      // display_in_test_mode_single_text("P2: " + String(pull_up_pressure_reading), 0);
      display_in_center("P2: " + String(pull_up_pressure_reading), 0, 2);
      // to get out of loop.
      input_key = keypad.getKey();
      if (input_key == '0') {
        fire_Buzzer();
        break;
      }
    }

    // test five - pressure 3 - burpee pressure
    while (true) {
      burpee_pressure_reading = analogRead(Burpee_Pressure_PIN);
      Serial.println("burpee_pressure_reading: " + String(burpee_pressure_reading));
      // display_in_test_mode_single_text("P3: " + String(burpee_pressure_reading), 0);
      display_in_center("P3: " + String(burpee_pressure_reading), 0, 2);

      input_key = keypad.getKey();
      if (input_key == '0') {
        fire_Buzzer();
        break;
      }
    }
  }
  if (input_key == '#') {

    // Sequence to display the word restarting -- takes 2 sec
    digitalWrite(Buzzer, HIGH);
    display_in_center("R", 100, 2);
    display_in_center("Re", 100, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Res", 100, 2);
    display_in_center("Rest", 100, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Resta", 100, 2);
    display_in_center("Restar", 100, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Restart", 100, 2);
    display_in_center("Restarti", 100, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Restartin", 100, 2);
    display_in_center("Restarting", 100, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("R", 100, 2);
    display_in_center("Re", 100, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Res", 100, 2);
    display_in_center("Rest", 100, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Resta", 100, 2);
    display_in_center("Restar", 100, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Restart", 100, 2);
    display_in_center("Restarti", 100, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Restartin", 100, 2);
    display_in_center("Restarting", 100, 2);

    // restart esp
    ESP.restart();
  }
}

// Connection
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
    fire_Buzzer();
    display_in_center("Connecting.", 50, 2);
    display_in_center("Connecting..", 50, 2);
    display_in_center("Connecting...", 50, 2);
    display_in_center("Connecting.", 50, 2);
    display_in_center("Connecting..", 50, 2);

    //........................................ Countdown "connecting_process_timed_out".
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      display_in_center("Restarting", 1000, 2);
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
  display_in_center("Connected", 500, 2);
}

// RFID
void read_id_card() {
  /*Look for the RFID Card*/
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /*Select Card*/
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
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
    Serial.println("id_card: " + id_card);
    post_id_card_and_get_data();
    fire_Buzzer();
    // [WIP] check if remove the following if, but leave read_id_card_flag = true.
    if (id_card.length() > 0) {
      read_id_card_flag = true;
    }
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = mfrc522.uid.uidByte[i];
    }
  } else {
    Serial.println(F("Card read previously."));
  }
}

// Ultrasonic
long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

void calculate_distance_one() {
  unsigned long duration;
  pinMode(UltraSonic_One, OUTPUT);
  digitalWrite(UltraSonic_One, LOW);
  delayMicroseconds(2);
  digitalWrite(UltraSonic_One, HIGH);
  delayMicroseconds(5);
  digitalWrite(UltraSonic_One, LOW);
  pinMode(UltraSonic_One, INPUT);
  duration = pulseIn(UltraSonic_One, HIGH);
  distance_ultra_one = microsecondsToCentimeters(duration);
}

void calculate_distance_two() {
  unsigned long duration;
  pinMode(UltraSonic_Two, OUTPUT);
  digitalWrite(UltraSonic_Two, LOW);
  delayMicroseconds(2);
  digitalWrite(UltraSonic_Two, HIGH);
  delayMicroseconds(5);
  digitalWrite(UltraSonic_Two, LOW);
  pinMode(UltraSonic_Two, INPUT);
  duration = pulseIn(UltraSonic_Two, HIGH);
  distance_ultra_two = microsecondsToCentimeters(duration);
}

void calculate_distance_three() {
  unsigned long duration;
  pinMode(UltraSonic_Three, OUTPUT);
  digitalWrite(UltraSonic_Three, LOW);
  delayMicroseconds(2);
  digitalWrite(UltraSonic_Three, HIGH);
  delayMicroseconds(5);
  digitalWrite(UltraSonic_Three, LOW);
  pinMode(UltraSonic_Three, INPUT);
  duration = pulseIn(UltraSonic_Three, HIGH);
  distance_ultra_three = microsecondsToCentimeters(duration);
}

void calculate_distance_four() {
  unsigned long duration;
  digitalWrite(Ultrasonic_Four_trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Ultrasonic_Four_trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(Ultrasonic_Four_trig, LOW);
  duration = pulseIn(Ultrasonic_Four_echo, HIGH);
  distance_ultra_four = duration * 0.034 / 2;
}

void fire_Buzzer() {
  digitalWrite(Buzzer, HIGH);
  delay(250);
  digitalWrite(Buzzer, LOW);
}

// ------------------------------------------- START PUSH UP LOGIC
void push_up_exercise(bool with_timer) {

  int countdown_timer;
  // choose time
  if (with_timer) {
    // default timer is 1 min
    countdown_timer = 60000;
    while (true) {
      input_key = keypad.getKey();
      /*
        1. 30 sec
        2. 60 sec
        3. 2 min
        4. 3 min        
      */
      display_options("Choose time", "1. 30 sec", "2. 60 sec", "3. 2 min", "4. 3 min", 0, 1);

      if (input_key == '1') {
        // 30 sec
        countdown_timer = 30000;
        break;
      } else if (input_key == '2') {
        // 60 sec == 1 min
        countdown_timer = 60000;
        break;
      } else if (input_key == '3') {
        // 120 sec = 2 min
        countdown_timer = 120000;
        break;
      } else if (input_key == '4') {
        // 180 sec = 2 min
        countdown_timer = 180000;
        break;
      } else if (input_key == '0') {
        break;
      }
    }
  }

  exercise_counter = 0;
  temp_exercise_counter = 0;
  exercise_flag = true;
  bool start_countdown_timer_flag = false;
  display_in_center("PUSH UP", 0, 2);


  if (player_age >= String(60)) {
    stand_down_distance = 20;
  } else {
    stand_down_distance = 10;
  }
  // logic for male person
  while (exercise_flag) {
    calculate_distance_one();
    delay(5);
    calculate_distance_two();
    // Testing
    Serial.println("distance one: " + String(distance_ultra_one));
    Serial.println("distance two: " + String(distance_ultra_two));

    if (distance_ultra_one < stand_down_distance && distance_ultra_one != 0 && distance_ultra_two < stand_down_distance && distance_ultra_two != 0) {
      temp_exercise_counter += 1;
      display_in_center(String(exercise_counter), 0, 2);

      // working with time
      if (with_timer) {
        // start countdown will begin when player goes to start position
        if (!start_countdown_timer_flag) {
          start_countdown_timer = millis();
          start_countdown_timer_flag = true;
        }
        stay_steady_countdown_timer = millis();
      }

      while (distance_ultra_one < height * ARM_RATIO || distance_ultra_two < height * WAIST_RATIO && exercise_flag) {

        if (with_timer) {
          stop_exercise_with_stay_steady_timer(20000, "push_up");
        }

        if (gender == "ذكر") {
          stop_exercise_with_pressure("push_up");
        }
        stop_exercise_with_keypad("push_up");
        calculate_distance_one();
        delay(5);
        calculate_distance_two();
      }

      if (exercise_flag) {
        exercise_counter = temp_exercise_counter;
      }
      display_in_center(String(exercise_counter), 0, 2);
      if (with_timer) {
        stop_exercise_with_timer("push_up", countdown_timer);
      }
      stop_exercise_with_keypad("push_up");
    }

    // temp push up counter > 1 means that the player achieved start position. and with_timer flag already = true.
    if (temp_exercise_counter > 1 && with_timer) {
      stop_exercise_with_timer("push_up", countdown_timer);
    }

    stop_push_up_when_stand_up();
    if (gender == "ذكر") {
      stop_exercise_with_pressure("push_up");
    }
    stop_exercise_with_keypad("push_up");
  }
}

void stop_exercise_with_pressure(String exercise_name) {
  push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);
  if (push_up_pressure_reading > 1500 && temp_exercise_counter > 0 && exercise_flag) {
    Serial.println("Touch Pressure ... end exercise, save counter");
    update_exercise_counter(exercise_name);

    digitalWrite(Buzzer, HIGH);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Touch LAND", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center(String(exercise_counter), 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center(String(exercise_counter), 750, 2);

    exercise_flag = false;
  }
}

void stop_push_up_when_stand_up() {
  // temp_exercise_counter > 1 means that player already entered the exercise.
  if (temp_exercise_counter > 1 && distance_ultra_one > 100 && distance_ultra_two > 100 && exercise_flag) {
    digitalWrite(Buzzer, HIGH);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, HIGH);
    display_in_center("Stand UP", 250, 2);
    digitalWrite(Buzzer, LOW);
    display_in_center(String(exercise_counter), 1000, 2);
    update_exercise_counter("push_up");
    exercise_flag = false;
  }
}

void stop_exercise_with_keypad(String exercise_name) {
  input_key = keypad.getKey();
  if (input_key == '0') {
    exercise_flag = false;
    update_exercise_counter(exercise_name);
    // try formating string to make it dynamic
    Serial.println("Stop exercise");
    display_in_center("STOPPING", 2000, 2);
    display_in_center(String(exercise_counter), 1000, 2);
  }
}

void stop_exercise_with_stay_steady_timer(int stay_steady_timeout, String exercise_name) {
  // if a player stands down for more than 20 sec, stop exercise.
  if (millis() - stay_steady_countdown_timer > stay_steady_timeout && exercise_flag) {
    display_in_center("Stand Down Time out", 0, 2);
    update_exercise_counter(exercise_name);
    exercise_flag = false;
  }
}

void stop_exercise_with_timer(String exercise_name, int countdown_timer) {
  // exercise will end after the provided timeout
  if (millis() - start_countdown_timer > countdown_timer && exercise_flag) {
    Serial.println("Exercise Time Out");
    update_exercise_counter(exercise_name);
    display_in_center("Time OUT", 2000, 2);
    display_in_center(String(exercise_counter), 2000, 2);
    exercise_flag = false;
  }
}
// ------------------------------------------- End PUSH UP LOGIC

//-------------------------------------------- START ABS logic
void abs_exercise(bool with_timer) {

  int countdown_timer;
  // choose time
  if (with_timer) {
    // default timer is 2 min
    countdown_timer = 120000;
    while (true) {
      input_key = keypad.getKey();
      /*
        1. 30 sec
        2. 60 sec
        3. 2 min
        4. 3 min        
      */
      display_options("Choose time", "1. 30 sec", "2. 60 sec", "3. 2 min", "4. 3 min", 0, 1);

      if (input_key == '1') {
        // 30 sec
        countdown_timer = 30000;
        break;
      } else if (input_key == '2') {
        // 60 sec == 1 min
        countdown_timer = 60000;
        break;
      } else if (input_key == '3') {
        // 120 sec = 2 min
        countdown_timer = 120000;
        break;
      } else if (input_key == '4') {
        // 180 sec = 2 min
        countdown_timer = 180000;
        break;
      } else if (input_key == '0') {
        break;
      }
    }
  }

  exercise_counter = 0;
  temp_exercise_counter = 0;
  exercise_flag = true;
  bool start_exercise_flag = false;
  bool start_countdown_timer_flag = false;
  bool pressure_reading_flag = false;

  display_in_center("ABS", 0, 2);

  while (exercise_flag) {
    calculate_distance_three();
    delay(10);
    push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);

    // Testing
    Serial.println("-----------------------");
    Serial.println("Touch Pressure " + String(push_up_pressure_reading));
    Serial.println("distance ultra three: " + String(distance_ultra_three));
    Serial.println("-----------------------");

    if (start_countdown_timer_flag && with_timer) {
      stop_exercise_with_timer("abs", countdown_timer);
    }

    if (push_up_pressure_reading > 800) {

      // start timer when player start to play (start timer, when player left his back of the pressure sensor.) one time.
      while (!start_exercise_flag && exercise_flag) {
        push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);

        // Testing
        Serial.println("-----------------------");
        Serial.println("Touch Pressure " + String(push_up_pressure_reading));
        Serial.println("distance three: " + String(distance_ultra_three));
        Serial.println("-----------------------");

        if (push_up_pressure_reading < 100) {

          if (with_timer) {
            // start exercise timer, start position
            start_countdown_timer = millis();
            start_countdown_timer_flag = true;
          }

          start_exercise_flag = true;
        }
        stop_exercise_with_keypad("abs");
      }

      pressure_reading_flag = true;
      display_in_center(String(exercise_counter), 0, 2);

      if (with_timer) {
        stay_steady_countdown_timer = millis();
      }

      while (pressure_reading_flag && exercise_flag) {
        calculate_distance_three();
        delay(10);
        push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);


        // Testing
        Serial.println("-----------------------");
        Serial.println("Touch Pressure " + String(push_up_pressure_reading));
        Serial.println("distance ultra three: " + String(distance_ultra_three));
        Serial.println("-----------------------");

        if (distance_ultra_three < 20 && distance_ultra_three != 0 && push_up_pressure_reading < 100) {
          exercise_counter += 1;
          pressure_reading_flag = false;
        }

        if (with_timer) {
          stop_exercise_with_stay_steady_timer(30000, "abs");
        }

        stop_exercise_with_keypad("abs");
      }
    }

    stop_exercise_with_keypad("abs");
  }
}
// ------------------------------------------- END abs EXERCISE

// ------------------------------------------- START BURPEE EXERCISE
void burpee_exercise(bool with_timer) {

  int countdown_timer;
  // choose time
  if (with_timer) {
    // default timer is 2 min
    countdown_timer = 120000;
    exercise_time = countdown_timer;
    while (true) {
      input_key = keypad.getKey();
      /*
        1. 30 sec
        2. 60 sec
        3. 2 min
        4. 3 min
      */
      display_options("Choose time", "1. 1 min", "2. 1.5 min", "3. 2 min", "4. 3 min", 0, 1);
      if (input_key == '1') {
        // 60 sec
        countdown_timer = 60000;
        exercise_time = countdown_timer;
        break;
      } else if (input_key == '2') {
        // 90 sec == 1.5 min
        countdown_timer = 90000;
        exercise_time = countdown_timer;

        break;
      } else if (input_key == '3') {
        // 120 sec = 2 min
        countdown_timer = 120000;
        exercise_time = countdown_timer;
        break;
      } else if (input_key == '4') {
        // 180 sec = 3 min
        countdown_timer = 180000;
        exercise_time = countdown_timer;
        break;
      } else if (input_key == '0') {
        break;
      }
    }
  }

  exercise_counter = 0;
  exercise_flag = true;
  bool start_exercise_flag = false;
  bool start_countdown_timer_flag = false;
  bool stand_up_flag = false;
  bool push_up_ready_position_flag = false;

  display_in_center("Burpee", 0, 2);

  // Step 1
  while (!stand_up_flag && exercise_flag) {

    // program start here.
    /*
      Step 1 => start position -- stand up, 
      Step 2 => second position -- squat then kick feet back (push_up_ready_position_flag)
      Step 3 => third position -- return to squat (check if player return both foot at the same time)
      Step 4 => finial poistion -- return to stand up positon --optional 
    */

    calculate_distance_three();
    delay(5);
    // Testing
    Serial.println("-----------Step one------------");
    Serial.println("distance three: " + String(distance_ultra_three));
    Serial.println("-----------------------");

    if (elapsed_time >= exercise_time && with_timer) {
      exercise_flag = false;
      display_in_center("Time Out", 1000, 2);
      count_time(elapsed_time / 1000);
      display_in_center_double_text(String(exercise_counter), "T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd), 2000, 2);
      update_exercise_counter("burpee");
    }

    // logic - start position
    // ultrasonic is adjustable, here 20 cm to make it in safe zoom
    if (distance_ultra_three < 20) {
      start_exercise_flag = true;
      stand_up_flag = true;
      start_time = millis();
    }

    if (start_exercise_flag) {
      elapsed_time += interval;
      count_time(elapsed_time / 1000);
      display_in_center_double_text("Counter: " + String(exercise_counter), "T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd), 0, 2);
    }


    // Step 2
    while (stand_up_flag && !push_up_ready_position_flag && exercise_flag) {
      if (millis() - start_time >= elapsed_time + interval) {

        elapsed_time += interval;
        count_time(elapsed_time / 1000);
        display_in_center_double_text("Counter: " + String(exercise_counter), "T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd), 0, 2);

        push_up_pressure_reading = analogRead(Push_Up_Pressure_PIN);
        if (push_up_pressure_reading > 1500) {
          // return to step 1
          stand_up_flag = false;
          Serial.println("Touch Pressure push up pressure");
          digitalWrite(Buzzer, HIGH);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("Touch LAND", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("Touch LAND", 250, 2);
          break;
        }
        calculate_distance_one();
        delay(5);
        calculate_distance_two();

        // Testing
        Serial.println("-----------Step two------------");
        Serial.println("distance one: " + String(distance_ultra_one));
        Serial.println("distance two: " + String(distance_ultra_two));
        Serial.println("-----------------------");

        // achieving push up ready position
        if (distance_ultra_one < height * ARM_RATIO && distance_ultra_one != 0 && distance_ultra_two < height * WAIST_RATIO && distance_ultra_two != 0 && push_up_pressure_reading < 100) {
          push_up_ready_position_flag = true;
        }
        stop_exercise_with_keypad("burpee");

        if (elapsed_time >= exercise_time && with_timer) {
          exercise_flag = false;
          display_in_center("Time Out", 2000, 2);
          display_in_center("Counter: " + String(exercise_counter), 2000, 2);
          update_exercise_counter("burpee");
        }
      }
    }

    // Step 3
    while (stand_up_flag && push_up_ready_position_flag && exercise_flag) {
      if (millis() - start_time >= elapsed_time + interval) {
        elapsed_time += interval;
        count_time(elapsed_time / 1000);
        display_in_center_double_text("Counter: " + String(exercise_counter), "T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd), 0, 2);

        // return to step 1 condition, if player move one foot forward and still the other foot is back
        // player must move his feet together (on the specified marks).

        burpee_pressure_reading = analogRead(Burpee_Pressure_PIN);
        Serial.println("burpee_pressure_reading: " + String(burpee_pressure_reading));
        if (burpee_pressure_reading > 1500) {
          // return to step 1
          stand_up_flag = false;
          Serial.println("Touch burpee pressure");
          digitalWrite(Buzzer, HIGH);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, LOW);
          display_in_center("wrong position", 250, 2);
          digitalWrite(Buzzer, HIGH);
          display_in_center("wrong position", 250, 2);
          break;
        }

        calculate_distance_three();
        delay(5);
        // if reach finial postion, increase counter by 1, break the loop (return to step 1)
        if (distance_ultra_three < 20) {
          exercise_counter += 1;
          count_time(elapsed_time / 1000);
          display_in_center_double_text("Counter: " + String(exercise_counter), "T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd), 0, 2);
          stand_up_flag = false;
          push_up_ready_position_flag = false;
          break;
        }

        stop_exercise_with_keypad("burpee");

        if (elapsed_time >= exercise_time && with_timer) {
          exercise_flag = false;
          display_in_center("Time Out", 1000, 2);
          display_in_center(String(exercise_counter), 1000, 2);
          update_exercise_counter("burpee");
        }
      }
    }

    stop_exercise_with_keypad("burpee");
  }
}
// ------------------------------------------- END BURPEE EXERCISE

// ------------------------------------------- START PULL UP LOGIC

void pull_up_exercise() {
  exercise_flag = true;
  exercise_counter = 0;
  bool pull_up_flag = false;
  holding_bar = false;
  display_in_center("PULL UP", 0, 2);
  while (exercise_flag) {
    calculate_distance_three();
    delay(5);
    calculate_distance_four();

    // Testing
    Serial.println("-----------------------");
    Serial.println("distance three: " + String(distance_ultra_three));
    Serial.println("distance four: " + String(distance_ultra_four));
    Serial.println("-----------------------");

    pull_up_pressure_reading = analogRead(Pull_Up_Pressure_PIN);
    Serial.println("-----------------------");
    Serial.println("pull_up_pressure_reading: " + String(pull_up_pressure_reading));
    Serial.println("-----------------------");

    pull_up_flag = false;

    // height * (1.5/8)  >> example: height = 182 cm >> ratio == 34.2 cm
    // start position >>
    if (distance_ultra_three > 100 && pull_up_pressure_reading != 0 && pull_up_pressure_reading > 1000) {
      // start position
      holding_bar = true;
      display_in_center(String(exercise_counter), 0, 2);
      // loop until getting player goes up to the end.,
      while (!pull_up_flag && holding_bar) {
        calculate_distance_four();
        Serial.println("-----------------------");
        Serial.println("distance four: " + String(distance_ultra_four));
        Serial.println("-----------------------");
        stop_pull_up_when_not_holding_bar();

        if (distance_ultra_four < 30 && distance_ultra_four != 0) {
          exercise_counter += 1;
          display_in_center(String(exercise_counter), 0, 2);
          pull_up_flag = true;
        } else {
          Serial.println("not achieving right pull up");
        }
        stop_exercise_with_keypad("push_up");
      }
    }

    // check error in ultrasonics
    if (distance_ultra_three == 0 || distance_ultra_four == 0) {
      display_in_center("Error in Ultrasonic", 0, 2);
    }

    if (holding_bar) {
      stop_pull_up_when_not_holding_bar();
    }
    stop_exercise_with_keypad("push_up");
  }
}

void stop_pull_up_when_not_holding_bar() {
  // letting bar go, finish exercise
  pull_up_pressure_reading = analogRead(Pull_Up_Pressure_PIN);
  Serial.println("pull_up_pressure_reading: " + String(pull_up_pressure_reading));
  if (pull_up_pressure_reading == 0) {
    holding_bar = false;
    exercise_flag = false;
    Serial.println("not holding bar");
    display_in_center("lefting bar", 2000, 2);
    display_in_center(String(exercise_counter), 1000, 2);
    update_exercise_counter("pull_up");
  }
}
// ------------------------------------------- End PULL UP LOGIC

// ------------------------------------------- Work with Time
void count_time(unsigned long Time) {
  hour = Time / 3600;
  int Minutes = (Time - hour * 3600) / 60;
  int Seconds = Time % 60;
  secLsd = Seconds % 10;
  secMsd = Seconds / 10;
  minLsd = Minutes % 10;
  minMsd = Minutes / 10;

  Serial.println("T: " + String(hour) + ":" + String(minMsd) + String(minLsd) + ":" + String(secMsd) + String(secLsd));
}
// ------------------------------------------- Work with Time

// ------------------------------------------- Logic for read another player
void prepare_to_take_another_player() {
  // prepare to enter another player.
  exercise_counter = 0;
  read_id_card_flag = false;
  // Clear array that hold previous id card
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = 0;
  }
  fire_Buzzer();
  display_in_center("Put Card", 0, 2);

  bool stop_flag = false;
  while (!read_id_card_flag && !stop_flag) {
    input_key = keypad.getKey();
    if (input_key == '0') {
      stop_flag = true;
      fire_Buzzer();
    }
    read_id_card();
  }

  if (!stop_flag) {
    while (!height && !stop_flag) {
      input_key = keypad.getKey();
      if (input_key == '0') {
        stop_flag = true;
        fire_Buzzer();
      }
      display_in_center("Error in getting player data", 0, 2);
      post_id_card_and_get_data();
    }

    if (!stop_flag) {
      digitalWrite(Buzzer, HIGH);
      display_in_center("R", 100, 2);
      display_in_center("Re", 100, 2);
      digitalWrite(Buzzer, LOW);
      display_in_center("Rea", 100, 2);
      display_in_center("Read", 100, 2);
      digitalWrite(Buzzer, HIGH);
      display_in_center("Ready", 100, 2);
      display_in_center("R", 100, 2);
      digitalWrite(Buzzer, LOW);
      display_in_center("Re", 100, 2);
      display_in_center("Rea", 100, 2);
      digitalWrite(Buzzer, HIGH);
      display_in_center("Read", 100, 2);
      display_in_center("Ready", 100, 2);
      digitalWrite(Buzzer, LOW);
      display_in_center("Ready", 100, 2);

      display_in_center(service_number, 2000, 2);
    }
  }
}

// OLED display
void initialize_oled() {
  // initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
}

void display_starting_sequence() {
  display_in_center("S", 100, 2);
  display_in_center("St", 100, 2);
  display_in_center("Sta", 100, 2);
  display_in_center("Star", 100, 2);
  display_in_center("Start", 100, 2);
  display_in_center("Starti", 100, 2);
  display_in_center("Startin", 100, 2);
  display_in_center("Starting", 100, 2);
  display_in_center("Starting.", 100, 2);
  display_in_center("Starting..", 100, 2);
  display_in_center("Starting...", 0, 2);
}

template<class T>
void display_something_on_OLED(T something, int textSize, int delay_in_milli_second) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(WHITE);
  display.println(something);
  display.setCursor(0, 28);
  display.display();
  delay(delay_in_milli_second);
}

void display_in_test_mode(String text_one, String text_two, int delay_in_milli_second) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(text_one);
  display.println(text_two);
  display.setCursor(0, 28);
  display.display();
  delay(delay_in_milli_second);
}

void display_in_test_mode_single_text(String text_one, int delay_in_milli_second) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(text_one);
  display.setCursor(0, 28);
  display.display();
  delay(delay_in_milli_second);
}

void display_in_center(String text, int delay_in_milli_second, int text_size) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(text);
  display.display();
  delay(delay_in_milli_second);
}

void display_Ready_squence_with_Buzzer() {
  digitalWrite(Buzzer, HIGH);
  display_in_center("R", 100, 2);
  display_in_center("Re", 100, 2);
  digitalWrite(Buzzer, LOW);
  display_in_center("Rea", 100, 2);
  display_in_center("Read", 100, 2);
  digitalWrite(Buzzer, HIGH);
  display_in_center("Ready", 100, 2);
  display_in_center("R", 100, 2);
  digitalWrite(Buzzer, LOW);
  display_in_center("Re", 100, 2);
  display_in_center("Rea", 100, 2);
  digitalWrite(Buzzer, HIGH);
  display_in_center("Read", 100, 2);
  display_in_center("Ready", 100, 2);
  digitalWrite(Buzzer, LOW);
  display_in_center("Ready", 100, 2);
}

void display_options(String title, String op1, String op2, String op3, String op4, int delay_in_milli_second, int text_size) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(title, 0, 0, &x1, &y1, &width, &height);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(text_size);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(title);
  display.println(op1);
  display.println(op2);
  display.println(op3);
  display.println(op4);
  display.display();
  delay(delay_in_milli_second);
}

void display_in_center_double_text(String text_one, String text_two, int delay_in_milli_second, int text_size) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text_one, 0, 0, &x1, &y1, &width, &height);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(text_size);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(text_one);
  display.println(text_two);
  display.display();
  delay(delay_in_milli_second);
}


// http client post and get
void post_id_card_and_get_data() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    int httpCode;
    post_data = "id_card=" + id_card;

    http.begin(HOST_NAME + PATH_NAME + GET_SCRIPT_FILE);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpCode = http.POST(post_data);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println("---------------");
        Serial.println("Payload: ");
        Serial.println(payload);
        Serial.println("---------------");
        JSONVar myObject = JSON.parse(payload);

        Serial.println("---------------");
        Serial.println("JSON Object: ");
        Serial.println(myObject);
        Serial.println("---------------");

        // testing if parsing succeseed
        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          Serial.println("---------------");
          return;
        }

        // extract the data from the json object
        String height_obj = myObject["height"];
        String gender_obj = myObject["gender"];
        String service_number_obj = myObject["service_number"];
        String player_age_obj = myObject["age"];
        height = height_obj.toInt();
        gender = gender_obj;
        service_number = service_number_obj;
        player_age = player_age_obj;

        // print data on serial monitor
        Serial.println("-----------------------------");
        Serial.println("service_number: " + service_number);
        Serial.println("height: " + String(height));
        Serial.println("gender: " + gender);
        Serial.println("age: " + String(player_age));
        Serial.println("-----------------------------");


      } else {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        display_in_center("error in sending", 2000, 2);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      display_in_center("error in sending", 2000, 2);
    }
    http.end();
  } else {
    display_in_center("no wifi connection", 2000, 2);
    restart_ESP_if_not_connected();
  }
}

void update_exercise_counter(String exercise_name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    int httpCode;
    post_data = "id_card=" + id_card + "&" + exercise_name + "=" + exercise_counter;

    http.begin(HOST_NAME + PATH_NAME + UPDATE_SCRIPT_FILE);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpCode = http.POST(post_data);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        display_in_center("sending to db", 1000, 2);
        Serial.println("update field in database.");
      } else {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        display_in_center("error in sending", 2000, 2);
      }
    } else {
      display_in_center("error in sending", 2000, 2);
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    display_in_center("no wifi connection", 2000, 2);
    restart_ESP_if_not_connected();
  }
}
