// AlashPMS5003: датчики пыли PM1.0 / PM2.5 / PM10 серии Plantower (PMS5003, PMS7003, PMSA003, PMS1003, PMS3003) по UART:
// приём кадров без блокировки, проверка контрольной суммы, индекс качества воздуха AQI, активный и пассивный режим, сон.
// Работает с любым портом (Serial, Serial2, SoftwareSerial). Без зависимостей. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashPMS5003
#ifndef ALASH_PMS5003_H
#define ALASH_PMS5003_H

#include <Arduino.h>

class AlashPMS5003 {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_DATA,        // по порту не пришло ни одного байта
    ERR_NO_FRAME,       // байты идут, но кадра датчика (42 4D) нет: не та скорость или чужое устройство
    ERR_CHECKSUM,       // кадр пришёл, но сумма не сошлась: помехи или длинные провода
    ERR_TIMEOUT         // свежего кадра не дождались за отведённое время
  };
  enum Quality : uint8_t { Q_GOOD = 0, Q_MODERATE, Q_SENSITIVE, Q_UNHEALTHY, Q_VERY_UNHEALTHY, Q_HAZARDOUS };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  explicit AlashPMS5003(Stream &port) : _s(&port) {}

  void begin();                                // сбросить состояние и ждать кадры (порт открывайте сами: Serial1.begin(9600))
  bool update();                               // вызывайте в loop(): разбирает пришедшие байты; true, если принят новый исправный кадр
  bool read(uint32_t timeoutMs = 2500);        // подождать новый кадр (блокирует); в пассивном режиме сам делает запрос

  // ---- Концентрации, мкг/м³ (атмосферные значения) --------------------------------------
  uint16_t pm1() const { return _pm[3]; }
  uint16_t pm25() const { return _pm[4]; }
  uint16_t pm10() const { return _pm[5]; }
  // «Стандартные» значения CF=1 (по заводской калибровке, обычно для сравнения с другими датчиками)
  uint16_t pm1Standard() const { return _pm[0]; }
  uint16_t pm25Standard() const { return _pm[1]; }
  uint16_t pm10Standard() const { return _pm[2]; }
  // Число частиц крупнее 0.3 / 0.5 / 1.0 / 2.5 / 5.0 / 10 мкм в 0.1 л воздуха (только PMS5003, PMS7003, PMSA003; у PMS1003 и PMS3003 нули)
  uint16_t particles(uint8_t index) const { return index < 6 ? _cnt[index] : 0; }
  bool hasCounts() const { return _hasCounts; }

  // ---- Качество воздуха ------------------------------------------------------------------------
  uint16_t aqi() const;                        // индекс US EPA (редакция 2024) по худшему из PM2.5 и PM10, 0..500
  uint16_t aqiPm25() const { return aqiFor25(_pm[4]); }
  uint16_t aqiPm10() const { return aqiFor10(_pm[5]); }
  Quality quality() const { return qualityFor(aqi()); }
  const __FlashStringHelper *qualityText();    // «хороший», «умеренный»... на выбранном языке
  static Quality qualityFor(uint16_t aqi);
  static uint16_t aqiFor25(uint16_t pm25);
  static uint16_t aqiFor10(uint16_t pm10);

  // ---- Режимы ---------------------------------------------------------------------------------------
  void setPassive(bool passive);               // пассивный режим: данные только по запросу (request() или read())
  void request();                              // запросить кадр (пассивный режим)
  void sleep();                                // выключить вентилятор и лазер: ресурс датчика около 8000 часов, сон его экономит
  void wake();                                 // включить; стабильные данные через 30 секунд
  bool warmedUp() const { return !_asleep && (uint32_t)(millis() - _wakeAt) >= _warmupMs; }
  void setWarmupMs(uint32_t ms) { _warmupMs = ms; }
  bool isAsleep() const { return _asleep; }

  // ---- Состояние -------------------------------------------------------------------------------------
  bool hasData() const { return _have; }
  uint32_t ageMs() const { return _have ? (uint32_t)(millis() - _frameAt) : 0xFFFFFFFFUL; }   // сколько мс назад пришёл последний кадр
  uint32_t framesOk() const { return _ok; }
  uint32_t framesBad() const { return _bad; }
  uint32_t bytesSeen() const { return _bytes; }
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);                   // слушает порт 3 секунды, считает кадры, подсказывает

  static uint16_t checksum(const uint8_t *data, uint8_t len);   // сумма байт (16 бит)

 private:
  void command(uint8_t cmd, uint8_t dataLow);
  void parse(const uint8_t *payload, uint8_t words);
  Stream *_s;
  uint8_t _buf[32]; uint8_t _i = 0; uint16_t _len = 0;
  uint16_t _pm[6] = {0}, _cnt[6] = {0};
  bool _hasCounts = false, _have = false, _passive = false, _asleep = false;
  uint32_t _frameAt = 0, _wakeAt = 0, _warmupMs = 30000UL, _ok = 0, _bad = 0, _bytes = 0;
  Error _err = ERR_OK; Language _lang = LANG_RU;
};

#endif
