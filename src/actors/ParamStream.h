#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#ifndef UNIT_TEST

class ParamStream : public Stream {

#else // UNIT_TEST

#define uint8_t unsigned char
#define size_t int
class ParamStream {

#endif // UNIT_TEST

private:
  Buffer<MAX_JSON_STR_LENGTH> bytesReceived;
  void append(uint8_t b);
public:
  ParamStream();
  size_t write(uint8_t);
  int available();
  int read();
  int peek();
  void flush();

};

#endif // PARAM_STREAM_INC
