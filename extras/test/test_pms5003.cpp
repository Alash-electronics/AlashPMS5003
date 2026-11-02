// Тесты AlashPMS5003 без железа: порт имитируется очередью байтов; кадры собираются независимо по описанию Plantower.
//   g++ -std=c++17 -Wall -fsanitize=address,undefined -I mock -I ../../src test_pms5003.cpp ../../src/AlashPMS5003.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "AlashPMS5003.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
typedef AlashPMS5003 P;

// Кадр PMS5003/7003/A003 (28) или PMS1003/3003 (20): 42 4D, длина, слова данных, сумма
static std::vector<uint8_t> frame(const std::vector<uint16_t> &words, bool goodSum = true) {
  std::vector<uint8_t> f = {0x42, 0x4D};
  uint16_t len = (uint16_t)(words.size() * 2 + 2);
  f.push_back(len >> 8); f.push_back((uint8_t)len);
  for (uint16_t w : words) { f.push_back(w >> 8); f.push_back((uint8_t)w); }
  uint16_t s = 0; for (uint8_t b : f) s += b;
  if (!goodSum) s ^= 0x0101;
  f.push_back(s >> 8); f.push_back((uint8_t)s);
  return f;
}
static std::vector<uint16_t> pms5003(uint16_t p1, uint16_t p25, uint16_t p10) {
  return {(uint16_t)(p1 - 1), (uint16_t)(p25 - 2), (uint16_t)(p10 - 3), p1, p25, p10, 1000, 500, 100, 20, 5, 1, 0x9100};
}
static void feed(Stream &s, const std::vector<uint8_t> &f) { for (uint8_t b : f) s.rx.push_back(b); }
static void restart() { g_ms += 100000; g_onDelay = nullptr; }

int main() {
  // 1) приём кадра PMS5003: значения, счётчики частиц, сумма
  { restart(); Stream s; P p(s); p.begin();
    feed(s, frame(pms5003(10, 25, 40)));
    CHECK(p.update() && p.hasData() && p.error() == P::ERR_OK);
    CHECK(p.pm1() == 10 && p.pm25() == 25 && p.pm10() == 40);
    CHECK(p.pm1Standard() == 9 && p.pm25Standard() == 23 && p.pm10Standard() == 37);
    CHECK(p.hasCounts() && p.particles(0) == 1000 && p.particles(1) == 500 && p.particles(5) == 1 && p.particles(6) == 0);
    CHECK(p.framesOk() == 1 && p.framesBad() == 0 && p.bytesSeen() == 32 && p.ageMs() < 5);
    const uint8_t v[] = {0x42, 0x4D, 0x00, 0x1C}; CHECK(P::checksum(v, 4) == 0x42 + 0x4D + 0x1C);
  }

  // 2) PMS1003 / PMS3003 (20 байт, 9 слов): без счётчиков
  { restart(); Stream s; P p(s); p.begin();
    feed(s, frame({4, 12, 20, 5, 13, 21, 0, 0, 0})); CHECK(p.update() && p.pm25() == 13 && !p.hasCounts() && p.particles(0) == 0);
  }

  // 3) мусор перед кадром, кадр по частям, два кадра подряд
  { restart(); Stream s; P p(s); p.begin();
    for (uint8_t b : {0x00, 0xFF, 0x42, 0x42, 0x13}) s.rx.push_back(b);               // мусор, в том числе «ложное начало» 42
    auto f = frame(pms5003(1, 2, 3));
    int half = 11; for (int i = 0; i < half; i++) s.rx.push_back(f[i]);
    CHECK(!p.update());                                                              // кадр ещё не весь
    for (size_t i = half; i < f.size(); i++) s.rx.push_back(f[i]);
    CHECK(p.update() && p.pm25() == 2);
    feed(s, frame(pms5003(7, 8, 9))); feed(s, frame(pms5003(17, 18, 19)));
    CHECK(p.update() && p.pm25() == 18 && p.framesOk() == 3);                        // два кадра подряд: берём последний
  }

  // 4) испорченная сумма: кадр отброшен, прежние данные целы, следующий кадр принят
  { restart(); Stream s; P p(s); p.begin();
    feed(s, frame(pms5003(5, 6, 7))); p.update();
    feed(s, frame(pms5003(50, 60, 70), false));
    CHECK(!p.update() && p.error() == P::ERR_CHECKSUM && p.framesBad() == 1 && p.pm25() == 6);
    feed(s, frame(pms5003(51, 61, 71)));
    CHECK(p.update() && p.error() == P::ERR_OK && p.pm25() == 61);
  }

  // 5) чужая длина (ответ на команду, помеха): пропускается без сбоя
  { restart(); Stream s; P p(s); p.begin();
    for (uint8_t b : {0x42, 0x4D, 0x00, 0x04, 0xE1, 0x00, 0x01, 0x74}) s.rx.push_back(b);   // подтверждение смены режима
    feed(s, frame(pms5003(3, 4, 5)));
    CHECK(p.update() && p.pm25() == 4 && p.framesBad() == 0);
    for (int i = 0; i < 3000; i++) s.rx.push_back((uint8_t)(i * 7 + 3));                    // много случайных байтов
    p.update();
    feed(s, frame(pms5003(8, 9, 10))); CHECK(p.update() && p.pm25() == 9);
  }

  // 6) команды побайтно, как в документации Plantower
  { restart(); Stream s; P p(s); p.begin();
    p.sleep(); CHECK(s.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73}) && p.isAsleep()); s.tx.clear();
    p.wake(); CHECK(s.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74}) && !p.isAsleep()); s.tx.clear();
    p.setPassive(true); CHECK(s.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70})); s.tx.clear();
    p.request(); CHECK(s.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71})); s.tx.clear();
    p.setPassive(false); CHECK(s.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71}));
  }

  // 7) прогрев 30 секунд после включения и сна
  { restart(); Stream s; P p(s); p.begin(); CHECK(!p.warmedUp());
    g_ms += 29000; CHECK(!p.warmedUp()); g_ms += 2000; CHECK(p.warmedUp());
    p.sleep(); CHECK(!p.warmedUp()); p.wake(); CHECK(!p.warmedUp()); g_ms += 31000; CHECK(p.warmedUp());
    p.setWarmupMs(5000); p.sleep(); p.wake(); g_ms += 5100; CHECK(p.warmedUp());
  }

  // 8) блокирующее чтение: активный режим, пассивный с запросом, тайм-ауты
  { restart(); Stream s; P p(s); p.begin(); int n = 0;
    g_onDelay = [&]() { if (++n == 10) feed(s, frame(pms5003(11, 22, 33))); };
    CHECK(p.read(2500) && p.pm25() == 22);
    Stream q; P pp(q); pp.begin(); pp.setPassive(true); q.tx.clear(); n = 0;
    g_onDelay = [&]() { if (!q.tx.empty() && n++ == 3) feed(q, frame(pms5003(1, 1, 1))); };       // отвечает только на запрос
    CHECK(pp.read(2500) && q.tx == std::vector<uint8_t>({0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71}));
    Stream e; P pe(e); pe.begin(); g_onDelay = nullptr;
    uint32_t t = g_ms; CHECK(!pe.read(1000) && pe.error() == P::ERR_NO_DATA && g_ms - t >= 1000);
    Stream g; P pg(g); pg.begin(); g_onDelay = [&]() { g.rx.push_back(0x55); };
    CHECK(!pg.read(500) && pg.error() == P::ERR_TIMEOUT);                                       // байты идут, кадра нет
  }

  // 9) индекс качества воздуха по таблицам US EPA
  { CHECK(P::aqiFor25(0) == 0 && P::aqiFor25(9) == 50 && P::aqiFor25(10) == 51 && P::aqiFor25(35) == 100);
    CHECK(P::aqiFor25(36) == 101 && P::aqiFor25(55) == 150 && P::aqiFor25(56) == 151 && P::aqiFor25(125) == 200);
    CHECK(P::aqiFor25(126) == 201 && P::aqiFor25(225) == 300 && P::aqiFor25(226) == 301 && P::aqiFor25(325) == 500 && P::aqiFor25(900) == 500);
    CHECK(P::aqiFor10(54) == 50 && P::aqiFor10(55) == 51 && P::aqiFor10(154) == 100 && P::aqiFor10(155) == 101 && P::aqiFor10(254) == 150);
    CHECK(P::aqiFor10(354) == 200 && P::aqiFor10(424) == 300 && P::aqiFor10(604) == 500);
    for (uint16_t c = 1; c < 400; c++) CHECK(P::aqiFor25(c) >= P::aqiFor25(c - 1));                  // не убывает
    CHECK(P::qualityFor(50) == P::Q_GOOD && P::qualityFor(51) == P::Q_MODERATE && P::qualityFor(151) == P::Q_UNHEALTHY && P::qualityFor(301) == P::Q_HAZARDOUS);
    restart(); Stream s; P p(s); p.begin(); feed(s, frame(pms5003(30, 40, 200))); p.update();   // PM2.5 = 40 даёт 112, PM10 = 200 даёт 125
    CHECK(p.aqiPm25() == P::aqiFor25(40) && p.aqiPm10() == P::aqiFor10(200) && p.aqi() == (p.aqiPm25() > p.aqiPm10() ? p.aqiPm25() : p.aqiPm10()));
    p.setLanguage(P::LANG_RU); const char *ru = (const char *)p.qualityText(); p.setLanguage(P::LANG_KK); const char *kk = (const char *)p.qualityText(); p.setLanguage(P::LANG_EN); const char *en = (const char *)p.qualityText();
    CHECK(strcmp(ru, kk) && strcmp(kk, en) && strlen(ru) > 3);
  }

  // 10) ошибки на трёх языках и диагностика
  { for (int e = P::ERR_NO_DATA; e <= P::ERR_TIMEOUT; e++) {
      restart(); Stream s; P p(s); p.begin();
      if (e == P::ERR_NO_DATA) p.read(100);
      else if (e == P::ERR_NO_FRAME) { s.rx.push_back(0x11); p.read(100); }
      else if (e == P::ERR_CHECKSUM) { feed(s, frame(pms5003(1, 2, 3), false)); p.update(); }
      else { g_onDelay = [&]() { s.rx.push_back(0x55); }; p.read(100); g_onDelay = nullptr; }
      if (e == P::ERR_NO_FRAME) { CHECK(p.error() == P::ERR_TIMEOUT); p.clearError(); CHECK(p.error() == P::ERR_OK); continue; }   // «байты есть, кадра нет» при read даёт TIMEOUT; NO_FRAME ставит diagnose
      CHECK(p.error() == e);
      p.setLanguage(P::LANG_RU); const char *ru = (const char *)p.errorText(); p.setLanguage(P::LANG_KK); const char *kk = (const char *)p.errorText(); p.setLanguage(P::LANG_EN); const char *en = (const char *)p.errorText();
      CHECK(strlen(ru) > 20 && strlen(kk) > 20 && strlen(en) > 20 && strcmp(ru, kk) && strcmp(kk, en) && strcmp(ru, en));
    }
    restart(); Stream s; P p(s); p.begin(); p.setLanguage(P::LANG_EN);
    int n = 0; g_onDelay = [&]() { if (++n % 100 == 0) feed(s, frame(pms5003(10, 25, 40))); };
    Print out; p.diagnose(out); g_onDelay = nullptr;
    CHECK(out.text.find("good frames: 5") != std::string::npos && out.text.find("PM2.5=25") != std::string::npos && out.text.find("AQI=") != std::string::npos && out.text.find("warming up") != std::string::npos);
    restart(); Stream e; P pe(e); pe.begin(); pe.setLanguage(P::LANG_KK); Print oe; pe.diagnose(oe); CHECK(pe.error() == P::ERR_NO_DATA && oe.text.find("Датчик үнсіз") != std::string::npos);
    restart(); Stream g; P pg(g); pg.begin(); pg.setLanguage(P::LANG_EN); g_onDelay = [&]() { g.rx.push_back(0x55); }; Print og; pg.diagnose(og); g_onDelay = nullptr;
    CHECK(pg.error() == P::ERR_NO_FRAME && og.text.find("no sensor frames") != std::string::npos);
    CHECK(strcmp((const char *)P(g).errorText(), "OK") == 0);
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
