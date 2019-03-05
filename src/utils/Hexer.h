#ifndef HEXER_INC
#define HEXER_INC

#include <log4ino/Log.h>
#include <main4ino/Misc.h>

#define CLASS_HEXER "HX"

/**
 * This class is a helper for hexadecimal transformations.
 */
class Hexer {

private:
  static uint8_t hexToValue(char v) {
    if (v >= '0' && v <= '9') {
      return v - '0';
    } else if (v >= 'a' && v <= 'f') {
      return v - 'a' + 10;
    } else if (v >= 'A' && v <= 'F') {
      return v - 'A' + 10;
    } else {
      return 0;
    }
  }

public:
  static void hexToByte(uint8_t *bytes, const char *inputHex, size_t inputLen) {
    if (inputLen % 2 != 0) {
      log(CLASS_HEXER, Error, "Bad hexa string (odd %d)", inputLen);
    } else {
      for (size_t i = 0; i < inputLen; i = i + 2) {
        bytes[i / 2] = hexToValue(inputHex[i]) * 16 + hexToValue(inputHex[i + 1]);
      }
    }
  }
};

#endif // HEXER_INC
