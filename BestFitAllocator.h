#ifndef BEST_FIT_ALLOCATOR_H
#define BEST_FIT_ALLOCATOR_H

#include "SharedMemory.h"

// Function prototypes for Best Fit allocation
void bestFitAllocate(SharedMemory* shm, int pid, int size);
void bestFitFree(SharedMemory* shm, int pid, size_t offset);

#endif
