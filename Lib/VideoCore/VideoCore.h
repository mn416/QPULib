#ifdef QPU_MODE

#ifndef _VIDEOCORE_H_
#define _VIDEOCORE_H_

// Globals
extern int mailbox;
extern int numQPUUsers;

// Operations
int getMailbox();
void enableQPUs();
void disableQPUs();

#endif
#endif
