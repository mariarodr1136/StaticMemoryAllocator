/******************************************************************************
 * File: static_memory_allocation.c
 *
 * Purpose:
 * This program demonstrates different contiguous static memory allocation
 * techniques including First Fit, Best Fit, and Worst Fit.
 *
 * Compilation:
 * gcc static_memory_allocation.c -o static_memory_allocation -Wall
 *
 * Usage:
 * ./static_memory_allocation [input_file]
 *
 *****************************************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <string.h>
 #include <time.h>
 #include <limits.h>
 
 /* Constants */
 #define MAX_BLOCKS 100     // Maximum number of memory blocks
 #define MAX_PROCESSES 20   // Maximum number of processes
 #define MEM_VISUAL_SIZE 60 // Memory visualization size
 #define DEFAULT_INPUT_FILE "input.txt"
 #define MAX_LINE_LENGTH 1024
 
 /* Global variables*/
 int memory_size; // Total memory size (in KB)
 
 /* Allocation strategies */
 typedef enum
 {
     FIRST_FIT,
     BEST_FIT,
     WORST_FIT
 } AllocationStrategy;
 
 /* Process states */
 typedef enum
 {
     PROCESS_NEW,
     PROCESS_RUNNING,
     PROCESS_TERMINATED
 } ProcessState;
 
 /* Memory block structure */
 typedef struct
 {
     int start_address; // Start address of the block
     int size;          // Size of the block
     bool is_free;      // Is the block free?
     int process_id;    // ID of the process using this block (-1 if free)
 } MemoryBlock;
 
 /* Process structure */
 typedef struct
 {
     int id;             // Process ID
     int size;           // Process size in KB
     ProcessState state; // Process state
     int block_index;    // Index of allocated block (-1 if none)
 } Process;
 
 /* Memory manager structure */
 typedef struct
 {
     int total_size;                 // Total memory size
     int free_size;                  // Available free memory
     int block_count;                // Number of blocks
     MemoryBlock blocks[MAX_BLOCKS]; // Array of memory blocks
     AllocationStrategy strategy;    // Current allocation strategy
 } MemoryManager;
 
 /* Statistics structure */
 typedef struct
 {
     int allocation_attempts;
     int successful_allocations;
     int failed_allocations;
     double avg_utilization;
     double peak_utilization;
     int external_fragmentation;      // Number of free blocks
     double fragmentation_percentage; // Percentage of memory fragmented
     double avg_fragment_size;        // Average size of fragments
 } Statistics;
 
 /* Function prototypes */
 void initialize_memory_manager(MemoryManager *manager, AllocationStrategy strategy);
 int find_first_fit(MemoryManager *manager, int size);
 int find_best_fit(MemoryManager *manager, int size);
 int find_worst_fit(MemoryManager *manager, int size);
 bool allocate_memory(MemoryManager *manager, Process *process);
 void deallocate_memory(MemoryManager *manager, Process *process);
 bool coalesce_memory(MemoryManager *manager, Process processes[]); // New separate coalescing function
 bool read_processes_from_file(const char *filename, Process processes[], int *num_processes, int *memory_size);
 void print_memory_state_simplified(MemoryManager *manager, Process processes[], int num_processes);
 void print_memory_state_detailed(MemoryManager *manager, Process processes[], int num_processes);
 void update_fragmentation_metrics(MemoryManager *manager, Process processes[], int num_processes, Statistics *stats);
 void run_simulation(MemoryManager *manager, AllocationStrategy strategy, Process processes[], int num_processes, Statistics *stats);
 
 /*#####################################################################################################################*/
 // Implement the following 4 functions
 /*#####################################################################################################################*/
 /**
  * Find the first free block that fits the requested size (First Fit)
  *
  * This function scans the memory blocks from the beginning and returns
  * the index of the first free block that can fit the requested size.
  *
  * Parameters:
  *   manager - Pointer to the memory manager
  *   size - Size of memory requested
  *
  * Returns:
  *   Index of the suitable block, or -1 if no suitable block was found
  */
 int find_first_fit(MemoryManager *manager, int size)
 {
     // Iterate through all blocks
     for (int i = 0; i < manager->block_count; i++) {
         // Check if the block is free and large enough
         if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
             return i; // Return the index of the first suitable block
         }
     }
     
     // No suitable block found
     return -1;
 }
 
 /**
  * Find the smallest free block that fits the requested size (Best Fit)
  *
  * This function searches for the smallest free block that can fit the requested size.
  * It aims to minimize wasted space.
  *
  * Parameters:
  *   manager - Pointer to the memory manager
  *   size - Size of memory requested
  *
  * Returns:
  *   Index of the suitable block, or -1 if no suitable block was found
  */
 int find_best_fit(MemoryManager *manager, int size)
 {
     int best_fit_index = -1;
     int min_size_diff = INT_MAX;
     
     // Iterate through all blocks
     for (int i = 0; i < manager->block_count; i++) {
         // Check if the block is free and large enough
         if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
             // Calculate the size difference
             int size_diff = manager->blocks[i].size - size;
             
             // Update best fit if this block has a smaller size difference
             if (size_diff < min_size_diff) {
                 min_size_diff = size_diff;
                 best_fit_index = i;
             }
         }
     }
     
     return best_fit_index;
 }
 
 /**
  * Find the largest free block that fits the requested size (Worst Fit)
  *
  * This function searches for the largest free block that can fit the requested size.
  * It aims to leave the largest possible leftover space after allocation.
  *
  * Parameters:
  *   manager - Pointer to the memory manager
  *   size - Size of memory requested
  *
  * Returns:
  *   Index of the suitable block, or -1 if no suitable block was found
  */
 int find_worst_fit(MemoryManager *manager, int size)
 {
     int worst_fit_index = -1;
     int max_size_diff = -1;
     
     // Iterate through all blocks
     for (int i = 0; i < manager->block_count; i++) {
         // Check if the block is free and large enough
         if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
             // Calculate the size difference
             int size_diff = manager->blocks[i].size - size;
             
             // Update worst fit if this block has a larger size difference
             if (size_diff > max_size_diff) {
                 max_size_diff = size_diff;
                 worst_fit_index = i;
             }
         }
     }
     
     return worst_fit_index;
 }
 
 /**
  * Coalesce adjacent free memory blocks
  *
  * This function merges adjacent free blocks in memory to reduce fragmentation.
  * It should be called after a block is freed to consolidate free space.
  *
  * Parameters:
  *   manager - Pointer to the memory manager
  *   processes - Array of processes (needed to update block indices)
  *
  * Returns:
  *   true if any blocks were merged, false otherwise
  */
 bool coalesce_memory(MemoryManager *manager, Process processes[])
 {
     bool merged = false;
     int i = 0;
     
     // Continue until we reach the second-to-last block
     while (i < manager->block_count - 1) {
         // Check if current block and next block are both free
         if (manager->blocks[i].is_free && manager->blocks[i + 1].is_free) {
             // Merge the blocks by adding the size of the next block to the current one
             manager->blocks[i].size += manager->blocks[i + 1].size;
             
             // Shift the remaining blocks to close the gap
             for (int j = i + 1; j < manager->block_count - 1; j++) {
                 manager->blocks[j] = manager->blocks[j + 1];
             }
             
             // Update process block indices that are affected by the shift
             for (int j = 0; j < MAX_PROCESSES; j++) {
                 if (processes[j].block_index > i + 1) {
                     // Decrement the block index for processes that were using
                     // blocks after the merged one
                     processes[j].block_index--;
                 }
             }
             
             // Decrement the block count
             manager->block_count--;
             
             // Set the merged flag to true
             merged = true;
             
             // Don't increment i as we need to check the newly merged block
             // against the next block (which has been shifted)
         } else {
             // If no merge is possible, move to the next block
             i++;
         }
     }
     
     return merged;
 }
 
 /*######################################################################################################################*/
 /**
  * Main function
  */
 int main(int argc, char *argv[])
 {
     char input_file[256] = DEFAULT_INPUT_FILE;
 
     // Parse command line arguments
     if (argc > 1)
         strcpy(input_file, argv[1]);
 
     // Initialize random seed
     srand(time(NULL));
 
     // Load processes from input file
     Process processes[MAX_PROCESSES];
     int num_processes = 0;
     int memory_size = 0;
 
     if (!read_processes_from_file(input_file, processes, &num_processes, &memory_size))
     {
         fprintf(stderr, "Failed to read processes from input file.\n");
         return EXIT_FAILURE;
     }
 
     printf("\n===== STATIC MEMORY ALLOCATION SIMULATION =====\n\n");
     printf("Input file: %s\n", input_file);
     printf("Memory size: %d KB\n", memory_size);
     printf("Number of processes: %d\n\n", num_processes);
 
     // Print process info before simulation
     printf("-------------------------------------------------\n");
     printf("Processes Loaded:\n");
     printf("%-10s %-10s\n", "ProcessID", "Size (KB)");
     printf("-------------------------------------------------\n");
     for (int i = 0; i < num_processes; i++)
     {
         printf("%-10d %-10d\n", processes[i].id, processes[i].size);
     }
     printf("\n");
 
     // Initialize statistics for each strategy
     Statistics stats[3] = {0};
     AllocationStrategy strategies[3] = {FIRST_FIT, BEST_FIT, WORST_FIT};
 
     // Run simulations for each allocation strategy
     for (int i = 0; i < 3; i++)
     {
         MemoryManager manager;
         initialize_memory_manager(&manager, strategies[i]);
 
         // Set the correct memory size from the input file
         manager.total_size = memory_size;
         manager.free_size = memory_size;
         manager.blocks[0].size = memory_size;
 
         // Make a copy of processes for each simulation
         Process simulation_processes[MAX_PROCESSES];
         memcpy(simulation_processes, processes, sizeof(Process) * num_processes);
 
         run_simulation(&manager, strategies[i], simulation_processes, num_processes, &stats[i]);
     }
 
     /**
      * Print simplified summary table
      */
     printf("\n=== Summary of Allocation Methods ===\n");
     printf("%-10s %-15s %-15s %-15s\n", "Strategy", "Success Rate", "Fragmentation", "Block Count");
     printf("----------------------------------------------------------\n");
 
     for (int i = 0; i < 3; i++)
     {
         char *strategy_name;
         switch (strategies[i])
         {
         case FIRST_FIT:
             strategy_name = "First Fit";
             break;
         case BEST_FIT:
             strategy_name = "Best Fit";
             break;
         case WORST_FIT:
             strategy_name = "Worst Fit";
             break;
         default:
             strategy_name = "Unknown";
             break;
         }
 
         double success_rate =
             (stats[i].allocation_attempts > 0) ? ((double)stats[i].successful_allocations / stats[i].allocation_attempts * 100.0) : 0.0;
 
         // Create formatted strings with percentages
         char success_str[20], frag_str[20];
         sprintf(success_str, "%.1f%%", success_rate);
         sprintf(frag_str, "%.1f%%", stats[i].fragmentation_percentage);
 
         // Print with strings that already include the percent symbol
         printf("%-10s %-15s %-15s %-15d\n",
                strategy_name,
                success_str,
                frag_str,
                stats[i].external_fragmentation);
     }
 }
 
 /**
  * Initialize the memory manager
  */
 void initialize_memory_manager(MemoryManager *manager, AllocationStrategy strategy)
 {
     manager->total_size = memory_size;
     manager->free_size = manager->total_size;
     manager->block_count = 1;
     manager->strategy = strategy;
 
     // Create initial free block covering all memory
     manager->blocks[0].start_address = 0;
     manager->blocks[0].size = manager->total_size;
     manager->blocks[0].is_free = true;
     manager->blocks[0].process_id = -1;
 }
 
 /**
  * Allocate memory for a process using the selected strategy
  */
 bool allocate_memory(MemoryManager *manager, Process *process)
 {
     // Check if enough total memory is available
     if (process->size > manager->free_size)
     {
         return false;
     }
 
     // Find a suitable block based on the allocation strategy
     int block_index = -1;
 
     switch (manager->strategy)
     {
     case FIRST_FIT:
         block_index = find_first_fit(manager, process->size);
         break;
     case BEST_FIT:
         block_index = find_best_fit(manager, process->size);
         break;
     case WORST_FIT:
         block_index = find_worst_fit(manager, process->size);
         break;
     }
 
     if (block_index == -1)
     {
         return false; // No suitable block found
     }
 
     // If the block is larger than needed, split it
     // Only split if the remaining size would be at least 10KB
     if (manager->blocks[block_index].size > process->size + 10)
     {
         if (manager->block_count >= MAX_BLOCKS)
         {
             return false;
         }
 
         // Shift blocks to make space for new one
         for (int i = manager->block_count; i > block_index + 1; i--)
         {
             manager->blocks[i] = manager->blocks[i - 1];
         }
 
         // Create new free block with remaining space
         manager->blocks[block_index + 1].start_address =
             manager->blocks[block_index].start_address + process->size;
         manager->blocks[block_index + 1].size =
             manager->blocks[block_index].size - process->size;
         manager->blocks[block_index + 1].is_free = true;
         manager->blocks[block_index + 1].process_id = -1;
 
         // Resize the original block
         manager->blocks[block_index].size = process->size;
 
         // Increment block count
         manager->block_count++;
     }
 
     // Allocate the block to the process
     manager->blocks[block_index].is_free = false;
     manager->blocks[block_index].process_id = process->id;
     process->block_index = block_index;
     process->state = PROCESS_RUNNING;
     manager->free_size -= process->size;
 
     return true;
 }
 
 /**
  * Deallocate memory for a process and coalesce adjacent free blocks
  */
 void deallocate_memory(MemoryManager *manager, Process *process)
 {
     // If the process has no memory block assigned, exit early
     if (process->block_index == -1)
     {
         return;
     }
 
     int index = process->block_index;
 
     // Mark the memory block as free
     manager->blocks[index].is_free = true;
     manager->blocks[index].process_id = -1;
     manager->free_size += manager->blocks[index].size;
 
     // Mark the process as terminated
     process->state = PROCESS_TERMINATED;
     process->block_index = -1;
 
     // Coalescing: merge adjacent free blocks
     bool merged;
     int coalesce_operations = 0;
 
     printf("\nCoalescing Process: Checking for adjacent free blocks after P%d termination\n", process->id);
 
     do
     {
         merged = false;
 
         // Check each block for possible merging
         for (int i = 0; i < manager->block_count - 1; i++)
         {
             // If this block and the next one are both free, merge them
             if (manager->blocks[i].is_free && manager->blocks[i + 1].is_free)
             {
                 printf("  Coalescing blocks at addresses %d and %d (sizes: %d KB + %d KB = %d KB)\n",
                        manager->blocks[i].start_address,
                        manager->blocks[i + 1].start_address,
                        manager->blocks[i].size,
                        manager->blocks[i + 1].size,
                        manager->blocks[i].size + manager->blocks[i + 1].size);
 
                 // Add the size of the next block to this one
                 manager->blocks[i].size += manager->blocks[i + 1].size;
 
                 // Shift all subsequent blocks down
                 for (int j = i + 1; j < manager->block_count - 1; j++)
                 {
                     manager->blocks[j] = manager->blocks[j + 1];
                 }
 
                 // Update process block indices that are affected by the shift
                 for (int j = 0; j < MAX_PROCESSES; j++)
                 {
                     if (process[j].block_index > i + 1)
                     {
                         process[j].block_index--;
                     }
                 }
 
                 // Decrement the block count
                 manager->block_count--;
                 merged = true;
                 coalesce_operations++;
                 break; // Start the merging process again
             }
         }
     } while (merged);
 
     if (coalesce_operations == 0)
     {
         printf("  No adjacent free blocks found for coalescing\n");
     }
     else
     {
         printf("  Completed %d coalescing operations\n", coalesce_operations);
     }
 }
 
 /**
  * Read processes from input file
  */
 bool read_processes_from_file(const char *filename, Process processes[], int *num_processes, int *memory_size)
 {
     FILE *input_file = fopen(filename, "r");
     if (input_file == NULL)
     {
         fprintf(stderr, "Error: Could not open input file '%s'\n", filename);
         return false;
     }
 
     char line[MAX_LINE_LENGTH];
     int line_number = 0;
 
     // Read first line for memory size
     if (fgets(line, MAX_LINE_LENGTH, input_file) != NULL)
     {
         line_number++;
         int mem_size;
         if (sscanf(line, "%d", &mem_size) == 1)
         {
             *memory_size = mem_size;
         }
     }
 
     // Read subsequent lines for process information
     *num_processes = 0;
     while (fgets(line, MAX_LINE_LENGTH, input_file) != NULL && *num_processes < MAX_PROCESSES)
     {
         line_number++;
 
         // Skip empty lines and comments
         if (line[0] == '\n' || line[0] == '#')
         {
             continue;
         }
 
         // Parse process information: ID Size ArrivalTime Duration
         int id, size, arrival_time = 0, duration = 10;
         int fields = sscanf(line, "%d %d %d %d", &id, &size, &arrival_time, &duration);
 
         if (fields < 2)
         {
             fprintf(stderr, "Warning: Line %d in input file has invalid format, skipping\n", line_number);
             continue;
         }
 
         // Validate process size
         if (size <= 0)
         {
             fprintf(stderr, "Warning: Line %d in input file has invalid process size (%d), skipping\n", line_number, size);
             continue;
         }
 
         // Initialize process
         processes[*num_processes].id = id;
         processes[*num_processes].size = size;
         processes[*num_processes].state = PROCESS_NEW;
         processes[*num_processes].block_index = -1;
 
         (*num_processes)++;
     }
 
     fclose(input_file);
 
     if (*num_processes == 0)
     {
         fprintf(stderr, "Warning: No valid processes found in input file\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * Print a simplified memory state overview
  */
 void print_memory_state_simplified(MemoryManager *manager, Process processes[], int num_processes)
 {
     // Print memory usage summary
     printf("\nMemory Summary: Used: %d KB (%.1f%%), Free: %d KB (%.1f%%)\n",
            manager->total_size - manager->free_size,
            ((double)(manager->total_size - manager->free_size) / manager->total_size) * 100.0,
            manager->free_size,
            ((double)manager->free_size / manager->total_size) * 100.0);
 
     // Count free blocks
     int free_block_count = 0;
     for (int i = 0; i < manager->block_count; i++)
     {
         if (manager->blocks[i].is_free)
             free_block_count++;
     }
 
     // Print blocks info
     printf("Blocks: Total: %d, Free: %d\n", manager->block_count, free_block_count);
 
     // Print process count
     int running = 0, terminated = 0, new_count = 0;
     for (int i = 0; i < num_processes; i++)
     {
         if (processes[i].state == PROCESS_RUNNING)
             running++;
         else if (processes[i].state == PROCESS_TERMINATED)
             terminated++;
         else if (processes[i].state == PROCESS_NEW)
             new_count++;
     }
 
     printf("Processes: Running: %d, Terminated: %d, Unallocated: %d\n",
            running, terminated, new_count);
 }
 
 /**
  * Print the current memory state (detailed version)
  */
 void print_memory_state_detailed(MemoryManager *manager, Process processes[], int num_processes)
 {
     // Print memory allocation table
     printf("\nMemory Allocation Table:\n");
     printf("%-4s %-15s %-12s %-12s\n", "ID", "State", "Size", "Location");
     printf("------------------------------------------\n");
 
     for (int i = 0; i < num_processes; i++)
     {
         if (processes[i].state != PROCESS_NEW)
         {
             const char *state_str = (processes[i].state == PROCESS_RUNNING) ? "Running" : "Terminated";
 
             printf("%-4d %-15s %-12d ",
                    processes[i].id,
                    state_str,
                    processes[i].size);
 
             if (processes[i].block_index != -1)
             {
                 printf("%-12d\n", manager->blocks[processes[i].block_index].start_address);
             }
             else
             {
                 printf("N/A\n");
             }
         }
     }
 
     // Print memory usage summary
     printf("\nMemory Status:\n");
     printf("Total Memory: %d KB, Used: %d KB, Free: %d KB\n",
            manager->total_size,
            manager->total_size - manager->free_size,
            manager->free_size);
 
     // Print block details
     printf("\nBlock List Details:\n");
     printf("%-8s %-8s %-16s %-8s\n", "Start", "Size", "Status", "Process");
     printf("------------------------------------------\n");
 
     for (int i = 0; i < manager->block_count; i++)
     {
         printf("%-8d %-8d %-16s %-8d\n",
                manager->blocks[i].start_address,
                manager->blocks[i].size,
                manager->blocks[i].is_free ? "Free" : "Allocated",
                manager->blocks[i].process_id);
     }
 
     printf("\n");
 }
 
 /**
  * Calculate and update fragmentation metrics
  */
 void update_fragmentation_metrics(MemoryManager *manager, Process processes[],
                                   int num_processes, Statistics *stats)
 {
     // Reset metrics
     stats->external_fragmentation = 0;
     stats->fragmentation_percentage = 0.0;
     stats->avg_fragment_size = 0.0;
 
     int total_free_size = 0;
     int free_block_count = 0;
 
     // Count free blocks
     for (int i = 0; i < manager->block_count; i++)
     {
         if (manager->blocks[i].is_free)
         {
             stats->external_fragmentation++;
             total_free_size += manager->blocks[i].size;
             free_block_count++;
         }
     }
 
     // Calculate average fragment size if there are any fragments
     if (free_block_count > 0)
     {
         stats->avg_fragment_size = (double)total_free_size / free_block_count;
     }
 
     // Calculate fragmentation percentage
     if (manager->free_size > 0)
     {
         // If there's more than one free block, we have fragmentation
         if (free_block_count > 1)
         {
             // Find the largest free block
             int largest_free_block = 0;
             for (int i = 0; i < manager->block_count; i++)
             {
                 if (manager->blocks[i].is_free && manager->blocks[i].size > largest_free_block)
                 {
                     largest_free_block = manager->blocks[i].size;
                 }
             }
 
             // External fragmentation percentage
             stats->fragmentation_percentage =
                 ((double)(manager->free_size - largest_free_block) / manager->free_size) * 100.0;
         }
     }
 }
 
 /**
  * Run a simulation with the specified allocation strategy
  */
 void run_simulation(MemoryManager *manager, AllocationStrategy strategy,
                     Process processes[], int num_processes, Statistics *stats)
 {
     // Initialize statistics
     memset(stats, 0, sizeof(Statistics));
 
     // Print allocation strategy
     printf("\n=== %s Strategy Simulation ===\n",
            strategy == FIRST_FIT ? "First-Fit" : (strategy == BEST_FIT ? "Best-Fit" : "Worst-Fit"));
 
     // --- Phase 1: Initial Process Allocation ---
     printf("\n--- Phase 1: Initial Process Allocation ---\n");
     int num_to_allocate;
     printf("How many processes do you want to allocate initially? (max %d): ", num_processes);
     scanf("%d", &num_to_allocate);
 
     if (num_to_allocate <= 0)
         num_to_allocate = 1;
     else if (num_to_allocate > num_processes)
         num_to_allocate = num_processes;
 
     double total_utilization = 0.0;
     int utilization_samples = 0;
 
     for (int i = 0; i < num_to_allocate; i++)
     {
         stats->allocation_attempts++;
 
         if (allocate_memory(manager, &processes[i]))
         {
             stats->successful_allocations++;
             printf("P%d ", processes[i].id);
         }
         else
         {
             stats->failed_allocations++;
             printf("P%d(FAILED) ", processes[i].id);
         }
     }
     printf("\n");
 
     double current_utilization = (double)(manager->total_size - manager->free_size) / manager->total_size;
     total_utilization += current_utilization;
     utilization_samples++;
     stats->peak_utilization = current_utilization;
 
     print_memory_state_simplified(manager, processes, num_processes);
 
     // --- Phase 2: Process Termination ---
     printf("\n--- Phase 2: Process Termination ---\n");
     printf("Running processes: ");
     int running_count = 0;
     for (int i = 0; i < num_processes; i++)
     {
         if (processes[i].state == PROCESS_RUNNING)
         {
             printf("P%d ", processes[i].id);
             running_count++;
         }
     }
     printf("\n");
 
     if (running_count > 0)
     {
         printf("Enter number of processes to terminate ([0] for none, [-1] for all, [1-%d] for specific processes): ", running_count);
         int num_to_terminate;
         scanf("%d", &num_to_terminate);
 
         if (num_to_terminate == -1)
         {
             printf("Terminating all running processes\n");
             for (int i = 0; i < num_processes; i++)
             {
                 if (processes[i].state == PROCESS_RUNNING)
                 {
                     deallocate_memory(manager, &processes[i]);
                 }
             }
         }
         else
         {
             if (num_to_terminate < 0)
                 num_to_terminate = 0;
             if (num_to_terminate > running_count)
                 num_to_terminate = running_count;
 
             for (int i = 0; i < num_to_terminate; i++)
             {
                 int process_id;
                 printf("Enter process ID to terminate: ");
                 scanf("%d", &process_id);
 
                 bool found = false;
                 for (int j = 0; j < num_processes; j++)
                 {
                     if (processes[j].id == process_id && processes[j].state == PROCESS_RUNNING)
                     {
                         found = true;
                         deallocate_memory(manager, &processes[j]);
                         printf("Terminated P%d\n", process_id);
                         break;
                     }
                 }
 
                 if (!found)
                 {
                     printf("P%d not found or not running\n", process_id);
                 }
             }
         }
     }
     else
     {
         printf("No running processes to terminate.\n");
     }
 
     current_utilization = (double)(manager->total_size - manager->free_size) / manager->total_size;
     total_utilization += current_utilization;
     utilization_samples++;
 
     print_memory_state_simplified(manager, processes, num_processes);
 
     // --- Phase 3: Additional Process Allocation ---
     printf("\n--- Phase 3: Additional Process Allocation ---\n");
     printf("Remaining unallocated processes: ");
     int unallocated_count = 0;
     for (int i = 0; i < num_processes; i++)
     {
         if (processes[i].state == PROCESS_NEW)
         {
             printf("P%d ", processes[i].id);
             unallocated_count++;
         }
     }
     printf("\n");
 
     if (unallocated_count > 0)
     {
         printf("How many more processes do you want to allocate ([0] for none, [1-%d] for specific processes): ", unallocated_count);
         int more_to_allocate;
         scanf("%d", &more_to_allocate);
 
         if (more_to_allocate < 0)
             more_to_allocate = 0;
         if (more_to_allocate > unallocated_count)
             more_to_allocate = unallocated_count;
 
         int allocated_count = 0;
 
         for (int i = 0; i < num_processes && allocated_count < more_to_allocate; i++)
         {
             if (processes[i].state == PROCESS_NEW)
             {
                 stats->allocation_attempts++;
 
                 if (allocate_memory(manager, &processes[i]))
                 {
                     stats->successful_allocations++;
                     printf("P%d ", processes[i].id);
                 }
                 else
                 {
                     stats->failed_allocations++;
                     printf("P%d(FAILED) ", processes[i].id);
                 }
 
                 allocated_count++;
             }
         }
         printf("\n");
     }
     else
     {
         printf("No more processes to allocate.\n");
     }
 
     current_utilization = (double)(manager->total_size - manager->free_size) / manager->total_size;
     total_utilization += current_utilization;
     utilization_samples++;
     if (current_utilization > stats->peak_utilization)
     {
         stats->peak_utilization = current_utilization;
     }
 
     print_memory_state_simplified(manager, processes, num_processes);
 
     // --- Phase 4: Large Process Allocation ---
     printf("\n--- Phase 4: Large Process Allocation ---\n");
     float percent_input = 0.0f;
     do
     {
         printf("Enter size for a large process (P9999) allocation (as %% of available free memory, 1–100): ");
         if (scanf("%f", &percent_input) != 1)
         {
             printf("Invalid input. Please enter a number.\n");
             // Clear invalid input from buffer
             while (getchar() != '\n')
                 ;
             percent_input = 0.0f; // force retry
         }
         else if (percent_input < 1.0f || percent_input > 100.0f)
         {
             printf("Please enter a valid percentage between 1 and 100.\n");
             percent_input = 0.0f; // force retry
         }
     } while (percent_input == 0.0f);
 
     int large_size = (int)((manager->free_size * percent_input) / 100.0f);
 
     Process large_process;
     large_process.id = 9999; // the largest process ID
     large_process.size = large_size;
     large_process.state = PROCESS_NEW;
     large_process.block_index = -1;
 
     stats->allocation_attempts++;
     printf("Attempting large allocation (P9999, %dKB - %.2f%% of availablr free memory): ", large_process.size, percent_input);
 
     if (allocate_memory(manager, &large_process))
     {
         stats->successful_allocations++;
         printf("SUCCESS\n");
         processes[num_processes] = large_process;
         num_processes++;
     }
     else
     {
         stats->failed_allocations++;
         printf("FAILED (not enough contiguous space)\n");
     }
 
     current_utilization = (double)(manager->total_size - manager->free_size) / manager->total_size;
     total_utilization += current_utilization;
     utilization_samples++;
     if (current_utilization > stats->peak_utilization)
     {
         stats->peak_utilization = current_utilization;
     }
 
     if (utilization_samples > 0)
     {
         stats->avg_utilization = total_utilization / utilization_samples;
     }
 
     update_fragmentation_metrics(manager, processes, num_processes, stats);
     print_memory_state_simplified(manager, processes, num_processes);
 
     printf("\n--- Final Memory State (Detailed) ---\n");
     print_memory_state_detailed(manager, processes, num_processes);
 
     // Final statistics for this simulation
     printf("\n--- Final Results (%s) ---\n",
            strategy == FIRST_FIT ? "First-Fit" : (strategy == BEST_FIT ? "Best-Fit" : "Worst-Fit"));
 
     printf("Success Rate: %.1f%% (%d/%d)\n",
            ((double)stats->successful_allocations / stats->allocation_attempts) * 100.0,
            stats->successful_allocations, stats->allocation_attempts);
     printf("Peak Memory Usage: %.1f%%\n", stats->peak_utilization * 100.0);
     printf("Fragmentation: %.1f%%\n", stats->fragmentation_percentage);
     printf("Final Block Count: %d\n", manager->block_count);
 
     printf("\n--- %s Simulation Completed ---\n",
            strategy == FIRST_FIT ? "First-Fit" : (strategy == BEST_FIT ? "Best-Fit" : "Worst-Fit"));
     printf("\n\n****************************************************************************************************************************\n\n");
 }