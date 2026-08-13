#ifndef FIRST_FIT_ALLOCATOR_H   // header guard start
#define FIRST_FIT_ALLOCATOR_H

#include "SharedMemory.h"       // needed for SharedMemory struct

// allocate 'size' bytes to process 'pid' using First Fit strategy
void firstFitAllocate(SharedMemory* shm, int pid, int size);

// free a block previously allocated to process 'pid' at given offset
void freeBlock(SharedMemory* shm, int pid, size_t offset);

#endif                           // header guard end