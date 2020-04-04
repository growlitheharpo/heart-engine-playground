#pragma once

#include <heart/codegen/codegen.h>
#include <heart/stl/vector.h>

// TODO: A better way to do this!! Codegen fails to parse certain template
// types if it encounters an unrecognized include
#ifndef __HEART_CODEGEN_ACTIVE
#include <SFML/Graphics/Texture.hpp>
#endif

SERIALIZE_STRUCT()
struct TileSpritesheetEntry
{
	SerializedString<48> name;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;

	HIDE_FROM_CODEGEN(uint32_t key);
};

SERIALIZE_STRUCT()
struct TileSpritesheet
{
	SERIALIZE_AS_REF() hrt::vector<TileSpritesheetEntry> spritelist;

	// Not populated via serialization, must be loaded
	HIDE_FROM_CODEGEN(sf::Texture texture);
};

SERIALIZE_STRUCT()
struct TileSpritesheetList
{
	SERIALIZE_AS_REF() hrt::vector<SerializedDataPath> filelist;
};
