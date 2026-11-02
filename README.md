# AlashPMS5003

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **датчиков пыли PMS5003 / PMS7003 / PMSA003** (Plantower, UART). Пыль PM1.0, PM2.5, PM10 в мкг/м³, **индекс качества воздуха AQI** и оценка словами. Приём без блокировки, проверка контрольной суммы, сон для экономии ресурса. Работает с любым портом. Только встроенное API Arduino. Сделана для уроков экологии и наборов Alash Electronics.

```cpp
#include <AlashPMS5003.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashPMS5003 pms(port);

void setup() { Serial.begin(115200); port.begin(9600); pms.begin(); }
void loop() {
  if (pms.update()) {                 // пришёл новый кадр
    Serial.println(pms.pm25());       // мкг/м³
  }
}
```

### Что умеет
- **`pm25()`, `pm10()`, `pm1()` в мкг/м³:** атмосферные значения и «стандартные» CF=1 (`pm25Standard()`).
- **Индекс AQI:** `aqi()` по методике US EPA (редакция 2024), `qualityText()` словами на трёх языках.
- **Без блокировки:** `update()` в `loop()`; `read()` ждёт новый кадр, если так удобнее.
- **Счётчики частиц:** `particles(0..5)` для PMS5003, PMS7003, PMSA003.
- **Проверка данных:** контрольная сумма, поиск начала кадра после мусора, подсчёт исправных и испорченных кадров.
- **Сон и пассивный режим:** `sleep()` / `wake()` продлевают жизнь вентилятора и лазера, `setPassive()` и `request()` дают данные по запросу.
- **Любой порт:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **Видно, что происходит:** `diagnose(Serial)` слушает порт и подсказывает (питание, TX / RX, скорость, SET) на трёх языках.
- Работает на **Arduino AVR, ESP32, ESP8266**.

### Подключение
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

Датчику нужно 5 В и около 100 мА, а логика у него 3.3 В, поэтому к ESP32 и ESP8266 он подключается напрямую. TX датчика идёт на RX платы (крест-накрест). Скорость порта всегда 9600. Вывод SET, прижатый к GND, усыпляет датчик.

### Чем отличается от других библиотек
Мы изучили PMS (fu-hsi, MIT), pms5003 (jbanaszczyk), PMSerial (avaldebe, MIT) и библиотеку AirGradient:
- PMS (fu-hsi) разбирает кадр, но не считает AQI, не сообщает причину молчания датчика и не предупреждает о прогреве; у нас AQI, `errorText()` и `warmedUp()`.
- У большинства разбор написан на конкретный порт или `Stream` без диагностики; у нас `diagnose()` слушает порт и объясняет, что проверить.
- Нет готовых сообщений на русском и казахском; у нас три языка.
- Сон и пассивный режим есть не везде; у нас с понятным примером экономии ресурса.

### Примеры
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

### Тесты
Без железа (с AddressSanitizer) на имитации порта: кадры PMS5003 (28 байт) и PMS1003 (20 байт), мусор до кадра и «ложное начало» 42, кадр по частям, два кадра подряд, испорченная сумма, чужая длина, команды сна, пробуждения, режимов и запроса побайтно по описанию Plantower, прогрев, блокирующее чтение в активном и пассивном режиме, тайм-ауты, AQI по таблицам EPA на границах диапазонов, ошибки и диагностика: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**PMS5003 / PMS7003 / PMSA003 шаң датчиктеріне** (Plantower, UART) арналған қарапайым кітапхана. Шаң PM1.0, PM2.5, PM10 мкг/м³-пен, **ауа сапасы AQI индексі** және сөзбен баға. Блоктамай қабылдау, бақылау сомасын тексеру, ресурсты үнемдеу үшін ұйқы. Кез келген портпен жұмыс істейді.

```cpp
#include <AlashPMS5003.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashPMS5003 pms(port);

void setup() { Serial.begin(115200); port.begin(9600); pms.begin(); }
void loop() {
  if (pms.update()) {                 // пришёл новый кадр
    Serial.println(pms.pm25());       // мкг/м³
  }
}
```

### Мүмкіндіктері
- **`pm25()`, `pm10()`, `pm1()` мкг/м³-пен:** атмосфералық мәндер және «стандартты» CF=1 (`pm25Standard()`).
- **AQI индексі:** US EPA әдістемесі бойынша `aqi()` (2024 редакциясы), үш тілде сөзбен `qualityText()`.
- **Блоктамай:** `loop()` ішінде `update()`; ыңғайлы болса `read()` жаңа кадрды күтеді.
- **Бөлшек санауыштары:** PMS5003, PMS7003, PMSA003 үшін `particles(0..5)`.
- **Деректерді тексеру:** бақылау сомасы, қоқыстан кейін кадр басын іздеу, дұрыс және бүлінген кадрларды санау.
- **Ұйқы және пассивті режим:** `sleep()` / `wake()` желдеткіш пен лазер өмірін ұзартады, `setPassive()` және `request()` деректерді сұраныс бойынша береді.
- **Кез келген порт:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **Не болып жатқаны көрінеді:** `diagnose(Serial)` портты тыңдап, үш тілде кеңес береді (қуат, TX / RX, жылдамдық, SET).
- **Arduino AVR, ESP32, ESP8266** тақталарында жұмыс істейді.

### Қосу
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

Датчикке 5 В және шамамен 100 мА керек, ал логикасы 3.3 В, сондықтан ESP32 және ESP8266-ға тікелей қосылады. Датчик TX тақта RX-ке (айқастырып). Порт жылдамдығы әрқашан 9600. GND-ға тартылған SET шықпасы датчикті ұйықтатады.

### Басқа кітапханалардан айырмашылығы
PMS (fu-hsi, MIT), pms5003 (jbanaszczyk), PMSerial (avaldebe, MIT) және AirGradient кітапханасын қарадық:
- PMS (fu-hsi) кадрды талдайды, бірақ AQI есептемейді, датчиктің үнсіздік себебін айтпайды және қызу туралы ескертпейді; бізде AQI, `errorText()` және `warmedUp()`.
- Көпшілігінде диагностикасыз нақты портқа немесе `Stream`-ге жазылған; бізде `diagnose()` портты тыңдап, нені тексеру керегін түсіндіреді.
- Орысша және қазақша дайын хабарлар жоқ; бізде үш тіл.
- Ұйқы және пассивті режим барлығында жоқ; бізде ресурсты үнемдеудің түсінікті мысалы.

### Мысалдар
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

### Тесттер
Құрылғысыз (AddressSanitizer-мен) порт имитациясында: PMS5003 (28 байт) және PMS1003 (20 байт) кадрлары, кадрға дейінгі қоқыс және 42 «жалған басы», бөліктеп келген кадр, қатар екі кадр, бүлінген сома, бөтен ұзындық, ұйқы, ояту, режимдер және сұраныс командалары Plantower сипаттамасы бойынша байтпа-байт, қызу, белсенді және пассивті режимде блоктайтын оқу, EPA кестелері бойынша AQI, қателер: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for the **PMS5003 / PMS7003 / PMSA003 dust sensors** (Plantower, UART). Dust PM1.0, PM2.5, PM10 in ug/m3, the **air quality index AQI** and a verbal rating. Non-blocking reception, checksum verification, sleep to save the sensor life. Works with any port. Only the built-in Arduino API. Made for ecology lessons and Alash Electronics kits.

```cpp
#include <AlashPMS5003.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashPMS5003 pms(port);

void setup() { Serial.begin(115200); port.begin(9600); pms.begin(); }
void loop() {
  if (pms.update()) {                 // пришёл новый кадр
    Serial.println(pms.pm25());       // мкг/м³
  }
}
```

### Features
- **`pm25()`, `pm10()`, `pm1()` in ug/m3:** atmospheric values and the "standard" CF=1 ones (`pm25Standard()`).
- **The AQI index:** `aqi()` by the US EPA method (2024 edition), `qualityText()` in words in three languages.
- **Non-blocking:** `update()` in `loop()`; `read()` waits for a new frame if that is more convenient.
- **Particle counts:** `particles(0..5)` for the PMS5003, PMS7003, PMSA003.
- **Data checks:** the checksum, finding the frame start after garbage, counting good and bad frames.
- **Sleep and passive mode:** `sleep()` / `wake()` extend the life of the fan and laser, `setPassive()` and `request()` give data on request.
- **Any port:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **See what happens:** `diagnose(Serial)` listens to the port and gives tips (power, TX / RX, speed, SET) in three languages.
- Works on **Arduino AVR, ESP32, ESP8266**.

### Wiring
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

The sensor needs 5 V and about 100 mA but has 3.3 V logic, so it connects directly to ESP32 and ESP8266. The sensor TX goes to the board RX (crossed). The port speed is always 9600. The SET pin pulled to GND puts the sensor to sleep.

### How it differs from other libraries
We studied PMS (fu-hsi, MIT), pms5003 (jbanaszczyk), PMSerial (avaldebe, MIT) and the AirGradient library:
- PMS (fu-hsi) parses the frame but does not compute the AQI, does not tell why the sensor is silent and does not warn about warm-up; ours has the AQI, `errorText()` and `warmedUp()`.
- Most are written for a particular port or `Stream` without diagnostics; ours has `diagnose()` that listens to the port and explains what to check.
- No ready messages in Russian and Kazakh; ours has three languages.
- Sleep and passive mode are not everywhere; ours has them with a clear example for saving the sensor life.

### Examples
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

### Tests
Without hardware (with AddressSanitizer) on a simulated port: PMS5003 (28 bytes) and PMS1003 (20 bytes) frames, garbage before a frame and a "false start" 42, a frame in parts, two frames in a row, a bad checksum, a foreign length, the sleep, wake, mode and request commands byte by byte per the Plantower description, warm-up, blocking reads in active and passive mode, timeouts, the AQI at the EPA table boundaries, errors and diagnostics: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
