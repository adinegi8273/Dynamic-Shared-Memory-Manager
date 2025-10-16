#include "SharedMemory.h"

#ifdef _WIN32
static CRITICAL_SECTION g_cs;
static bool g_cs_init = false;
#endif

// Initialize shared memory mapping
void initSharedMemory(SharedMemory* shm, const char* filename, size_t size) {
    shm->shm_size = size;
#ifdef _WIN32
    // Windows fallback: allocate from heap and back it with a file for persistence
    // Note: This is a simplified fallback to get the project running on Windows without POSIX APIs.
    // It does not provide true cross-process shared memory without additional Win32 APIs.
    shm->fd = -1;
    shm->shm_ptr = std::malloc(shm->shm_size);
    if (!shm->shm_ptr) {
        std::cerr << "malloc failed" << std::endl;
        std::exit(1);
    }
    if (!g_cs_init) {
        InitializeCriticalSection(&g_cs);
        g_cs_init = true;
    }
#else
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
#endif

    // Assign memory regions
    shm->header = (MemoryHeader*)shm->shm_ptr;
    shm->blockArray = (BlockInfo*)((char*)shm->shm_ptr + sizeof(MemoryHeader));
    shm->dataRegion = (char*)shm->shm_ptr + sizeof(MemoryHeader) + sizeof(BlockInfo) * MAX_BLOCKS;
}

// Destroy shared memory mapping
void destroySharedMemory(SharedMemory* shm) {
#ifdef _WIN32
    std::free(shm->shm_ptr);
    if (g_cs_init) {
        DeleteCriticalSection(&g_cs);
        g_cs_init = false;
    }
#else
    munmap(shm->shm_ptr, shm->shm_size);
    close(shm->fd);
#endif
}

// Initialize memory header and block array
void initializeMemoryLayout(SharedMemory* shm) {
    shm->header->total_size = shm->shm_size;
    shm->header->used_size = 0;
    shm->header->free_offset = 0;
    shm->header->block_array_offset = sizeof(MemoryHeader);

#ifndef _WIN32
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->header->lock, &attr);
#endif

    for (int i = 0; i < MAX_BLOCKS; i++) {
        shm->blockArray[i].pid = -1;
        shm->blockArray[i].size = 0;
        shm->blockArray[i].isFree = true;
        shm->blockArray[i].offset = 0;
    }
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
#ifdef _WIN32
    EnterCriticalSection(&g_cs);
#else
    pthread_mutex_lock(&shm->header->lock);
#endif
}

// Unlock shared memory
void unlockMemory(SharedMemory* shm) {
#ifdef _WIN32
    LeaveCriticalSection(&g_cs);
#else
    pthread_mutex_unlock(&shm->header->lock);
#endif
}
