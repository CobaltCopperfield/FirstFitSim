# First Fit Memory Allocation Simulator

This project implements a **First Fit Memory Allocation Simulator** in C, simulating memory management using first-fit allocation strategy. The simulator handles memory allocation, deallocation, and manages processes in a waiting queue when sufficient memory is unavailable.

## Features
- **First Fit Allocation**: Allocates memory to the first available block that fits the process size.
- **Dynamic Memory Management**: Splits and merges memory blocks dynamically during allocation and deallocation.
- **Wait Queue Management**: Handles processes waiting for memory allocation.
- **User Interaction**: Provides a simple menu-driven interface for memory management operations.
- **Memory Layout Visualization**: Displays the current memory allocation status, active processes, and waiting queue.

## System Limitations
- **Maximum Memory Blocks**: 50
- **Maximum Processes**: 50
- **Maximum Waiting Queue Size**: 50

## Requirements
- GCC/clang Compiler (or any C compiler)
- Command-line interface

## How It Works
### Memory Allocation
- When a process requests memory, the simulator searches for the first free block large enough to accommodate the process.
- If the block is larger than required, it splits the block.
- If no suitable block is found, the process is added to the wait queue.

### Memory Deallocation
- When a process releases memory, its corresponding block is marked as free.
- The simulator checks the wait queue and tries to allocate memory for waiting processes.

### Waiting Queue
- Processes in the waiting queue are allocated memory when enough contiguous free space becomes available.

## Build and Run
### Step 1: Clone the Repository
```bash
$ git clone <repository-url>
$ cd FirstFitSim
```

### Step 2: Compile the Code
Use the GCC compiler to compile the `ff_sim.c` file:
```bash
$ gcc ff_sim.c -o simulator
```

### Step 3: Run the Simulator
```bash
$ ./simulator
```

## Usage
Upon running the program, follow these steps:
1. **Set Up Memory Blocks**: Specify the number of memory blocks and their sizes.
2. **Allocate Memory**: Choose option 1 to allocate memory for a process by specifying its size.
3. **Free Memory**: Choose option 2 to release memory occupied by a process by specifying its process ID.
4. **Exit**: Choose option 3 to exit the program.


## File Structure
```
.
├── ff_sim.c            # Source code for the simulator
├── README.md         # Documentation
```

## Future Enhancements
- Implement additional allocation strategies (Best Fit, Worst Fit).
- Introduce memory defragmentation.
- Add persistent storage for memory configurations.
