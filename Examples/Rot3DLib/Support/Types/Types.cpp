#include "Types.h"
#include <cassert>
#include <iostream>
#include <sstream>   // ostringstream
#include "NoneParameter.h"
#include "IntParameter.h"


CmdParameter *DefParameter_factory(DefParameter &item) {
  CmdParameter *p = nullptr;

  switch (item.param_type) {
  case NONE:             p = new NoneParameter(item);          break;
  case INTEGER:          p = new IntParameter(item);           break;

  default: {
      std::ostringstream msg;
      msg << "Error: Unhandled parameter type " << item.param_type;
      std::cout << msg.str() << std::endl;
    }
  }

  assert(p != nullptr);
  return p;
}
