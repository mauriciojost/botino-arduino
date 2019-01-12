#ifndef MOVES_INC
#define MOVES_INC

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Clock.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/Misc.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Table.h>

#define CLASS_MOVES "MV"

#define NRO_MOVES 5
#define MOVE_NAME_MAX_LENGTH 16
#define MOVE_NAME_VALUE_MAX_LENGTH 64
#define MOVE_NAME_VALUE_SEPARATOR ':'

enum MovesProps {
  MovesMoveNameValue0Prop = 0,
  MovesMoveNameValue1Prop,
  MovesMoveNameValue2Prop,
  MovesMoveNameValue3Prop,
  MovesMoveNameValue4Prop,
  MovesPropsDelimiter // delimiter of the configuration states
};

/**
 * This actor stores the names of moves, and the moves themselves
 *
 * It is meant to be used to set a predefined set of moves that
 * the user often uses. If in Arduino HW, they can be invoked via a
 * button being pressed for instance (1 sec = 1st move, 2 sec = 2nd move, etc.).
 */
class Moves : public Actor {

private:
  const char *name;
  Metadata *md;
  Buffer *moves[NRO_MOVES];
  Buffer *moveNames[NRO_MOVES];

  void updateMoveNames() {
    for (int i = 0; i < NRO_MOVES; i++) {
      moveNames[i]->load(moves[i]);
      moveNames[i]->replace(MOVE_NAME_VALUE_SEPARATOR, 0);
    }
  }

public:
  Moves(const char *n) {
    name = n;
    md = new Metadata(n);
    for (int i = 0; i < NRO_MOVES; i++) {
      moves[i] = new Buffer(MOVE_NAME_VALUE_MAX_LENGTH);
      moves[i]->fill("Move %d%cmove Z.", i, MOVE_NAME_VALUE_SEPARATOR);
      moveNames[i] = new Buffer(MOVE_NAME_MAX_LENGTH);
    }
    moves[0]->fill("Done!%cack", MOVE_NAME_VALUE_SEPARATOR);
    moves[1]->fill("Time%cmove Mc3.W3.Z.", MOVE_NAME_VALUE_SEPARATOR);
    updateMoveNames();
  }

  const char *getMoveNameValue(int i) {
    return moves[i]->getBuffer();
  }

  const char *getMoveValue(int i) {
    return moves[i]->since(MOVE_NAME_VALUE_SEPARATOR);
  }

  const char *getMoveName(int i) {
    return moveNames[i]->getBuffer();
  }

  const char *getName() {
    return name;
  }

  void act() {}

  void getSetPropValue(int propIndex, GetSetMode setMode, const Value *targetValue, Value *actualValue) {
    if (propIndex >= MovesMoveNameValue0Prop && propIndex < (NRO_MOVES + MovesMoveNameValue0Prop)) {
      int i = (int)propIndex - (int)MovesMoveNameValue0Prop;
      setPropValue(setMode, targetValue, actualValue, moves[i]);
    }
    if (setMode != GetValue) {
      updateMoveNames();
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return MovesPropsDelimiter;
  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (MovesMoveNameValue0Prop):
        return "mv0";
      case (MovesMoveNameValue1Prop):
        return "mv1";
      case (MovesMoveNameValue2Prop):
        return "mv2";
      case (MovesMoveNameValue3Prop):
        return "mv3";
      case (MovesMoveNameValue4Prop):
        return "mv4";
      default:
        return "";
    }
  }

  void getInfo(int infoIndex, Buffer *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }
};

#endif // MOVES_INC
