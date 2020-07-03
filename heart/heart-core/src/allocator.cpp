#include "heart/allocator.h"

#include "heart/types.h"

void* HeartDefaultAllocator::allocate(size_t size)
{
	return new uint8_t[size];
}

void HeartDefaultAllocator::deallocate(void* data)
{
	delete[] data;
}
