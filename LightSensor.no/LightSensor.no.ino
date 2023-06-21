#define LIGHT_SENSOR A4 // = GPIO32
const int interval = 1;

void setup() {
    Serial.begin(115200);
    pinMode(LIGHT_SENSOR, ANALOG);
}

void loop()
{
    int lightData = analogRead(LIGHT_SENSOR);
    Serial.printf("あかるさ: %d\n", lightData);

    delay(interval * 1000);
}
