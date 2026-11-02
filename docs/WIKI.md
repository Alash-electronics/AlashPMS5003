# AlashPMS5003: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **датчиков пыли PMS5003 / PMS7003 / PMSA003** (Plantower, UART). Пыль PM1.0, PM2.5, PM10 в мкг/м³, **индекс качества воздуха AQI** и оценка словами. Приём без блокировки, проверка контрольной суммы, сон для экономии ресурса. Работает с любым портом. Только встроенное API Arduino. Сделана для уроков экологии и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashPMS5003** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

Датчику нужно 5 В и около 100 мА, а логика у него 3.3 В, поэтому к ESP32 и ESP8266 он подключается напрямую. TX датчика идёт на RX платы (крест-накрест). Скорость порта всегда 9600. Вывод SET, прижатый к GND, усыпляет датчик.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `AlashPMS5003(port)` | Датчик на порту Serial2 / SoftwareSerial / Serial1 |
| `begin()` | Сбросить состояние; порт открывайте сами на 9600 |
| `update()` | В loop(): разбирает байты; true, если пришёл новый кадр |
| `read(timeoutMs)` | Ждёт новый кадр (блокирует) |
| `pm1() / pm25() / pm10()` | Пыль, мкг/м³ (атмосферные) |
| `pm25Standard() / particles(i)` | Значение CF=1 / число частиц в 0.1 л |
| `aqi() / quality() / qualityText()` | Индекс US EPA и оценка словами |
| `sleep() / wake() / warmedUp()` | Сон, пробуждение, прошло ли 30 с |
| `setPassive(on) / request()` | Пассивный режим и запрос кадра |
| `framesOk() / framesBad() / ageMs()` | Счётчики кадров и возраст данных |
| `error() / errorText() / diagnose(Serial)` | Ошибка, сообщение, самопроверка на ru/kk/en |

### Если что-то не работает
**Датчик молчит (ERR_NO_DATA)**

Проверьте питание 5 В (вентилятор должен шуметь), TX датчика на RX платы (крест-накрест), общий GND и скорость 9600. Вывод SET, прижатый к GND, держит датчик в режиме сна. Запустите пример `Diagnose`.

**Данные скачут или кадры с ошибкой суммы**

Помехи: укоротите провода, добавьте общий GND, питайте датчик от нормального источника (USB-порта ноутбука может не хватать на пусковой ток вентилятора). Счётчик `framesBad()` покажет, как часто это случается.

**Первые показания странные**

Это нормально: первые 30 секунд после включения или `wake()` вентилятор раскручивается, и данные недостоверны. Проверяйте `warmedUp()` или ждите 30 секунд.

**На Uno не печатается в монитор порта или данные теряются**

Аппаратный `Serial` занят USB. Подключите датчик через `SoftwareSerial` (пины 10, 11) или на Mega к `Serial1`. На скорости 9600 SoftwareSerial работает надёжно.

**Показания сильно отличаются от другого прибора**

Датчики пыли дают разные числа в зависимости от влажности, типа пыли и калибровки. Показания растут при влажности выше 80 % (капли тумана принимаются за частицы). Для сравнения используйте `pm25Standard()` (CF=1) или усредняйте за 10 минут.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

---

## Қазақша

**PMS5003 / PMS7003 / PMSA003 шаң датчиктеріне** (Plantower, UART) арналған қарапайым кітапхана. Шаң PM1.0, PM2.5, PM10 мкг/м³-пен, **ауа сапасы AQI индексі** және сөзбен баға. Блоктамай қабылдау, бақылау сомасын тексеру, ресурсты үнемдеу үшін ұйқы. Кез келген портпен жұмыс істейді.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashPMS5003** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

Датчикке 5 В және шамамен 100 мА керек, ал логикасы 3.3 В, сондықтан ESP32 және ESP8266-ға тікелей қосылады. Датчик TX тақта RX-ке (айқастырып). Порт жылдамдығы әрқашан 9600. GND-ға тартылған SET шықпасы датчикті ұйықтатады.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `AlashPMS5003(port)` | Порттағы датчик |
| `begin()` | Күйді тастау |
| `update()` | loop() ішінде |
| `read(timeoutMs)` | Жаңа кадрды күтеді |
| `pm1() / pm25() / pm10()` | Шаң, мкг/м³ |
| `pm25Standard() / particles(i)` | CF=1 мәні / бөлшектер саны |
| `aqi() / quality() / qualityText()` | US EPA индексі және баға |
| `sleep() / wake() / warmedUp()` | Ұйқы, ояту |
| `setPassive(on) / request()` | Пассивті режим |
| `framesOk() / framesBad() / ageMs()` | Кадр есептегіштері |
| `error() / errorText() / diagnose(Serial)` | Қате, хабарлама, өзін-өзі тексеру |

### Бірдеңе жұмыс істемесе
**Датчик үнсіз (ERR_NO_DATA)**

5 В қуатын (желдеткіш шулауы керек), датчик TX-ін тақта RX-ке (айқастырып), ортақ GND және 9600 жылдамдығын тексеріңіз. GND-ға тартылған SET шықпасы датчикті ұйқы режимінде ұстайды. `Diagnose` мысалын іске қосыңыз.

**Деректер секіреді немесе кадрларда сома қатесі**

Кедергі: сымдарды қысқартыңыз, ортақ GND қосыңыз, датчикті қалыпты көзден қоректендіріңіз (ноутбук USB-порты желдеткіштің іске қосу тогына жетпеуі мүмкін). `framesBad()` есептегіші қаншалықты жиі болатынын көрсетеді.

**Алғашқы көрсеткіштер біртүрлі**

Бұл қалыпты: қосқаннан немесе `wake()` кейінгі алғашқы 30 секундта желдеткіш айналып жатады, деректер сенімсіз. `warmedUp()` тексеріңіз немесе 30 секунд күтіңіз.

**Uno-да порт мониторына басылмайды немесе деректер жоғалады**

Аппараттық `Serial` USB-мен бос емес. Датчикті `SoftwareSerial` (10, 11 пиндері) арқылы немесе Mega-да `Serial1`-ге қосыңыз. 9600 жылдамдығында SoftwareSerial сенімді жұмыс істейді.

**Көрсеткіштер басқа құралдан көп өзгешеленеді**

Шаң датчиктері ылғалдылыққа, шаң түріне және калибрлеуге байланысты әртүрлі сан береді. Ылғалдылық 80 %-дан жоғары болғанда көрсеткіштер өседі (тұман тамшылары бөлшек деп қабылданады). Салыстыру үшін `pm25Standard()` (CF=1) қолданыңыз немесе 10 минутқа орташалаңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

---

## English

A simple library for the **PMS5003 / PMS7003 / PMSA003 dust sensors** (Plantower, UART). Dust PM1.0, PM2.5, PM10 in ug/m3, the **air quality index AQI** and a verbal rating. Non-blocking reception, checksum verification, sleep to save the sensor life. Works with any port. Only the built-in Arduino API. Made for ecology lessons and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashPMS5003** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| PMS5003 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |
| SET, RESET | свободны | свободны |

The sensor needs 5 V and about 100 mA but has 3.3 V logic, so it connects directly to ESP32 and ESP8266. The sensor TX goes to the board RX (crossed). The port speed is always 9600. The SET pin pulled to GND puts the sensor to sleep.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `AlashPMS5003(port)` | A sensor on the port Serial2 / SoftwareSerial / Serial1 |
| `begin()` | Reset the state; open the port yourself at 9600 |
| `update()` | In loop(): parses bytes; true if a new frame arrived |
| `read(timeoutMs)` | Waits for a new frame (blocks) |
| `pm1() / pm25() / pm10()` | Dust, ug/m3 (atmospheric) |
| `pm25Standard() / particles(i)` | The CF=1 value / particle count per 0.1 L |
| `aqi() / quality() / qualityText()` | The US EPA index and a verbal rating |
| `sleep() / wake() / warmedUp()` | Sleep, wake, have 30 s passed |
| `setPassive(on) / request()` | Passive mode and requesting a frame |
| `framesOk() / framesBad() / ageMs()` | Frame counters and the data age |
| `error() / errorText() / diagnose(Serial)` | Error, message, self-check in ru/kk/en |

### If something does not work
**The sensor is silent (ERR_NO_DATA)**

Check the 5 V power (the fan must run), the sensor TX to the board RX (crossed), a common GND and the speed 9600. The SET pin pulled to GND keeps the sensor asleep. Run the `Diagnose` example.

**Data jumps or frames have a bad checksum**

Noise: shorten the wires, add a common GND, power the sensor from a proper supply (a laptop USB port may not cover the fan start-up current). The `framesBad()` counter shows how often it happens.

**The first readings look strange**

This is normal: in the first 30 seconds after switch-on or `wake()` the fan spins up and the data is unreliable. Check `warmedUp()` or wait 30 seconds.

**On Uno nothing prints to the serial monitor or data is lost**

The hardware `Serial` is busy with USB. Connect the sensor through `SoftwareSerial` (pins 10, 11) or to `Serial1` on a Mega. At 9600 SoftwareSerial is reliable.

**The readings differ a lot from another instrument**

Dust sensors give different numbers depending on humidity, the kind of dust and calibration. The readings grow above 80 % humidity (fog droplets are counted as particles). For comparison use `pm25Standard()` (CF=1) or average over 10 minutes.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `AirQuality`, `LowPower`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashPMS5003
