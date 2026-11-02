// [ru] AlashPMS5003: самый простой пример. Пыль PM1.0 / PM2.5 / PM10 в мкг/м³ раз в секунду с датчика PMS5003 (или PMS7003, PMSA003).
//
//      Подключение: VCC датчика к 5 В, GND к GND, TX датчика к RX платы (Uno: пин 10, ESP32: GPIO16, ESP8266: D5), RX датчика к TX платы (Uno: пин 11, ESP32: GPIO17, ESP8266: D6).
//      Выводы SET и RESET оставьте свободными. Первые 30 секунд после включения данные нестабильны (раскручивается вентилятор).
// [kk] AlashPMS5003: ең қарапайым мысал. PMS5003 (немесе PMS7003, PMSA003) датчигінен шаң PM1.0 / PM2.5 / PM10 мкг/м³-пен секунд сайын.
//
//      Қосу: датчик VCC 5 В-қа, GND GND-ға, датчик TX тақта RX-ке (Uno: 10 пин, ESP32: GPIO16), датчик RX тақта TX-ке (Uno: 11 пин, ESP32: GPIO17).
//      SET және RESET шықпаларын бос қалдырыңыз. Қосқаннан кейінгі алғашқы 30 секундта деректер тұрақсыз (желдеткіш айналып жатады).
// [en] AlashPMS5003: the simplest example. Dust PM1.0 / PM2.5 / PM10 in ug/m3 once a second from a PMS5003 (or PMS7003, PMSA003) sensor.
//
//      Wiring: sensor VCC to 5 V, GND to GND, sensor TX to the board RX (Uno: pin 10, ESP32: GPIO16), sensor RX to the board TX (Uno: pin 11, ESP32: GPIO17).
//      Leave the SET and RESET pins free. The first 30 seconds after switch-on the data is unstable (the fan is spinning up).
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

void setup() {
  Serial.begin(115200);
#if defined(ARDUINO_ARCH_ESP32)
  Serial2.begin(9600);
#else
  sensorPort.begin(9600);                        // датчик всегда работает на 9600
#endif
  pms.begin();
}

void loop() {
  if (pms.read()) {                              // ждёт новый кадр до 2.5 с
    Serial.print(F("PM1.0=")); Serial.print(pms.pm1());
    Serial.print(F("  PM2.5=")); Serial.print(pms.pm25());
    Serial.print(F("  PM10=")); Serial.println(pms.pm10());
  } else {
    Serial.println(pms.errorText());             // что проверить
  }
}
