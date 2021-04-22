#pragma once

#include <heart/types.h>

static constexpr size_t MaxFilePath = 127;

class IoFileDescriptor;
class IoCmdList;
class IoCmdPool;

struct IoUncheckedTargetBuffer;
struct IoCheckedTargetBuffer;

enum class IoOffsetType : uint8_t;
