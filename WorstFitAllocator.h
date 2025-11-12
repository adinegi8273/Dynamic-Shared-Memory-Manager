#ifndef WORST_FIT_ALLOCATOR_H
#define WORST_FIT_ALLOCATOR_H

#include "SharedMemory.h"

// Function prototypes for Worst Fit allocation
void worstFitAllocate(SharedMemory* shm, int pid, int size);
void worstFitFree(SharedMemory* shm, int pid, size_t offset);

#endif

