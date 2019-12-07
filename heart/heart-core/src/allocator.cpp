#include "heart/allocator.h"

#include "heart/types.h"

void* HeartDefaultAllocator::Allocate(size_t size)
{
	return new uint8_t[size];
}

void HeartDefaultAllocator::Deallocate(void* data, size_t size)
{
	delete[] data;
}
