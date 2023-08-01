#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

#define LED_PIN 15
#define NUM_LEDS 30
#define BRIGHTNESS 100  //明るさ
#define BUZZER_PIN 25
#define FREQUENCY 10  // 周波数
#define LED_DELAY 50  //LEDの点滅間隔
#define BEAT 200      // 音を鳴らす間隔

uint8_t senderAddress[] = { 0x40, 0x91, 0x51, 0xBE, 0xFB, 0x50 };  //送信機のMACアドレス
esp_now_peer_info_t sender;                                        // ESP-Nowの送信機の情報
bool alarmState = false;

// int randomNumber = random(0, 2);
int randomNumber = 2;

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

struct __attribute__((packed)) SENSOR_DATA {
  bool lightState;   // 明るさ〇〇以上かどうか
  bool switchState;  // スイッチの状態
  bool isLightData;  // 光センサデータであるかどうか
} sensorData;

/*
#define NTP_SERVER "pool.ntp.org"
#define TIME_ZONE 9  // 日本のタイムゾーン
#define DST 0        // 夏時間 (日本では通常は0)
const char* ssid = "aterm-3663ca-g";     // ネットワーク名
const char* password = "8cc6fa2fe57bd";  // ネットワークパスワード
*/

// LEDの制御用変数と関数の定義
CRGB leds[NUM_LEDS];
uint8_t hue = 0;               // 色相値を初期化
float frequency = 0.1;         // サインカーブの周波数を設定
float phaseShift = 0.0;        // サインカーブの位相を設定
unsigned long lastUpdate = 0;  // 前回のLED更新時間を記録
int loops = 0;                 //ループの回数を数える
float frequencies[] = { 554.36, 440.00, 880.00, 659.25, 738.98, 587.33, 1100.66, 783.99, 987.76 };
int numFrequencies = sizeof(frequencies) / sizeof(frequencies[0]);
int currentFrequencyIndex = 0;  // 現在の周波数のインデックス

// ブザーを鳴らす処理
void playmusic() {
  ledcWriteTone(1, FREQUENCY);
  delay(LED_DELAY);
}

// LEDが光る処理
void blinkLED() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));  // (R, G, B)
  }
  // ピクセルの状態を更新
  pixels.show();
  delay(LED_DELAY);
  // すべてのLEDを消灯
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }

  pixels.show();
  delay(LED_DELAY);
}

void playTone(float frequency, unsigned long duration) {
  unsigned long endTime = millis() + duration;

  while (millis() < endTime) {
    ledcWriteTone(1, frequency);
    for (int i = 0; i < NUM_LEDS; i++) {
      float value = sin(2 * PI * (i * frequency + phaseShift)) * 127.5 + 127.5;
      uint8_t saturation = 255;
      leds[i] = CHSV(hue + i * 2, saturation, value);
    }
    FastLED.show();
    hue++;
    if (hue == 256) {
      hue = 0;
    }
    phaseShift += 0.1;
    delayMicroseconds(BEAT);
  }
  ledcWrite(BUZZER_PIN, 0);
}

// 受信コールバック
void onReceive(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.printf("Last Packet Recv from: %s\n", macStr);  // 最後に受信したパケットのMACアドレス
  memcpy(&sensorData, data, data_len);

  // 受信データの確認
  Serial.println("Received now");
  Serial.println("lightState: " + String(sensorData.lightState));
  Serial.println("switchState: " + String(sensorData.switchState));
  // Serial.println("isLightData: " + String(sensorData.isLightData));

  if (sensorData.lightState && sensorData.switchState) {
    alarmState = true;
    playmusic();  // ブザーを鳴らす
  } else {
    alarmState = false;
    ledcWriteTone(1, 0);  // Stop buzzer
  }
}

void setup() {
  Serial.begin(115200);  // シリアル通信の開始

  WiFi.mode(WIFI_STA);  // WiFiをステーションモードに設定
  WiFi.disconnect();    // 初期化時にWiFiを切断

  // ESP-Nowの初期化
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow 初期化成功");
  } else {
    Serial.println("ESPNow 初期化失敗");
    ESP.restart();  //再起動
  }

  // 送信機の情報を設定
  memcpy(sender.peer_addr, senderAddress, 6);
  sender.channel = 0;
  sender.encrypt = false;

  // 送信機をペアリングリストに追加
  esp_err_t addStatus = esp_now_add_peer(&sender);
  if (addStatus == ESP_OK) {  // ペアリング成功
    Serial.println("ペアリング成功");
  } else {  // ペアリング失敗
    Serial.print("ペアリング失敗, error code: ");
    Serial.println(addStatus);
  }

  /*
  // WiFi設定

  // WiFiに接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  // WiFiに接続するまで待つ
    delay(1000);
    Serial.println("Connecting ...");
  }
  Serial.println("Successfully connected to WiFi");
  */

  // 初期化設定
  ledcSetup(1, 12000, 8);
  ledcAttachPin(BUZZER_PIN, 1);
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);  // 明るさの設定
  pixels.clear();
  pixels.show();
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // NeoPixel LEDストリップを初期化
  FastLED.setBrightness(BRIGHTNESS);                   // 明るさを設定

  // 受信コールバックの登録
  esp_now_register_recv_cb(onReceive);

  /*
  // NTPサーバから時間を取得し、ESP32の時計を設定
  configTime(TIME_ZONE * 3600, DST * 0, NTP_SERVER);
  delay(1000);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.println(&timeinfo, "時間設定: %B %d %Y %H:%M:%S");
  */
}

void loop() {
  /*
  // 時間の制約
  struct tm timeinfo;
  getLocalTime(&timeinfo);  // 時間の指定
  if (timeinfo.tm_hour < 5 || timeinfo.tm_hour >= 8) {
  }
  */
  // 明るい + スイッチがONのとき LEDテープを光らせる
  if (alarmState) {
    if (randomNumber == 0) {
      blinkLED();
      playmusic();
    } else {
      unsigned long currentMillis = millis();
      if (currentMillis - lastUpdate >= 10) {  //LEDを更新
        lastUpdate = currentMillis;
        // ブザーを鳴らす
        float currentFrequency = frequencies[currentFrequencyIndex];
        playTone(currentFrequency, BEAT);  // ブザーを周波数に応じて鳴らす
        // 次の周波数へのインデックスを更新
        currentFrequencyIndex = (currentFrequencyIndex + 1) % numFrequencies;
      }
    }
  } else {
    // LEDテープを消灯
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
  }
}
