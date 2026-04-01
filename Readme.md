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

**Next Steps:** Implement Deallocation and the Block Coalescing (Merging) logic to fuse adjacent free blocks.