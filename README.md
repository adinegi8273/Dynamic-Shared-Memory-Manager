# Multi-Process Shared Memory Allocator (C++ / POSIX)

A C++ systems programming project that simulates OS-level memory allocation across multiple **concurrent processes**, using POSIX shared memory (`mmap`) and process-shared synchronization (`pthread_mutex_t`).

Multiple child processes, created via `fork()`, all read from and write to a single shared memory pool — safely, without race conditions — to request memory using the **First Fit** allocation strategy.

## Overview

This project demonstrates:

- **Shared memory via `mmap()`** — a file-backed memory region (`shared_mem.txt`) mapped into multiple processes, so they all see and modify the exact same physical memory.
- **Process synchronization** — a `pthread_mutex_t`, configured with `PTHREAD_PROCESS_SHARED`, ensures only one process can modify shared allocation state at a time.
- **Multi-process concurrency** — the parent process `fork()`s several child processes, each of which independently requests memory from the shared pool.
- **First Fit allocation** — scans a shared block-tracking array and allocates the requested memory at the next available offset.
- **Deterministic test scenarios** — a helper function that pre-hardcodes part of the memory as "already allocated," so allocation/failure behavior can be predicted and verified reproducibly.

## How It Works

1. The parent process creates and initializes a 64KB shared memory region, backed by a memory-mapped file.
2. The shared region is divided into three sections:
   - **Header** — tracks total size, used size, the next free offset, and holds the shared mutex.
   - **Block array** — a fixed-size array of block records (owner PID, size, free/used status, offset).
   - **Data region** — the actual bytes handed out to processes.
3. The parent optionally pre-fills some blocks as already allocated (for repeatable testing), then `fork()`s several child processes.
4. Each child process calls `firstFitAllocate()`, requesting a specific amount of memory. Access to the shared header and block array is protected by a mutex, so concurrent requests from different processes never corrupt each other's data.
5. The parent waits for all children to finish (`waitpid()`), then cleans up the shared memory mapping.

Because process execution order is scheduled by the OS and is **not guaranteed**, running the program multiple times may show child processes completing (and receiving offsets) in a different order each time — while the *correctness* of the final allocation state remains guaranteed, thanks to the mutex.

## Project Structure

```
.
├── SharedMemory.h / SharedMemory.cpp        # shared memory setup, teardown, locking, layout printing
├── FirstFitAllocator.h / FirstFitAllocator.cpp  # First Fit allocation and freeing logic
├── main.cpp                                  # forks child processes, drives the demo
```

## Requirements

- Linux or macOS (uses POSIX APIs: `mmap`, `fork`, `pthread`)
- `g++` with C++11 or later
- POSIX threads support (`-lpthread`)

## Build Instructions

Clone the repository, then compile all source files together:

```bash
g++ -o allocator_demo main.cpp SharedMemory.cpp FirstFitAllocator.cpp -lpthread
```

This produces an executable named `allocator_demo`.

## Running

```bash
./allocator_demo
```

Each run will:
1. Initialize shared memory and print a confirmation.
2. Pre-allocate two hardcoded blocks (for a reproducible test scenario).
3. Fork multiple child processes, each requesting a fixed amount of memory.
4. Print each child's allocation result (success with offset, or failure if there isn't enough space).
5. Print a completion message once all children have finished.

### Example Output

```
Shared memory initialized.
Pre-allocated: Process 9001 (10000 bytes at offset 0), Process 9002 (20000 bytes at offset 10000).
Process 1327 allocated 20000 bytes at offset 30000
Process 1328 allocated 10000 bytes at offset 50000
Process 1329: Allocation failed for 8000 bytes!
Parent: child finished.
```

> Note: the order in which child processes complete (and thus which one fails, if total demand exceeds capacity) may vary between runs — this reflects real, non-deterministic OS process scheduling, not a bug.

## Cleaning Up

The program creates a file named `shared_mem.txt` in the working directory to back the shared memory region. This file persists after the program exits. Delete it if you want to start with a completely fresh memory state on the next run:

```bash
rm shared_mem.txt
```

(If the file already exists from a previous run, the program will reuse and re-initialize it — memory state is explicitly reset at the start of every run via `initializeMemoryLayout()`.)

## Key OS Concepts Demonstrated

- Virtual memory and per-process address spaces
- Memory-mapped files (`mmap`) as a mechanism for shared memory / IPC
- File descriptors and their role independent of memory mappings
- Process creation and duplication semantics (`fork()`)
- Process-shared mutexes vs. thread-only mutexes
- Race conditions and how mutual exclusion prevents them
- Non-deterministic process scheduling vs. deterministic, synchronized shared-state correctness

## Future Improvements

- Extend the block model so freed memory can be reused by future allocations (the current design always advances a high-water-mark offset and never reclaims freed gaps).
- Implement Best Fit, Worst Fit, and Next Fit with genuine placement differences (requires reworking the block array into a proper segment-based free-list model).
- Add block coalescing (merging adjacent free segments) after frees.
