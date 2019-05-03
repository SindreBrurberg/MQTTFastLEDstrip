#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "env.h"

CRGB leds[NUM_LEDS];


WiFiClient espClient;
PubSubClient client(espClient);

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

long lastMsg = 0;
char msg[50];
//int value = 0;

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;
uint8_t hue = 0;
uint8_t hue_gain = 0;
uint8_t mode = 0;
int updates_per_second = 500;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  switch ((char)payload[0])
  {
    case 'm':
      mode = payload[1];
      break;
    case 'p': // Brightness
      FastLED.setBrightness(payload[1]);
      break;
    case 'u': // Updates per secound
      updates_per_second = payload[1] * 10;
      break;
    case 'r':
      r = payload[1];
      break;
    case 'g':
      g = payload[1];
      break;
    case 'b':
      b = payload[1];
      break;
    case 'd':
      hue_gain = payload[1];
      break;
    case 'h':
      hue = payload[1];
      break;
    case 'l':
      Serial.println(payload[1]);
      break;
    //default:
      //mode = 0;
      //fill_solid(leds, NUM_LEDS, CRGB::White);
      //break;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  delay( 3000 ); // power-up safety delay
	Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);

  setup_wifi();

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);


  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop()
{
  reconnect();
  client.loop();
  switch (mode)
  {
    case 1:
      fill_solid(leds, NUM_LEDS, CRGB(255, 117, 26));
      break;
    case 2:
      fill_solid(leds, NUM_LEDS, CHSV(35, 175, 255));
      break;
    case 3:
      fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
      break;
    case 4:
      fill_solid(leds, NUM_LEDS, CHSV(hue += hue_gain, 255, 255));
      break;
    case 5:
	    fill_rainbow(leds, NUM_LEDS, hue += hue_gain, 255 / NUM_LEDS);
      break;
    default:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();
  FastLED.delay(1000 / updates_per_second);
}
