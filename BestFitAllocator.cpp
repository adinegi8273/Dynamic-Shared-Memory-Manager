#include "BestFitAllocator.h"
#include <iostream>
#include <climits>

using namespace std;

// Allocate memory using Best Fit algorithm
void bestFitAllocate(SharedMemory* shm, int pid, int size) {
    lockMemory(shm);

    int bestFitIndex = -1;
    size_t bestFitSize = SIZE_MAX; // Initialize to maximum possible size
    
    // Find the smallest free block that can accommodate the request
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (shm->blockArray[i].isFree) {
            // Calculate available space from this block to the end of memory
            size_t availableSpace = shm->header->total_size - shm->blockArray[i].offset;
            
            if (availableSpace >= size && availableSpace < bestFitSize) {
                bestFitIndex = i;
                bestFitSize = availableSpace;
            }
        }
    }
    
    // If we found a suitable block, allocate it
    if (bestFitIndex != -1) {
        shm->blockArray[bestFitIndex].pid = pid;
        shm->blockArray[bestFitIndex].size = size;
        shm->blockArray[bestFitIndex].isFree = false;
        shm->blockArray[bestFitIndex].offset = shm->header->free_offset;

        shm->header->used_size += size;
        shm->header->free_offset += size;

        cout << "Process " << pid << " allocated " << size 
             << " bytes at offset " << shm->blockArray[bestFitIndex].offset 
             << " (Best Fit)" << endl;

        unlockMemory(shm);
        return;
    }

    cout << "Process " << pid << ": Best Fit allocation failed for " << size << " bytes!" << endl;
    unlockMemory(shm);
}

// Free a previously allocated block using Best Fit strategy
void bestFitFree(SharedMemory* shm, int pid, size_t offset) {
    lockMemory(shm);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!shm->blockArray[i].isFree && shm->blockArray[i].pid == pid &&
            shm->blockArray[i].offset == offset) {

            // Free the block
            shm->blockArray[i].isFree = true;
            shm->blockArray[i].pid = -1;

            // Update used size
            shm->header->used_size -= shm->blockArray[i].size;

            cout << "Process " << pid << " freed block at offset " << offset 
                 << " (Best Fit)" << endl;
            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Best Fit free failed at offset " << offset << endl;
    unlockMemory(shm);
}
