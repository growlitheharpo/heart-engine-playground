function get_root_location()
	return "%{wks.location}/../"
end

function get_output_location(prj_name)
	return get_root_location() .. "build/bin/" .. prj_name .. "/%{cfg.longname}/"
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
		get_root_location() .. "heart/heart-core/include/",
		get_root_location() .. "heart/heart-stl/include/",
	}
	if also_link then
		dependson('heart-stl')
		links {
			'heart-core',
			-- 'heart-stl', -- does not actually "build", so no need to link
		}
	end
end

function set_location()
	location "%{wks.location}/proj/%{prj.name}/"
end

workspace "heart-engine-playground"
	location "build/"
	language "C++"
	cppdialect "c++20"
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

	disablewarnings {
		"5056",
	}

	defines {
		"_ITERATOR_DEBUG_LEVEL=0",
	}

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")

include "external/"

group "heart"
	include "heart/heart-core"
	include "heart/heart-codegen"
	include "heart/heart-stl"
	include "heart/heart-test"
group "*"

include "game/"

project "data"
	set_location()
	kind "None"
	files {
		"data/**"
	}
