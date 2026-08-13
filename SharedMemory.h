
/*first place header guards*/

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H


/*include the necessay libararies*/

#include <fcntl.h>      /* for open(), O_CREAT */
#include <sys/mman.h>   /* for mmap() POSIX system call */
#include <sys/stat.h>   /* for file permissions */
#include <unistd.h>     /* for POSIX system calls: ftruncate(), close() */
#include <pthread.h>    /* for pthread_mutex_t, mutex functions */
#include <cstdlib>      /* exit() */
#include <cstring>    /* memory/ string utilities */
#include <iostream>   /* for cout */

using namespace std;

/*define MACRO for number of blocks in block array*/

#define MAX_BLOCKS 1024



/* structure definitions*/


struct MemoryHeader{
    size_t total_size;  /*size_t is an unsigned integer - cant take values specific represent size in bytes*/
    size_t used_size;
    size_t free_offset;
    size_t block_array_offset;
    pthread_mutex_t lock;
};


struct BlockInfo{
    
    int pid;
    size_t size;
    bool isFree;
    size_t offset;
    
};

struct SharedMemory{    /* this is the master structure which is in each process */
    
    int fd;
    size_t shm_size;
    void* shm_ptr;      /* pointing to shared memory */
    
    MemoryHeader* header;   /* each process will get a separate copy of this SharedMemory but all will point at same memory location */
    BlockInfo* blockArray;
    char* dataRegion;
};



/* These declare the functions whose implementation will live in SharedMemory.cpp. 
Declaring them here lets other files (main.cpp, the allocator files) call these functions 
after just #include-ing this header.*/

void initSharedMemory(SharedMemory* shm, const char* filename, size_t size);
void destroySharedMemory(SharedMemory* shm);
void initializeMemoryLayout(SharedMemory* shm);
void setupPreAllocatedBlocks(SharedMemory* shm);   // hardcode some blocks as already allocated, for testing
void printMemoryLayout(SharedMemory* shm);
void lockMemory(SharedMemory* shm);
void unlockMemory(SharedMemory* shm);


//header guard ending
#endif