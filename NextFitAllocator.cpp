#include "NextFitAllocator.h"
#include <iostream>

using namespace std;

// Allocate memory using Next Fit algorithm
void nextFitAllocate(SharedMemory* shm, int pid, int size) {
    lockMemory(shm);

    static int lastIndex = 0;
    int startIndex = lastIndex;

    for (int count = 0; count < MAX_BLOCKS; count++) {
        int index = (startIndex + count) % MAX_BLOCKS;

        if (shm->blockArray[index].isFree &&
            shm->header->free_offset + size <= shm->header->total_size) {

            shm->blockArray[index].pid = pid;
            shm->blockArray[index].size = size;
            shm->blockArray[index].isFree = false;
            shm->blockArray[index].offset = shm->header->free_offset;

            shm->header->used_size += size;
            shm->header->free_offset += size;

            lastIndex = (index + 1) % MAX_BLOCKS;

            cout << "Process " << pid << " allocated " << size
                 << " bytes at offset " << shm->blockArray[index].offset
                 << " (Next Fit)" << endl;

            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Next Fit allocation failed for " << size << " bytes!" << endl;
    unlockMemory(shm);
}

// Free a previously allocated block using Next Fit strategy
void nextFitFree(SharedMemory* shm, int pid, size_t offset) {
    lockMemory(shm);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!shm->blockArray[i].isFree && shm->blockArray[i].pid == pid &&
            shm->blockArray[i].offset == offset) {

            shm->blockArray[i].isFree = true;
            shm->blockArray[i].pid = -1;

            shm->header->used_size -= shm->blockArray[i].size;

            cout << "Process " << pid << " freed block at offset " << offset
                 << " (Next Fit)" << endl;
            unlockMemory(shm);
            return;
        }
    }

    cout << "Process " << pid << ": Next Fit free failed at offset " << offset << endl;
    unlockMemory(shm);
}

