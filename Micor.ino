/*/////////////////////////////////////////////////////////////////////////////
Automated Trashcan with Weight Sensing and Notification System
Team Name: [BSCPE 3B - Group 1] - Batch 2020-2024
Description:
  This code is for an ESP32-based automated trashcan with weight sensing, proximity detection, 
  and notification system. The device utilizes a load cell to measure the weight of the trashcan 
  and a proximity sensor to detect the presence of a person approaching the trashcan. When the weight 
  exceeds a preset threshold and a person is detected nearby, the trashcan automatically opens for 
  a designated time period, allowing the person to dispose of the trash. Real-time notifications are 
  sent to a designated user to indicate when the trashcan needs to be emptied or if any issues occur. 
  The device connects to a WiFi network using WiFi Manager, providing a web portal for easy configuration 
  of custom parameters. The configuration data is saved and loaded from a JSON file stored in the SPIFFS 
  filesystem. The code is developed using the Arduino IDE and is compatible with ESP32 boards.

Team Members:

    [Kenneth Harold Panis]
    [Hannah Mae Mascardo]
    [Daisy Marie Bernante]

Date: [May 2023]
*/////////////////////////////////////////////////////////////////////////////

// Libraries
#include <ESP32Servo.h>
#include <NewPing.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "6157153234:AAHFHKeJUN85OZk-eQGXanOQNe8TalouppQ"
#define CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
#define JSON_CONFIG_FILE "/test_config.json"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Define WiFiManager and Servo Object
WiFiManager wm;

void saveConfigFile();
bool loadConfigFile();
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);

// Pin Definitions
const int ULTRASONIC_TRIGGER_PIN = 32;
const int ULTRASONIC_ECHO_PIN = 33;
const int ULTRASONIC_BUTTON_PIN = 21;
const int BUTTON_TRIGGER_PIN = 5;
const int HX711_DOUT_PIN = 17;
const int HX711_SCK_PIN = 16;
const int GREEN_LED_PIN_ULTRASONIC = 19;
const int RED_LED_PIN_ULTRASONIC = 18;
const int GREEN_LED_PIN_HX711 = 25;
const int RED_LED_PIN_HX711 = 26;
const int SERVO_DATA_PIN = 2;
const int GREEN_LED_PIN_WIFI = 27;
const int BLUE_LED_PIN_WIFI = 13;
const int RED_LED_PIN_WIFI = 14;
const int WIFI_BUTTON_PIN = 36;

// Constants
const int MAX_DISTANCE = 200;
const float WEIGHT_THRESHOLD = 1000.0;
const unsigned long INTERVAL_MS = 0;
const int CALIBRATION_EEPROM_ADDRESS = 0;
const unsigned long LONG_PRESS_THRESHOLD = 300;
const int OPEN_SERVO_ANGLE = 0;
const int CLOSE_SERVO_ANGLE = 90;
const int timeout = 60;  // seconds to run for

// Variables
Servo servoMotor;
NewPing ultrasonicSensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, MAX_DISTANCE);
HX711_ADC loadCell(HX711_DOUT_PIN, HX711_SCK_PIN);
char testString[50] = "Wala diri Wala didto";
int testNumber = 420;
unsigned long buttonPressStartTime = 0;
unsigned long buttonReleaseTime = 0;
unsigned long currentHX711Millis = 0;
unsigned long previousHX711Millis = 0;
float calibrationValue = 696.0;
bool isULTRASONICRedLedOn = false;
bool isButtonPressed = false;
bool isLongPressActive = false;
bool isReleaseActive = false;
bool newDataReady = false;
bool tareComplete = false;
bool weightExceededThreshold = false;
// Flag for saving WiFi Manager data
bool shouldSaveConfig = true;
// Change to true when testing to force configuration every time we run
bool forceConfig = false;

void setup() {
  Serial.begin(57600);
  // Initialize pins
  pinMode(RED_LED_PIN_ULTRASONIC, OUTPUT);
  pinMode(GREEN_LED_PIN_ULTRASONIC, OUTPUT);
  pinMode(RED_LED_PIN_HX711, OUTPUT);
  pinMode(GREEN_LED_PIN_HX711, OUTPUT);
  pinMode(BUTTON_TRIGGER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN_WIFI, OUTPUT);
  pinMode(BLUE_LED_PIN_WIFI, OUTPUT);
  pinMode(GREEN_LED_PIN_WIFI, OUTPUT);
  pinMode(ULTRASONIC_BUTTON_PIN, INPUT);
  pinMode(WIFI_BUTTON_PIN, INPUT);

  servoMotor.attach(SERVO_DATA_PIN, 500, 2500);

  // Initialize load cell
  loadCell.begin();
  #if defined(ESP8266) || defined(ESP32)
  EEPROM.begin(512);
  #endif
  EEPROM.get(CALIBRATION_EEPROM_ADDRESS, calibrationValue);
  unsigned long stabilizingTime = 2000;
  bool doTare = true;
  loadCell.start(stabilizingTime, doTare);

  // Check for tare timeout
  if (loadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU > HX711 wiring and pin designations");
    while (1);
  } else {
    loadCell.setCalFactor(calibrationValue);
    Serial.println("Startup is complete");
  }

  //WIFI MANAGER SETUP
   bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup) {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }
  // Explicitly set WiFi mode
  WiFi.mode(WIFI_STA);
  Serial.println("\n Wifi Setup");

  delay(10);

  digitalWrite(BLUE_LED_PIN_WIFI, false);
  digitalWrite(RED_LED_PIN_WIFI, false);
  digitalWrite(GREEN_LED_PIN_WIFI, false);
  digitalWrite(BLUE_LED_PIN_WIFI, true);

  // Reset settings (only for development)
  // wm.resetSettings();

  // set configportal timeout
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  wm.setConfigPortalTimeout(timeout);
  wm.setConnectTimeout(15);
  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);
  // Custom elements
  // Text box (String) - 50 characters maximum
  WiFiManagerParameter custom_text_box("key_text", "Device Location", testString, 50);
  // Need to convert numerical input to string to display the default value.
  char convertedValue[6];
  sprintf(convertedValue, "%d", testNumber);
  // Text box (Number) - 7 characters maximum
  WiFiManagerParameter custom_text_box_num("key_num", "Identification Number", convertedValue, 7);
  // Add all defined parameters
  wm.addParameter(&custom_text_box);
  wm.addParameter(&custom_text_box_num);

  if (forceConfig)
  // Run if we need a configuration
  {
    if (!wm.startConfigPortal("Trash_Bin_AP", "password")) {
      digitalWrite(BLUE_LED_PIN_WIFI, false);
      digitalWrite(RED_LED_PIN_WIFI, true);
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      Serial.println("Restarting");
      ESP.restart();
      delay(5000);
    }
  } else {
    if (!wm.autoConnect("Trash_Bin_AP", "password")) {
      digitalWrite(BLUE_LED_PIN_WIFI, false);
      digitalWrite(RED_LED_PIN_WIFI, true);
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      Serial.println("Restarting");
      ESP.restart();
      delay(5000);
    }
  }

  // If we get here, we are connected to the WiFi
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Lets deal with the user config values

  // Copy the string value
  strncpy(testString, custom_text_box.getValue(), sizeof(testString));
  Serial.print("testString: ");
  Serial.println(testString);

  //Convert the number value
  testNumber = atoi(custom_text_box_num.getValue());
  Serial.print("testNumber: ");
  Serial.println(testNumber);

  // Save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfigFile();
  }

  //bot Setup
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  bot.sendMessage(CHAT_ID, String("A Device Started up! ") + String("Trash bin with an ID of ") + String(testNumber) + String(" located at ") + String(testString) + String(" is now active!"), "");

  //////////////////////////////////////////////
  digitalWrite(BLUE_LED_PIN_WIFI, false);
  digitalWrite(RED_LED_PIN_WIFI, false);
  digitalWrite(GREEN_LED_PIN_WIFI, true);

  Serial.println("\n Device Starting...");
}

void loop() {
  // Ultrasonic Sensor Code
  unsigned int distance = ultrasonicSensor.ping_cm();
  if (distance != 0 && distance != NO_ECHO) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Error: No echo");
  }

  // Control trigger pin based on distance
  if (distance <= 20) {
    digitalWrite(BUTTON_TRIGGER_PIN, HIGH);
  } else {
    digitalWrite(BUTTON_TRIGGER_PIN, LOW);
  }

  // Button and LED (Ultrasonic) Code
  if (digitalRead(ULTRASONIC_BUTTON_PIN)) {
    if (!isButtonPressed) {
      isButtonPressed = true;
      buttonPressStartTime = millis();
    }

    if ((millis() - buttonPressStartTime > LONG_PRESS_THRESHOLD) && !isLongPressActive && !weightExceededThreshold) {
      isLongPressActive = true;
      servoMotor.write(OPEN_SERVO_ANGLE);
      isULTRASONICRedLedOn = !isULTRASONICRedLedOn;
      digitalWrite(GREEN_LED_PIN_ULTRASONIC, isULTRASONICRedLedOn);
      buttonPressStartTime = 0;
    }
  } else {
    if (isButtonPressed) {
      if (isLongPressActive) {
        isReleaseActive = true;
        buttonReleaseTime = millis();
        Serial.println(buttonReleaseTime);
      } else {
        // Do something for short press here
      }
      isButtonPressed = false;
    }

    if (isReleaseActive && (millis() - buttonReleaseTime > 3000)) {
      isLongPressActive = false;
      isReleaseActive = false;
      isULTRASONICRedLedOn = !isULTRASONICRedLedOn;
      servoMotor.write(CLOSE_SERVO_ANGLE);
      digitalWrite(GREEN_LED_PIN_ULTRASONIC, isULTRASONICRedLedOn);
    }
  }

  // HX711 Code
  if (loadCell.update()) {
    newDataReady = true;
  }

  currentHX711Millis = millis();
  if (currentHX711Millis - previousHX711Millis >= INTERVAL_MS) {
    previousHX711Millis = currentHX711Millis;

    if (newDataReady) {
      float weight = loadCell.getData();
      weight = max(weight, 0.0f);
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      newDataReady = false;

      int brightness = map(weight, 0, WEIGHT_THRESHOLD, 0, 255);
      brightness = constrain(brightness, 0, 255);

      analogWrite(GREEN_LED_PIN_HX711, 255 - brightness);
      analogWrite(RED_LED_PIN_HX711, brightness);

      // Check if weight exceeds threshold
      if (weight > WEIGHT_THRESHOLD) {
        if (!weightExceededThreshold) {
          // First time weight exceeds threshold
          bot.sendMessage(CHAT_ID, "Trash Bin Alert! Trash bin with an ID of " + String(testNumber) + " located at " + String(testString)
            + " has exeeded the " + String(WEIGHT_THRESHOLD) + " grams Limit! Please Collect.", "");
          weightExceededThreshold = true;
          isLongPressActive = false;
          isReleaseActive = false;
          isULTRASONICRedLedOn = false;
          servoMotor.write(CLOSE_SERVO_ANGLE);
          digitalWrite(GREEN_LED_PIN_ULTRASONIC, isULTRASONICRedLedOn);
        }
      } else if (weight < WEIGHT_THRESHOLD*0.8) {
        // Weight goes down below 80% of the limit
        weightExceededThreshold = false;
        // Add any additional actions here
      }
    }
  }

  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      loadCell.tareNoDelay();
    }
  }

  tareComplete = loadCell.getTareStatus();
  if (tareComplete) {
    Serial.println("Tare complete");
  }

  // Read the state of the button
  // If the button is pressed
  if (digitalRead(WIFI_BUTTON_PIN)) {
    digitalWrite(BLUE_LED_PIN_WIFI, true);
    digitalWrite(GREEN_LED_PIN_WIFI, false);
    ////////////////////////////////////////////////
    //Reset Wifi Settings
    ////////////////////////////////////////////////
    // Reset settings (only for development)
    wm.resetSettings();
    // Set config save notify callback
    wm.setSaveConfigCallback(saveConfigCallback);
    // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wm.setAPCallback(configModeCallback);
    // Custom elements
    // Text box (String) - 50 characters maximum
    WiFiManagerParameter custom_text_box("key_text", "Device Location", testString, 50);
    // Need to convert numerical input to string to display the default value.
    char convertedValue[6];
    sprintf(convertedValue, "%d", testNumber);
    // Text box (Number) - 7 characters maximum
    WiFiManagerParameter custom_text_box_num("key_num", "Identification Number", convertedValue, 7);
    if (forceConfig)
    // Run if we need a configuration
    {
      if (!wm.startConfigPortal("Trash_Bin_AP", "password")) {
        digitalWrite(BLUE_LED_PIN_WIFI, false);
        digitalWrite(RED_LED_PIN_WIFI, true);
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // if we still have not connected restart and try all over again
        Serial.println("Restarting");
        ESP.restart();
        delay(5000);
      }
    } else {
      if (!wm.autoConnect("Trash_Bin_AP", "password")) {
        digitalWrite(BLUE_LED_PIN_WIFI, false);
        digitalWrite(RED_LED_PIN_WIFI, true);
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // if we still have not connected restart and try all over again
        Serial.println("Restarting");
        ESP.restart();
        delay(5000);
      }
    }

    // If we get here, we are connected to the WiFi
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Lets deal with the user config values

    // Copy the string value
    strncpy(testString, custom_text_box.getValue(), sizeof(testString));
    Serial.print("testString: ");
    Serial.println(testString);

    //Convert the number value
    testNumber = atoi(custom_text_box_num.getValue());
    Serial.print("testNumber: ");
    Serial.println(testNumber);

    // Save the custom parameters to FS
    if (shouldSaveConfig) {
      saveConfigFile();
    }

    //bot Setup
    Serial.print("Retrieving time: ");
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    while (now < 24 * 3600)
    {
      Serial.print(".");
      delay(100);
      now = time(nullptr);
    }
    Serial.println(now);

    bot.sendMessage(CHAT_ID, String("A Device Started up! ") + String("Trash bin with an ID of ") + String(testNumber) + String(" located at ") + String(testString) + String(" is now active!"), "");

    ///////////////////////////////////////////
    digitalWrite(BLUE_LED_PIN_WIFI, false);
    digitalWrite(RED_LED_PIN_WIFI, false);
    digitalWrite(GREEN_LED_PIN_WIFI, true);
  }
}

void saveConfigFile()
// Save Config in JSON format
{
  Serial.println(F("Saving configuration..."));

  // Create a JSON document
  StaticJsonDocument<512> json;
  json["testString"] = testString;
  json["testNumber"] = testNumber;

  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile) {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }

  // Serialize JSON data to write to file
  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0) {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
  // Close file
  configFile.close();
}

bool loadConfigFile()
// Load existing configuration file
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();

  // Read configuration from FS json
  Serial.println("Mounting File System...");

  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE)) {
      // The file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile) {
        Serial.println("Opened configuration file");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error) {
          Serial.println("Parsing JSON");

          strcpy(testString, json["testString"]);
          testNumber = json["testNumber"].as<int>();

          return true;
        } else {
          // Error loading JSON data
          Serial.println("Failed to load json config");
        }
      }
    }
  } else {
    // Error mounting file system
    Serial.println("Failed to mount FS");
  }

  return false;
}

void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
  Serial.println("Entered Configuration Mode");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}