// [ru] AlashPMS5003: экономия ресурса датчика. Вентилятор и лазер живут около 8000 часов непрерывной работы. Здесь датчик спит 4 минуты,
//      просыпается, 30 секунд раскручивается, делает замер и снова засыпает: ресурс растёт примерно в 8 раз. Подключение как в примере Basic.
// [kk] AlashPMS5003: датчик ресурсын үнемдеу. Желдеткіш пен лазер үздіксіз жұмыста шамамен 8000 сағат қызмет етеді. Мұнда датчик 4 минут ұйықтайды,
//      оянады, 30 секунд айналады, өлшейді де қайта ұйықтайды: ресурс шамамен 8 есе артады. Қосу Basic мысалындағыдай.
// [en] AlashPMS5003: saving the sensor life. The fan and laser last about 8000 hours of continuous work. Here the sensor sleeps for 4 minutes,
//      wakes up, spins up for 30 seconds, takes a measurement and sleeps again: the life grows about 8 times. Wiring as in the Basic example.
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
const uint32_t SLEEP_MS = 4UL * 60UL * 1000UL;

void setup() {
  Serial.begin(115200);
#if defined(ARDUINO_ARCH_ESP32)
  Serial2.begin(9600);
#else
  sensorPort.begin(9600);
#endif
  pms.begin();
}

void loop() {
  pms.wake();
  delay(30000);                                  // раскрутка вентилятора: до этого данные недостоверны
  if (pms.read()) {
    Serial.print(F("PM2.5=")); Serial.print(pms.pm25());
    Serial.print(F("  AQI=")); Serial.println(pms.aqi());
  } else {
    Serial.println(pms.errorText());
  }
  pms.sleep();
  delay(SLEEP_MS);
}
