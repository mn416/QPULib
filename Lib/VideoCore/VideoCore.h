#ifdef QPU_MODE

#ifndef _QPULIB_VIDEOCORE_H_
#define _QPULIB_VIDEOCORE_H_

namespace qpulib {

// Globals
extern int mailbox;
extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

}  // namespace qpulib

#endif  // _QPULIB_VIDEOCORE_H_
#endif  // QPU_MODE
