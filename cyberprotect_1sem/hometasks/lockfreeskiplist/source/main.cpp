#include <iostream>

#include "tagged_ptr.h"

using namespace util;

int main()
{
    int* ptr = new int(5);
    TaggedPtr<int> taggedPtr(ptr, 9);
    *taggedPtr = 10;

    std::cout << *ptr << std::endl;

    delete ptr;
    return 0;   
}