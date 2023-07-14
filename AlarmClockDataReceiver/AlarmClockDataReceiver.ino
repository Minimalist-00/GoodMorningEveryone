#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 12
#define NUM_LEDS 30
#define BRIGHTNESS 3000
#define BUZZER_PIN 25
#define BEAT 500     // 音を鳴らす間隔
#define Hz 10        // 周波数
#define DELAYVAL 50  //LEDの点滅間隔

uint8_t senderAddress[] = { 0x40, 0x91, 0x51, 0xBE, 0xFB, 0x50 };  //送信機のMACアドレス
esp_now_peer_info_t sender;                                        // ESP-Nowの送信機の情報

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
volatile bool receivedFlag = false; // 受信フラグ デフォルトではfalse

void playmusic() {
  ledcWriteTone(1, Hz);
  delay(BEAT);
}

// LEDが光る処理
void blinkLED() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));  // (R, G, B)
  }

  // ピクセルの状態を更新
  pixels.show();
  delay(DELAYVAL);

  // すべてのLEDを消灯
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
  delay(DELAYVAL);
}


// 受信コールバック
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("Last Packet Recv from: %s\n", macStr);  // 最後に受信したパケットのMACアドレス

  // 受信データの解析
  uint16_t brightnessData = 0;
  brightnessData = (uint16_t)data[0] << 8 | data[1];  // 2つのuint8_t型を組み合わせてuint16_t型へ戻す

  // 受信したデータの表示
  Serial.printf("Now brightness: %d\n", brightnessData);

    unsigned long blink_start_time = millis();    // 現在の時間を取得
    while (millis() - blink_start_time < 1000) {  // 1秒間繰り返す
      playmusic();                                //ブザーを鳴らす
      blinkLED();                                 //LEDテープの点滅
    }
  // receivedFlag = true;
}

void setup() {
  Serial.begin(115200);  // シリアル通信の開始

  WiFi.mode(WIFI_STA);  // WiFiをステーションモードに設定
  WiFi.disconnect();    // 初期化時にWiFiを切断

  // ESP-Nowの初期化
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");  // 初期化成功
  } else {
    Serial.println("ESPNow Init Failed");  // 初期化失敗
    ESP.restart();
  }

  // 送信機の情報を設定
  memcpy(sender.peer_addr, senderAddress, 6);
  sender.channel = 0;
  sender.encrypt = false;

  // 送信機をペアリングリストに追加
  if (esp_now_add_peer(&sender) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;

    ledcSetup(1, 12000, 8);
    ledcAttachPin(BUZZER_PIN, 1);

    pixels.begin();
    pixels.setBrightness(BRIGHTNESS);  //明るさの設定
    Serial.begin(115200);

    pixels.clear();
  }

  // 受信コールバックの登録
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // if (receivedFlag) {

  //   receivedFlag = false;
  // }
}
