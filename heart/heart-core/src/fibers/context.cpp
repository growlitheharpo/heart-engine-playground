#include "heart/fibers/context.h"

#include "heart/fibers/system.h"

HeartFiberContext& HeartFiberContext::Get()
{
	static thread_local HeartFiberContext context;
	return context;
}

void HeartFiberContext::Yield()
{
	HeartFiberContext& ctx = Get();
	if (!ctx.currentWorkUnit || !ctx.currentSystem)
	{
		// TODO: warn here? we tried to yield from a thread that's not a fiber
		return;
	}

	ctx.currentSystem->YieldUnit(*ctx.currentWorkUnit);
}
