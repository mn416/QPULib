#ifndef _QPULIB_VIDEOCORE_H_
#define _QPULIB_VIDEOCORE_H_

#include "qpulib_config.h"
#ifdef QPULIB_QPU_MODE

namespace QPULib {

// Globals
extern int mailbox;
extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

}  // namespace QPULib

#endif  // QPULIB_QPU_MODE
#endif  // _QPULIB_VIDEOCORE_H_
