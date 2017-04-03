#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#include <main4ino/WebBot.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#define CLASS_PARAM_STREAM "ParamStream"


#ifndef UNIT_TEST

class ParamStream : public Stream {

#else // UNIT_TEST

#define uint8_t unsigned char
#define size_t int
class ParamStream {

#endif // UNIT_TEST

private:
  Buffer<MAX_JSON_STR_LENGTH> bytesReceived;

public:

  ParamStream() { }

  size_t write(uint8_t b) {
    append(b);
    return 1;
  }

  void append(uint8_t b) {
    log(CLASS_PARAM_STREAM, Debug, "Byte: ", (int)b);
    bytesReceived.append(b);
  }

  int available() {
    // Not supported.
    return 0;
  }
  int read() {
    // Not supported.
    return -1;
  }
  int peek() {
    // Not supported.
    return -1;
  }
  void flush() {
    bytesReceived.clear();
  }

};

#endif // PARAM_STREAM_INC
