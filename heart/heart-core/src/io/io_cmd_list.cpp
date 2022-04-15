/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "heart/io/io_cmd_list.h"
#include "heart/io/io_op_type.h"

#include "heart/debug/assert.h"

#include "heart/config.h"
#include "heart/countof.h"
#include "heart/stream.h"

IoFileDescriptor::IoFileDescriptor(const char* f) :
	IoFileDescriptor(f, strlen(f))
{
}

IoFileDescriptor::IoFileDescriptor(const char* f, size_t l) :
	m_size(uint8_t(l))
{
	HEART_ASSERT(strlen(f) == m_size);
	memcpy(m_filename, f, l);
}

void IoCmdList::BindIoFileDescriptor(const IoFileDescriptor& d)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::BindDescriptor));
	HEART_CHECK(writer.Write(d.GetSize()));
	HEART_CHECK(writer.Write(d.GetFilename(), d.GetSize()));
}

void IoCmdList::BindIoTargetBuffer(const IoUncheckedTargetBuffer& b)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::BindBufferUnchecked));
	HEART_CHECK(writer.Write(b));
}

void IoCmdList::BindIoTargetBuffer(const IoCheckedTargetBuffer& b)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

#if !HEART_STRICT_PERF
	HEART_CHECK(writer.Write(IoOpType::BindBufferChecked));
	HEART_CHECK(writer.Write(b));
#else
	IoUncheckedTargetBuffer unchecked = {b.ptr};
	HEART_CHECK(writer.Write(IoOpType::BindBufferUnchecked));
	HEART_CHECK(writer.Write(unchecked));
#endif
}

void IoCmdList::ReadEntire()
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::ReadEntire));
}

void IoCmdList::ReadPartial(size_t readLength)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::ReadPartial));
	HEART_CHECK(writer.Write(readLength));
}

void IoCmdList::Offset(int64_t offset, IoOffsetType type)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::Offset));
	HEART_CHECK(writer.Write(offset));
	HEART_CHECK(writer.Write(type));
}

void IoCmdList::UnbindFileDescriptor()
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::UnbindDescriptor));
}

void IoCmdList::UnbindTargetBuffer()
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::UnbindTarget));
}

void IoCmdList::Signal(HeartFence* fence, uint32_t value)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::SignalFence));
	HEART_CHECK(writer.Write(fence));
	HEART_CHECK(writer.Write(value));
}

void IoCmdList::Wait(HeartFence* fence, uint32_t value)
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::WaitForFence));
	HEART_CHECK(writer.Write(fence));
	HEART_CHECK(writer.Write(value));
}

void IoCmdList::Reset()
{
	HeartStreamWriter writer(m_cmdPool, m_writeHead);

	HEART_CHECK(writer.Write(IoOpType::Reset));
}

void IoCmdList::Finalize()
{
	m_writeHead = 0;
}
