#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#ifndef UNIT_TEST
#include <Stream.h>
#endif // UNIT_TEST

#define MAX_NRO_COMMANDS 4

struct Command {
  int configurableIndex;
  int propertyIndex;
  int newValue;
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
  int bytesReceived;

  int configurableIndex;
  int propertyIndex;
  int newValue;

  void addCommand();

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
