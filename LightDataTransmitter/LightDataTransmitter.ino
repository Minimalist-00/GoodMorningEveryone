#include <esp_now.h>
#include <WiFi.h>

#define LIGHT_SENSOR A4 // 光センサのピン番号, =GPIO32
const int interval = 1;
int col = 0; // 光センサからのデータ
uint8_t slaveAddress[] = { 0x40, 0x91, 0x51, 0xBD, 0xDC, 0x8C }; //受信側のmacアドレス

esp_now_peer_info_t slave; // ESP-Nowのスレーブデバイスの情報

// 送信コールバック
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr); // 最後に送信したパケットのMACアドレス
  Serial.print("Last Packet Send Status: ");
  // 送信成功か失敗かを表示
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

// 受信コールバック
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("Last Packet Recv from: %s\n", macStr); // 最後に受信したパケットのMACアドレス
  Serial.printf("Last Packet Recv Data(%d): ", data_len); // 受信データとその長さ
  for ( int i = 0 ; i < data_len ; i++ ) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

void setup() {
  Serial.begin(115200); // シリアル通信の開始
  pinMode(LIGHT_SENSOR, ANALOG); // 光センサのピン設定

  WiFi.mode(WIFI_STA); // WiFiをステーションモードに設定
  WiFi.disconnect(); // 初期化時にWiFiを切断

  // ESP-Nowの初期化
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success"); // 初期化成功
  } else {
    Serial.println("ESPNow Init Failed"); // 初期化失敗
    ESP.restart();
  }

  // スレーブの追加
  esp_err_t addStatus = esp_now_add_peer(&slave);
  if (addStatus == ESP_OK) {
    // ペアリング成功
    Serial.println("Pair success");
  }

  // 送受信コールバックの登録
  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onReceive);
}

void EspNowSend() {
  uint16_t data[1] = {col}; // 送信データ
  esp_err_t result = esp_now_send(slaveAddress, data, sizeof(data)); // データの送信

  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success"); // 送信成功
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    Serial.println("ESPNOW not Init."); // ESP-Now未初期化
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument"); // 不正な引数
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error"); // 内部エラー
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM"); // メモリ不足
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found."); // ピアが見つからない
  } else {
    Serial.println("Not sure what happened"); // 不明なエラー
  }
  Serial.println(col); // 送信データの表示
}

void loop() {
  col = analogRead(LIGHT_SENSOR); // 光センサからデータを読み取る
  Serial.printf("明るさ: %d\n", col); // 取得したデータをシリアルモニターに出力
  EspNowSend(); // データの送信

  delay(interval * 1000); // 指定したインターバルでデータを送信
}
