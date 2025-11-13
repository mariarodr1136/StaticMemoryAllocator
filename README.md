# Static Memory Allocation Simulator 🧠

The **Static Memory Allocation Simulator** is an educational **C application** designed to demonstrate different contiguous memory allocation techniques used in operating systems. By implementing **First Fit**, **Best Fit**, and **Worst Fit** allocation algorithms, this simulator provides valuable insights into how memory allocation works, the impact of different strategies on memory utilization, and the effects of **memory fragmentation**. The application features an interactive interface that allows users to allocate processes, deallocate them, and observe memory state changes in real-time. Through comprehensive visualizations and statistics, users can gain a deeper understanding of memory management concepts essential to operating system design and optimization.

The ultimate goal is to provide students and professionals with a **practical learning tool** to explore memory allocation dynamics, understand fragmentation issues, and compare the effectiveness of different allocation strategies in various scenarios. 💾

---

![C](https://img.shields.io/badge/C-Programming_Language-A8B9CC) ![Memory Allocation](https://img.shields.io/badge/Memory-Allocation-orange) ![OS Concepts](https://img.shields.io/badge/OS-Concepts-lightgrey) ![First Fit](https://img.shields.io/badge/First_Fit-Algorithm-blue) ![Best Fit](https://img.shields.io/badge/Best_Fit-Algorithm-green) ![Worst Fit](https://img.shields.io/badge/Worst_Fit-Algorithm-red)

---

<img width="892" alt="Screenshot 2025-04-13 at 9 04 02 PM" src="https://github.com/user-attachments/assets/b3278468-f8b8-4bbe-86d2-fd2b032fc6f8" />
<img width="893" alt="Screenshot 2025-04-13 at 9 04 38 PM" src="https://github.com/user-attachments/assets/7d9a5548-f455-481e-a358-3ad8732c9b0f" />

---

### Table of Contents
- [Features](#-features)
- [Installation](#-installation)
- [Usage](#usage)
- [Simulation Phases](#-simulation-phases)
- [Algorithms Explained](#-algorithms-explained)
- [File Format](#-file-format)
- [Sample Output](#-sample-output)
- [Code Structure](#code-structure)
- [Development](#-development)
- [Contributing](#contributing)
- [Contact](#contact-)

---

### 🚀 Features

- **Multiple Allocation Strategies**: Compare First Fit, Best Fit, and Worst Fit algorithms
- **Interactive Simulation**: Step through different phases of memory allocation and deallocation
- **Memory Visualization**: View simplified and detailed memory state representations
- **Coalescing**: Automatic merging of adjacent free memory blocks to reduce fragmentation
- **Performance Metrics**: Track statistics like allocation success rates and fragmentation percentages
- **Process Management**: Allocate, deallocate, and track the state of processes
- **Customizable Scenarios**: Import process configurations from input files

---

### 📥 Installation

#### Prerequisites
- GCC compiler
- Make (optional)
- Linux/Unix environment (recommended) or Windows with MinGW

#### Compilation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mariarodr1136/StaticMemoryAllocator.git
   cd StaticMemoryAllocator
   cd CODE
   ```

2. **Compile the source code**:
   ```bash
   gcc static_memory_allocation.c -o static_memory_allocation -Wall
   ```

   Alternatively, a Makefile is available:
   ```bash
   make
   ```

---


### Usage

1. **Prepare an input file** (see [File Format](#-file-format) section) or use the default `input.txt`.

2. **Run the program**:
   ```bash
   ./memory_allocator
   ```
   If no input file is specified, the program will look for `input.txt` in the current directory.

3. **Follow the interactive prompts** to allocate and deallocate processes through the simulation phases.

---

### 🔄 Simulation Phases

The simulation runs through four distinct phases for each allocation strategy:

1. **Initial Process Allocation**:
   - Select how many processes to allocate initially
   - Observe the success or failure of each allocation attempt

2. **Process Termination**:
   - Choose processes to terminate (deallocate)
   - Observe memory coalescing as adjacent free blocks merge

3. **Additional Process Allocation**:
   - Allocate previously unallocated processes
   - Observe how fragmentation affects new allocations

4. **Large Process Allocation**:
   - Attempt to allocate a large process that takes a percentage of available free memory
   - Test the limits of contiguous allocation with current fragmentation state

After all phases, the program presents a summary comparing the performance of all three allocation strategies.

---

### 🧩 Algorithms Explained

#### First Fit
Allocates the first free block found that is large enough to accommodate the process. This is typically the fastest algorithm but may lead to fragmentation at the beginning of memory.

```c
int find_first_fit(MemoryManager *manager, int size)
{
    for (int i = 0; i < manager->block_count; i++) {
        if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
            return i;
        }
    }
    return -1;
}
```

#### Best Fit
Allocates the smallest free block that is large enough to accommodate the process. This minimizes wasted space but may lead to unusable small fragments.

```c
int find_best_fit(MemoryManager *manager, int size)
{
    int best_fit_index = -1;
    int min_size_diff = INT_MAX;
    
    for (int i = 0; i < manager->block_count; i++) {
        if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
            int size_diff = manager->blocks[i].size - size;
            if (size_diff < min_size_diff) {
                min_size_diff = size_diff;
                best_fit_index = i;
            }
        }
    }
    
    return best_fit_index;
}
```

#### Worst Fit
Allocates the largest free block for the process. This aims to leave larger leftover blocks which are more likely to be usable for future allocations.

```c
int find_worst_fit(MemoryManager *manager, int size)
{
    int worst_fit_index = -1;
    int max_size_diff = -1;
    
    for (int i = 0; i < manager->block_count; i++) {
        if (manager->blocks[i].is_free && manager->blocks[i].size >= size) {
            int size_diff = manager->blocks[i].size - size;
            if (size_diff > max_size_diff) {
                max_size_diff = size_diff;
                worst_fit_index = i;
            }
        }
    }
    
    return worst_fit_index;
}
```

#### Memory Coalescing
After deallocating a process, the simulator attempts to merge adjacent free blocks to reduce fragmentation.

```c
bool coalesce_memory(MemoryManager *manager, Process processes[])
{
    bool merged = false;
    int i = 0;
    
    while (i < manager->block_count - 1) {
        if (manager->blocks[i].is_free && manager->blocks[i + 1].is_free) {
            manager->blocks[i].size += manager->blocks[i + 1].size;
            // Shift remaining blocks and update process indices
            // ...
            manager->block_count--;
            merged = true;
        } else {
            i++;
        }
    }
    
    return merged;
}
```

---

### 📄 File Format

The input file format is as follows:

```
[Memory Size in KB]
[Process ID] [Process Size in KB] [optional: Arrival Time] [optional: Duration]
[Process ID] [Process Size in KB] [optional: Arrival Time] [optional: Duration]
...
```

Example `input.txt`:
```
1024
1 150
2 300
3 200
4 120
5 250
```

In this example:
- Total memory size is 1024 KB
- 5 processes are defined with IDs 1-5 and respective sizes

---

### 📊 Sample Output

```
===== STATIC MEMORY ALLOCATION SIMULATION =====

Input file: input.txt
Memory size: 1024 KB
Number of processes: 5

-------------------------------------------------
Processes Loaded:
ProcessID  Size (KB)
-------------------------------------------------
1          150
2          300
3          200
4          120
5          250

=== First-Fit Strategy Simulation ===

--- Phase 1: Initial Process Allocation ---
How many processes do you want to allocate initially? (max 5): 3
P1 P2 P3 

Memory Summary: Used: 650 KB (63.5%), Free: 374 KB (36.5%)
Blocks: Total: 2, Free: 1
Processes: Running: 3, Terminated: 0, Unallocated: 2

--- Phase 2: Process Termination ---
...

=== Summary of Allocation Methods ===
Strategy  Success Rate    Fragmentation   Block Count
----------------------------------------------------------
First Fit 85.7%          12.5%           3
Best Fit  85.7%          8.3%            4
Worst Fit 71.4%          33.8%           2
```

---

### Code Structure

The simulator is organized into the following components:

- **Data Structures**:
  - `MemoryBlock`: Represents a block of memory (free or allocated)
  - `Process`: Represents a process with memory requirements
  - `MemoryManager`: Manages memory blocks and allocation strategies
  - `Statistics`: Tracks performance metrics for each strategy

- **Core Functions**:
  - `find_first_fit`: Implements First Fit allocation algorithm
  - `find_best_fit`: Implements Best Fit allocation algorithm
  - `find_worst_fit`: Implements Worst Fit allocation algorithm
  - `allocate_memory`: Allocates memory to a process
  - `deallocate_memory`: Frees memory when a process terminates
  - `coalesce_memory`: Merges adjacent free blocks

- **Utility Functions**:
  - `read_processes_from_file`: Parses input file
  - `print_memory_state_simplified`: Displays simplified memory status
  - `print_memory_state_detailed`: Shows detailed memory block information
  - `run_simulation`: Executes the simulation phases

---

### 🔧 Development

To extend or modify the simulator:

1. **Add New Allocation Strategies**:
   - Create a new finder function (similar to existing ones)
   - Add a new entry to the `AllocationStrategy` enum
   - Update the `allocate_memory` function to use the new strategy

2. **Enhance Visualization**:
   - Modify the `print_memory_state_*` functions to add graphical representations

3. **Additional Statistics**:
   - Extend the `Statistics` structure with new metrics
   - Update the `update_fragmentation_metrics` function

---

### Contributing 
Feel free to submit issues or pull requests for improvements or bug fixes. You can also open issues to discuss potential changes or enhancements. All contributions are welcome to enhance the app’s features or functionality!

To contribute, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix:
   ```bash
   git checkout -b feat/your-feature-name
- Alternatively, for bug fixes:
   ```bash
   git checkout -b fix/your-bug-fix-name
3. Make your changes and run all tests before committing the changes and make sure all tests are passed.
4. After all tests are passed, commit your changes with descriptive messages:
   ```bash
   git commit -m 'add your commit message'
5. Push your changes to your forked repository:
   ```bash
   git push origin feat/your-feature-name.
6. Submit a pull request to the main repository, explaining your changes and providing any necessary details.

---

### Contact 🌐
If you have any questions or feedback, feel free to reach out at [mrodr.contact@gmail.com](mailto:mrodr.contact@gmail.com).
