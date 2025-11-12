#ifndef NEXT_FIT_ALLOCATOR_H
#define NEXT_FIT_ALLOCATOR_H

#include "SharedMemory.h"

// Function prototypes for Next Fit allocation
void nextFitAllocate(SharedMemory* shm, int pid, int size);
void nextFitFree(SharedMemory* shm, int pid, size_t offset);

#endif

