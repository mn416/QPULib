#ifdef QPU_MODE

#ifndef _QPULIB_VIDEOCORE_H_
#define _QPULIB_VIDEOCORE_H_

namespace QPULib {

// Globals
extern int mailbox;
extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

}  // namespace QPULib

#endif  // _QPULIB_VIDEOCORE_H_
#endif  // QPU_MODE
