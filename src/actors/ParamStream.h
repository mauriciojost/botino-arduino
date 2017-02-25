#ifndef PARAM_STREAM_INC
#define PARAM_STREAM_INC

#include <log4ino/Log.h>
#include <Stream.h>

#define MAX_NRO_COMMANDS 4

struct Command {
  int configurableIndex;
  int propertyIndex;
  int newValue;
};

class ParamStream : public Stream {

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
