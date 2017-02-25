#include <actors/ParamStream.h>

#define CLASS "ParamStream"

ParamStream::ParamStream() {
  commandsAvailable = 0;
  bytesReceived = 0;
}

size_t ParamStream::write(uint8_t b) {
  switch(bytesReceived) {
    case 0:
      if (b == 'c') { bytesReceived++; } else { bytesReceived = 0; }
      break;
    case 1:
      configurableIndex = b - '0';
      bytesReceived++;
      log(CLASS, Debug, "c", configurableIndex);
      break;
    case 2:
      if (b == 'p') { bytesReceived++; } else { bytesReceived = 0; }
      break;
    case 3:
      propertyIndex = b - '0';
      bytesReceived++;
      log(CLASS, Debug, "p", propertyIndex);
      break;
    case 4:
      if (b == '=') { bytesReceived++; } else { bytesReceived = 0; }
      break;
    case 5:
      newValue = b - '0';
      bytesReceived = 0;
      addCommand();
      log(CLASS, Debug, "=", newValue);
      break;
  }
	return 0;
}

void ParamStream::addCommand() {
  if (commandsAvailable < MAX_NRO_COMMANDS) {
    commands[commandsAvailable].configurableIndex = configurableIndex;
    commands[commandsAvailable].propertyIndex = propertyIndex;
    commands[commandsAvailable].newValue = newValue;
    commandsAvailable++;
  }

}

int ParamStream::available() {
  // Not supported.
	return 0;
}
int ParamStream::read() {
  // Not supported.
	return -1;
}
int ParamStream::peek() {
  // Not supported.
	return -1;
}
void ParamStream::flush() {
  commandsAvailable = 0;
  bytesReceived = 0;
}

int ParamStream::getNroCommandsAvailable() {
  return commandsAvailable;
}

Command* ParamStream::getCommands() {
  return commands;
}
