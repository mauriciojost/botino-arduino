#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#define CLASS_PARAM_STREAM "PR"

#ifndef MAX_STREAM_LENGTH
#define MAX_STREAM_LENGTH 512
#endif // MAX_STREAM_LENGTH

#ifndef UNIT_TEST

/**
 * Stream implementation.
 */
class ParamStream : public Stream {

#else // UNIT_TEST

#define uint8_t unsigned char
#define size_t int
class ParamStream {

#endif // UNIT_TEST

private:
  Buffer<MAX_STREAM_LENGTH> bytesReceived;

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

  void fill(const char *str) {
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

  const char *content() {
    return bytesReceived.getBuffer();
  }
};

#endif // PARAM_STREAM_INC
