uint64_t chipid;  

void setup() {
    Serial.begin(115200);
}

const int pinLight = A0;

void loop()
{
    int sensorValue = analogRead(pinLight);    //the light sensor is attached to analog 0
    Serial.printf("Light Strength = %d\n",sensorValue);

    delay(2000);
}
