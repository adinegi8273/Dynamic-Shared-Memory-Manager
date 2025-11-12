#include "WorstFitAllocator.h"
#include <iostream>

using namespace std;

// Allocate memory using Worst Fit algorithm
void worstFitAllocate(SharedMemory* shm, int pid, int size) {
    lockMemory(shm);

    int worstFitIndex = -1;
    size_t worstFitSize = 0;

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (shm->blockArray[i].isFree) {
            size_t availableSpace = shm->header->total_size - shm->blockArray[i].offset;

            if (availableSpace >= static_cast<size_t>(size) && availableSpace > worstFitSize) {
                worstFitIndex = i;
                worstFitSize = availableSpace;
            }
        }
    }

    if (worstFitIndex != -1) {
        shm->blockArray[worstFitIndex].pid = pid;
        shm->blockArray[worstFitIndex].size = size;
        shm->blockArray[worstFitIndex].isFree = false;
        shm->blockArray[worstFitIndex].offset = shm->header->free_offset;

        shm->header->used_size += size;
        shm->header->free_offset += size;

        cout << "Process " << pid << " allocated " << size
             << " bytes at offset " << shm->blockArray[worstFitIndex].offset
             << " (Worst Fit)" << endl;

        unlockMemory(shm);
        return;
    }

    cout << "Process " << pid << ": Worst Fit allocation failed for " << size << " bytes!" << endl;
    unlockMemory(shm);
}

// Free a previously allocated block using Worst Fit strategy
void worstFitFree(SharedMemory* shm, int pid, size_t offset) {
    lockMemory(shm);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!shm->blockArray[i].isFree && shm->blockArray[i].pid == pid &&
            shm->blockArray[i].offset == offset) {

            shm->blockArray[i].isFree = true;
            shm->blockArray[i].pid = -1;

            shm->header->used_size -= shm->blockArray[i].size;

            cout << "Process " << pid << " freed block at offset " << offset
                 << " (Worst Fit)" << endl;
            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Worst Fit free failed at offset " << offset << endl;
    unlockMemory(shm);
}

