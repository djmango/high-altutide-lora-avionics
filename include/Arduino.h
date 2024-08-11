#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Print.h"

class __FlashStringHelper;
#define F(string_literal)                                                      \
  (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

class Serial_ : public Print {
public:
  void begin(unsigned long);
  void println(const char *);
  void println(const __FlashStringHelper *);
  void println(int);
  void println(unsigned int);
  void println(long);
  void println(unsigned long);
  void println(double);
  void println(void);
};

extern Serial_ Serial;

#endif
