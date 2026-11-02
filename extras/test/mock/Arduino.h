// Имитация Arduino.h и порта Stream для тестов AlashPMS5003: очередь входящих байтов, журнал исходящих, время.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <deque>
#include <vector>
#include <functional>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#define PROGMEM
inline uint32_t g_ms = 1000;
inline std::function<void()> g_onDelay;           // «датчик» шлёт байты, пока тест ждёт
inline uint32_t millis() { return g_ms; }
inline void delay(uint32_t ms) { g_ms += ms; if (g_onDelay) g_onDelay(); }
class Print {
 public:
  std::string text;
  void put(const char *s) { printf("%s", s); text += s; }
  void print(const char *s) { put(s); }
  void print(const __FlashStringHelper *s) { put((const char *)s); }
  void println(const __FlashStringHelper *s) { put((const char *)s); put("\n"); }
  void print(long v) { char b[24]; snprintf(b, sizeof b, "%ld", v); put(b); }
  void println(long v) { print(v); put("\n"); }
  void println(const char *s = "") { put(s); put("\n"); }
  virtual size_t write(const uint8_t *, size_t n) { return n; }
  virtual ~Print() {}
};
class Stream : public Print {
 public:
  std::deque<uint8_t> rx; std::vector<uint8_t> tx;
  int available() { return (int)rx.size(); }
  int read() { if (rx.empty()) return -1; uint8_t b = rx.front(); rx.pop_front(); return b; }
  size_t write(const uint8_t *d, size_t n) override { tx.insert(tx.end(), d, d + n); return n; }
};
