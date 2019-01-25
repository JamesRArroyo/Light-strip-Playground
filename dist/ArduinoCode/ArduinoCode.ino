/*
  .______   .______    __    __   __    __          ___      __    __  .___________.  ______   .___  ___.      ___   .___________. __    ______   .__   __.
  |   _  \  |   _  \  |  |  |  | |  |  |  |        /   \    |  |  |  | |           | /  __  \  |   \/   |     /   \  |           ||  |  /  __  \  |  \ |  |
  |  |_)  | |  |_)  | |  |  |  | |  |__|  |       /  ^  \   |  |  |  | `---|  |----`|  |  |  | |  \  /  |    /  ^  \ `---|  |----`|  | |  |  |  | |   \|  |
  |   _  <  |      /  |  |  |  | |   __   |      /  /_\  \  |  |  |  |     |  |     |  |  |  | |  |\/|  |   /  /_\  \    |  |     |  | |  |  |  | |  . `  |
  |  |_)  | |  |\  \-.|  `--'  | |  |  |  |     /  _____  \ |  `--'  |     |  |     |  `--'  | |  |  |  |  /  _____  \   |  |     |  | |  `--'  | |  |\   |
  |______/  | _| `.__| \______/  |__|  |__|    /__/     \__\ \______/      |__|      \______/  |__|  |__| /__/     \__\  |__|     |__|  \______/  |__| \__|

  Thanks much to @corbanmailloux for providing a great framework for implementing flash/fade with HomeAssistant https://github.com/corbanmailloux/esp-mqtt-rgb-led
  
  To use this code you will need the following dependancies: 
  
  - Support for the ESP8266 boards. 
        - You can add it to the board manager by going to File -> Preference and pasting http://arduino.esp8266.com/stable/package_esp8266com_index.json into the Additional Board Managers URL field.
        - Next, download the ESP8266 dependancies by going to Tools -> Board -> Board Manager and searching for ESP8266 and installing it.
  
  - You will also need to download the follow libraries by going to Sketch -> Include Libraries -> Manage Libraries
      - FastLED 
      - PubSubClient
      - ArduinoJSON
*/

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "FastLED.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>



/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) ******************/
const char* ssid = "printer"; //type your WIFI information inside the quotes
const char* password = "magnolia";
const char* mqtt_server = "192.168.1.118";
const char* mqtt_username = "admin";
const char* mqtt_password = "password";
const int mqtt_port = 1883;



/**************************** FOR OTA **************************************************/
char* sensor_name = "stripRoss"; //change this to whatever you want to call your device

/* HTTP Server OTA */
const int FW_VERSION = 65; // increment this on each update.
const char* fwUrlBase = "http://192.168.1.118:8266/"; // Url to the http server that will provide update.


/************* MQTT TOPICS (change these topics as you wish)  **************************/
const char* light_state_topic = "bruh/porch";
char* light_set_topic = "bruh/porch/set";
const char* sound_sensor = "bruh/sound";

const char* on_cmd = "ON";
const char* off_cmd = "OFF";
const char* effect = "solid";
String effectString = "solid";
String oldeffectString = "solid";
int MILLION = 100000;



/****************************************FOR JSON***************************************/
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
#define MQTT_MAX_PACKET_SIZE 512



/*********************************** FastLED Defintions ********************************/
byte Light_ID = 0;

int NUM_LEDSX = 200;
#define NUM_LEDS    200
#define DATA_PIN    5
#define AOUT_PIN    2
#define DOUT_PIN    4
int analogValue;
//#define CLOCK_PIN 5
#define CHIPSET     WS2811
#define COLOR_ORDER BRG

byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;

byte red = 255;
byte green = 255;
byte blue = 255;
byte brightness = 255;



/******************************** GLOBALS for fade/flash *******************************/
bool stateOn = false;
bool startFade = false;
bool onbeforeflash = false;
unsigned long lastLoop = 0;
int transitionTime = 0;
int effectSpeed = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB;
int redVal, grnVal, bluVal;

bool flash = false;
bool startFlash = false;
int flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = red;
byte flashGreen = green;
byte flashBlue = blue;
byte flashBrightness = brightness;



/********************************** GLOBALS for EFFECTS ******************************/
//RAINBOW
uint8_t thishue = 0;                                          // Starting hue value.
uint8_t deltahue = 10;

//CANDYCANE
CRGBPalette16 currentPalettestriped; //for Candy Cane
CRGBPalette16 gPal; //for fire

//NOISE
static uint16_t dist;         // A random number for our noise generator.
uint16_t scale = 30;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint8_t maxChanges = 48;      // Value for blending between palettes.
CRGBPalette16 targetPalette(OceanColors_p);
CRGBPalette16 currentPalette(CRGB::Black);

//TWINKLE
#define DENSITY     80
int twinklecounter = 0;

//RIPPLE
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.
uint8_t bgcol = 0;                                            // Background colour rotates.
int thisdelay = 20;                                           // Standard delay value.

//DOTS
uint8_t   count =   0;                                        // Count up to 255 and then reverts to 0
uint8_t fadeval = 224;                                        // Trail behind the LED's. Lower => faster fade.
uint8_t bpm = 30;

//LIGHTNING
uint8_t frequency = 50;                                       // controls the interval between strikes
uint8_t flashes = 8;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;
uint8_t ledstart;                                             // Starting location of a flash
uint8_t ledlen;
int lightningcounter = 0;

//FUNKBOX
int idex = 0;                //-LED INDEX (0 to NUM_LEDS-1
int TOP_INDEX = int(NUM_LEDS / 2);
int thissat = 255;           //-FX LOOPS DELAY VAR
uint8_t thishuepolice = 0;
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % NUM_LEDS;
  }
  return iN;
}

//FIRE
#define COOLING  55
#define SPARKING 120
bool gReverseDirection = false;

//BPM
uint8_t gHue = 0;


WiFiClient espClient;
PubSubClient client(espClient);
struct CRGB leds[NUM_LEDS];



void oneWhite() {
  leds[0] = CRGB(255, 255, 235);
  FastLED.show();
}

void oneOrange() {
  leds[0] = CRGB(255, 69, 0);
  FastLED.show();
}

void oneGreen() {
  leds[0] = CRGB(0, 255, 0);
  FastLED.show();
  delay(5000);
  FastLED.clear();
}

void oneRed() {
  leds[0] = CRGB(255, 0, 0);
  delay(5);
  FastLED.show();
}

void oneBlue() {
  leds[0] = CRGB(0, 0, 255);
  delay(5);
  FastLED.show();
}


/********************************** START SETUP*****************************************/
void setup() {
  Serial.begin(115200);
  Serial.print("My big MAC: ");
  Serial.println(WiFi.macAddress());
  setup_board_params();
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  pinMode(AOUT_PIN, INPUT); //  The analog pin for the sound board on the arduino
  
  setupStripedPalette( CRGB::Red, CRGB::Red, CRGB::White, CRGB::White); //for CANDY CANE
  gPal = HeatColors_p; //for FIRE

  setup_wifi();
 

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  checkForUpdates();

}


void setup_board_params() {
  String mac = WiFi.macAddress();

  if (mac == "84:0D:8E:83:13:6C") {
     Light_ID = 1;
     Serial.print("I want to be 1 and am actually ");
     Serial.println(Light_ID);
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     sensor_name = "light1";
     NUM_LEDSX = 50;
     light_set_topic = "wall";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:0D:8E:83:11:E3")  {
     Light_ID = 2;
     Serial.print("I want to be 3 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light2";
     NUM_LEDSX = 150;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "wall";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:0D:8E:83:12:52")  {
     Light_ID = 3;
     Serial.print("I want to be 3 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light3";
     NUM_LEDSX = 150;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "wall";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:F3:EB:B6:FA:A0")  {
     Light_ID = 4;
     Serial.print("I want to be 4 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light4";
     NUM_LEDSX = 50;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "wall";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:0D:8E:83:12:E4")  {
     Light_ID = 5;
     Serial.print("I want to be 5 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light5";
     NUM_LEDSX = 50;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "elevator";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:0D:8E:83:13:E1")  {
     Light_ID = 6;
     Serial.print("I want to be 6 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light6";
     NUM_LEDSX = 50;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light6";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "CC:50:E3:4A:4B:1E")  {
     Light_ID = 7;
     Serial.print("I want to be 7 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light7";
     NUM_LEDSX = 50;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light7";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:F3:EB:2F:DB:7D")  {
     Light_ID = 8;
     Serial.print("I want to be 8 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light8";
     NUM_LEDSX = 150;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light8";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "84:0D:8E:83:06:76")  {
     Light_ID = 9;
     Serial.print("I want to be 9 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light9";
     NUM_LEDSX = 150;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light9";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "CC:50:E3:4A:4F:7F")  {
     Light_ID = 10;
     Serial.print("I want to be 10 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light10";
     NUM_LEDSX = 50;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light10";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "CC:50:E3:47:9D:FF")  {
     Light_ID = 11;
     sensor_name = "light11";
     NUM_LEDSX = 50;
     light_set_topic = "light11";
    return;
  }else if (mac == "CC:50:E3:4A:50:48")  {
     Light_ID = 12;
     sensor_name = "light12";
     NUM_LEDSX = 50;
     light_set_topic = "light12";
    return;
  }else if (mac == "84:0D:8E:83:9B:EC")  {
     Light_ID = 13;
     sensor_name = "light13";
     NUM_LEDSX = 50;
     light_set_topic = "light13";
    return;
  }else if (mac == "CC:50:E3:4A:4C:91")  {
     Light_ID = 14;
     sensor_name = "light14";
     NUM_LEDSX = 50;
     light_set_topic = "light14";
    return;
  }else if (mac == "CC:50:E3:4A:4B:3C")  {
     Light_ID = 15;
     sensor_name = "light15";
     NUM_LEDSX = 50;
     light_set_topic = "light15";
    return;
  }else if (mac == "80:7D:3A:3D:AD:F4")  {
     Light_ID = 16;
     sensor_name = "light16";
     NUM_LEDSX = 50;
     light_set_topic = "light16";
    return;
  }else if (mac == "84:0D:8E:83:03:CD")  {
     Light_ID = 17;
     Serial.print("I want to be 17 and am actually ");
     Serial.println(Light_ID);
     sensor_name = "light17";
     NUM_LEDSX = 117;
     Serial.print("num of LEDs is ");
     Serial.print(NUM_LEDSX);
     light_set_topic = "light17";
     Serial.print("My topic is ");
     Serial.println(light_set_topic);
    return;
  }else if (mac == "CC:50:E3:20:3D:CE")  {
     Light_ID = 18;
     sensor_name = "light18";
     NUM_LEDSX = 50;
     light_set_topic = "light18";
    return;
  }else if (mac == "84:0D:8E:83:94:24")  { 
     Light_ID = 19;
     sensor_name = "light19";
     NUM_LEDSX = 50;
     light_set_topic = "light19";
    return;
  }else if (mac == "80:7D:3A:3D:B1:A7")  {
     Light_ID = 20;
     sensor_name = "light20";
     NUM_LEDSX = 50;
     light_set_topic = "light20";
    return;
  }else if (mac == "84:0D:8E:83:12:80")  {
     Light_ID = 21;
     sensor_name = "light21";
     NUM_LEDSX = 50;
     light_set_topic = "light21";
    return;
  }else if (mac == "CC:50:E3:4A:4E:6E")  {
     Light_ID = 22;
     sensor_name = "light22";
     NUM_LEDSX = 50;
     light_set_topic = "light22";
    return;
  }else if (mac == "CC:50:E3:4A:4F:q3")  { // MISSING
     Light_ID = 23;
     sensor_name = "light23";
     NUM_LEDSX = 50;
     light_set_topic = "light23";
    return;
  }else if (mac == "CC:50:E3:4A:4F:q4")  { // MISSING
     Light_ID = 24;
     sensor_name = "light24";
     NUM_LEDSX = 50;
     light_set_topic = "light24";
    return;
  }else if (mac == "CC:50:E3:4A:4F:q5")  { // MISSING
     Light_ID = 25;
     sensor_name = "light25";
     NUM_LEDSX = 50;
     light_set_topic = "light25";
    return;
  }else if (mac == "84:0D:8E:83:03:44")  {
     Light_ID = 7;
     sensor_name = "strip1";
     NUM_LEDSX = 117;
     light_set_topic = "jimmy/strip1";
    return;
  }else {
    Serial.print("Failed to set board Params");
    Serial.print("I dont know who I am ");
    Serial.println(Light_ID);
  }   
Serial.print("For Giggles My ID is ");
Serial.println(Light_ID);
}

/********************************** CHECK FOR UPDATES (OTA) *****************************************/
void checkForUpdates() {
  effectString == "solid";
  oneWhite();
  String mac = WiFi.macAddress();
  String fwURL = String( fwUrlBase );
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "update_firmware/version" );

  Serial.println( "Checking for firmware updates." );
  Serial.print( "MAC address: " );
  Serial.println( "ArduinoCode.ino.nodemcu.bin" );
  Serial.print( "Firmware version URL: " );
  Serial.println( fwVersionURL );

  


  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();

    Serial.print( "Current firmware version: " );
    Serial.println( FW_VERSION );
    Serial.print( "Available firmware version: " );
    Serial.println( newFWVersion );

    int newVersion = newFWVersion.toInt();

    if( newVersion > FW_VERSION ) {
      oneOrange();
      Serial.println( "Preparing to update" );

      String fwImageURL = fwURL;
      Serial.println( fwImageURL );
      fwImageURL.concat("update_firmware");
      Serial.println( fwImageURL );
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
      }
    }
    else {
      oneGreen();
      Serial.println( "Already on latest version" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
  }
  httpClient.end();
}


String getMAC()
{
  uint8_t mac[6];
  char result[14];

 snprintf( result, sizeof( result ), "%02x%02x%02x%02x%02x%02x", mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  return String( result );
}




/********************************** START SETUP WIFI*****************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/*  SAMPLE PAYLOAD:
  {
    "brightness": 120,
    "color": {
      "r": 255,
      "g": 100,
      "b": 100
    },
    "flash": 2,
    "transition": 5,
    "state": "ON"
  }
*/



/********************************** START CALLBACK*****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }

  if (stateOn) {

    realRed = map(red, 0, 255, 0, brightness);
    realGreen = map(green, 0, 255, 0, brightness);
    realBlue = map(blue, 0, 255, 0, brightness);
  }
  else {

    realRed = 0;
    realGreen = 0;
    realBlue = 0;
  }

  Serial.println(effect);

  startFade = true;
  inFade = false; // Kill the current fade

  sendState();
  String logUrl = String(fwUrlBase);
  logUrl.concat( "lights/log");

  String msgPayload = "{ \"topic\" : \"" + String(topic) +"\", \"message\" : \"" + String(message) +"\"}";

  Serial.println('******************************');
  Serial.println(msgPayload);
  Serial.println('******************************');
  Serial.println(logUrl);

  HTTPClient http;
  http.begin( logUrl );
  http.addHeader("Content-Type", "application/json"); 
  http.POST(msgPayload);
  http.writeToStream(&Serial);
  http.end();
}



/********************************** START PROCESS JSON*****************************************/
bool processJson(char* message) {
  StaticJsonBuffer<512> jsonBuffer;
  /*DynamicJsonBuffer jsonBuffer(512);*/

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  if (root.containsKey("state")) {
    if (strcmp(root["state"], on_cmd) == 0) {
      stateOn = true;
    }
    else if (strcmp(root["state"], off_cmd) == 0) {
      stateOn = false;
      onbeforeflash = false;
    }
  }

  // If "flash" is included, treat RGB and brightness differently
  if (root.containsKey("flash")) {
    flashLength = (int)root["flash"] * 1000;

    oldeffectString = effectString;

    if (root.containsKey("brightness")) {
      flashBrightness = root["brightness"];
    }
    else {
      flashBrightness = brightness;
    }

    if (root.containsKey("color")) {
      flashRed = root["color"]["r"];
      flashGreen = root["color"]["g"];
      flashBlue = root["color"]["b"];
    }
    else {
      flashRed = red;
      flashGreen = green;
      flashBlue = blue;
    }

    if (root.containsKey("effect")) {
      effect = root["effect"];
      effectString = effect;
      twinklecounter = 0; //manage twinklecounter
    }

    if (root.containsKey("transition")) {
      transitionTime = root["transition"];
    }
    else if ( effectString == "solid") {
      transitionTime = 0;
    }

    flashRed = map(flashRed, 0, 255, 0, flashBrightness);
    flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
    flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);

    flash = true;
    startFlash = true;
  }
  else { // Not flashing
    flash = false;

    if (stateOn) {   //if the light is turned on and the light isn't flashing
      onbeforeflash = true;
    }

    if (root.containsKey("color")) {
      red = root["color"]["r"];
      green = root["color"]["g"];
      blue = root["color"]["b"];
    }
    
    if (root.containsKey("color_temp")) {
      //temp comes in as mireds, need to convert to kelvin then to RGB
      int color_temp = root["color_temp"];
      unsigned int kelvin  = MILLION / color_temp;
      
      temp2rgb(kelvin);
      
    }

    if (root.containsKey("brightness")) {
      brightness = root["brightness"];
    }

    if (root.containsKey("effect")) {
      effect = root["effect"];
      effectString = effect;
      twinklecounter = 0; //manage twinklecounter
    }

    if (root.containsKey("transition")) {
      transitionTime = root["transition"];
    }
    else if ( effectString == "solid") {
      transitionTime = 0;
    }

  }

  return true;
}



/********************************** START SEND STATE*****************************************/
void sendState() {
  StaticJsonBuffer<512> jsonBuffer;
  /*DynamicJsonBuffer jsonBuffer(512);*/

  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (stateOn) ? on_cmd : off_cmd;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = red;
  color["g"] = green;
  color["b"] = blue;

  root["brightness"] = brightness;
  root["effect"] = effectString.c_str();


  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(light_state_topic, buffer, true);
}



/********************************** START RECONNECT*****************************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(sensor_name, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic, 1);
      setColor(0, 0, 0);
      client.publish("esp/test", "Hello from ESP8266", 1);
      sendState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



/********************************** START Set Color*****************************************/
void setColor(int inR, int inG, int inB) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].red   = inR;
    leds[i].green = inG;
    leds[i].blue  = inB;
  }

  FastLED.show();

  Serial.println("Setting LEDs:");
  Serial.print("r: ");
  Serial.print(inR);
  Serial.print(", g: ");
  Serial.print(inG);
  Serial.print(", b: ");
  Serial.println(inB);
}

int stompFadeAmount = 5;  // Set the amount to fade I usually do 5, 10, 15, 20, 25 etc even up to 255.
int stompBrightness = 0;
int stompLoopCount = 0;

/********************************** START MAIN LOOP*****************************************/
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }

  client.loop();

  /*analogValue= analogRead(AOUT_PIN);// hoping to read messages from the sound sensor
  Serial.print("analogValue: ");
  Serial.println(analogValue);// prints the CO value
  delay(60000);
  */

// UPDATE
  if (effectString == "update"){
    checkForUpdates();
  }


  //EFFECT STOMP
  if (effectString == "stomp") {
    Serial.print(effectString);
    stompLoopCount++;
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setRGB(255, 255, 255);
      leds[i].fadeLightBy(stompBrightness);
    }
    FastLED.show();
    stompBrightness = stompBrightness + stompFadeAmount;
    // reverse the direction of the fading at the ends of the fade:
    if (stompBrightness == 0 || stompBrightness == 255) {
      stompFadeAmount = -stompFadeAmount;
    }
    delay(5); // This delay sets speed of the fade. I usually do from 5-75 but you can always go higher.
    Serial.println(stompLoopCount);
    if (stompLoopCount == 2) {
      Serial.println("Should Stop");
      stateOn = false;
      setColor(0, 0, 0);
      sendState();
      stompLoopCount = 0;
    }

    

  }

  //EFFECT BPM
  if (effectString == "bpm") {
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for ( int i = 0; i < NUM_LEDS; i++) { //9948
      leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }


  //EFFECT Candy Cane
  if (effectString == "candy cane") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */
    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  currentPalettestriped, 255, LINEARBLEND);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 0;
    }
    showleds();
  }


  //EFFECT CONFETTI
  if (effectString == "confetti" ) {
    fadeToBlackBy( leds, NUM_LEDS, 25);
    int pos = random16(NUM_LEDS);
    leds[pos] += CRGB(realRed + random8(64), realGreen, realBlue);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }


  //EFFECT CYCLON RAINBOW
  if (effectString == "cyclon rainbow") {                    //Single Dot Down
    static uint8_t hue = 0;
    // First slide the led in one direction
    for (int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to red
      leds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      showleds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeall();
      // Wait a little bit before we loop around and do it again
      delay(10);
    }
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      // Set the i'th led to red
      leds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      showleds();
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeall();
      // Wait a little bit before we loop around and do it again
      delay(10);
    }
  }


  //EFFECT DOTS
  if (effectString == "dots") {
    uint8_t inner = beatsin8(bpm, NUM_LEDS / 4, NUM_LEDS / 4 * 3);
    uint8_t outer = beatsin8(bpm, 0, NUM_LEDS - 1);
    uint8_t middle = beatsin8(bpm, NUM_LEDS / 3, NUM_LEDS / 3 * 2);
    leds[middle] = CRGB::Purple;
    leds[inner] = CRGB::Blue;
    leds[outer] = CRGB::Aqua;
    nscale8(leds, NUM_LEDS, fadeval);

    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }


  //EFFECT FIRE
  if (effectString == "fire") {
    Fire2012WithPalette();
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 150;
    }
    showleds();
  }

  random16_add_entropy( random8());


  //EFFECT Glitter
  if (effectString == "glitter") {
    fadeToBlackBy( leds, NUM_LEDS, 20);
    addGlitterColor(80, realRed, realGreen, realBlue);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }


  //EFFECT JUGGLE
  if (effectString == "juggle" ) {                           // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    for (int i = 0; i < 8; i++) {
      leds[beatsin16(i + 7, 0, NUM_LEDS - 1  )] |= CRGB(realRed, realGreen, realBlue);
    }
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 130;
    }
    showleds();
  }


  //EFFECT LIGHTNING
  if (effectString == "lightning") {
    twinklecounter = twinklecounter + 1;                     //Resets strip if previous animation was running
    if (twinklecounter < 2) {
      FastLED.clear();
      FastLED.show();
    }
    ledstart = random8(NUM_LEDS);           // Determine starting location of flash
    ledlen = random8(NUM_LEDS - ledstart);  // Determine length of flash (not to go beyond NUM_LEDS-1)
    for (int flashCounter = 0; flashCounter < random8(3, flashes); flashCounter++) {
      if (flashCounter == 0) dimmer = 5;    // the brightness of the leader is scaled down by a factor of 5
      else dimmer = random8(1, 3);          // return strokes are brighter than the leader
      fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 255 / dimmer));
      showleds();    // Show a section of LED's
      delay(random8(4, 10));                // each flash only lasts 4-10 milliseconds
      fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 0)); // Clear the section of LED's
      showleds();
      if (flashCounter == 0) delay (130);   // longer delay until next flash after the leader
      delay(50 + random8(100));             // shorter delay between strokes
    }
    delay(random8(frequency) * 100);        // delay between strikes
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 0;
    }
    showleds();
  }


  //EFFECT POLICE ALL
  if (effectString == "police all") {                 //POLICE LIGHTS (TWO COLOR SOLID)
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    leds[idexR] = CHSV(thishuepolice, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }

  //EFFECT POLICE ONE
  if (effectString == "police one") {
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    for (int i = 0; i < NUM_LEDS; i++ ) {
      if (i == idexR) {
        leds[i] = CHSV(thishuepolice, thissat, 255);
      }
      else if (i == idexB) {
        leds[i] = CHSV(thathue, thissat, 255);
      }
      else {
        leds[i] = CHSV(0, 0, 0);
      }
    }
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 30;
    }
    showleds();
  }


  //EFFECT RAINBOW
  if (effectString == "rainbow") {
    // FastLED's built-in rainbow generator
    static uint8_t starthue = 0;    thishue++;
    fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 130;
    }
    showleds();
  }


  //EFFECT RAINBOW WITH GLITTER
  if (effectString == "rainbow with glitter") {               // FastLED's built-in rainbow generator with Glitter
    static uint8_t starthue = 0;
    thishue++;
    fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
    addGlitter(80);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 130;
    }
    showleds();
  }


  //EFFECT SIENLON
  if (effectString == "sinelon") {
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    leds[pos] += CRGB(realRed, realGreen, realBlue);
    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 150;
    }
    showleds();
  }


  //EFFECT TWINKLE
  if (effectString == "twinkle") {
    twinklecounter = twinklecounter + 1;
    if (twinklecounter < 2) {                               //Resets strip if previous animation was running
      FastLED.clear();
      FastLED.show();
    }
    const CRGB lightcolor(8, 7, 1);
    for ( int i = 0; i < NUM_LEDS; i++) {
      if ( !leds[i]) continue; // skip black pixels
      if ( leds[i].r & 1) { // is red odd?
        leds[i] -= lightcolor; // darken if red is odd
      } else {
        leds[i] += lightcolor; // brighten if red is even
      }
    }
    if ( random8() < DENSITY) {
      int j = random16(NUM_LEDS);
      if ( !leds[j] ) leds[j] = lightcolor;
    }

    if (transitionTime == 0 or transitionTime == NULL) {
      transitionTime = 0;
    }
    showleds();
  }


  EVERY_N_MILLISECONDS(10) {

    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // FOR NOISE ANIMATIon
    {
      gHue++;
    }

    //EFFECT NOISE
    if (effectString == "noise") {
      for (int i = 0; i < NUM_LEDS; i++) {                                     // Just onE loop to fill up the LED array as all of the pixels change.
        uint8_t index = inoise8(i * scale, dist + i * scale) % 255;            // Get a value from the noise function. I'm using both x and y axis.
        leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
      }
      dist += beatsin8(10, 1, 4);                                              // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
      // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
      if (transitionTime == 0 or transitionTime == NULL) {
        transitionTime = 0;
      }
      showleds();
    }

    //EFFECT RIPPLE
    if (effectString == "ripple") {
      for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(bgcol++, 255, 15);  // Rotate background colour.
      switch (step) {
        case -1:                                                          // Initialize ripple variables.
          center = random(NUM_LEDS);
          colour = random8();
          step = 0;
          break;
        case 0:
          leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
          step ++;
          break;
        case maxsteps:                                                    // At the end of the ripples.
          step = -1;
          break;
        default:                                                             // Middle of the ripples.
          leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);   // Simple wrap from Marc Miller
          leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);
          step ++;                                                         // Next step.
          break;
      }
      if (transitionTime == 0 or transitionTime == NULL) {
        transitionTime = 30;
      }
      showleds();
    }

  }


  EVERY_N_SECONDS(5) {
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }

  //FLASH AND FADE SUPPORT
  if (flash) {
    if (startFlash) {
      startFlash = false;
      flashStartTime = millis();
    }

    if ((millis() - flashStartTime) <= flashLength) {
      if ((millis() - flashStartTime) % 1000 <= 500) {
        setColor(flashRed, flashGreen, flashBlue);
      }
      else {
        setColor(0, 0, 0);
        // If you'd prefer the flashing to happen "on top of"
        // the current color, uncomment the next line.
        // setColor(realRed, realGreen, realBlue);
      }
    }
    else {
      flash = false;
      effectString = oldeffectString;
      if (onbeforeflash) { //keeps light off after flash if light was originally off
        setColor(realRed, realGreen, realBlue);
      }
      else {
        stateOn = false;
        setColor(0, 0, 0);
        sendState();
      }
    }
  }

  if (startFade && effectString == "solid") {
    // If we don't want to fade, skip it.
    if (transitionTime == 0) {
      setColor(realRed, realGreen, realBlue);

      redVal = realRed;
      grnVal = realGreen;
      bluVal = realBlue;

      startFade = false;
    }
    else {
      loopCount = 0;
      stepR = calculateStep(redVal, realRed);
      stepG = calculateStep(grnVal, realGreen);
      stepB = calculateStep(bluVal, realBlue);

      inFade = true;
    }
  }

  if (inFade) {
    startFade = false;
    unsigned long now = millis();
    if (now - lastLoop > transitionTime) {
      if (loopCount <= 1020) {
        lastLoop = now;

        redVal = calculateVal(stepR, redVal, loopCount);
        grnVal = calculateVal(stepG, grnVal, loopCount);
        bluVal = calculateVal(stepB, bluVal, loopCount);

        if (effectString == "solid") {
          setColor(redVal, grnVal, bluVal); // Write current values to LED pins
        }
        loopCount++;
      }
      else {
        inFade = false;
      }
    }
  }
}


/**************************** START TRANSITION FADER *****************************************/
// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
  The program works like this:
  Imagine a crossfade that moves the red LED from 0-10,
    the green from 0-5, and the blue from 10 to 7, in
    ten steps.
    We'd want to count the 10 steps and increase or
    decrease color values in evenly stepped increments.
    Imagine a + indicates raising a value by 1, and a -
    equals lowering it. Our 10 step fade would look like:
    1 2 3 4 5 6 7 8 9 10
  R + + + + + + + + + +
  G   +   +   +   +   +
  B     -     -     -
  The red rises from 0 to 10 in ten steps, the green from
  0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
  In the real program, the color percentages are converted to
  0-255 values, and there are 1020 steps (255*4).
  To figure out how big a step there should be between one up- or
  down-tick of one of the LED values, we call calculateStep(),
  which calculates the absolute gap between the start and end values,
  and then divides that gap by 1020 to determine the size of the step
  between adjustments in the value.
*/
int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero,
    step = 1020 / step;          //   divide by 1020
  }

  return step;
}
/* The next function is calculateVal. When the loop value, i,
   reaches the step size appropriate for one of the
   colors, it increases or decreases the value of that color by 1.
   (R, G, and B are each calculated separately.)
*/
int calculateVal(int step, int val, int i) {
  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;
    }
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    }
  }

  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  }
  else if (val < 0) {
    val = 0;
  }

  return val;
}



/**************************** START STRIPLED PALETTE *****************************************/
void setupStripedPalette( CRGB A, CRGB AB, CRGB B, CRGB BA) {
  currentPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                            //    A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}



/********************************** START FADE************************************************/
void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);  //for CYCLon
  }
}



/********************************** START FIRE **********************************************/
void Fire2012WithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}



/********************************** START ADD GLITTER *********************************************/
void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}



/********************************** START ADD GLITTER COLOR ****************************************/
void addGlitterColor( fract8 chanceOfGlitter, int red, int green, int blue)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB(red, green, blue);
  }
}



/********************************** START SHOW LEDS ***********************************************/
void showleds() {

  delay(1);

  if (stateOn) {
    FastLED.setBrightness(brightness);  //EXECUTE EFFECT COLOR
    FastLED.show();
    if (transitionTime > 0 && transitionTime < 130) {  //Sets animation speed based on receieved value
      FastLED.delay(1000 / transitionTime);
      //delay(10*transitionTime);
    }
  }
  else if (startFade) {
    setColor(0, 0, 0);
    startFade = false;
  }
}
void temp2rgb(unsigned int kelvin) {
    int tmp_internal = kelvin / 100.0;
    
    // red 
    if (tmp_internal <= 66) {
        red = 255;
    } else {
        float tmp_red = 329.698727446 * pow(tmp_internal - 60, -0.1332047592);
        if (tmp_red < 0) {
            red = 0;
        } else if (tmp_red > 255) {
            red = 255;
        } else {
            red = tmp_red;
        }
    }
    
    // green
    if (tmp_internal <=66){
        float tmp_green = 99.4708025861 * log(tmp_internal) - 161.1195681661;
        if (tmp_green < 0) {
            green = 0;
        } else if (tmp_green > 255) {
            green = 255;
        } else {
            green = tmp_green;
        }
    } else {
        float tmp_green = 288.1221695283 * pow(tmp_internal - 60, -0.0755148492);
        if (tmp_green < 0) {
            green = 0;
        } else if (tmp_green > 255) {
            green = 255;
        } else {
            green = tmp_green;
        }
    }
    
    // blue
    if (tmp_internal >=66) {
        blue = 255;
    } else if (tmp_internal <= 19) {
        blue = 0;
    } else {
        float tmp_blue = 138.5177312231 * log(tmp_internal - 10) - 305.0447927307;
        if (tmp_blue < 0) {
            blue = 0;
        } else if (tmp_blue > 255) {
            blue = 255;
        } else {
            blue = tmp_blue;
        }
    }
}
