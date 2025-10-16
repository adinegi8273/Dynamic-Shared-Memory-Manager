#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <iostream>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <mutex>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#endif

#define MAX_BLOCKS 1024

using namespace std;

struct MemoryHeader {
    size_t total_size;
    size_t used_size;
    size_t free_offset;
    size_t block_array_offset;
#ifndef _WIN32
    pthread_mutex_t lock;
#endif
};

struct BlockInfo {
    int pid;       // PID of process (-1 if free)
    size_t size;   // size of allocated block
    bool isFree;
    size_t offset; // offset from start of shared memory
};

// Shared memory structure
struct SharedMemory {
    int fd;
    size_t shm_size;
    void* shm_ptr;

    MemoryHeader* header;
    BlockInfo* blockArray;
    char* dataRegion;
};

// Function prototypes
void initSharedMemory(SharedMemory* shm, const char* filename, size_t size);
void destroySharedMemory(SharedMemory* shm);
void initializeMemoryLayout(SharedMemory* shm);
void printMemoryLayout(SharedMemory* shm);
void lockMemory(SharedMemory* shm);
void unlockMemory(SharedMemory* shm);

#endif
