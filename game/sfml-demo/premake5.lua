project "sfml-demo"
	kind "WindowedApp"
	set_location()

	include_self()
	include_heart()
	debugdir "src/"

	defines { "SFML_STATIC" }
	includedirs "../../external/sfml/include/"

	libdirs { "../../external/sfml/extlibs/libs-msvc-universal/x64" }
	links {
		'sfml-system',
		'sfml-graphics',
		'sfml-window',
		'sfml-audio',
		'heart-core',

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
