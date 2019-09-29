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
		"../../external/sfml/include/",
		"../../external/rapidjson/include",
		"src/"
	}

	libdirs { "../../external/sfml/extlibs/libs-msvc-universal/x64" }
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
		"%{wks.location}/bin/heart-codegen/%{cfg.longname}/heart-codegen.exe -in %{prj.location}"
	}
	-- At premake time, we create an empty generated for heart-codegen to fill; this is so that
	-- it gets added to the project files and VS knows to build it
	local tmpgen = io.open("src/gen/reflection.heartgen.cpp", "w")
	tmpgen.close()
