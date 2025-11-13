#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>   // For O_CREAT
#include "SharedMemory.h"
#include "FirstFitAllocator.h"
#include "NextFitAllocator.h"
#include "WorstFitAllocator.h"
#include "BestFitAllocator.h"

using namespace std;

#define SHM_FILE "shared_mem.txt"
#define SHM_SIZE 65536  // 64 KB
#define NUM_THREADS 3

// Named semaphore used across ALL processes & threads
sem_t* inputSem = nullptr; 


struct ThreadArgs {
    SharedMemory* shm;   // pointer to shared memory
    int processID;       // pid of the process using the thread
    int allocationSize;  // memory request for this thread
};


// ---------------------- Thread Function ---------------------------
void* threadFunc(void* arg) {

    ThreadArgs* args = (ThreadArgs*)arg;

    int choice;

    // ************* CRITICAL SECTION FOR USER INPUT ****************
    sem_wait(inputSem);

    cout << "====================================================\n";
    cout << "Thread for Process " << args->processID
         << " requesting " << args->allocationSize << " bytes.\n";

    cout << "Choose Memory Allocation Strategy:\n";
    cout << "1. First Fit\n";
    cout << "2. Best Fit\n";
    cout << "3. Next Fit\n";
    cout << "4. Worst Fit\n";
    cout << "Enter your choice: " << flush;

    cin >> choice;

    // Release semaphore so next thread/process can read input
    sem_post(inputSem);
    // ***************************************************************


    // Perform allocation
    switch (choice) {
        case 1: firstFitAllocate(args->shm, args->processID, args->allocationSize); break;
        case 2: bestFitAllocate(args->shm, args->processID, args->allocationSize); break;
        case 3: nextFitAllocate(args->shm, args->processID, args->allocationSize); break;
        case 4: worstFitAllocate(args->shm, args->processID, args->allocationSize); break;
        default:
            cout << "Invalid choice! Using First Fit by default.\n";
            firstFitAllocate(args->shm, args->processID, args->allocationSize);
    }

    return nullptr;
}



// ---------------------- Child Process Work ------------------------
void runProcess(SharedMemory* shm, int processID) {
    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].shm = shm;
        args[i].processID = processID;
        args[i].allocationSize = (i + 1) * 256;

        pthread_create(&threads[i], nullptr, threadFunc, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    printMemoryLayout(shm);
}



// ------------------------------ MAIN ------------------------------
int main() {

    // Create named semaphore BEFORE forking the children
    inputSem = sem_open("/my_input_sem", O_CREAT, 0666, 1);
    if (inputSem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    SharedMemory shm;
    initSharedMemory(&shm, SHM_FILE, SHM_SIZE);
    initializeMemoryLayout(&shm);

    cout << "Shared memory initialized." << endl;


    // --------------------- FIRST CHILD ---------------------
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        runProcess(&shm, getpid());
        destroySharedMemory(&shm);
        exit(0);
    }


    // --------------------- SECOND CHILD ---------------------
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        runProcess(&shm, getpid());
        destroySharedMemory(&shm);
        exit(0);
    }


    // ----------------------- PARENT -------------------------
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    cout << "Final memory layout in parent:" << endl;
    printMemoryLayout(&shm);

    destroySharedMemory(&shm);

    // Cleanup semaphore
    sem_close(inputSem);
    sem_unlink("/my_input_sem");

    return 0;
}
