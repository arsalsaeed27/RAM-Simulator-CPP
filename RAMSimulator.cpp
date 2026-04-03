#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

enum class AllocState
{
    FREE,
    OCCUPIED
};

struct MemChunk
{
    int chunkID, startAddr, sizeKB, ownerProcessID;
    AllocState state;
    MemChunk *next;
    MemChunk *prev;

    MemChunk(int id, int start, int size, AllocState s, int pid = -1) : chunkID(id), startAddr(start), sizeKB(size), state(s), ownerProcessID(pid), prev(NULL), next(NULL)
    {
    }
};

class RAMSimulator
{
private:
    MemChunk *head;
    MemChunk *tail;
    MemChunk *lastAllocNode;
    int idCounter;

    void executeAllocation(MemChunk *current, int processID, int requestedSize, const string &algoName)
    {
        int remainder = current->sizeKB - requestedSize;
        if (remainder > 20)
        {
            MemChunk *splitChunk = new MemChunk(idCounter++, current->startAddr + requestedSize, remainder, AllocState::FREE);
            splitChunk->next = current->next;
            splitChunk->prev = current;

            if (current->next != NULL)
                current->next->prev = splitChunk;
            else
                tail = splitChunk;

            current->next = splitChunk;
            current->sizeKB = requestedSize;

            cout << "[" << algoName << "] Process P" << processID << " allocated " << requestedSize
                 << " KB. Chunk " << current->chunkID << " split! Remainder: " << remainder << " KB." << endl;
        }
        else
        {
            cout << "[" << algoName << "] Process P" << processID << " allocated " << requestedSize
                 << " KB in Chunk " << current->chunkID << ". (No split)." << endl;
        }
        current->state = AllocState::OCCUPIED;
        current->ownerProcessID = processID;
        lastAllocNode = current;
    }

public:
    RAMSimulator() : head(NULL), tail(NULL), idCounter(0) {}
    ~RAMSimulator()
    {
        clearPool();
    }

    void clearPool() // its just a "delete node" function for a DDLL
    {
        MemChunk *current = head;
        while (current != NULL)
        {
            MemChunk *toDelete = current;
            current = current->next;
            delete toDelete;
        }
        head = NULL;
        tail = NULL;
        lastAllocNode = NULL;
    }

    void addChunk(int size) // its like a  "add node" function for a DDLL
    {
        int startAddress = 0;
        if (tail != NULL)
        {
            startAddress = tail->startAddr + tail->sizeKB;
        }
        MemChunk *newChunk = new MemChunk(idCounter++, startAddress, size, AllocState::FREE);

        if (head == NULL)
        {
            head = tail = newChunk;
        }
        else
        {
            tail->next = newChunk;
            newChunk->prev = tail;
            tail = newChunk;
        }
    }

    void initializePool() // used to initialize the sizes to get 1024kb requirement
    {
        clearPool();
        idCounter = 0;

        int initialSizes[] = {100, 50, 120, 200, 30, 80, 150, 44, 200, 50};
        for (int i = 0; i < 10; i++)
        {
            addChunk(initialSizes[i]);
        }
        lastAllocNode = head;
    }
    bool allocateFirstFit(int processID, int requestedSize)
    {
        MemChunk *current = head;

        while (current != nullptr)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                executeAllocation(current, processID, requestedSize, "First-Fit");
                return true; // means it was a success
            }
            current = current->next;
        }
        cout << "[ERROR] Allocation FAILED for Process P" << processID << " (" << requestedSize << " KB). No contiguous block large enough." << endl;
        return false; // meaning that it was a ffailure
    }

    bool allocateNextFit(int processID, int requestedSize)
    {
        if (lastAllocNode == NULL)
            lastAllocNode = head;
        MemChunk *current = lastAllocNode;
        while (current != NULL)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                executeAllocation(current, processID, requestedSize, "Next-fit");
                return true;
            }
            current = current->next;
        }
        current = head;
        while (current != lastAllocNode && current != NULL)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                executeAllocation(current, processID, requestedSize, "Next-Fit");
                return true;
            }
            current = current->next;
        }
        cout << "ERROR NEXT FIT FAIELD FOR P" << processID << " (" << requestedSize << " KB)" << endl;
        return false;
    }

    bool allocateBestFit(int processID, int requestedSize)
    {
        MemChunk *current = head;
        MemChunk *bestFit = NULL;
        int minDifference = 9999999; // just a random number

        while (current != NULL)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                int diff = current->sizeKB - requestedSize;
                if (diff < minDifference)
                {
                    minDifference = diff;
                    bestFit = current;
                }
            }
            current = current->next;
        }
        if (bestFit != NULL)
        {
            executeAllocation(bestFit, processID, requestedSize, "Best-Fit");
            return true;
        }
        cout << "ERROR BEST FIT FAILED FOR P" << processID << " (" << requestedSize << " KB)" << endl;
        return false;
    }

    bool allocateWorstFit(int processID, int requestedSize)
    {
        MemChunk *current = head;
        MemChunk *worstFit = NULL;
        int maxDifference = -1;

        while (current != NULL)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                int diff = current->sizeKB - requestedSize;
                if (diff > maxDifference)
                {
                    maxDifference = diff;
                    worstFit = current;
                }
            }
            current = current->next;
        }
        if (worstFit != NULL)
        {
            executeAllocation(worstFit, processID, requestedSize, "Worst-Fit");
            return true;
        }
        cout << "ERROR WORST FIT FAILED FOR P" << processID << " (" << requestedSize << "KB" << endl;
        return false;
    }
    void deallocate(int processID)
    {
        MemChunk *current = head;
        bool found = false;

        while (current != NULL)
        {
            if (current->state == AllocState::OCCUPIED && current->ownerProcessID == processID)
            {
                cout << " DEALLOCATED Process P" << processID << " released Chunk" << current->chunkID << " (" << current->sizeKB << " KB)." << endl;

                current->state = AllocState::FREE;
                current->ownerProcessID = -1;
                found = true;
                current = coalesce(current);
            }
            current = current->next;
        }
        if (!found)
        {
            cout << "ERROR in DEALLOCATION. Process P" << processID << " not found in memory" << endl;
        }
    }

    MemChunk *coalesce(MemChunk *current) // this func will merge free nearby blocks
    {
        MemChunk *survivingNode = current;
        // for merging right side
        if (current->next != NULL && current->next->state == AllocState ::FREE)
        {
            MemChunk *rightChunk = current->next;
            cout << "  -> MERGE Coalescing Chunk" << current->chunkID << " with adjacent right chunk " << rightChunk->chunkID << endl;

            if (lastAllocNode == rightChunk)
                lastAllocNode = current;

            current->sizeKB += rightChunk->sizeKB;
            current->next = rightChunk->next;

            if (rightChunk->next != NULL)
                rightChunk->next->prev = current;
            else
                tail = current;

            delete rightChunk;
        }
        // for merging left side
        if (current->prev != NULL && current->prev->state == AllocState ::FREE)
        {
            MemChunk *leftChunk = current->prev;
            cout << "  -> MERGE Coalescing Left Chunk" << leftChunk->chunkID << " with chunk " << current->chunkID << endl;
            if (lastAllocNode == current)
                lastAllocNode = leftChunk;

            leftChunk->sizeKB += current->sizeKB;
            leftChunk->next = current->next;

            if (current->next != NULL)
                current->next->prev = leftChunk;
            else
                tail = leftChunk;

            delete current;
            survivingNode = leftChunk;
        }
        return survivingNode;
    }

    void printReport(const string &algoName)
    {
        int totalFree = 0, freeBlocksCount = 0, largestFree = 0;
        MemChunk *current = head;
        while (current != NULL)
        {
            if (current->state == AllocState::FREE)
            {
                totalFree += current->sizeKB;
                freeBlocksCount++;
                if (current->sizeKB > largestFree)
                    largestFree = current->sizeKB;
            }
            current = current->next;
        }
        cout << "\n=== " << algoName << " FRAGMENTATION REPORT ===" << endl;
        cout << "Total Free Memory  : " << totalFree << " KB" << endl;
        cout << "Free Blocks Count  : " << freeBlocksCount << endl;
        cout << "Largest Free Block : " << largestFree << " KB" << endl;
        cout << "External Frag.     : " << (freeBlocksCount > 1 ? "YES (Free space is divided)" : "NO (Free space is contiguous)") << endl;
        cout << "=======================================\n"
             << endl;
    }

    void renderGUI(const string &filename, const string &stepDescription)
    {
        ofstream htmlFile(filename);

        htmlFile << "<!DOCTYPE html><html><head><title>RAM Simulator GUI</title>" << endl;
        htmlFile << "<style>" << endl;
        htmlFile << "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #1e1e2e; color: #cdd6f4; padding: 20px; }" << endl;
        htmlFile << "h2 { color: #89b4fa; border-bottom: 2px solid #313244; padding-bottom: 10px; }" << endl;
        htmlFile << ".memory-board { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 20px; padding: 15px; background: #181825; border-radius: 8px; }" << endl;
        htmlFile << ".chunk { display: flex; flex-direction: column; justify-content: center; align-items: center; border-radius: 6px; padding: 10px; font-size: 14px; font-weight: bold; border: 2px solid #11111b; transition: transform 0.2s; }" << endl;
        htmlFile << ".chunk:hover { transform: scale(1.05); }" << endl;
        htmlFile << ".free { background-color: #a6e3a1; color: #11111b; }" << endl;
        htmlFile << ".occupied { background-color: #f38ba8; color: #11111b; }" << endl;
        htmlFile << ".details { font-size: 11px; font-weight: normal; margin-top: 5px; opacity: 0.8; }" << endl;
        htmlFile << "</style></head><body>" << endl;

        htmlFile << "<h2>State: " << stepDescription << "</h2>" << endl;
        htmlFile << "<div class='memory-board'>" << endl;

        MemChunk *current = head;
        while (current != nullptr)
        {
            int visualWidth = std::max(60, std::min(current->sizeKB * 2, 300));
            string statusClass = (current->state == AllocState::FREE) ? "free" : "occupied";
            string statusText = (current->state == AllocState::FREE) ? "FREE" : "P" + to_string(current->ownerProcessID);

            htmlFile << "<div class='chunk " << statusClass << "' style='width: " << visualWidth << "px;'>" << endl;
            htmlFile << "<div>" << current->sizeKB << " KB</div>" << endl;
            htmlFile << "<div class='details'>" << statusText << " | Addr: " << current->startAddr << "</div>" << endl;
            htmlFile << "<div class='details'>ID: [BLK " << current->chunkID << "]</div>" << endl;
            htmlFile << "</div>" << endl;

            current = current->next;
        }

        htmlFile << "</div></body></html>" << endl;
        htmlFile.close();

        cout << ">>> GUI updated! Open '" << filename << "' in your web browser to see the visual layout." << endl;
    }
};
int main()
{

    srand(static_cast<unsigned>(time(0)));
    RAMSimulator sim;

    cout << "===== DYNAMIC MEMORY ALLOCATION STRESS TEST =====" << endl;

    string algorithms[] = {"First-Fit", "Next-Fit", "Best-Fit", "Worst-Fit"};

    for (int algoIndex = 0; algoIndex < 4; algoIndex++)
    {

        cout << "\n\n>>> EXECUTING: " << algorithms[algoIndex] << " <<<" << endl;
        sim.initializePool();

        // stress Test :10 random allocations
        for (int i = 1; i <= 10; i++)
        {
            int randomSize = 10 + (rand() % 141); // to get a random size between 10 and 150 KB

            if (algoIndex == 0)
                sim.allocateFirstFit(i, randomSize);
            else if (algoIndex == 1)
                sim.allocateNextFit(i, randomSize);
            else if (algoIndex == 2)
                sim.allocateBestFit(i, randomSize);
            else if (algoIndex == 3)
                sim.allocateWorstFit(i, randomSize);
        }

        sim.printReport(algorithms[algoIndex]);

        string filename = "gui_stress_test_" + algorithms[algoIndex] + ".html";
        sim.renderGUI(filename, "Aftermath of Stress Test using " + algorithms[algoIndex]);
    }

    return 0;
}