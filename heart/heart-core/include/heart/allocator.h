#pragma once

struct HeartDefaultAllocator
{
	void* allocate(size_t size);
	void deallocate(void* data);
};
