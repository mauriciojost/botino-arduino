#ifndef COMMANDS_INC
#define COMMANDS_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Table.h>

#define CLASS_COMMANDS "CM"

#define NRO_COMMANDS 8
#define COMMAND_NAME_MAX_LENGTH 16
#define COMMAND_NAME_VALUE_MAX_LENGTH 64
#define COMMAND_NAME_VALUE_SEPARATOR ':'

enum CommandsProps {
  CommandsCmdNameValue0Prop = 0,
  CommandsCmdNameValue1Prop,
  CommandsCmdNameValue2Prop,
  CommandsCmdNameValue3Prop,
  CommandsCmdNameValue4Prop,
  CommandsCmdNameValue5Prop,
  CommandsCmdNameValue6Prop,
  CommandsCmdNameValue7Prop,
  CommandsPropsDelimiter // delimiter of the configuration states
};

/**
 * This actor stores the names of commands, and the commands themselves
 *
 * It is meant to be used to set a predefined set of commands that
 * the user often uses. If in Arduino HW, they can be invoked via a
 * button being pressed for instance (1 sec = 1st move, 2 sec = 2nd move, etc.).
 * It also allows to execute a specific command (cmd0) with a predefined frequency,
 * which allows for instance to have regular auto-updates of firmware.
 */
class Commands : public Actor {

private:
  const char *name;
  Metadata *md;
  Buffer *cmds[NRO_COMMANDS];
  Buffer *cmdNames[NRO_COMMANDS];

  void updateCmdNames() {
    for (int i = 0; i < NRO_COMMANDS; i++) {
      cmdNames[i]->load(cmds[i]->getBuffer());
      cmdNames[i]->replace(COMMAND_NAME_VALUE_SEPARATOR, 0);
    }
  }

public:
  Commands(const char *n) {
    name = n;
    md = new Metadata(n);
    md->getTiming()->setFreq("never");
    for (int i = 0; i < NRO_COMMANDS; i++) {
      cmds[i] = new Buffer(COMMAND_NAME_VALUE_MAX_LENGTH);
      cmds[i]->fill("Cmd %d%cmove Z.", i, COMMAND_NAME_VALUE_SEPARATOR);
      cmdNames[i] = new Buffer(COMMAND_NAME_MAX_LENGTH);
    }
    cmds[0]->fill("Done?%cack", COMMAND_NAME_VALUE_SEPARATOR);
    cmds[1]->fill("Time?%cmove Mc3.W2.Z.", COMMAND_NAME_VALUE_SEPARATOR);
    cmds[2]->fill("Dance?%cmove Fs.D3.D2.D1.D0.", COMMAND_NAME_VALUE_SEPARATOR);
    updateCmdNames();
  }

  const char *getCmdNameValue(int i) {
    return cmds[i]->getBuffer();
  }

  const char *getCmdValue(int i) {
    return cmds[i]->since(COMMAND_NAME_VALUE_SEPARATOR);
  }

  const char *getCmdName(int i) {
    return cmdNames[i]->getBuffer();
  }

  const char *getName() {
    return name;
  }

  Act act(Metadata *md) {
    return Act("");
  }

  CmdExecStatus command(Cmd *) {
    return NotFound;
  }

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex >= CommandsCmdNameValue0Prop && propIndex < (NRO_COMMANDS + CommandsCmdNameValue0Prop)) {
      int i = (int)propIndex - (int)CommandsCmdNameValue0Prop;
      setPropValue(setMode, targetValue, actualValue, cmds[i]);
    }
    if (setMode != GetValue) {
      updateCmdNames();
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return CommandsPropsDelimiter;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (CommandsCmdNameValue0Prop):
        return "cm0";
      case (CommandsCmdNameValue1Prop):
        return "cm1";
      case (CommandsCmdNameValue2Prop):
        return "cm2";
      case (CommandsCmdNameValue3Prop):
        return "cm3";
      case (CommandsCmdNameValue4Prop):
        return "cm4";
      case (CommandsCmdNameValue5Prop):
        return "cm5";
      case (CommandsCmdNameValue6Prop):
        return "cm6";
      case (CommandsCmdNameValue7Prop):
        return "cm7";
      default:
        return "";
    }
  }

  Metadata *getMetadata() {
    return md;
  }
};

#endif // COMMANDS_INC
