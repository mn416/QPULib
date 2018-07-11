/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:22 +0200
 */
#ifndef PARAMTYPE_H
#define PARAMTYPE_H

enum ParamType {
  NONE,             //> Parameter without a value, e.g. `-enabled`
  INTEGER,          //> Parameter takes any integer

  NUM_PARAMTYPES   // TODO assert valid type
};

#endif // PARAMTYPE_H
