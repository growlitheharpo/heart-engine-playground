#pragma once

#include <heart/io/io_forward_decl.h>

class IoCmdQueue
{
	void Submit(IoCmdList* cmdList);
};
