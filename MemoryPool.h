//
// Created by Administrator on 25-4-3.
//


#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <iostream>
#include <cstdlib>
#include <cstddef>
class MemoryPool {
private:
    enum { ALIGN = 8 };             // 内存对齐
    enum { MAX_BYTES = 128 };        // 最大分配块
    enum { NFREELISTS = MAX_BYTES / ALIGN }; // 自由链表数组大小

    union Obj {
        Obj* next;       // 指向下一个块
        char data[1];    // 占位数据
    };

    Obj* freeList[NFREELISTS] = {nullptr}; // 自由链表
    char* startFree = nullptr;  // 内存池起始地址
    char* endFree = nullptr;    // 内存池结束地址
    size_t heapSize = 0;        // 记录已分配的堆大小

    size_t roundUp(size_t bytes) {
        return (bytes + ALIGN - 1) & ~(ALIGN - 1);
    }
    size_t freeListIndex(size_t bytes) {
        return (bytes + ALIGN - 1) / ALIGN - 1;
    }

    void* refill(size_t size);
    char* chunkAlloc(size_t size, int& nobjs);

public:
    void* allocate(size_t n);
    void  deallocate(void* p, size_t n);
    void* reallocate(void* p, size_t oldSize, size_t newSize);
};

#endif //MEMORYPOOL_H
