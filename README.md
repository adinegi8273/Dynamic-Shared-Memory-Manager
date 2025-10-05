# 🧠 Dynamic Shared Memory Manager

### Cross-Platform Shared Memory Allocator for Inter-Process Communication

This project implements a **Dynamic Shared Memory Manager** — a C++17 library that provides dynamic memory allocation and deallocation across multiple processes using a shared memory region.  
It supports multiple allocation strategies (**First Fit**, **Best Fit**, **Worst Fit**) and works on both **Linux** and **Windows** platforms.

---

## 🚀 Project Objectives

- Implement a cross-platform shared memory management system.
- Support dynamic memory allocation/deallocation across multiple processes.
- Provide configurable allocation strategies (First Fit, Best Fit, Worst Fit).
- Design synchronization mechanisms for safe concurrent access.
- Offer a simple C++ API (`alloc`, `free`, `init`, `close`, etc.).
- Test and validate performance, fragmentation, and concurrency behavior.
- Produce technical documentation, diagrams, and final report.

---

## 🧩 Core Features

| Feature | Description |
|----------|-------------|
| **Cross-platform** | Compatible with Linux (POSIX APIs) and Windows (Win32 APIs) |
| **Dynamic allocation** | Supports variable-size memory allocation/deallocation |
| **Multiple strategies** | First Fit, Best Fit, and Worst Fit algorithms |
| **Offset-based memory model** | Ensures pointer validity across process address spaces |
| **Synchronization** | Interprocess-safe mutex/spinlock for concurrent access |
| **Diagnostics** | Heap validation, debug logging, and state dumps |
| **Testing Suite** | Unit tests and multi-process functional tests |

---

## 🛠️ Tools & Technologies

### Development Environment

| Category | Tool | Purpose |
|-----------|------|----------|
| Compiler | GCC / MSVC / Clang | C++17 or newer support |
| Build System | CMake | Cross-platform build configuration |
| Version Control | Git | Track changes and collaborate |
| IDE | VS Code / Visual Studio / CLion | Development and debugging |

### OS-Level APIs

| OS | API / Header | Purpose |
|----|---------------|----------|
| Linux | `shm_open`, `ftruncate`, `mmap`, `pthread_mutex_t` | Shared memory and interprocess synchronization |
| Windows | `CreateFileMapping`, `MapViewOfFile`, `CreateMutex` | Shared memory mapping and synchronization |

### Optional Libraries

| Library | Use |
|----------|-----|
| GoogleTest (gtest) | Unit testing |
| spdlog / loguru | Logging |
| Valgrind / AddressSanitizer | Memory debugging |
| Draw.io / Figma | Diagrams |
| Doxygen | Documentation generation |

---

## 🧪 Testing Tools

- **CTest** for integration with CMake
- **Python 3** scripts for multi-process tests
- **Shell / PowerShell** automation
- **Valgrind / Helgrind** for race detection
- **Visual Studio Debugger / GDB** for step debugging

---

## 📚 Documentation Tools

- **Draw.io** or **Figma** for architecture diagrams  
- **Markdown / Doxygen** for inline API documentation  
- **MS Word / Google Docs** for project report  
- **MS PowerPoint / Google Slides** for presentation slides  

---

## 🔐 Debugging & Profiling Tools

| Tool | Use |
|------|-----|
| GDB / LLDB | Debugging on Linux/macOS |
| Visual Studio Debugger | Debugging on Windows |
| strace / ltrace | Trace system calls |
| Process Explorer | Inspect shared memory handles |
| Perf / WPR | Performance profiling |

---

## 🗂️ Project Roadmap (Phases)

### **Phase 0 — Kickoff & Planning**
- Finalize project scope, OS targets, and team roles.
- Initialize Git repository and CMake structure.
- Create initial `README.md` and diagrams folder.

---

### **Phase 1 — Research & Design**
- Define shared memory layout (header, freelist, block structure).
- Choose synchronization model (POSIX mutex / Win32 mutex / spinlock).
- Decide pointer model (offset-based addressing).
- Draft API surface (`alloc`, `free`, `init`, etc.).
- Create architecture and memory layout diagrams.

---

### **Phase 2 — Shared Mapping Layer**
- Implement cross-platform shared memory mapping:
  - **Linux:** `shm_open`, `mmap`
  - **Windows:** `CreateFileMapping`, `MapViewOfFile`
- Create `SharedMapping` abstraction class.
- Test by creating and opening mappings in multiple processes.

---

### **Phase 3 — Heap Initialization & API**
- Define `SharedHeapHeader` and `BlockHeader` structures.
- Implement minimal allocation API:
  ```cpp
  bool shm_heap_init(const char* name, size_t size, AllocStrategy s);
  uint64_t shm_heap_alloc(uint64_t size);
  void shm_heap_free(uint64_t offset);
  void* shm_heap_local_ptr(uint64_t offset);
  void shm_heap_close();

  ---

### **Phase 4 — Allocation Strategies & Concurrency**
- Implement allocation strategies:
  - **First Fit**
  - **Best Fit**
  - **Worst Fit**
- Wrap allocation and deallocation with synchronization mechanisms:
  - Linux: `pthread_mutex_t` in shared memory
  - Windows: `CreateMutex` or custom spinlock
- Test concurrent allocations and frees from multiple processes
- Validate correctness and ensure no race conditions or deadlocks

---

### **Phase 5 — Testing & Validation**
- **Functional Testing**
  - Allocation and deallocation correctness
  - Edge cases: zero-size requests, oversized allocations, full heap
- **Concurrency Testing**
  - Multiple processes allocating/freeing simultaneously
  - Verify absence of race conditions or deadlocks
- **Performance Testing**
  - Measure allocation latency and throughput
  - Analyze fragmentation for different allocation strategies
- **Memory Debugging**
  - Use Valgrind or AddressSanitizer to detect leaks or invalid accesses

---

### **Phase 6 — Documentation & Diagrams**
- Generate API documentation using **Doxygen**
- Provide usage examples in `README.md` or `examples/` folder
- Create diagrams illustrating:
  - Memory layout
  - Allocation workflow
  - Synchronization and concurrency

---

### **Phase 7 — Final Report & Delivery**
- Compile a comprehensive project report including:
  - Design decisions
  - Testing methodology and results
  - Performance benchmarks
  - Lessons learned
- Include all diagrams and API references
- Archive final code, documentation, and example programs for release

