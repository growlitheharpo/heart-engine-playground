#pragma once

#include <heart/io/io_forward_decl.h>

class HeartFence;

enum class IoFileMode
{
	Read,
	Write,
};

class IoFileDescriptor
{
public:
	IoFileDescriptor(const char* f);
	IoFileDescriptor(const char* f, size_t l);

	const char* GetFilename() const
	{
		return m_filename;
	}

private:
	static_assert(MaxFilePath < 255, "Cannot fit MaxFilePath size in uint8!");

	uint8_t m_size;
	char m_filename[MaxFilePath];
};

struct IoUncheckedTargetBuffer
{
	void* ptr;
};

struct IoCheckedTargetBuffer
{
	void* ptr;
	size_t size;
};

enum class IoOffsetType : uint8_t
{
	FromStart,
	FromCurrent,
	FromEnd,
};

class IoCmdList
{
	friend class IoCmdQueue;

public:
	void BindIoFileDescriptor(const IoFileDescriptor& d);

	void BindIoTargetBuffer(const IoUncheckedTargetBuffer& b);
	void BindIoTargetBuffer(const IoCheckedTargetBuffer& b);

	void ReadEntire();
	void ReadPartial(size_t readLength);

	void Offset(int64_t offset, IoOffsetType type);

	void UnbindFileDescriptor();
	void UnbindTargetBuffer();

	void Signal(HeartFence* fence, uint32_t value);
	void Wait(HeartFence* fence, uint32_t value);

	void Reset();

private:
	uint16_t m_writeHead = 0;
	uint8_t m_cmdPool[8ull * Kilo];

	void Finalize();
};
