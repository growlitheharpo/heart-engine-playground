project "sfml-demo"
	kind "WindowedApp"
	set_location()

	include_self()
	include_heart(true)
	include_imgui(true)
	debugdir "%{wks.location}/"

	defines {
		"SFML_STATIC",
		"RAPIDJSON_ASSERT=HEART_ASSERT",
	}
	includedirs {
		"../../external/sfml/include/",
		"../../external/rapidjson/include",
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
