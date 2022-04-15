--[[ Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
--]]
project "heart-test"
	kind "ConsoleApp"
	language "C++"
	set_location()
	include_self()
	include_heart(true)
	include_entt()
	include_murmerhash(true)
	include_googletest(true)

	includedirs {
		get_root_location() .. "external/rapidjson/include",
		"src/",
	}

	dependson "heart-codegen"
	prebuildcommands {
		get_output_location("heart-codegen") .. "heart-codegen.exe"
		.. " -in " .. get_root_location() .. "heart/heart-test/src/"
		.. " -heart " .. get_root_location() .. "heart/heart-core/"
	}
	-- At premake time, we create an empty generated for heart-codegen to fill; this is so that
	-- it gets added to the project files and VS knows to build it
	io.writefile("src/gen/reflection.heartgen.cpp", "\n")

