#ifndef PARAMTYPE_H
#define PARAMTYPE_H

enum ParamType {
  NONE,             //> Parameter without a value, e.g. `-enabled`
  INTEGER,          //> Parameter takes any integer

  NUM_PARAMTYPES   // TODO assert valid type
};

#endif // PARAMTYPE_H
