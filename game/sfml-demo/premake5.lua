project "sfml-demo"
	kind "WindowedApp"
	set_location()

	include_self()
	include_heart(true)
	include_imgui(true)
	include_entt()
	debugdir "%{wks.location}/"

	defines {
		"SFML_STATIC",
		"RAPIDJSON_ASSERT=HEART_ASSERT",
	}
	includedirs {
		get_root_location() .. "external/sfml/include/",
		get_root_location() .. "external/rapidjson/include",
		"src/"
	}

	libdirs { get_root_location() .. "external/sfml/extlibs/libs-msvc-universal/x64" }
	links {
		'sfml-system',
		'sfml-graphics',
		'sfml-window',
		'sfml-audio',

		'flac.lib',
		'freetype.lib',
		'ogg.lib',
		'openal32.lib',
		'vorbis.lib',
		'vorbisenc.lib',
		'vorbisfile.lib',
		'opengl32.lib',
		'freetype.lib',
		'winmm.lib',
		'gdi32.lib',
	}

	dependson "heart-codegen"
	prebuildcommands {
		get_output_location("heart-codegen") .. "heart-codegen.exe -in " .. get_root_location() .. "game/sfml-demo/src/"
	}
	-- At premake time, we create an empty generated for heart-codegen to fill; this is so that
	-- it gets added to the project files and VS knows to build it
	io.open("src/gen/reflection.heartgen.cpp", "w").close()
