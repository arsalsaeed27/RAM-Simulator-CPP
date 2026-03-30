# RAMSimulator: Dynamic Memory Allocation OS Engine

## Commit 1: Core Architecture and GUI Engine Setup
**Changes in this commit:**
* Initialized the `RAMSimulator` class to replace standard memory manager naming conventions.
* Defined the `MemChunk` structure as a doubly linked list to support future O(1) coalescing (merging).
* Implemented `initializePool()` to generate 10 unique memory blocks totaling exactly 1024 KB, satisfying the lab rubric.
* Built the `renderGUI()` function. Instead of a standard CLI output, the C++ engine now generates a dynamic `index.html` file using CSS flexbox. This provides a visually appealing, browser-based GUI to track memory state, fragmentation, and block details without relying on external C++ UI libraries.
