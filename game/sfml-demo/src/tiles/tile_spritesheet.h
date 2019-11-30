#pragma once

#include <heart/deserialization_fwd.h>
#include <heart/stl/vector.h>

SERIALIZE_STRUCT()
struct TileSpritesheetEntry
{
	SerializedString<48> name;
	int x;
	int y;
	int width;
	int height;
};

SERIALIZE_STRUCT()
struct TileSpritesheet
{
	SERIALIZE_AS_REF() hrt::vector<TileSpritesheetEntry> spritelist;
};

SERIALIZE_STRUCT()
struct TileSpritesheetList
{
	size_t fileCount;
	SERIALIZE_AS_REF() SerializedString<64> filelist[16];
};
