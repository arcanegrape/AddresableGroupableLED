// Load libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <esp_now.h>

// Access point network credentials
const char* ssid     = "Tenda_61F820";
const char* password = "Ch@rlie123";

//Set web server port
AsyncWebServer server(80);

//LED strip variables, and array of LEDs
#define LED_PIN     32
#define NUM_LEDS    46
CRGB leds[NUM_LEDS];

CHSV hsv_led;
CRGB rgb_led;

//Struct type to hold LED addreses

  //  int ledAddress[9];
  
 struct Led_Address{
  int initial_led_target;
  int brightness;
  int color;
  int ms_to_next_position;
  int total_leds_on;
  int type_of_wave;
  int parameter_for_amplitude;
  float amplitude_range;
  int beats_per_minute;
}; 

//MAC address and variable for ESPNow communication with other strips
uint8_t broadcastAddress1[] = {0x40, 0x22, 0xD8, 0x77, 0x22, 0x18};
Led_Address led_Address = {1,100,0xffffff,400,1,0,0,0,0};
esp_now_peer_info_t peerInfo;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="stylecheet" href="style.css"/>
    <title>CS 494P LED Project</title>
</head>
    <h1>LED Project</h1>

<body>

<form action="/" method="POST">
  
  <label for="target">Initial Target LED:</label>
  <input type="text" 
  id="initial_led_target" 
  value="1" 
  name="initial_led_target" ><br><br>

  <label for="target">Brightness:</label>
  <input type="text" 
  id="brightness"
  value="100" 
  name="brightness"><br><br>

  <label for="target">Color:</label>
  <input type="color" 
  id="color"
  name="color"><br><br>

  <label for="target">Milliseconds to next position:</label>
  <input type="text" 
  id="ms_to_next_position"
  value="10" 
  name="ms_to_next_position"><br><br>

  <label for="target">Consecutive LEDs on:</label>
  <input type="text" 
  id="total_leds_on"
  value="1" 
  name="total_leds_on"><br><br>

  <label for="target">Type of wave for control of a parameter:</label>
  <div>
    <input type="radio" id="type_of_wave1" name="type_of_wave" value="1" 
    checked/>
    <label for="type_of_wave1">Square</label>
    <input type="radio" id="type_of_wave2" name="type_of_wave" value="2" />
    <label for="type_of_wave2">Triangular</label>
    <input type="radio" id="type_of_wave3" name="type_of_wave" value="3" />
    <label for="type_of_wave3">Sine</label>
  </div><br><br>

  <label for="target">Parameter controlled by amplitude of wave:</label>
  <div>
    <input type="radio" id="parameter_for_amplitude0" name="parameter_for_amplitude" value="0" checked/>
    <label for="parameter_for_amplitude0">None</label>
    <input type="radio" id="parameter_for_amplitude1" name="parameter_for_amplitude" value="1" />
    <label for="parameter_for_amplitude1">Hue</label>
    <input type="radio" id="parameter_for_amplitude2" name="parameter_for_amplitude" value="2" />
    <label for="parameter_for_amplitude2">Brightness</label>
    <input type="radio" id="parameter_for_amplitude3" name="parameter_for_amplitude" value="3" />
    <label for="parameter_for_amplitude3">ms to next pos</label>
  </div><br><br>
  
  <label for="target">amplitude range (in %):</label>
  <input type="text" 
  id="amplitude_range"
  value="100" 
  name="amplitude_range"><br><br>

  <label for="target">wave's beats per minute:</label>
  <input type="text" 
  id="beats_per_minute"
  value="60" 
  name="beats_per_minute"><br><br>


  <label for="Priority">Priority of Message:</label>

<select name="Priority" id="priority">
  <option value="1">1</option>
  <option value="2">2</option>
  <option value="3">3</option>
  <option value="4">4</option>
  <option value="5">5</option>
</select><br><br>

    <div>
    <button>Submit</button>
    </div>
</form>


<br><br>
<label for="Group1">Assign LEDStrip MAC 40:22:D8:77:22:18 to group: </label>

<select name="Group1" id="group">
  <option value="1">1</option>
  <option value="2">2</option>
  <option value="3">3</option>
  <option value="4">4</option>
  <option value="5">5</option>
  <p>Current group:</p><b>1</b>
</select>

<br><br>

<label for="Group2">Assign LEDStrip MAC C0:49:EF:E5:64:24 to group: </label>

<select name="Group2" id="group">
  <option value="1">1</option>
  <option value="2">2</option>
  <option value="3">3</option>
  <option value="4">4</option>
  <option value="5">5</option>
  <p>Current group:</p><b>1</b>
</select>



</body>
</html>


)rawliteral";

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&led_Address, incomingData, sizeof(led_Address));
}

void setup() {

  
  //Initialize LEDs
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  Serial.begin(115200);
  
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }

  Serial.print("Station IP Address in Room of Requirement: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  Serial.println("ESP MAC address is:");
  Serial.println(WiFi.macAddress());
  
    //ESPNow Init
    if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
   }

    // Register peer ESPNow
    memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

    // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/", HTTP_POST, [] (AsyncWebServerRequest *request) {  
    AsyncWebParameter* p = request->getParam(0);
    //Print all data to serial for testing
      int params = request->params();
        for (int i = 0; i < params; i++)
          {
            AsyncWebParameter* p = request->getParam(i);
             Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());


      if(p->name() == "initial_led_target"){
        led_Address.initial_led_target = p->value().toInt();
      }
      if(p->name() == "brightness"){
        led_Address.brightness = p->value().toInt();
      }
      if(p->name() == "color"){
        String color_str = p->value().substring(1);
        led_Address.color = strtol(color_str.c_str(), NULL, 16);
        Serial.printf("The color variable is: %X \n:",  led_Address.color);
      }
      if(p->name() == "ms_to_next_position"){
        led_Address.ms_to_next_position = p->value().toInt();
      }
      if(p->name() == "total_leds_on"){
        led_Address.total_leds_on = p->value().toInt();
      }
      if(p->name() == "type_of_wave"){
        led_Address.type_of_wave = p->value().toInt();
      }
      if(p->name() == "parameter_for_amplitude"){
        led_Address.parameter_for_amplitude = p->value().toInt();
      }
      if(p->name() == "amplitude_range"){
        led_Address.amplitude_range = ((float)(p->value().toInt())/100.0);
      }
      if(p->name() == "beats_per_minute"){
        led_Address.beats_per_minute = p->value().toInt();
      }
          }
   

    // Bring html parameters into LedAddress struct    
    request->send(200, "text/html", index_html);
  });

  //Part of setup, not upper function
  esp_now_register_recv_cb(OnDataRecv);
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:



    //for loop for moving LED position forward
    for(;true ; led_Address.initial_led_target = (led_Address.initial_led_target + 1) % NUM_LEDS){

       rgb_led.setColorCode(led_Address.color);
       uint8_t true_brightness = map(led_Address.brightness, 0,100,0,255);
       rgb_led %= true_brightness;
       int true_ms_to_next_pos = led_Address.ms_to_next_position;

       for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Black; 
       }
        
      //If statements for handling Square
      //Hue
      if(led_Address.type_of_wave == 1){
        if(led_Address.parameter_for_amplitude == 1) {
          hsv_led = rgb2hsv_approximate(rgb_led);
          hsv_led.hue = hsv_led.hue + (int)(led_Address.amplitude_range *(squarewave8(beatsin8(led_Address.beats_per_minute), 128)/2)) ;
          rgb_led = hsv_led;
          //Serial.printf("hue of hsv_led %i \n", hsv_led.hue); 
          //Serial.printf("range value %f \n", led_Address.amplitude_range);    
          }
        if(led_Address.parameter_for_amplitude == 2){
          true_brightness += (int)(led_Address.amplitude_range*(squarewave8(beatsin8(led_Address.beats_per_minute), 128)/2));
          rgb_led %= true_brightness;
          //Serial.printf("B of hsv_led %i \n", true_brightness);
        }
        if(led_Address.parameter_for_amplitude == 3){
          
          true_ms_to_next_pos = led_Address.amplitude_range + (squarewave8(beatsin8(led_Address.beats_per_minute), 128)); 
          //Serial.printf("ms to next of strip %i \n", true_ms_to_next_pos);    
        }
       }

                 //Handling of Triangular wave
       if(led_Address.type_of_wave == 2){
        if(led_Address.parameter_for_amplitude == 1) {
          hsv_led = rgb2hsv_approximate(rgb_led);
          hsv_led.hue = hsv_led.hue + (int)(led_Address.amplitude_range * beat8(led_Address.beats_per_minute)) ;
          rgb_led = hsv_led;
          //Serial.printf("hue of hsv_led %i \n", hsv_led.hue); 
          //Serial.printf("range value %f \n", led_Address.amplitude_range);    
          }
        if(led_Address.parameter_for_amplitude == 2){
          true_brightness += (int)(led_Address.amplitude_range*(beat8(led_Address.beats_per_minute)));
          rgb_led %= true_brightness;
          //Serial.printf("B of hsv_led %i \n", true_brightness);
        }
        if(led_Address.parameter_for_amplitude == 3){
          
          true_ms_to_next_pos = led_Address.ms_to_next_position + led_Address.amplitude_range * (beat8(led_Address.beats_per_minute)); 
          //Serial.printf("ms to next of strip %i \n", true_ms_to_next_pos);    
        }
       }


       //Handling of Sine wave
       if(led_Address.type_of_wave == 3){
        if(led_Address.parameter_for_amplitude == 1) {
          hsv_led = rgb2hsv_approximate(rgb_led);
          hsv_led.hue = hsv_led.hue + (int)(led_Address.amplitude_range * beatsin8(led_Address.beats_per_minute)) ;
          rgb_led = hsv_led;
          //Serial.printf("hue of hsv_led %i \n", hsv_led.hue); 
          //Serial.printf("range value %f \n", led_Address.amplitude_range);    
          }
        if(led_Address.parameter_for_amplitude == 2){
          true_brightness += (int)(led_Address.amplitude_range*(beatsin8(led_Address.beats_per_minute)));
          rgb_led %= true_brightness;
          //Serial.printf("B of hsv_led %i \n", true_brightness);
        }
        if(led_Address.parameter_for_amplitude == 3){
          
          true_ms_to_next_pos = led_Address.ms_to_next_position + led_Address.amplitude_range * (beatsin8(led_Address.beats_per_minute)); 
          //Serial.printf("ms to next of strip %i \n", true_ms_to_next_pos);    
        }
       }


        
      
        for(int i = 0; i < led_Address.total_leds_on; i++){
          leds[(led_Address.initial_led_target + i) % NUM_LEDS] = rgb_led;
         }  
      FastLED.show();     
      delay(true_ms_to_next_pos);
      //Send parameters into target ESP32
      }
      
}
