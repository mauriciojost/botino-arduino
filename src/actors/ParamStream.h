#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <main4ino/Buffer.h>
#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#define MAX_NRO_COMMANDS 4

struct Command {
  int confIndex;
  int propIndex;
  Buffer<MAX_VALUE_STR_LENGTH> newValue;
};

#ifndef UNIT_TEST
class ParamStream : public Stream {
#else // UNIT_TEST
#define uint8_t unsigned char
#define size_t int
class ParamStream {
#endif // UNIT_TEST

private:
  int commandsAvailable;
  Command commands[MAX_NRO_COMMANDS];
  int nroBytesReceived;
  Buffer<MAX_VALUE_STR_LENGTH> bytesReceived;

  void addCommand(int confIndex, int propIndex, const Value* newValue);
  void append(uint8_t b);

public:
  ParamStream();
  size_t write(uint8_t);
  int available();
  int read();
  int peek();
  void flush();

  Command* getCommands();
  int getNroCommandsAvailable();

};

#endif // PARAM_STREAM_INC
