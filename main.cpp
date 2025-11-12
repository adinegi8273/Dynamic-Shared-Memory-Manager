#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include "SharedMemory.h"
#include "FirstFitAllocator.h"
#include "BestFitAllocator.h"
#include "WorstFitAllocator.h"
#include "NextFitAllocator.h"

using namespace std;

#define SHM_FILE "shared_mem.bin"
#define SHM_SIZE 65536  // 64 KB
#define NUM_THREADS 3

enum class AllocationStrategy {
    FirstFit,
    BestFit,
    WorstFit,
    NextFit
};

struct ThreadArgs {
    SharedMemory* shm;
    int processID;
    int allocationSize;
    AllocationStrategy strategy;
};

const char* strategyToString(AllocationStrategy strategy) {
    switch (strategy) {
        case AllocationStrategy::FirstFit:
            return "First Fit";
        case AllocationStrategy::BestFit:
            return "Best Fit";
        case AllocationStrategy::WorstFit:
            return "Worst Fit";
        case AllocationStrategy::NextFit:
            return "Next Fit";
    }
    return "Unknown";
}

// Thread function
void* threadFunc(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    switch (args->strategy) {
        case AllocationStrategy::FirstFit:
            firstFitAllocate(args->shm, args->processID, args->allocationSize);
            break;
        case AllocationStrategy::BestFit:
            bestFitAllocate(args->shm, args->processID, args->allocationSize);
            break;
        case AllocationStrategy::WorstFit:
            worstFitAllocate(args->shm, args->processID, args->allocationSize);
            break;
        case AllocationStrategy::NextFit:
            nextFitAllocate(args->shm, args->processID, args->allocationSize);
            break;
    }
    return nullptr;
}

// Function to run in child process
void runProcess(SharedMemory* shm, int processID, AllocationStrategy strategy) {
    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];

    cout << "Process " << processID << " using " << strategyToString(strategy) << " algorithm" << endl;

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].shm = shm;
        args[i].processID = processID;
        args[i].allocationSize = (i + 1) * 256; // each thread allocates 256, 512, 768 bytes etc
        args[i].strategy = strategy;
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

    struct StrategyRun {
        AllocationStrategy strategy;
        int processID;
    };

    StrategyRun runs[] = {
        {AllocationStrategy::FirstFit, 1},
        {AllocationStrategy::BestFit, 2},
        {AllocationStrategy::WorstFit, 3},
        {AllocationStrategy::NextFit, 4}
    };

    for (const auto& run : runs) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            destroySharedMemory(&shm);
            exit(1);
        }

        if (pid == 0) {
            runProcess(&shm, run.processID, run.strategy);
            destroySharedMemory(&shm);
            exit(0);
        }

        waitpid(pid, nullptr, 0);

        cout << "Parent observed layout after " << strategyToString(run.strategy) << ":" << endl;
        printMemoryLayout(&shm);
        resetMemoryLayout(&shm);
    }

    destroySharedMemory(&shm);

    return 0;
}
