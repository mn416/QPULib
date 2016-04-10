#ifdef QPU_MODE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "VideoCore/VideoCore.h"
#include "VideoCore/Mailbox.h"

// Globals
int mailbox = -1;
int numQPUUsers = 0;

// Get mailbox (open if not already opened)
int getMailbox()
{
  if (mailbox < 0) mailbox = mbox_open();
  return mailbox;
}

// Enable QPUs (if not already enabled)
void enableQPUs()
{
  int mb = getMailbox();
  if (numQPUUsers == 0) {
    int qpu_enabled = !qpu_enable(mb, 1);
    if (!qpu_enabled) {
      printf("Unable to enable QPUs. Check your firmware is latest.");
      exit(EXIT_FAILURE);
    }
  }
  numQPUUsers++;
}

// Disable QPUs
void disableQPUs()
{
  assert(numQPUUsers > 0);
  int mb = getMailbox();
  numQPUUsers--;
  if (numQPUUsers == 0) {
    qpu_enable(mb, 0);
  }
}


#endif
