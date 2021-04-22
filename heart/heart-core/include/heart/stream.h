#pragma once

#include <heart/stl/type_traits.h>

#include <heart/types.h>

// TODO: wrap for memcopy
// TODO: wrap for numeric limits
#include <cstring>
#include <limits>

template <typename BufferSizeT>
class HeartStreamWriter
{
	uint8_t* m_buffer;
	BufferSizeT* m_head;
	BufferSizeT m_size;

private:
	bool Check(size_t w)
	{
		return size_t(m_size - *m_head) >= w;
	}

public:
	template <typename P>
	HeartStreamWriter(P* buffer, BufferSizeT size, BufferSizeT& head) :
		m_buffer(reinterpret_cast<uint8_t*>(buffer)),
		m_head(&head),
		m_size(size)
	{
	}

	template <typename P, decltype(sizeof(void*)) N>
	HeartStreamWriter(P (&buffer)[N], BufferSizeT& head) :
		HeartStreamWriter(buffer, BufferSizeT(N), head)
	{
	}

	~HeartStreamWriter() = default;

	template <typename T, hrt::enable_if_t<hrt::is_trivially_copyable_v<T>, void*> = nullptr>
	bool Write(T v)
	{
		static_assert(sizeof(v) < std::numeric_limits<BufferSizeT>::max(), "Value of this type will *never* fit in storage!");

		if (!Check(sizeof(v)))
			return false;

		memcpy(m_buffer + *m_head, &v, sizeof(v));
		*m_head += BufferSizeT(sizeof(v));
		return true;
	}

	template <typename T, hrt::enable_if_t<hrt::is_trivially_copyable_v<T>, void*> = nullptr>
	bool Write(T* v, size_t count)
	{
		static_assert(sizeof(v) < std::numeric_limits<size_t>::max(), "Value of this type will *never* fit in storage!");

		if (!Check(sizeof(v) * count))
			return false;

		memcpy(m_buffer + *m_head, v, sizeof(v) * count);
		*m_head += BufferSizeT(sizeof(v) * count);
		return true;
	}
};

template <typename BufferSizeT>
class HeartStreamReader
{
	const uint8_t* m_buffer;
	BufferSizeT* m_head;

	BufferSizeT m_headInternal = 0;

public:
	struct Copy_
	{
	};

	struct GetPtr_
	{
	};

	static constexpr Copy_ Copy;

	static constexpr GetPtr_ GetPtr;

	HeartStreamReader(const void* buffer) :
		m_buffer(reinterpret_cast<const uint8_t*>(buffer)),
		m_head(&m_headInternal)
	{
	}

	HeartStreamReader(const void* buffer, BufferSizeT* readHead) :
		m_buffer(reinterpret_cast<const uint8_t*>(buffer))
	{
		if (readHead != nullptr)
		{
			m_head = readHead;
		}
		else
		{
			m_head = &m_headInternal;
		}
	}

	template <typename T>
	T Read(Copy_)
	{
		const T* h = reinterpret_cast<const T*>(m_buffer + *m_head);
		*m_head += BufferSizeT(sizeof(T));

		return *h;
	}

	template <typename T>
	const T* Read(GetPtr_)
	{
		const T* h = reinterpret_cast<const T*>(m_buffer + *m_head);
		*m_head += BufferSizeT(sizeof(T));

		return h;
	}
};
