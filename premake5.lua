#!lua

-- Stolen from https://github.com/premake/premake-core/issues/935#
function os.winSdkVersion()
	local reg_arch = iif( os.is64bit(), "\\Wow6432Node\\", "\\" )
	local sdk_version = os.getWindowsRegistry( "HKLM:SOFTWARE" .. reg_arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion" )
	if sdk_version ~= nil then return sdk_version end
end

workspace "fun-with-sfml"
	location "build/"
	language "C++"
	cppdialect "c++17"

	architecture "x86_64"
	configurations { "Debug", "Release" }

	filter { "configurations:Debug" }
		defines { "DEBUG", "_DEBUG" }
		symbols "On"

	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"
	
	filter {"system:windows", "action:vs*"}
		systemversion("10.0.17763.0")
		toolset "v142"

	filter {}

	flags {
		"FatalWarnings"
	}

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")

project "sfml-demo"
	kind "WindowedApp"
	-- entrypoint "main"

	files "src/**"
	includedirs "src/"
	location "build/proj/"
	debugdir "src/"

	defines { "SFML_STATIC" }
	includedirs "external/sfml/include/"

	dependson {
		"sfml-system",
		"sfml-graphics",
		"sfml-window",
		"sfml-audio",
	}

	libdirs { "external/sfml/extlibs/libs-msvc-universal/x64" }
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


include "external/"
