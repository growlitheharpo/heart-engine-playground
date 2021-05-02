#pragma once

#include <heart/types.h>

enum class IoOpType : uint16_t
{
	BindDescriptor,
	BindBufferUnchecked,
	BindBufferChecked,
	ReadEntire,
	ReadPartial,
	Offset,
	UnbindDescriptor,
	UnbindTarget,
	Reset,
	SignalFence,
	WaitForFence,
};
