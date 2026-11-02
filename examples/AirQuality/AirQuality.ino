// [ru] AlashPMS5003: качество воздуха. Индекс AQI (по методике US EPA) и оценка словами; светодиод на пине 13 горит, когда воздух вреден.
//      update() вызывается в loop() и не блокирует программу: можно параллельно делать другое. Подключение как в примере Basic.
// [kk] AlashPMS5003: ауа сапасы. AQI индексі (US EPA әдістемесі бойынша) және сөзбен баға; ауа зиянды болғанда 13 пиндегі жарықдиод жанады.
//      update() loop() ішінде шақырылады және бағдарламаны блоктамайды: қатар басқа іс істеуге болады. Қосу Basic мысалындағыдай.
// [en] AlashPMS5003: air quality. The AQI index (US EPA method) and a verbal rating; the LED on pin 13 lights when the air is unhealthy.
//      update() is called in loop() and does not block the program: you can do other things in parallel. Wiring as in the Basic example.
#include <AlashPMS5003.h>

#if defined(ARDUINO_ARCH_ESP32)
HardwareSerial &sensorPort = Serial2;            // ESP32: RX2 = GPIO16, TX2 = GPIO17
#elif defined(ARDUINO_ARCH_ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial sensorPort(14, 12);               // ESP8266: RX = GPIO14 (D5), TX = GPIO12 (D6); пины 9-11 заняты флеш-памятью
#else
#include <SoftwareSerial.h>
SoftwareSerial sensorPort(10, 11);               // Uno / Nano / Mega: RX = пин 10, TX = пин 11
#endif
AlashPMS5003 pms(sensorPort);
const uint8_t LED = 13;                          // на ESP32 укажите свой пин светодиода

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
  Serial2.begin(9600);
#else
  sensorPort.begin(9600);
#endif
  pms.begin();
  pms.setLanguage(AlashPMS5003::LANG_RU);        // LANG_KK или LANG_EN
}

void loop() {
  if (pms.update()) {                            // true, когда пришёл новый кадр (примерно раз в секунду)
    Serial.print(F("PM2.5 = ")); Serial.print(pms.pm25());
    Serial.print(F(" ug/m3   AQI = ")); Serial.print(pms.aqi());
    Serial.print(F("   ")); Serial.println(pms.qualityText());
    digitalWrite(LED, pms.quality() >= AlashPMS5003::Q_SENSITIVE ? HIGH : LOW);
  }
}
