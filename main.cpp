#include "MemoryPool.h"
#include <iostream>
#include <cstring>

int main() {
    system("chcp 65001");
    MemoryPool pool;
    std::cout << "ptr1分配 16 字节：" << std::endl;
    void* ptr1 = pool.allocate(16);
    std::cout << "Allocated memory at: " << ptr1 << std::endl;
    pool.deallocate(static_cast<char *>(ptr1),16);

    return 0;
}