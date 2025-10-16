#ifndef FIRST_FIT_ALLOCATOR_H
#define FIRST_FIT_ALLOCATOR_H

#include "SharedMemory.h"

// Function prototypes
void firstFitAllocate(SharedMemory* shm, int pid, int size);
void freeBlock(SharedMemory* shm, int pid, size_t offset);

#endif
