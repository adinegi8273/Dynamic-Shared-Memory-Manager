#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include "SharedMemory.h"
#include "FirstFitAllocator.h"

using namespace std;

#define SHM_FILE "shared_mem.bin"
#define SHM_SIZE 65536  // 64 KB
static void printHelp() {
	cout << "Commands:" << endl;
	cout << "  alloc <pid> <size>\tAllocate <size> bytes for process <pid>" << endl;
	cout << "  free <pid> <offset>\tFree block at <offset> for process <pid>" << endl;
	cout << "  show\t\t\tShow current memory layout" << endl;
	cout << "  help\t\t\tShow this help" << endl;
	cout << "  exit|quit\t\tExit the program" << endl;
}

int main() {
	SharedMemory shm;
	initSharedMemory(&shm, SHM_FILE, SHM_SIZE);
	initializeMemoryLayout(&shm);

	cout << "Shared memory initialized (file: " << SHM_FILE << ", size: " << SHM_SIZE << ")." << endl;
	printHelp();

	string line;
	while (true) {
		cout << "> ";
		if (!std::getline(cin, line)) {
			break;
		}

		// Trim leading/trailing spaces
		size_t start = line.find_first_not_of(" \t\r\n");
		size_t end = line.find_last_not_of(" \t\r\n");
		if (start == string::npos) {
			continue;
		}
		line = line.substr(start, end - start + 1);

		// Extract command token
		istringstream iss(line);
		string cmd;
		iss >> cmd;
		for (char &ch : cmd) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

		if (cmd == "alloc") {
			long long pidLL; long long sizeLL;
			if (!(iss >> pidLL >> sizeLL)) {
				cout << "Usage: alloc <pid> <size>" << endl;
				continue;
			}
			if (pidLL < 0 || sizeLL <= 0) {
				cout << "Error: pid must be >= 0 and size > 0" << endl;
				continue;
			}
			firstFitAllocate(&shm, static_cast<int>(pidLL), static_cast<int>(sizeLL));
		} else if (cmd == "free") {
			long long pidLL; long long offsetLL;
			if (!(iss >> pidLL >> offsetLL)) {
				cout << "Usage: free <pid> <offset>" << endl;
				continue;
			}
			if (pidLL < 0 || offsetLL < 0) {
				cout << "Error: pid and offset must be >= 0" << endl;
				continue;
			}
			freeBlock(&shm, static_cast<int>(pidLL), static_cast<size_t>(offsetLL));
		} else if (cmd == "show") {
			printMemoryLayout(&shm);
		} else if (cmd == "help") {
			printHelp();
		} else if (cmd == "exit" || cmd == "quit") {
			break;
		} else {
			cout << "Unknown command. Type 'help' for usage." << endl;
		}
	}

	destroySharedMemory(&shm);
	return 0;
}
