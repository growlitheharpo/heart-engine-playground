#pragma once

struct HeartDefaultAllocator
{
	void* Allocate(size_t size);
	void Deallocate(void* data, size_t size);
};
