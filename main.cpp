/* include the libararies and introduce constants */

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "SharedMemory.h"
#include "FirstFitAllocator.h"

using namespace std;

#define SHM_FILE "shared_mem.txt"
#define SHM_SIZE 65536



/* main function - control starts here */

int main(){
    
    
    /* before fork() the parent process we need to initialize the Memory layout ans shared memory */
    
    SharedMemory shm;
    
    initSharedMemory(&shm, SHM_FILE, SHM_SIZE);
    initializeMemoryLayout(&shm);
    
    cout << "Shared memory initialized." << endl;
    
    setupPreAllocatedBlocks(&shm);
    
    /* now use fork() to create multiple processes */
    
    pid_t pid1 = fork();
    
    if(pid1 < 0){
        
        perror("fork");
        exit(1);
    }
    
    if(pid1 == 0){
        
        /* child process*/
        firstFitAllocate(&shm, getpid(), 20000);
        destroySharedMemory(&shm); /* this is because here we are destroying the memory mapped using mmap() for the child process only */
        exit(0);
    }
    
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid2 == 0) {
        firstFitAllocate(&shm, getpid(), 10000);
        destroySharedMemory(&shm);
        exit(0);
    }
    
    pid_t pid3 = fork();
    
    if (pid3 < 0) {
        perror("fork");
        exit(1);
    }
    if (pid3 == 0) {
        firstFitAllocate(&shm, getpid(), 8000);
        destroySharedMemory(&shm);
        exit(0);
    }
    
    /* This tells the parent: "pause here, and don't continue past this line until 
    the child with this specific PID (pid1) has finished executing. */
    
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    waitpid(pid3, nullptr, 0);
    
    cout << "Parent: child finished." << endl;

    destroySharedMemory(&shm);
    return 0;
}
