#pragma once

#include <heart/types.h>

#include <mutex>

static constexpr size_t Kilo(size_t n)
{
	return n * 1024;
}

static constexpr size_t Meg(size_t n)
{
	return n * Kilo(1) * Kilo(1);
}

struct MemoryPool
{
	enum Category
	{
		Generic,
		Graphics,
		UI,

		Count
	};

	constexpr static size_t sizes[3] = {
		Meg(32),  // Generic
		Meg(32),  // Graphics
		Kilo(512) // UI
	};
};

class MemoryManager
{
private:
	constexpr static size_t BlockSize = 256;
	constexpr static size_t BlockEndBuffer = 0;

	class Pool
	{
	private:
		friend class MemoryManager;
		typedef unsigned char byte_t;

		// TODO: shrink node size by using fake pointers?
		struct alignas(32) Node
		{
			Node* prev_;
			Node* next_;
			size_t size_;

			Node(Node* next, Node* prev, size_t s) : next_(next), prev_(prev), size_(s)
			{
			}
		};

		size_t size_ = 0;
		size_t usage_ = 0;
		Node* head_ = nullptr;

		std::mutex mutex_;

	private:
		inline ptrdiff_t CalculateRemainingGap(Node* node, size_t targetSize);
		inline size_t ToNearestBlockSize(size_t n);

	public:
		void Initialize(size_t size);
		void Destroy();

		void* Allocate(size_t size);
		void Free(void* ptr);
	};

	Pool pools_[MemoryPool::Count];

public:
	MemoryManager();
	~MemoryManager();

	static MemoryManager& Get();

	static void* Allocate(MemoryPool::Category pool, size_t size);
	static void Free(MemoryPool::Category pool, void* ptr);

	template <MemoryPool::Category pool>
	static void* Allocate(size_t size)
	{
		return Get().pools_[pool].Allocate(size);
	}

	template <MemoryPool::Category pool>
	static void Free(void* ptr)
	{
		return Get().pools_[pool].Free(ptr);
	}

	static void DrawReport();
};
