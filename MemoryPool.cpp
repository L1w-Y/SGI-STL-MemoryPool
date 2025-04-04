//
// Created by Administrator on 25-4-3.
//

#include "MemoryPool.h"
void* MemoryPool::allocate(size_t n) {
    if (n > MAX_BYTES) {
        return std::malloc(n);  // 直接使用 malloc
    }

    Obj** myFreeList = &freeList[freeListIndex(n)];
    Obj* result = *myFreeList;

    if (!result) {
        return refill(roundUp(n));
    }

    *myFreeList = result->next;
    return result;
}

void MemoryPool::deallocate(void* p, size_t n) {
    if (n > MAX_BYTES) {
        std::free(p);
        return;
    }

    Obj** myFreeList = &freeList[freeListIndex(n)];
    Obj* q = (Obj*)p;

    q->next = *myFreeList;
    *myFreeList = q;
}

void* MemoryPool::refill(size_t n) {
    int nobjs = 20;  // 申请多个对象，减少堆分配次数
    char* chunk = chunkAlloc(n, nobjs);
    if (!chunk) return nullptr;

    Obj** myFreeList = &freeList[freeListIndex(n)];
    Obj* result = (Obj*)chunk;
    Obj* nextObj;

    if (nobjs == 1) return result;

    *myFreeList = nextObj = (Obj*)(chunk + n);
    for (int i = 1; ; ++i) {
        Obj* current = nextObj;
        nextObj = (Obj*)((char*)nextObj + n);
        if (i == nobjs - 1) {
            current->next = nullptr;
            break;
        }
        else {
            current->next = nextObj;
        }
    }

    return result;
}

char* MemoryPool::chunkAlloc(size_t size, int& nobjs) {
    size_t totalBytes = size * nobjs;
    size_t bytesLeft = endFree - startFree;

    if (bytesLeft >= totalBytes) {
        char* result = startFree;
        startFree += totalBytes;
        return result;
    }
    else if (bytesLeft >= size) {
        nobjs = bytesLeft / size;
        char* result = startFree;
        startFree += nobjs * size;
        return result;
    }
    else {
        size_t bytesToGet = 2 * totalBytes + roundUp(heapSize >> 4);
        if (bytesLeft > 0) {
            Obj** myFreeList = &freeList[freeListIndex(bytesLeft)];
            ((Obj*)startFree)->next = *myFreeList;
            *myFreeList = (Obj*)startFree;
        }

        startFree = (char*)std::malloc(bytesToGet);
        if (!startFree) {
            for (size_t i = size; i <= MAX_BYTES; i += ALIGN) {
                Obj** myFreeList = &freeList[freeListIndex(i)];
                if (*myFreeList) {
                    startFree = (char*)*myFreeList;
                    *myFreeList = (*myFreeList)->next;
                    endFree = startFree + i;
                    return chunkAlloc(size, nobjs);
                }
            }
            return nullptr;
        }

        endFree = startFree + bytesToGet;
        heapSize += bytesToGet;
        return chunkAlloc(size, nobjs);
    }
}

void* MemoryPool::reallocate(void* p, size_t oldSize, size_t newSize) {
    deallocate(p, oldSize);
    return allocate(newSize);
}