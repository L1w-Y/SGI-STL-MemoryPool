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
    static constexpr int ALIGN = 8;
    static constexpr int MAX_BYTES = 128;
    static constexpr int NFREELISTS = MAX_BYTES / ALIGN;

    union Obj {
        Obj* next;  //指向下一个块
        char data[1];//占位数据
    };
    Obj* freelist[NFREELISTS]{};
    char* startFree;      //前一次分配内存块的起始位置
    char* endFree;        //前一次分配内存块的结束位置
    size_t heapSize;            //历史分配数

    static size_t roundUp(const size_t bytes) {    //将bytes向上对齐到 ALIGN 的倍数
        return (bytes + ALIGN - 1) & ~(ALIGN - 1);
    }
    static size_t freeListIndex(const size_t bytes) {     //计算给定 bytes（内存块的字节大小）对应的自由链表索引
        return (bytes + ALIGN - 1) / ALIGN - 1;
    }

    void* refill(size_t size);  //内存管理
    char* chunkAlloc(size_t size, int& nobjs);  //实际的内存分配
public:
    MemoryPool();
    ~MemoryPool();
    void* allocate(size_t size);   //内存分配
    void* reallocate(char* oldChunk,size_t oldSize,size_t newSize);  //扩容/缩容
    void deallocate(char* chunk,size_t n);  //归还内存块
};

#endif //MEMORYPOOL_H
