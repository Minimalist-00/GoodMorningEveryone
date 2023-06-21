#define LIGHT_SENSOR A4 // = GPIO32
const int interval = 1;
extern int lightData;

void setup() {
    Serial.begin(115200);
    pinMode(LIGHT_SENSOR, ANALOG);
}

void loop()
{
    lightData = analogRead(LIGHT_SENSOR);
    Serial.printf("明るさ: %d\n", lightData);

    delay(interval * 1000);
}
