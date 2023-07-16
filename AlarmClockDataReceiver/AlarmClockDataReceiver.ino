#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 4
#define NUM_LEDS 30
#define BRIGHTNESS 3000  //明るさ
#define BUZZER_PIN 25
#define BEAT 500      // 音を鳴らす間隔
#define FREQUENCY 10  // 周波数
#define LED_DELAY 50  //LEDの点滅間隔

uint8_t senderAddress[] = { 0x40, 0x91, 0x51, 0xBE, 0xFB, 0x50 };  //送信機のMACアドレス
esp_now_peer_info_t sender;                                        // ESP-Nowの送信機の情報
bool alarmState = false;

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

struct __attribute__((packed)) SENSOR_DATA {
  bool lightState;   // 明るさ〇〇以上かどうか
  bool switchState;  // スイッチの状態
  bool isLightData;  // 光センサデータであるかどうか
} sensorData;

// ブザーを鳴らす処理
void playmusic() {
  ledcWriteTone(1, FREQUENCY);
}

// LEDが光る処理
void blinkLED() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));  // (R, G, B)
  }
  pixels.show();
  delay(LED_DELAY);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
  delay(LED_DELAY);
}

// 受信コールバック
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
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
  Serial.println("isLightData: " + String(sensorData.isLightData));

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

  // 初期化設定
  ledcSetup(1, 12000, 8);
  ledcAttachPin(BUZZER_PIN, 1);
  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);  // 明るさの設定
  pixels.clear();
  pixels.show();

  // 受信コールバックの登録
  esp_now_register_recv_cb(onReceive);
}

void loop() {

  // 明るい + スイッチがONのとき LEDテープを光らせる
  if (alarmState) {
    blinkLED();
  } else {
    // LEDテープを消灯
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
  }
}
