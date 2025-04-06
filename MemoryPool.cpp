//
// Created by Administrator on 25-4-3.
//

#include "MemoryPool.h"
MemoryPool::MemoryPool() {
    startFree = nullptr;        // 初始为空
    endFree = nullptr;          // 初始为空
    heapSize = 0;
    // 初始无分配内存
    for (auto & i : freelist) {
        i = nullptr;  // 初始化所有自由链表为空
    }
}

MemoryPool::~MemoryPool() {
    // 释放堆内存
    if (startFree) {
        free(startFree);  // 只释放 startFree 指向的那一块 malloc 得到的内存
        startFree = nullptr;
        endFree = nullptr;
    }
    // 释放自由链表中的内存块
    for (auto& current : freelist) {
        while (current) {
            Obj* next = current->next;
            free(current);  // 释放自由链表中的内存块
            current = next;
        }
    }
}

void* MemoryPool::refill(const size_t size) {
    int nobjs = 20;
    Obj *current_obj;
    Obj* next_obj;
    char* chunk = chunkAlloc(size,nobjs); //分配连续内存，大小为 size*nobjs

    if (nobjs == 1) return  chunk;

    Obj **my_freelist = freelist + freeListIndex(size);

    const auto result = reinterpret_cast<Obj *>(chunk);   //类型转换
    *my_freelist = next_obj =  reinterpret_cast<Obj *>(chunk + size); //第一块内存是分配出去的，剩下的内存构建为静态链表
    for (int i = 0;i<nobjs-1;i++) {
        current_obj =next_obj;
        next_obj = reinterpret_cast<Obj *>(reinterpret_cast<char *>(next_obj) + size);
        current_obj->next = next_obj;
    }
    // ReSharper disable once CppLocalVariableMightNotBeInitialized
    current_obj->next = nullptr;
    return result;
}

char * MemoryPool::chunkAlloc(const size_t size, int &nobjs) {
    char* result;
    size_t total_bytes=size*nobjs;
    size_t bytes_left=endFree-startFree;
    if (bytes_left>=total_bytes){
        result= startFree;
        startFree+=total_bytes;
        return(result);
    }
    if (bytes_left>=size) {
        nobjs = static_cast<int>(bytes_left / size);
        total_bytes = size*nobjs;
        result=startFree;
        startFree+=total_bytes;
        return (result);
    }
    const size_t bytes_to_get = 2*total_bytes+ roundUp(heapSize>>4);
    if (bytes_left>0) {
        Obj** my_freelist = freelist + freeListIndex((bytes_left));
        reinterpret_cast<Obj *>(startFree)->next=*my_freelist;
        *my_freelist = reinterpret_cast<Obj *>(startFree);
    }
    startFree = static_cast<char *>(malloc(bytes_to_get));
    if (!startFree) {
        for (size_t i = size;i<=static_cast<size_t>(MAX_BYTES);i+= static_cast<size_t>(ALIGN)) {
            Obj **my_freelist = freelist + freeListIndex(i);
            if (Obj *p = *my_freelist; p) {
                *my_freelist=p->next;
                startFree=reinterpret_cast<char *>(p);
                endFree=startFree+i;
                return chunkAlloc(size, nobjs);
            }
        }
        // 实在没有可用内存了
        endFree = nullptr;
        throw std::bad_alloc();
    }
    heapSize += bytes_to_get;
    endFree=startFree+bytes_to_get;
    return chunkAlloc(size,nobjs);

}

void* MemoryPool::allocate(const size_t size) {
    void* ret;
    if (size>MAX_BYTES) { // 如果大于最大分配数，不由内存池管理
        ret = malloc(size);
        if (ret) {
            std::cout << "Allocated " << size << " bytes using malloc at: " << ret << std::endl;
            return ret;
        }
    }else {
        Obj** my_freelist = freelist + freeListIndex(size);
        if (Obj* result = *my_freelist) {   //如果对应链表有空闲内存，直接分配
            *my_freelist = result->next;
            ret = result;
        }else {//没有初始分配
            ret = refill(roundUp((size)));//传入前进行内存对齐（比如size=7 ->  8）
        }
    }
    return  ret;
}

void* MemoryPool::reallocate(char *oldChunk, const size_t oldSize, const size_t newSize) {
    if (oldSize>MAX_BYTES&&newSize>MAX_BYTES) {
        return nullptr;
    }
    if (roundUp(oldSize)==roundUp(newSize))return oldChunk;
    void *result = allocate(newSize);
    const size_t cpy_size = newSize > oldSize ? oldSize : newSize;
    memcpy(result,oldChunk,cpy_size);
    deallocate(oldChunk,oldSize);
    return  result;
}

void MemoryPool::deallocate(char *chunk, const size_t n) {
    if (n>MAX_BYTES) {
        std::cout << "Freeing large block: " << n << " bytes" << std::endl;
        free(chunk);
    }else {
        Obj** my_freelist = freelist + freeListIndex(n);
        const auto q = reinterpret_cast<Obj *>(chunk);
        q->next=*my_freelist;
        *my_freelist=q;
    }
}
