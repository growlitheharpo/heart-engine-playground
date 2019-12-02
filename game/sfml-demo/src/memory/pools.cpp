#include "pools.h"
#include <new>

#include "render/imgui_game.h"

#include <heart/config.h>
#include <heart/debug/assert.h>

#include <inttypes.h>

#if !HEART_STRICT_PERF
#include <cstring>
#endif

void* operator new(std::size_t s)
{
	return MemoryManager::Allocate<MemoryPool::Generic>(s);
}

void* operator new[](std::size_t s)
{
	return MemoryManager::Allocate<MemoryPool::Generic>(s);
}

void* operator new(std::size_t s, const std::nothrow_t&) noexcept
{
	return MemoryManager::Allocate<MemoryPool::Generic>(s);
}

void* operator new[](std::size_t s, const std::nothrow_t&) noexcept
{
	return MemoryManager::Allocate<MemoryPool::Generic>(s);
}

void operator delete(void* p) noexcept
{
	return MemoryManager::Free<MemoryPool::Generic>(p);
}

void operator delete[](void* p) noexcept
{
	return MemoryManager::Free<MemoryPool::Generic>(p);
}

MemoryManager::MemoryManager()
{
	for (size_t i = 0; i < MemoryPool::Count; ++i)
	{
		pools_[i].Initialize(MemoryPool::sizes[i]);
	}
}

MemoryManager::~MemoryManager()
{
	char finalReport[4096] = {};
	sprintf_s(finalReport, "Generic: %zu / %zu (%f)\nGraphics: %zu / %zu (%f)\nUI: %zu / %zu (%f)",
		pools_[MemoryPool::Generic].usage_, pools_[MemoryPool::Generic].size_,
		((float)pools_[MemoryPool::Generic].usage_ / (float)pools_[MemoryPool::Generic].size_ * 100.0f),
		pools_[MemoryPool::Graphics].usage_, pools_[MemoryPool::Graphics].size_,
		((float)pools_[MemoryPool::Graphics].usage_ / (float)pools_[MemoryPool::Graphics].size_ * 100.0f),
		pools_[MemoryPool::UI].usage_, pools_[MemoryPool::UI].size_,
		((float)pools_[MemoryPool::UI].usage_ / (float)pools_[MemoryPool::UI].size_ * 100.0f));

	bool dummy = false;
	DisplayAssertError("Final Memory Report", "Final usage counts:", finalReport, __FILE__, __LINE__, &dummy);

	for (auto& p : pools_)
		p.Destroy();
}

MemoryManager& MemoryManager::Get()
{
	static MemoryManager instance_;
	return instance_;
}

void* MemoryManager::Allocate(MemoryPool::Category pool, size_t size)
{
	return Get().pools_[pool].Allocate(size);
}

void MemoryManager::Free(MemoryPool::Category pool, void* ptr)
{
	return Get().pools_[pool].Free(ptr);
}

void MemoryManager::DrawReport()
{
#if IMGUI_ENABLED
	const char* labels[MemoryPool::Count] = {"Generic", "Graphics", "UI"};
	bool open = true;
	if (ImGui::Begin("MemoryReport", &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		for (size_t i = 0; i < MemoryPool::Count; ++i)
		{
			auto used = Get().pools_[i].usage_;
			auto total = Get().pools_[i].size_;
			auto perc = (float)used / (float)total * 100.0f;
			ImGui::Text("%s %zu / %zu (%f)", labels[i], used, total, perc);
		}
		ImGui::End();
	}
#endif
}

void MemoryManager::Pool::Initialize(size_t size)
{
	std::lock_guard<std::mutex> lock(mutex_);

	void* buffer = malloc(size);

	if (!HEART_CHECK(buffer != nullptr, "Malloc has failed!"))
		return; // the pool is in a bad state!!

	size_ = size;
	if (!HEART_CHECK(size >= 64, "Cannot allocate a very small pool!"))
		return;

	head_ = new (buffer) Node(nullptr, nullptr, 0);
}

void MemoryManager::Pool::Destroy()
{
	std::lock_guard<std::mutex> lock(mutex_);

	free(head_);
	head_ = nullptr;
	size_ = 0;
}

/**
 * \brief Calculates how much space will remain if `targetSize` were to be placed at `node`
 * \return The gap between the potential end of `node` and the next node or the end of the pool.
		returns < 0 if the `targetSize` will not fit in this location, 0 if it fits perfectly, or the remaining space
 */
ptrdiff_t MemoryManager::Pool::CalculateRemainingGap(Node* node, size_t targetSize)
{
	// find the pointer to the start of the actual usable memory for this node.
	byte_t* start = (byte_t*)(node + 1);

	byte_t* end;
	if (node->next_)
		end = (byte_t*)node->next_;
	else
		end = (byte_t*)(head_ + size_);

	ptrdiff_t size = end - start;
	return size - targetSize;
}

inline size_t MemoryManager::Pool::ToNearestBlockSize(size_t n)
{
	size_t p1 = n % BlockSize;
	size_t x = n / BlockSize;
	return (x + size_t(p1 > 0)) * BlockSize;
}

void* MemoryManager::Pool::Allocate(size_t size)
{
	std::lock_guard<std::mutex> lock(mutex_);

	// TODO: small allocation optimization
	size = ToNearestBlockSize(size);

	auto curr = head_;
	// HEART_ASSERT(curr->size_ == 0, "next_ was left in a bad state!");

	while (true)
	{
		if (curr->size_ == 0 && CalculateRemainingGap(curr, size + BlockEndBuffer) >= 0)
		{
			curr->size_ = size;

			byte_t* blockBegin = (byte_t*)(curr + 1);
			byte_t* blockEndPos = (byte_t*)(blockBegin + size);
			byte_t* nextNodePos = (byte_t*)(blockEndPos + BlockEndBuffer);

#if !HEART_STRICT_PERF
			memset(blockBegin, 0xA0, blockEndPos - blockBegin);
			memset(blockEndPos, 0xEE, nextNodePos - blockEndPos);
#endif

			// Check if we need to add a new empty node - we won't need to if our gap was 0
			//if ((void*)nextNodePos != (void*)curr->next_)
			if (curr->next_ == nullptr)
			{
				// Setup the next block
				Node* newNext = new (nextNodePos) Node(curr->next_, curr, 0);
				curr->next_ = newNext;
			}

			usage_ += size + BlockEndBuffer;
			return blockBegin;
		}

		if (!HEART_CHECK(curr->next_ != nullptr, "Failed to allocate block in pool!"))
		{
			return nullptr;
		}

		do
		{
			HEART_ASSERT(curr->next_ != nullptr, "Ran out of blocks without finding one with a size of 0???");
			curr = curr->next_;
		} while (curr->size_);
	}

	return nullptr;
}

void MemoryManager::Pool::Free(void* ptr)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (ptr == nullptr)
		return;

	Node* blockNode = ((Node*)ptr - 1);

	// "Grow" our empty block outward as much as we can by consuming any
	// consecutive freed nodes ahead of us
	while (blockNode->next_ && blockNode->next_->size_ == 0)
	{
		blockNode->next_ = blockNode->next_->next_;
	}

	// Now the hard part... we need to grow backwards if possible
	while (blockNode->prev_ && blockNode->prev_->prev_ && blockNode->prev_->size_ == 0)
	{
		blockNode->prev_->next_ = blockNode->next_;
		blockNode = blockNode->prev_;
	}

	usage_ -= blockNode->size_;
	blockNode->size_ = 0;

#if !HEART_STRICT_PERF
	{
		/*
		byte_t* end = (blockNode->next_) ? (byte_t*)blockNode->next_ : (byte_t*)(head_ + size_);
		ptrdiff_t size = end - (byte_t*)(blockNode + 1);
		memset(blockNode + 1, 0xFF, size);
		*/
	}
#endif
}
