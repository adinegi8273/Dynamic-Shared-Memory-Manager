#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include "SharedMemory.h"
#include "FirstFitAllocator.h"

using namespace std;

#define SHM_FILE "shared_mem.bin"
#define SHM_SIZE 65536  // 64 KB
#define NUM_THREADS 3

struct ThreadArgs {
    SharedMemory* shm;
    int processID;
    int allocationSize;
};

// Thread function
void* threadFunc(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    firstFitAllocate(args->shm, args->processID, args->allocationSize);
    return nullptr;
}

// Function to run in child process
void runProcess(SharedMemory* shm, int processID) {
    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].shm = shm;
        args[i].processID = processID;
        args[i].allocationSize = (i + 1) * 256; // each thread allocates 256, 512, 768 bytes etc
        pthread_create(&threads[i], nullptr, threadFunc, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    printMemoryLayout(shm);
}

int main() {
    SharedMemory shm;
    initSharedMemory(&shm, SHM_FILE, SHM_SIZE);
    initializeMemoryLayout(&shm);

    cout << "Shared memory initialized." << endl;

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // Child process 1
        runProcess(&shm, 1);
        destroySharedMemory(&shm);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        // Child process 2
        runProcess(&shm, 2);
        destroySharedMemory(&shm);
        exit(0);
    }

    // Parent waits for children
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    cout << "Final memory layout in parent:" << endl;
    printMemoryLayout(&shm);

    destroySharedMemory(&shm);

    return 0;
}
