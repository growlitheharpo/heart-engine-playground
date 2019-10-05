function get_root_location()
	return "%{wks.location}/../"
end

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
		get_root_location() .. "game/heart-core/include/",
		get_root_location() .. "game/heart-debug/include/",
		get_root_location() .. "game/heart-stl/include/",
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
	location "%{wks.location}/proj/%{prj.name}/"
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

	filter {}

	flags {
		"FatalWarnings"
	}

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")

include "external/"
include "game/"

project "data"
	set_location()
	kind "None"
	files {
		"data/**"
	}
