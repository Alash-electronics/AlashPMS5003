// [ru] AlashPMS5003: датчик молчит или врёт? Скетч 3 секунды слушает порт, считает исправные кадры и ошибки суммы и подсказывает, что проверить.
//      Язык подсказок: LANG_RU, LANG_KK или LANG_EN. Подключение как в примере Basic.
// [kk] AlashPMS5003: датчик үнсіз бе немесе өтірік айта ма? Скетч портты 3 секунд тыңдайды, дұрыс кадрлар мен сома қателерін санайды және нені тексеру керегін айтады.
//      Кеңес тілі: LANG_RU, LANG_KK немесе LANG_EN. Қосу Basic мысалындағыдай.
// [en] AlashPMS5003: the sensor is silent or lies? The sketch listens to the port for 3 seconds, counts good frames and checksum errors and tells what to check.
//      Tips language: LANG_RU, LANG_KK or LANG_EN. Wiring as in the Basic example.
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
  sensorPort.begin(9600);
#endif
  pms.begin();
  pms.setLanguage(AlashPMS5003::LANG_RU);
  pms.diagnose(Serial);
}

void loop() {}
