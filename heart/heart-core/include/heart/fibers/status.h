#pragma once

#include <heart/fibers/fwd.h>

enum class HeartFiberStatus : uint8_t
{
	Requeue,
	Complete,
};
