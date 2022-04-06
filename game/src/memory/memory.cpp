#include "memory.h"

#include <heart/config.h>
#include <heart/debug/assert.h>

#if !HEART_STRICT_PERF
#include "render/imgui_game.h"

#include <heart/scope_exit.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/utility.h>

#include <iostream>
#include <mutex>
#include <unordered_map>

template <typename T>
struct debug_allocator : HeartBaseTypedAllocator<T>
{
	debug_allocator() = default;
	USE_DEFAULT_COPY_SEMANTICS(debug_allocator);
	USING_STANDARD_TYPEDEFS(HeartBaseTypedAllocator<T>);
	using size_type = HeartBaseTypedAllocator<T>::size_type;
	using difference_type = HeartBaseTypedAllocator<T>::difference_type;

	template <typename U>
	debug_allocator(const debug_allocator<U>&)
	{
	}

	void* RawAllocate(size_type bytes, void* hint = nullptr)
	{
		return pointer(malloc(bytes));
	}

	void RawDeallocate(void* p, size_t count = 0)
	{
		free(p);
	}
};

// prevent SIOF
static auto& GetMutex()
{
	static std::recursive_mutex s_allocMutex;
	return s_allocMutex;
}

static auto& GetMap()
{
	typedef std::tuple<size_t, Memory::Pool, Memory::Period> debug_info_tuple;
	static hrt::unordered_map_a<void*, debug_info_tuple, debug_allocator> s_trackerMap;
	return s_trackerMap;
}

static size_t s_usageTracker[int(Memory::Pool::Count)][int(Memory::Period::Count)];

#if IMGUI_ENABLED
bool MemoryImguiPanelActive = false;
static bool s_processingImgui = false;
#endif
#else
#include <cstdlib>
#endif

void* operator new(decltype(sizeof(0)) n) noexcept(false)
{
	// Anything coming from the globally overridden new is unknown in origin.
	return Memory::Alloc(n, Memory::Pool::Unknown, Memory::Period::Long);
}

void operator delete(void* p) noexcept
{
	// Assume if it's coming from global delete, it was allocated using global new.
	return Memory::Free(p, Memory::Pool::Unknown, Memory::Period::Long);
}

void* Memory::Alloc(size_t size, Pool p, Period l)
{
	void* r = malloc(size);

#if !HEART_STRICT_PERF
	{
		std::scoped_lock lock {GetMutex()};
		GetMap().emplace(r, std::make_tuple(size, p, l));
		s_usageTracker[int(p)][int(l)] += size;
	}
#endif

	return r;
}

void Memory::Free(void* ptr, Pool p, Period l)
{
#if !HEART_STRICT_PERF
	{
		std::scoped_lock lock {GetMutex()};
		if (auto iter = GetMap().find(ptr); iter != GetMap().end())
		{
			auto [size, pool, period] = iter->second;
			HEART_ASSERT(p == pool, "Ptr was in the wrong pool!");
			HEART_ASSERT(l == period, "Ptr had the wrong period!");

			s_usageTracker[int(pool)][int(period)] -= size;
			GetMap().erase(iter);
		}
	}
#endif

	free(ptr);
}

void Memory::Init()
{
#if IMGUI_ENABLED
	ImGui::SetAllocatorFunctions(
		[](size_t s, void* u) {
			bool* reentrant = (bool*)u;
			if (*reentrant)
				return malloc(s);
			else
				return Alloc(s, Pool::Debug, Period::Short);
		},
		[](void* p, void* u) {
			bool* reentrant = (bool*)u;
			if (*reentrant)
				return free(p);
			else
				return Free(p, Pool::Debug, Period::Short);
		},
		&s_processingImgui);
#endif
}

void Memory::DebugDisplay()
{
#if !HEART_STRICT_PERF
#if IMGUI_ENABLED

	HEART_SCOPE_EXIT([] { s_processingImgui = false; });
	s_processingImgui = true;

	if (!ImGui::Game::IsActive() || !MemoryImguiPanelActive)
		return;

	HEART_SCOPE_EXIT([] { ImGui::End(); });
	if (!ImGui::Begin("Memory", &MemoryImguiPanelActive))
		return;

	const char* PeriodLabels[] = {
		"Short",
		"Long",
		"Frame",
	};
	static_assert(_countof(PeriodLabels) == size_t(Period::Count));

	const char* PoolLabels[] = {
		"Events",
		"UI",
		"Debug",
		"Generic",
		"Unknown",
	};
	static_assert(_countof(PoolLabels) == size_t(Pool::Count));

	ImGui::Columns(int(Period::Count) + 1);
	ImGui::TextUnformatted("Pool");
	ImGui::NextColumn();
	for (int i = 0; i < int(Period::Count); ++i)
	{
		ImGui::TextUnformatted(PeriodLabels[i]);
		ImGui::NextColumn();
	}

	for (int p = 0; p < int(Pool::Count); ++p)
	{
		ImGui::TextUnformatted(PoolLabels[p]);
		ImGui::NextColumn();

		for (int l = 0; l < int(Period::Count); ++l)
		{
			ImGui::Text("%zu kb", (s_usageTracker[p][l] + Kilo - 1) / Kilo);
			ImGui::NextColumn();
		}
	}
#endif
#endif
}
