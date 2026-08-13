/* include the header files and necessary libararies */

#include "FirstFitAllocator.h"
#include <iostream>

using namespace std;


/*

implement firstFitAllocate()

goal - allocate 'size' bytes to process 'pid' using the First Fit strategy:
           scan the block array from the beginning, and use the FIRST free slot
           that has enough remaining space to fit this request. Stop searching
           as soon as one is found — don't look for a "better" one.

*/


void firstFitAllocate(SharedMemory* shm, int pid,int size){
    
    /* first lock memory to prevent race conditions */
    lockMemory(shm);
    
    /* now scan all the blocks for any block first it should be free and second it should be big enough for required space */
    
    for(int i=0;i<MAX_BLOCKS;i++){
            
        if(shm->blockArray[i].isFree && shm->header->free_offset + size <= shm->header->total_size){
            
            
            /* allocate the process the first memory available */
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
    
    /* print allocation failed if control comes here */
    
    cout << "Process " << pid << ": Allocation failed for " << size << " bytes!" << endl;
    unlockMemory(shm);
}


/*

implement freeBlock()
    goal - free a previously allocated block. Search the block array for the
           slot that matches BOTH the given pid and offset (to make sure a
           process can only free its own block, not someone else's), and mark
           it as free again.


*/


/* here we need to use pid also because some other process also can unlock anohter;s processes memory by just using offset */


void freeBlock(SharedMemory* shm,int pid,size_t offset){
    
    lockMemory(shm);
    
    for(int i=0;i<MAX_BLOCKS;i++){
        
        if(!shm->blockArray[i].isFree && shm->blockArray[i].pid == pid && shm->blockArray[i].offset == offset){
            
            shm->blockArray[i].isFree = true;
            shm->blockArray[i].pid = -1;

            shm->header->used_size -= shm->blockArray[i].size;

            cout << "Process " << pid << " freed block at offset " << offset << endl;
            unlockMemory(shm);
            return;
        }
    }
    
    /* print that the freeing failed */
    
    cout << "Process " << pid << ": Free failed at offset " << offset << endl;
    unlockMemory(shm);
}


/*

Since we're doing contiguous memory allocation, free_offset acts as a one-way, 
ever-advancing marker — it only ever moves forward (increases) whenever a new allocation happens. 
Freeing a block does not move free_offset backward, 
even though that block's space is logically free again.

Only used_size actually decreases when a block is freed, 
since it's meant to reflect how much memory is currently in active use,

*/



