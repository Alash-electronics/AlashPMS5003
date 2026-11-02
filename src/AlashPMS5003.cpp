// MIT License. Alash Electronics.
#include "AlashPMS5003.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

uint16_t AlashPMS5003::checksum(const uint8_t *d, uint8_t n) {
  uint16_t s = 0;
  while (n--) s += *d++;
  return s;
}

void AlashPMS5003::begin() {
  _i = 0; _len = 0; _have = false; _err = ERR_OK; _ok = _bad = _bytes = 0;
  _wakeAt = millis();
  while (_s->available()) _s->read();
}

// Команда датчику: 42 4D, код, данные (2 байта), сумма (2 байта)
void AlashPMS5003::command(uint8_t cmd, uint8_t dataLow) {
  uint8_t c[7] = {0x42, 0x4D, cmd, 0x00, dataLow, 0, 0};
  uint16_t s = checksum(c, 5);
  c[5] = s >> 8; c[6] = (uint8_t)s;
  _s->write(c, 7);
}

void AlashPMS5003::setPassive(bool passive) { command(0xE1, passive ? 0x00 : 0x01); _passive = passive; }
void AlashPMS5003::request() { command(0xE2, 0x00); }
void AlashPMS5003::sleep() { command(0xE4, 0x00); _asleep = true; }
void AlashPMS5003::wake() { command(0xE4, 0x01); _asleep = false; _wakeAt = millis(); }

// Кадр: 42 4D, длина (2 байта, число байт после неё), данные (слова по 2 байта), сумма (2 байта).
// У PMS5003 / 7003 / A003 длина 28 (13 слов данных), у PMS1003 / 3003 длина 20 (9 слов).
bool AlashPMS5003::update() {
  bool got = false;
  while (_s->available()) {
    int c = _s->read();
    if (c < 0) break;
    _bytes++;
    uint8_t b = (uint8_t)c;
    if (_i == 0) { if (b == 0x42) _buf[_i++] = b; continue; }
    if (_i == 1) { if (b == 0x4D) _buf[_i++] = b; else _i = (b == 0x42) ? 1 : 0; continue; }
    _buf[_i++] = b;
    if (_i == 4) {
      _len = ((uint16_t)_buf[2] << 8) | _buf[3];
      if (_len != 20 && _len != 28) { _i = 0; continue; }          // другой датчик, помеха или ответ на команду: ищем заново
    }
    if (_i >= 4 && _i == _len + 4) {
      uint16_t calc = checksum(_buf, _i - 2);
      uint16_t got16 = ((uint16_t)_buf[_i - 2] << 8) | _buf[_i - 1];
      if (calc == got16) {
        parse(_buf + 4, (uint8_t)((_len - 2) / 2));
        _ok++; _err = ERR_OK; _have = true; _frameAt = millis(); got = true;
      } else { _bad++; _err = ERR_CHECKSUM; }
      _i = 0;
    }
  }
  return got;
}

void AlashPMS5003::parse(const uint8_t *p, uint8_t words) {
  auto w = [&](uint8_t k) -> uint16_t { return ((uint16_t)p[2 * k] << 8) | p[2 * k + 1]; };
  for (uint8_t k = 0; k < 6; k++) _pm[k] = w(k);
  _hasCounts = words >= 12;                    // 13 слов: ещё 6 счётчиков частиц и служебное слово
  for (uint8_t k = 0; k < 6; k++) _cnt[k] = _hasCounts ? w(6 + k) : 0;
}

bool AlashPMS5003::read(uint32_t timeoutMs) {
  uint32_t t0 = millis(); bool requested = false;
  if (_passive) { while (_s->available()) _s->read(); _i = 0; request(); requested = true; }
  (void)requested;
  uint32_t before = _bytes;
  while ((uint32_t)(millis() - t0) < timeoutMs) {
    if (update()) return true;
    delay(5);
  }
  if (_bytes == before) _err = ERR_NO_DATA;
  else if (_err != ERR_CHECKSUM) _err = ERR_TIMEOUT;
  return false;
}

// US EPA AQI, редакция 2024 для PM2.5; для PM10 прежняя. Линейная интерполяция внутри диапазона.
static uint16_t interp(uint16_t c, const uint16_t *lo, const uint16_t *hi, const uint16_t *ilo, const uint16_t *ihi, uint8_t n) {
  for (uint8_t i = 0; i < n; i++)
    if (c <= hi[i]) return (uint16_t)(((uint32_t)(ihi[i] - ilo[i]) * (c - lo[i]) + (hi[i] - lo[i]) / 2) / (hi[i] - lo[i]) + ilo[i]);
  return 500;
}

uint16_t AlashPMS5003::aqiFor25(uint16_t c) {
  static const uint16_t lo[] = {0, 10, 36, 56, 126, 226}, hi[] = {9, 35, 55, 125, 225, 325};   // целые мкг/м³ (датчик отдаёт целые)
  static const uint16_t ilo[] = {0, 51, 101, 151, 201, 301}, ihi[] = {50, 100, 150, 200, 300, 500};
  return interp(c, lo, hi, ilo, ihi, 6);
}

uint16_t AlashPMS5003::aqiFor10(uint16_t c) {
  static const uint16_t lo[] = {0, 55, 155, 255, 355, 425}, hi[] = {54, 154, 254, 354, 424, 604};
  static const uint16_t ilo[] = {0, 51, 101, 151, 201, 301}, ihi[] = {50, 100, 150, 200, 300, 500};
  return interp(c, lo, hi, ilo, ihi, 6);
}

uint16_t AlashPMS5003::aqi() const {
  uint16_t a = aqiFor25(_pm[4]), b = aqiFor10(_pm[5]);
  return a > b ? a : b;
}

AlashPMS5003::Quality AlashPMS5003::qualityFor(uint16_t a) {
  if (a <= 50) return Q_GOOD;
  if (a <= 100) return Q_MODERATE;
  if (a <= 150) return Q_SENSITIVE;
  if (a <= 200) return Q_UNHEALTHY;
  if (a <= 300) return Q_VERY_UNHEALTHY;
  return Q_HAZARDOUS;
}

const __FlashStringHelper *AlashPMS5003::qualityText() {
  switch (quality()) {
    case Q_GOOD: return T("хороший", "жақсы", "good");
    case Q_MODERATE: return T("умеренный", "орташа", "moderate");
    case Q_SENSITIVE: return T("вреден для чувствительных групп", "сезімтал топтар үшін зиянды", "unhealthy for sensitive groups");
    case Q_UNHEALTHY: return T("вредный", "зиянды", "unhealthy");
    case Q_VERY_UNHEALTHY: return T("очень вредный", "өте зиянды", "very unhealthy");
    case Q_HAZARDOUS: return T("опасный", "қауіпті", "hazardous");
  }
  return F("");
}

const __FlashStringHelper *AlashPMS5003::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_DATA:
      return T("Датчик молчит. Проверьте питание 5 В (вентилятор должен шуметь), провод TX датчика к RX платы (крест-накрест), общий GND, скорость порта 9600 и что вывод SET не прижат к GND (он включает сон).",
               "Датчик үнсіз. 5 В қуатын (желдеткіш шулауы керек), датчик TX сымын тақта RX-ке (айқастырып), ортақ GND, порт жылдамдығы 9600 екенін және SET шықпасы GND-ға тартылмағанын (ол ұйқыны қосады) тексеріңіз.",
               "The sensor is silent. Check the 5 V power (the fan must run), the sensor TX wire to the board RX (crossed), a common GND, the port speed 9600 and that the SET pin is not pulled to GND (that puts it to sleep).");
    case ERR_NO_FRAME:
      return T("Байты приходят, но кадров датчика (42 4D) нет. Скорость порта должна быть 9600, а к порту должен быть подключён именно датчик PMS.",
               "Байттар келеді, бірақ датчик кадрлары (42 4D) жоқ. Порт жылдамдығы 9600 болуы керек, портқа дәл PMS датчигі қосылуы керек.",
               "Bytes arrive but there are no sensor frames (42 4D). The port speed must be 9600 and the port must be connected to a PMS sensor.");
    case ERR_CHECKSUM:
      return T("Кадр пришёл, но контрольная сумма не сошлась: помехи. Укоротите провода, добавьте общий GND, не запитывайте датчик от слабого USB-порта.",
               "Кадр келді, бірақ бақылау сомасы сәйкес келмеді: кедергі. Сымдарды қысқартыңыз, ортақ GND қосыңыз, датчикті әлсіз USB-портынан қоректендірмеңіз.",
               "A frame arrived but the checksum did not match: noise. Shorten the wires, add a common GND, do not power the sensor from a weak USB port.");
    case ERR_TIMEOUT:
      return T("Свежий кадр не пришёл за отведённое время. После wake() датчику нужно до 30 секунд на раскрутку вентилятора; в пассивном режиме сделайте request().",
               "Белгіленген уақытта жаңа кадр келмеді. wake() кейін желдеткішті айналдыруға датчикке 30 секундқа дейін керек; пассивті режимде request() жасаңыз.",
               "No fresh frame arrived in time. After wake() the sensor needs up to 30 seconds to spin the fan up; in passive mode call request().");
  }
  return F("");
}

void AlashPMS5003::diagnose(Print &out) {
  out.println(T("AlashPMS5003: диагностика", "AlashPMS5003: диагностика", "AlashPMS5003: diagnostics"));
  out.println(T("Слушаю порт 3 секунды...", "Портты 3 секунд тыңдаймын...", "Listening to the port for 3 seconds..."));
  if (_passive) request();
  uint32_t t0 = millis(), b0 = _bytes, ok0 = _ok, bad0 = _bad;
  while ((uint32_t)(millis() - t0) < 3000) { update(); delay(5); }
  out.print(T("Байт принято: ", "Қабылданған байт: ", "Bytes received: ")); out.print((long)(_bytes - b0));
  out.print(T("   исправных кадров: ", "   дұрыс кадрлар: ", "   good frames: ")); out.print((long)(_ok - ok0));
  out.print(T("   с ошибкой суммы: ", "   қате сомалы: ", "   with a bad sum: ")); out.println((long)(_bad - bad0));
  if (_bytes == b0) _err = ERR_NO_DATA;
  else if (_ok == ok0 && _bad == bad0) _err = ERR_NO_FRAME;
  else if (_ok == ok0) _err = ERR_CHECKSUM;
  else _err = ERR_OK;
  if (_err != ERR_OK) out.println(errorText());
  if (_have) {
    out.print(F("PM1.0=")); out.print((long)pm1()); out.print(F("  PM2.5=")); out.print((long)pm25()); out.print(F("  PM10=")); out.println((long)pm10());
    out.print(F("AQI=")); out.print((long)aqi()); out.print(F(" (")); out.print(qualityText()); out.println(F(")"));
    if (!warmedUp()) out.println(T("Датчик ещё прогревается: стабильные данные через 30 секунд после включения.", "Датчик әлі қызып жатыр: қосқаннан кейін 30 секундтан соң тұрақты деректер.", "The sensor is still warming up: stable data 30 seconds after switch-on."));
  }
  out.println(T("Подсказки: датчику нужен 5 В и около 100 мА (вентилятор), логика 3.3 В, поэтому к ESP32 и ESP8266 он подключается напрямую. У Uno используйте SoftwareSerial или Serial1 на Mega/Leonardo: аппаратный Serial занят USB. Не закрывайте входное и выходное отверстия, держите датчик вертикально. Результат первых 30 секунд после включения считайте недостоверным.",
                "Кеңестер: датчикке 5 В және шамамен 100 мА (желдеткіш) керек, логика 3.3 В, сондықтан ол ESP32 және ESP8266-ға тікелей қосылады. Uno-да SoftwareSerial немесе Mega/Leonardo-да Serial1 қолданыңыз: аппараттық Serial USB-мен бос емес. Кіріс және шығыс тесіктерін жаппаңыз, датчикті тік ұстаңыз. Қосқаннан кейінгі алғашқы 30 секундтың нәтижесін сенімсіз деп санаңыз.",
                "Tips: the sensor needs 5 V and about 100 mA (the fan) with 3.3 V logic, so it connects directly to ESP32 and ESP8266. On Uno use SoftwareSerial or Serial1 on Mega/Leonardo: the hardware Serial is busy with USB. Do not block the inlet and outlet holes, keep the sensor upright. Treat the first 30 seconds after switch-on as unreliable."));
}
