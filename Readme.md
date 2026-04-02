# RAMSimulator: Dynamic Memory Allocation OS Engine

## Commit 1: Core Architecture and GUI Engine Setup
**Changes in this commit:**
* Initialized the `RAMSimulator` class to replace standard memory manager naming conventions.
* Defined the `MemChunk` structure as a doubly linked list to support future O(1) coalescing (merging).
* Implemented `initializePool()` to generate 10 unique memory blocks totaling exactly 1024 KB, satisfying the lab rubric.
* Built the `renderGUI()` function. Instead of a standard CLI output, the C++ engine now generates a dynamic `index.html` file using CSS flexbox. This provides a visually appealing, browser-based GUI to track memory state, fragmentation, and block details without relying on external C++ UI libraries.

## Commit 2: First-Fit Algorithm & Block Splitting
**Changes in this commit:**
* Implemented `allocateFirstFit(int processID, int requestedSize)`. This function scans the memory pool from the beginning (`head`) to find the first available chunk that fits the request.
* Implemented the **Block Splitting Requirement**: If the selected free chunk is significantly larger than the request (difference > 20 KB), the chunk is split. A new `MemChunk` node is dynamically created for the remainder, its new starting address is calculated, and it is cleanly spliced into the doubly linked list.
* Updated `main()` to simulate several allocation requests, including one that triggers a split and one that fits exactly, generating a new GUI state to visualize the fragmentation.

## Commit 3: Deallocation & Block Coalescing (Merging)
**Changes in this commit:**
* Implemented `deallocate(int processID)` to search the memory pool and release all chunks owned by a specific process.
* Implemented `coalesce(MemChunk* chunk)`, satisfying the rubric's merging requirement. When a chunk is freed, this function performs an $O(1)$ check of the `prev` and `next` pointers. If adjacent chunks are also free, they are spliced together, their sizes combined, and the redundant `MemChunk` nodes are cleanly deleted from memory.
* Added console alerts to print a specific message whenever a merge occurs, exactly as the assignment requires.
* Updated `main()` to simulate deallocations that trigger right-side merges, left-side merges, and double-sided merges to prove the robustness of the algorithm.

**Next Steps:** Implement the Next-Fit, Best-Fit, and Worst-Fit search algorithms, and build the final Stress Test loop.