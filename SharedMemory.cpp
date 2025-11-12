#include "SharedMemory.h"

// Initialize shared memory mapping
void initSharedMemory(SharedMemory* shm, const char* filename, size_t size) {
    shm->shm_size = size;
    shm->fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (shm->fd == -1) {
        perror("open");
        exit(1);
    }

    if (ftruncate(shm->fd, shm->shm_size) == -1) {
        perror("ftruncate");
        exit(1);
    }

    shm->shm_ptr = mmap(NULL, shm->shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
    if (shm->shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Assign memory regions
    shm->header = (MemoryHeader*)shm->shm_ptr;
    shm->blockArray = (BlockInfo*)((char*)shm->shm_ptr + sizeof(MemoryHeader));
    shm->dataRegion = (char*)shm->shm_ptr + sizeof(MemoryHeader) + sizeof(BlockInfo) * MAX_BLOCKS;
}

// Destroy shared memory mapping
void destroySharedMemory(SharedMemory* shm) {
    munmap(shm->shm_ptr, shm->shm_size);
    close(shm->fd);
}

// Initialize memory header and block array
void initializeMemoryLayout(SharedMemory* shm) {
    shm->header->total_size = shm->shm_size;
    shm->header->used_size = 0;
    shm->header->free_offset = 0;
    shm->header->block_array_offset = sizeof(MemoryHeader);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->header->lock, &attr);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        shm->blockArray[i].pid = -1;
        shm->blockArray[i].size = 0;
        shm->blockArray[i].isFree = true;
        shm->blockArray[i].offset = 0;
    }
}

// Reset memory header and block array while preserving mutex initialization
void resetMemoryLayout(SharedMemory* shm) {
    lockMemory(shm);

    shm->header->used_size = 0;
    shm->header->free_offset = 0;

    for (int i = 0; i < MAX_BLOCKS; i++) {
        shm->blockArray[i].pid = -1;
        shm->blockArray[i].size = 0;
        shm->blockArray[i].isFree = true;
        shm->blockArray[i].offset = 0;
    }

    unlockMemory(shm);
}

// Print memory layout
void printMemoryLayout(SharedMemory* shm) {
    lockMemory(shm);

    cout << "Shared Memory Layout:" << endl;
    cout << "Total: " << shm->header->total_size
         << ", Used: " << shm->header->used_size
         << ", Free Offset: " << shm->header->free_offset << endl;

    cout << "PID\tSize\tFree\tOffset" << endl;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!shm->blockArray[i].isFree) {
            cout << shm->blockArray[i].pid << "\t"
                 << shm->blockArray[i].size << "\tNo\t"
                 << shm->blockArray[i].offset << endl;
        }
    }

    unlockMemory(shm);
}

// Lock shared memory
void lockMemory(SharedMemory* shm) {
    pthread_mutex_lock(&shm->header->lock);
}

// Unlock shared memory
void unlockMemory(SharedMemory* shm) {
    pthread_mutex_unlock(&shm->header->lock);
}
