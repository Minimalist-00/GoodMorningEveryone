#include <Adafruit_NeoPixel.h>
#define LED_PIN     12
#define NUM_LEDS    30
#define BRIGHTNESS  3000
#define BUZZER_PIN 25
//音を鳴らす時間
#define BEAT 500
#define C1 10
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
void playmusic(){
   ledcWriteTone(1,C1);
      delay(BEAT);
}
 
#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels
 
int loops = 0; //ループの回数を数える
 
void setup() {
  ledcSetup(1,12000, 8);
  ledcAttachPin(BUZZER_PIN,1);

  playmusic();
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
 
  pixels.clear(); // Set all pixel colors to 'off'
}
 
void loop() {
  for (uint8_t i = 0; i < NUM_LEDS  ; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // (R, G, B)
  }
  
  // ピクセルの状態を更新
  pixels.show();
  

  delay(DELAYVAL);
  
  // すべてのLEDを消灯
  for (uint8_t i = 0; i < NUM_LEDS  ; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
  

  delay(DELAYVAL);
}
