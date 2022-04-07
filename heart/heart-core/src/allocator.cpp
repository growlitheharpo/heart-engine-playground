#include "heart/allocator.h"

HeartDefaultAllocator& GetHeartDefaultAllocator()
{
	static HeartDefaultAllocator GlobalDefaultAllocator;
	return GlobalDefaultAllocator;
}
