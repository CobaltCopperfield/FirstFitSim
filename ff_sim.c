#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Maximum limits for simulation system
#define MAX_BLOCKS 50      // Maximum number of memory blocks that can be managed
#define MAX_PROCESSES 50   // Maximum number of concurrent processes supported
#define MAX_WAIT_QUEUE 50  // Maximum number of processes that can wait for memory

// Represents a single memory block in the system
typedef struct {
    int start;     // Starting memory address of the block
    int size;      // Size of the memory block in kilobytes
    int is_free;   // Flag indicating whether the block is available (1) or allocated (0)
} MemoryBlock;

// Represents a process with its memory allocation details
typedef struct {
    int id;                // Unique identifier for the process
    int memory_address;    // Starting memory address allocated to the process
    int memory_size;       // Amount of memory allocated to the process
    int is_active;         // Flag indicating if the process is currently running
} Process;

// Represents a process waiting for memory allocation
typedef struct {
    int process_id;        // ID of the process waiting for memory
    int memory_size;       // Amount of memory the process needs
} WaitingProcess;

// Comprehensive system memory management structure
typedef struct {
    MemoryBlock blocks[MAX_BLOCKS];         // Array of memory blocks
    Process processes[MAX_PROCESSES];        // Array of active processes
    WaitingProcess wait_queue[MAX_WAIT_QUEUE]; // Queue for processes waiting for memory
    int num_blocks;                         // Current number of memory blocks
    int num_processes;                      // Current number of active processes
    int wait_queue_count;                   // Number of processes in waiting queue
    int wait_queue_front;                   // Index of the front of the waiting queue
    int wait_queue_rear;                    // Index of the rear of the waiting queue
} SystemMemory;

// Function prototypes to resolve circular dependencies
int first_fit_allocate(SystemMemory *sys, int process_id, int size);
int add_to_wait_queue(SystemMemory *sys, int process_id, int size);
int get_total_free_memory(SystemMemory *sys);

// Input validation helper function
int get_valid_integer(const char *prompt, int min, int max) {
    int value;
    char buffer[100];

    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Input error. Please try again.\n");
            continue;
        }

        // Remove newline character
        buffer[strcspn(buffer, "\n")] = 0;

        // Check if input is a number
        char *endptr;
        value = strtol(buffer, &endptr, 10);
        if (*endptr != '\0' || value < min || value > max) {
            printf("Invalid input. Please enter an integer between %d and %d.\n", min, max);
        } else {
            break;
        }
    }

    return value;
}

// Initialize the memory system with predefined memory blocks
void initialize_memory(SystemMemory *sys, int num_blocks, int block_sizes[]) {
    // Reset the entire system memory structure to zero
    memset(sys, 0, sizeof(SystemMemory));

    // Assign memory blocks with sequential starting addresses
    int start_address = 0;
    for (int i = 0; i < num_blocks; i++) {
        sys->blocks[i].start = start_address;      // Set starting address
        sys->blocks[i].size = block_sizes[i];      // Set block size
        sys->blocks[i].is_free = 1;                // Mark block as free initially
        start_address += block_sizes[i];           // Update start address for next block
    }

    // Set initial system memory parameters
    sys->num_blocks = num_blocks;
    sys->wait_queue_front = 0;
    sys->wait_queue_rear = -1;
    sys->wait_queue_count = 0;
}

// Calculate the total amount of free memory in the system
int get_total_free_memory(SystemMemory *sys) {
    int total_free = 0;
    // Sum up sizes of all free memory blocks
    for (int i = 0; i < sys->num_blocks; i++) {
        if (sys->blocks[i].is_free) {
            total_free += sys->blocks[i].size;
        }
    }
    return total_free;
}

// Add a process to the waiting queue when immediate memory allocation is not possible
int add_to_wait_queue(SystemMemory *sys, int process_id, int size) {
    // Check if wait queue is full
    if (sys->wait_queue_count >= MAX_WAIT_QUEUE) {
        printf("Wait queue is full. Cannot add process %d\n", process_id);
        return 0;
    }

    // Circular queue implementation: move rear and add process
    sys->wait_queue_rear = (sys->wait_queue_rear + 1) % MAX_WAIT_QUEUE;
    sys->wait_queue[sys->wait_queue_rear].process_id = process_id;
    sys->wait_queue[sys->wait_queue_rear].memory_size = size;
    sys->wait_queue_count++;

    printf("Process %d added to wait queue due to insufficient memory\n", process_id);
    return 1;
}

// Attempt to allocate memory for a process waiting in the queue
int try_allocate_waiting_process(SystemMemory *sys) {
    // If no processes are waiting, return immediately
    if (sys->wait_queue_count == 0) {
        return 0;
    }

    // Get the process at the front of the waiting queue
    WaitingProcess *current_waiting = &sys->wait_queue[sys->wait_queue_front];
    
    // Check if total free memory is sufficient for the waiting process
    int total_free = get_total_free_memory(sys);
    if (total_free < current_waiting->memory_size) {
        return 0; // Not enough total memory yet
    }

    // Try to allocate memory for the waiting process using first-fit algorithm
    int address = first_fit_allocate(sys, current_waiting->process_id, current_waiting->memory_size);
    
    if (address != -1) {
        // Successfully allocated memory, remove from wait queue
        sys->wait_queue_front = (sys->wait_queue_front + 1) % MAX_WAIT_QUEUE;
        sys->wait_queue_count--;
        printf("Process %d moved from waiting queue and allocated memory\n", 
               current_waiting->process_id);
        return 1;
    }
    
    return 0;
}

// First Fit memory allocation strategy
int first_fit_allocate(SystemMemory *sys, int process_id, int size) {
    // Iterate through memory blocks to find first block that can accommodate the process
    for (int i = 0; i < sys->num_blocks; i++) {
        if (sys->blocks[i].is_free && sys->blocks[i].size >= size) {
            // If block size exactly matches required size
            if (sys->blocks[i].size == size) {
                sys->blocks[i].is_free = 0;
            } else {
                // Split the block if it's larger than required
                // Shift existing blocks to make space for new block
                for (int j = sys->num_blocks; j > i + 1; j--) {
                    sys->blocks[j] = sys->blocks[j - 1];
                }
                
                // Create a new free block with remaining memory
                sys->blocks[i + 1].start = sys->blocks[i].start + size;
                sys->blocks[i + 1].size = sys->blocks[i].size - size;
                sys->blocks[i + 1].is_free = 1;

                // Adjust original block
                sys->blocks[i].size = size;
                sys->num_blocks++;
                sys->blocks[i].is_free = 0;
            }

            // Record the process in active processes list
            sys->processes[sys->num_processes].id = process_id;
            sys->processes[sys->num_processes].memory_address = sys->blocks[i].start;
            sys->processes[sys->num_processes].memory_size = size;
            sys->processes[sys->num_processes].is_active = 1;
            sys->num_processes++;
            
            return sys->blocks[i].start;
        }
    }

    // If no suitable block found, try to add to waiting queue
    if (add_to_wait_queue(sys, process_id, size)) {
        return -1; // Allocation failed
    }
    return -1;
}

// Free memory allocated to a specific process
void free_memory(SystemMemory *sys, int process_id) {
    // Find the process in the active processes list
    for (int i = 0; i < sys->num_processes; i++) {
        if (sys->processes[i].id == process_id && sys->processes[i].is_active) {
            // Mark the process as inactive
            sys->processes[i].is_active = 0;

            // Find and free the corresponding memory block
            for (int j = 0; j < sys->num_blocks; j++) {
                if (sys->blocks[j].start == sys->processes[i].memory_address) {
                    sys->blocks[j].is_free = 1;
                    printf("Memory for Process %d freed\n", process_id);
                    
                    // Attempt to allocate memory for waiting processes
                    while (try_allocate_waiting_process(sys));
                    return;
                }
            }
        }
    }
    printf("Process %d not found\n", process_id);
}

// Print detailed information about current memory layout
void print_memory_layout(SystemMemory *sys) {
    // Display details of all memory blocks
    printf("Memory Blocks:\n");
    for (int i = 0; i < sys->num_blocks; i++) {
        printf("Block %d: Start_address=%d, Size=%dKB, %s\n",
               i + 1,
               sys->blocks[i].start,
               sys->blocks[i].size,
               sys->blocks[i].is_free ? "Free" : "Allocated");
    }

    // Display active processes
    int active_count = 0;
    printf("\nActive Processes:\n");
    for (int i = 0; i < sys->num_processes; i++) {
        if (sys->processes[i].is_active) {
            printf("Process %d: Address=%d, Size=%dKB\n",
                   sys->processes[i].id,
                   sys->processes[i].memory_address,
                   sys->processes[i].memory_size);
            active_count++;
        }
    }

    // Handle case of no active processes
    if (active_count == 0) {
        printf("No active processes\n");
    }

    // Display waiting queue
    printf("\nWaiting Queue:\n");
    if (sys->wait_queue_count == 0) {
        printf("No processes waiting\n");
    } else {
        for (int i = 0; i < sys->wait_queue_count; i++) {
            int index = (sys->wait_queue_front + i) % MAX_WAIT_QUEUE;
            printf("Process %d: Waiting for %dKB\n", 
                   sys->wait_queue[index].process_id, 
                   sys->wait_queue[index].memory_size);
        }
    }
    printf("\n---------------------------------------------\n\n");
}

// Display interactive menu options
void display_menu() {
    printf("--Main Menu--\n");
    printf("1. Allocate Memory\n");
    printf("2. Free Memory\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    SystemMemory system_memory;

    printf("System Limitations:\n");
    printf("- Maximum Memory Blocks: %d\n", MAX_BLOCKS);
    printf("- Maximum Processes: %d\n", MAX_PROCESSES);
    printf("- Maximum Waiting Queue Size: %d\n", MAX_WAIT_QUEUE);
    printf("---------------------------------------------\n");

    // Get number of memory blocks from user
    int num_blocks = get_valid_integer("Enter the number of memory blocks you want to simulate: ", 1, MAX_BLOCKS);

    // Get sizes for each memory block
    int block_sizes[MAX_BLOCKS];
    for (int i = 0; i < num_blocks; i++) {
        char prompt[50];
        snprintf(prompt, sizeof(prompt), "Enter size of memory block %d (in KB): ", i + 1);
        block_sizes[i] = get_valid_integer(prompt, 1, INT_MAX);
    }

    // Initialize memory system
    initialize_memory(&system_memory, num_blocks, block_sizes);

    // Variables for process management
    int choice, size, process_id = 1;

    // Main program loop
    while (1) {
        printf("\n----First Fit Memory Allocation Simulator----\n\n");
        print_memory_layout(&system_memory);  // Display current memory state
        display_menu();  // Show menu options
        choice = get_valid_integer("", 1, 3);

        // Handle user choices
        switch (choice) {
            case 1:  // Allocate Memory
                size = get_valid_integer("Enter memory size to allocate (in KB): ", 1, INT_MAX);

                // Attempt to allocate memory
                int address = first_fit_allocate(&system_memory, process_id++, size);
                if (address != -1) {
                    printf("Memory allocated at address %d\n", address);
                } else {
                    printf("Process added to wait queue\n");
                }
                break;

            case 2:  // Free Memory
                size = get_valid_integer("Enter process number (ID) to free memory: ", 1, process_id - 1);
                free_memory(&system_memory, size);
                break;

            case 3:  // Exit Program
                printf("Exiting...\n");
                exit(0);

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}

