#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#define CLASS_PARAM_STREAM "PR"

#ifndef MAX_JSON_STR_LENGTH
#define MAX_JSON_STR_LENGTH 512
#endif // MAX_JSON_STR_LENGTH

#ifndef UNIT_TEST

class ParamStream : public Stream {

#else // UNIT_TEST

#define uint8_t unsigned char
#define size_t int
class ParamStream {

#endif // UNIT_TEST

private:
  Buffer<MAX_JSON_STR_LENGTH> bytesReceived;
  StaticJsonBuffer<MAX_JSON_STR_LENGTH> jsonBuffer;

public:
  ParamStream() {
  	bytesReceived.clear();
  }

  size_t write(uint8_t b) {
    append(b);
    return 1;
  }

  void append(uint8_t b) {
    bytesReceived.append(b);
  }

  void fill(const char* str) {
    bytesReceived.fill("%s", str);
  }

  int available() {
    return bytesReceived.getLength();
  }

  int read() {
    // Not supported.
    return -1;
  }

  int peek() {
    // Not supported.
    return -1;
  }

  JsonObject &parse() {
    log(CLASS_PARAM_STREAM, Debug, "Parsing: %s", bytesReceived.getBuffer());
    JsonObject &root = jsonBuffer.parseObject(bytesReceived.getUnsafeBuffer());
    return root;
  }
};

#endif // PARAM_STREAM_INC
