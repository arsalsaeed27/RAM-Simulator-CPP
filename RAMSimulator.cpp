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
            // Calculate visual width based on size (min 60px, max 300px)
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

    // Step 1: Initialize the memory pool
    sim.initializePool();

    // Step 2: Generate the GUI file
    sim.renderGUI("gui_output.html", "Initial Memory Layout (10 Blocks, 1024 KB Total)");

    return 0;
}