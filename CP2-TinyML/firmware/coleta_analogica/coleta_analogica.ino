#include <Arduino.h>
// Captura REAL do sensor analógico ligado ao GPIO39; CSV enviado via Serial.
static constexpr uint8_t SENSOR_PIN = 39;
static constexpr uint32_t PERIODO_US = 10000; // 100 Hz, como o projeto EI
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(SENSOR_PIN, INPUT);
  delay(1000);
  Serial.println("timestamp_ms,adc_bruto,valor_sensor");
}
void loop() {
  static uint32_t proximo = micros();
  if ((int32_t)(micros() - proximo) >= 0) {
    proximo += PERIODO_US;
    int bruto = analogRead(SENSOR_PIN);
    int valor = constrain(map(bruto,0,4095,0,1023),0,1023);
    Serial.printf("%lu,%d,%d\n",(unsigned long)millis(),bruto,valor);
  }
}
