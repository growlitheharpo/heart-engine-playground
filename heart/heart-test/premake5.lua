project "heart-test"
	kind "ConsoleApp"
	language "C++"
	set_location()
	include_self()
	include_heart(true)
	include_entt()
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
	io.open("src/gen/reflection.heartgen.cpp", "w").close()
