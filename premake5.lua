
function include_self()
	includedirs {
		"include/",
	}
	files {
		"include/**",
		"src/**",
	}
end

function include_heart(also_link)
	includedirs {
		"%{wks.location}/../game/heart-core/include/",
		"%{wks.location}/../game/heart-debug/include/",
		"%{wks.location}/../game/heart-stl/include/",
	}
	if also_link then
		links {
			'heart-core',
			'heart-debug',
			-- 'heart-stl', -- does not actually "build", so no need to link
		}
	end
end

function set_location()
	location "%{wks.location}/proj/"
end

workspace "fun-with-sfml"
	location "build/"
	language "C++"
	cppdialect "c++17"
	startproject "sfml-demo"

	architecture "x86_64"
	configurations { "Debug", "Release" }

	filter { "configurations:Debug" }
		defines { "DEBUG", "_DEBUG" }
		symbols "On"

	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"
	
	filter {"system:windows", "action:vs*"}
		systemversion "latest"
		toolset "v142"

	filter {}

	flags {
		"FatalWarnings"
	}

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")

include "external/"
include "game/"
