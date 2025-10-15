#include "FirstFitAllocator.h"
#include <iostream>

using namespace std;

// Allocate memory using First Fit algorithm
void firstFitAllocate(SharedMemory* shm, int pid, int size) {
    lockMemory(shm);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (shm->blockArray[i].isFree && shm->header->free_offset + size <= shm->header->total_size) {
            // Allocate memory
            shm->blockArray[i].pid = pid;
            shm->blockArray[i].size = size;
            shm->blockArray[i].isFree = false;
            shm->blockArray[i].offset = shm->header->free_offset;

            shm->header->used_size += size;
            shm->header->free_offset += size;

            cout << "Process " << pid << " allocated " << size 
                 << " bytes at offset " << shm->blockArray[i].offset << endl;

            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Allocation failed for " << size << " bytes!" << endl;
    unlockMemory(shm);
}

// Free a previously allocated block
void freeBlock(SharedMemory* shm, int pid, size_t offset) {
    lockMemory(shm);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!shm->blockArray[i].isFree && shm->blockArray[i].pid == pid &&
            shm->blockArray[i].offset == offset) {

            // Free the block
            shm->blockArray[i].isFree = true;
            shm->blockArray[i].pid = -1;

            // Update used size (simple approach, does not compact memory)
            shm->header->used_size -= shm->blockArray[i].size;

            cout << "Process " << pid << " freed block at offset " << offset << endl;
            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Free failed at offset " << offset << endl;
    unlockMemory(shm);
}
