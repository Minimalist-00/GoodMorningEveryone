#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#define LED_PIN     15
#define NUM_LEDS    30
#define BRIGHTNESS  3000
#define BUZZER_PIN 25
//音を鳴らす時間
#define C1 10
#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels
#define BEAT 200


CRGB leds[NUM_LEDS];
uint8_t hue = 0; // 色相値を初期化
float frequency = 0.1; // サインカーブの周波数を設定
float phaseShift = 0.0; // サインカーブの位相を設定
unsigned long lastUpdate = 0; // 前回のLED更新時間を記録
int loops = 0; //ループの回数を数える
float frequencies[] = {554.36, 440.00, 880.00, 659.25, 738.98, 587.33, 1100.66, 783.99, 987.76};
int numFrequencies = sizeof(frequencies) / sizeof(frequencies[0]);
int currentFrequencyIndex = 0; // 現在の周波数のインデックス

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
void playmusic(){
   ledcWriteTone(1,C1);
      delay(DELAYVAL);
    
}

int randomNumber = random(0,2);

void setup() {
  ledcSetup(1,12000, 8);
  ledcAttachPin(BUZZER_PIN,1);
   pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
   pixels.setBrightness(BRIGHTNESS);
   Serial.begin(115200);
   pixels.clear(); // Set all pixel colors to 'off'
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS); // NeoPixel LEDストリップを初期化
  FastLED.setBrightness(BRIGHTNESS); // 明るさを設定
  randomNumber = random(0, 2);
}
void playTone(float frequency, unsigned long duration) {
  unsigned long endTime = millis() + duration;
  
  while (millis() < endTime) {
    ledcWriteTone(1, frequency); // ブザーを鳴らす
    // LEDの制御も行う
    for (int i = 0; i < NUM_LEDS; i++) {
      float value = sin(2 * PI * (i * frequency + phaseShift)) * 127.5 + 127.5; // サインカーブを使って明るさを計算
      uint8_t saturation = 255; // 彩度を最大値に設定
      // 各LEDに個別の色を設定
      leds[i] = CHSV(hue + i * 2, saturation, value); // HSVカラーモデルで色を設定
    }
    FastLED.show(); // LEDに色を表示
    hue++; // 色相値を増加させる
    if (hue == 256) {
      hue = 0; // 色相値が255を超えたら0にリセット
    }
    phaseShift += 0.1; // サインカーブの位相を変化させる
    delayMicroseconds(BEAT); // ブザーの周期の半分だけ待機
  }
  ledcWrite(BUZZER_PIN, 0); // ブザーをOFFにする
}
void loop() {
  if(randomNumber == 0){
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
  playmusic();
}

else{
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= 5) { // 5ミリ秒ごとにLEDを更新
    lastUpdate = currentMillis;
    // ブザーを鳴らす
    float currentFrequency = frequencies[currentFrequencyIndex];
    playTone(currentFrequency, BEAT); // ブザーを周波数に応じて鳴らす
    // 次の周波数へのインデックスを更新
    currentFrequencyIndex = (currentFrequencyIndex + 1) % numFrequencies;
  }
  
}
}


