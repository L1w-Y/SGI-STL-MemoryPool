//
// Created by Administrator on 25-4-3.
//

#include "MemoryPool.h"
MemoryPool::MemoryPool() {
    startFree = nullptr;        // ��ʼΪ��
    endFree = nullptr;          // ��ʼΪ��
    heapSize = 0;
    // ��ʼ�޷����ڴ�
    for (auto & i : freelist) {
        i = nullptr;  // ��ʼ��������������Ϊ��
    }
}

MemoryPool::~MemoryPool() {
    // �ͷŶ��ڴ�
    if (startFree) {
        free(startFree);  // ֻ�ͷ� startFree ָ�����һ�� malloc �õ����ڴ�
        startFree = nullptr;
        endFree = nullptr;
    }
    // �ͷ����������е��ڴ��
    for (auto& current : freelist) {
        while (current) {
            Obj* next = current->next;
            free(current);  // �ͷ����������е��ڴ��
            current = next;
        }
    }
}

void* MemoryPool::refill(const size_t size) {
    int nobjs = 20;
    Obj *current_obj;
    Obj* next_obj;
    char* chunk = chunkAlloc(size,nobjs); //���������ڴ棬��СΪ size*nobjs

    if (nobjs == 1) return  chunk;

    Obj **my_freelist = freelist + freeListIndex(size);

    const auto result = reinterpret_cast<Obj *>(chunk);   //����ת��
    *my_freelist = next_obj =  reinterpret_cast<Obj *>(chunk + size); //��һ���ڴ��Ƿ����ȥ�ģ�ʣ�µ��ڴ湹��Ϊ��̬����
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
        // ʵ��û�п����ڴ���
        endFree = nullptr;
        throw std::bad_alloc();
    }
    heapSize += bytes_to_get;
    endFree=startFree+bytes_to_get;
    return chunkAlloc(size,nobjs);

}

void* MemoryPool::allocate(const size_t size) {
    void* ret;
    if (size>MAX_BYTES) { // ����������������������ڴ�ع���
        ret = malloc(size);
        if (ret) {
            std::cout << "Allocated " << size << " bytes using malloc at: " << ret << std::endl;
            return ret;
        }
    }else {
        Obj** my_freelist = freelist + freeListIndex(size);
        if (Obj* result = *my_freelist) {   //�����Ӧ�����п����ڴ棬ֱ�ӷ���
            *my_freelist = result->next;
            ret = result;
        }else {//û�г�ʼ����
            ret = refill(roundUp((size)));//����ǰ�����ڴ���루����size=7 ->  8��
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
