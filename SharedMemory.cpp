/*include the header files*/

#include "SharedMemory.h"


/*

implement initSharedMemory()
goal - to map the file in memory, resize it using ftruncate(), and fill in the SharedMemory structure fields so that 
rest of the program can use it

to do so we must have - shm pointer which we will point to shared memory, filename is the file which we want
to map in memory, and size -> mmap() expects the file we are trying to map to memory of same or greater size than this size
*/

void initSharedMemory(SharedMemory* shm,const char* filename, size_t size){
    
    
    /* first store the size */
    
    shm->shm_size = size;
    
    /* open the file and store the file descriptor */
    
    shm->fd = open(
        
        filename,   /* file name */    
        O_RDWR | O_CREAT,   /* modes in which file to open */
        0666
    );
    
    
    /* apply a check to verify if file opened succeessfully */
    
    if(shm->fd == -1){
        perror("open");     /* prints human readable error message */
        exit(1);            /* this immediately terminates the program with the error code */
    }
    
    /* resize the file with ftruncate(file descriptor, dezired size) */
    
    if(ftruncate(shm->fd, shm->shm_size)  == -1){
        perror("ftruncate");
        exit(1);
    }
    
    /* now map the file in virtual address space using mmap() */
    
    shm->shm_ptr = mmap(
        
        NULL,                   /* this represent position in virtual space you want to map, NULL means let OS desicde */
        shm->shm_size,          /* length - how many bytes to map */
        PROT_READ | PROT_WRITE, /* PROT means protected - allowed to read & allowed to write */
        MAP_SHARED,             /* this means changes made in this file by any process should go back to the actual file*/
        shm->fd,                /* this tells which file data to map */
        0                       /* offset - start from very beginning of this file */
    );
    
    
    /* apply a check if mmap() failed */
    
    if(shm->shm_ptr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    
    /* initialize the three derived pointers */
    
    shm->header = (MemoryHeader*)shm->shm_ptr;
    shm->blockArray = (BlockInfo*)((char*)shm->shm_ptr + sizeof(MemoryHeader));
    shm->dataRegion = (char*)shm->shm_ptr + sizeof(MemoryHeader) + sizeof(BlockInfo) * MAX_BLOCKS;
}


/*

implement destroySharedMemory
goal - clear up the two resources acquired 

1. memory mapped using mmap()
2. file opened using open()

*/

void destroySharedMemory(SharedMemory* shm){
    
    
    munmap(shm->shm_ptr, shm->shm_size);    /* munmap needs pointer and size - which mapping to remove */
    close(shm->fd);
}



/*

implement initalizeMemoryLayout()
goal - while sharedMemory mapped the file to memory, also intialize the fields of shared memory 
but here we are initlizing the fields of other structures memoryheader and blockinfo

*/

void initializeMemoryLayout(SharedMemory* shm){
    
    
    /* intialized the MemoryHeader fields */
    shm->header->total_size = shm->shm_size;
    shm->header->used_size = 0;
    shm->header->free_offset = 0;
    shm->header->block_array_offset = sizeof(MemoryHeader);
    
    /* the mutex setup */
    
    pthread_mutexattr_t attr; 
    
    /* this declares an attributes object - it is just a small config structure 
    that will describe how mutex should behave. It's not the mutex itself; 
    it's the settings you'll hand to the mutex when creating it.*/
    
    pthread_mutexattr_init(&attr);
    
    /* this initializes the attr with default settings first - you always need to call this before customizing this */

    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    
    /* 
        this changes one specific setting in attr : should this mutex be usable across multiple processes 
       Passing PTHREAD_PROCESS_SHARED says yes — by default, mutexes assume PTHREAD_PROCESS_PRIVATE 
       (only threads within the same process can use it)
       
    */
    
    pthread_mutex_init(&shm->header->lock, &attr);
    
    /* this initilizes the lock mutex shared among processes, using the settings from attr */
    
    
    /*
    
        resetting the block array: Every one of the 1024 BlockInfo slots needs to start in a known, clean state — no owner, zero size, marked free.
    */
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        shm->blockArray[i].pid = -1;
        shm->blockArray[i].size = 0;
        shm->blockArray[i].isFree = true;
        shm->blockArray[i].offset = 0;
    }
}


/*

implement lockMemory & unlockMemory

*/

void lockMemory(SharedMemory* shm){
    
    pthread_mutex_lock(&shm->header->lock);
}

void unlockMemory(SharedMemory* shm){
    pthread_mutex_unlock(&shm->header->lock);
}


/*

printMemoryLayout() function

goal - a debug/status printer so as to know the current state of SharedMmeory, totals and every currently
        allocated block

*/

void printMemoryLayout(SharedMemory* shm){
    
    /* first lock before reading - so to prevent race condition */
    
    lockMemory(shm);
    
    /* print the memory layout */
    
    cout << "Shared Memory Layout:" << endl;
    cout << "Total: " << shm->header->total_size
         << ", Used: " << shm->header->used_size
         << ", Free Offset: " << shm->header->free_offset << endl;
         
         
    /* print each allocated block */
    
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

/*
    setupPreAllocatedBlocks()
    goal - manually hardcode 2 blocks as already allocated, BEFORE any real
           process calls firstFitAllocate. This gives us a known, fixed
           starting state so we can predict and verify exactly what a new
           allocation request will do.
*/
void setupPreAllocatedBlocks(SharedMemory* shm) {

    lockMemory(shm);   // protect this setup too, just like any other write

    /* Block 0: pretend process 9001 already holds 10000 bytes, starting at offset 0 */
    shm->blockArray[0].pid = 9001;
    shm->blockArray[0].size = 10000;
    shm->blockArray[0].isFree = false;
    shm->blockArray[0].offset = 0;

    /* Block 1: pretend process 9002 already holds 20000 bytes, starting at offset 10000 */
    shm->blockArray[1].pid = 9002;
    shm->blockArray[1].size = 20000;
    shm->blockArray[1].isFree = false;
    shm->blockArray[1].offset = 10000;

    /* update the header bookkeeping to match these hardcoded blocks */
    shm->header->used_size = 10000 + 20000;      // = 30000
    shm->header->free_offset = 10000 + 20000;    // next real allocation must start at 30000

    cout << "Pre-allocated: Process 9001 (10000 bytes at offset 0), "
         << "Process 9002 (20000 bytes at offset 10000)." << endl;

    unlockMemory(shm);
}

