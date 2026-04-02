#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
    int idCounter;

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
    }
    bool allocateFirstFit(int processID, int requestedSize)
    {
        MemChunk *current = head;

        while (current != nullptr)
        {
            if (current->state == AllocState::FREE && current->sizeKB >= requestedSize)
            {
                int remainder = current->sizeKB - requestedSize;

                if (remainder > 20) // we will split if diff > 20
                {
                    MemChunk *splitChunk = new MemChunk( // creating a new remainder chunk
                        idCounter++,
                        current->startAddr + requestedSize,
                        remainder,
                        AllocState::FREE);

                    splitChunk->next = current->next; // connecting  it into theDLL
                    splitChunk->prev = current;

                    if (current->next != nullptr)
                    {
                        current->next->prev = splitChunk;
                    }
                    else
                    {
                        tail = splitChunk; // updating tail if we split the very last block
                    }
                    current->next = splitChunk;

                    // shrinking the current block to the requested size
                    current->sizeKB = requestedSize;

                    cout << "[FIRST-FIT] Process P" << processID << " allocated " << requestedSize
                         << " KB. Chunk " << current->chunkID << " split! Remainder: " << remainder << " KB." << endl;
                }
                else
                {
                    cout << "[FIRST-FIT] Process P" << processID << " allocated " << requestedSize
                         << " KB in Chunk " << current->chunkID << ". (No split, internal frag: " << remainder << " KB)." << endl;
                }

                // marking the chunk as occupied
                current->state = AllocState::OCCUPIED;
                current->ownerProcessID = processID;
                return true; // meaning that it was a success
            }
            current = current->next;
        }

        cout << "[ERROR] Allocation FAILED for Process P" << processID << " (" << requestedSize << " KB). No contiguous block large enough." << endl;
        return false; // meaning that it was a ffailure
    }
    void deallocate(int processID)
    {
        MemChunk *current = head;
        bool found = false;

        while (current != NULL)
        {
            MemChunk *nextNode = current->next;
            if (current->state == AllocState::OCCUPIED && current->ownerProcessID == processID)
            {
                cout << " DEALLOCATED Process P" << processID << " released Chunk" << current->chunkID << " (" << current->sizeKB << " KB)." << endl;

                current->state = AllocState::FREE;
                current->ownerProcessID = -1;
                found = true;
                coalesce(current);
            }
            current = nextNode;
        }
        if (!found)
        {
            cout << "ERROR in DEALLOCATION. Process P" << processID << " not found in memory" << endl;
        }
    }

    void coalesce(MemChunk *current) // this func will merge free nearby blocks
    {
        if (current->next != NULL && current->next->state == AllocState ::FREE)
        {
            MemChunk *rightChunk = current->next;
            cout << "  -> MERGE Coalescing Chunk" << current->chunkID << " with adjacent right chunk " << rightChunk->chunkID << endl;

            current->sizeKB += rightChunk->sizeKB;
            current->next = rightChunk->next;

            if (rightChunk->next != NULL)
                rightChunk->next->prev = current;
            else
                tail = current;

            delete rightChunk;
        }
        if (current->prev != NULL && current->prev->state == AllocState ::FREE)
        {
            MemChunk *leftChunk = current->prev;
            cout << "  -> MERGE Coalescing Left Chunk" << leftChunk->chunkID << " with chunk " << current->chunkID << endl;

            leftChunk->sizeKB += current->sizeKB;
            leftChunk->next = current->next;

            if (current->next != NULL)
                current->next->prev = leftChunk;
            else
                tail = leftChunk;

            delete current;
        }
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
    RAMSimulator sim;

    cout << "--- STEP 1: INITIALIZE ---" << endl;
    sim.initializePool();
    sim.renderGUI("gui_01_initial.html", "Initial Memory Layout");
    cout << endl;

    cout << "--- STEP 2: ALLOCATIONS (SPLITTING) ---" << endl;
    sim.allocateFirstFit(1, 40);
    sim.allocateFirstFit(2, 15);
    sim.allocateFirstFit(3, 40);
    sim.renderGUI("gui_02_allocated.html", "Memory After Allocations");
    cout << endl;

    cout << "--- STEP 3: DEALLOCATIONS (MERGING) ---" << endl;
    // Free P2 first. It is between P1 and P3, so it will NOT merge yet (neighbors are occupied).
    sim.deallocate(2);

    // Now free P1. It is on the left of P2's newly freed block. It SHOULD trigger a right-merge!
    sim.deallocate(1);

    // Now free P3. It is on the right of the giant P1+P2 free block. It SHOULD trigger a left-merge!
    sim.deallocate(3);
    cout << endl;

    sim.renderGUI("gui_03_coalesced.html", "Memory After Deallocations & Coalescing");

    return 0;
}