// Load libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// Access point network credentials
const char* ssid     = "ESP32-Server";
const char* password = "12345";

//Set web server port
AsyncWebServer server(80);

//LED strip variables, and array of LEDs
#define LED_PIN     32
#define NUM_LEDS    45
CRGB leds[NUM_LEDS];

//Struct type to hold LED addreses
struct ledAddress {
  int initial_led_target;
  int brightness;
  int ms_to_next_position;
  int total_leds_on;
  int type_of_wave;
  int parameter_for_amplitude;
  int amplitude_range;
  int beats_per_minute;
};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<body>
<form action="/form_handler" method="POST">
  <label for="target">Target LED:</label>
  <input type="text" id="target_led" name="target_led"><br><br>
    <div>
    <button>Submit</button>
  </div>
</form>

</body>
</html>
)rawliteral";

String processor(const String& var)
{
 
  Serial.println(var);
 
  if(var == "PLACEHOLDER_LED"){
    return "Hello from the processor function";
  }
}

void turn_on_target_led(int target){
        leds[target] = CRGB::Red; 
        FastLED.show(); 
}

void setup() {
  //Initialize LEDs
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  Serial.begin(115200);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

    // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/form_handler", HTTP_POST, [] (AsyncWebServerRequest *request) {
    AsyncWebParameter* p = request->getParam(0);
    turn_on_target_led(p->value().toInt() );  
    //request->send(200, "text/plain", "ok");
  });
  
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

}
