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

    MemChunk(int id, int start, int size, AllocState s, int pid = -1) : chunkID(id), startAddr(start), sizeKB(size), state(s), ownerProcessID(pid), prev(nullptr), next(nullptr)
    {
    }
};

class RAMSimulator
{
private:
    MemChunk *head;
    MemChunk *tail;
    int idCounter;
}