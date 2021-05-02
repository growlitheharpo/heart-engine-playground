group "external/SFML"

local export_include_root = "%{wks.location}/../external/"

function sfml_lib (libName, extra)
	project("sfml-" .. libName)
		kind "StaticLib"
		warnings "Off"

		includedirs {
			"sfml/extlibs/headers/",
			"sfml/include",
			"sfml/src/",
		}

		files { 
			"sfml/include/SFML/" .. libName .. "/**",
			"sfml/src/SFML/" .. libName .. "/**",
		}

		defines { "SFML_STATIC" }
		
		removefiles { 
			"sfml/src/SFML/" .. libName .. "/*/**", -- remove the platform folders
		}

		filter { "system:windows" }
			files { "sfml/src/SFML/" .. libName .. "/Win32/**" }

		set_location()
		debugdir "./"

		if extra then
			extra()
		end
end

sfml_lib("system")

sfml_lib("window", function ()
	removefiles {
		"sfml/src/SFML/window/EGL**",
	}
end)

sfml_lib("graphics", function ()
	includedirs {
		"sfml/extlibs/headers/stb_image",
		"sfml/extlibs/headers/freetype2",
	}
	files {
		"sfml/extlibs/headers/stb_image/**.h",
		"sfml/extlibs/headers/freetype2/**.h",
	}
end)

sfml_lib("audio", function ()
	includedirs {
		"sfml/extlibs/headers/AL",
	}
	files {
		"sfml/extlibs/headers/AL/**.h"
	}

	defines {
		"auto_ptr=shared_ptr" -- WOW this is a hack
	}

end)

group "external/Boost"

local boostLibs = os.matchdirs("boost/libs/*")

function boost_is_lib(libName)
	local srcFiles = os.matchfiles("boost/libs/" .. libName .. "/src/**")
	if #srcFiles > 0 then
		return true
	else
		return false
	end
end

if _OPTIONS["boostlib"] then
	print("Using boost location: " .. _OPTIONS["boostlib"])
else
	print("Using boost location: " .. export_include_root .. "boost/")
end

function include_boost(should_link)
	if _OPTIONS["boostlib"] then
		sysincludedirs(_OPTIONS["boostlib"])
	else
		for k,v in pairs(boostLibs) do
			includedirs(export_include_root .. v .. "/include")

			if should_link then
				if boost_is_lib(path.getbasename(v)) then
					links("boost." .. path.getbasename(v))
				end
			end
		end
	end
end

function boost_lib (libName)
	project("boost." .. libName)
		set_location()
		files {
			"boost/libs/" .. libName .. "/include/**.h",
			"boost/libs/" .. libName .. "/include/**.hpp",
			"boost/libs/" .. libName .. "/include/**.c",
			"boost/libs/" .. libName .. "/include/**.cpp",
			"boost/libs/" .. libName .. "/src/**.h",
			"boost/libs/" .. libName .. "/src/**.hpp",
			"boost/libs/" .. libName .. "/src/**.c",
			"boost/libs/" .. libName .. "/src/**.cpp",
		}

		defines {
			"BOOST_NO_EXCEPTIONS=1"
		}

		if boost_is_lib(libName) then
			kind "StaticLib"
		else
			kind "None"
		end

		include_boost()
end

if not _OPTIONS["boostlib"] then
	for k,v in pairs(boostLibs) do
		boost_lib(path.getbasename(v))
	end
end

group "external"

project "rapidjson"
	kind "None"
	set_location()
	files { "rapidjson/include/**" }
	includedirs { "rapidjson/include/" }

function include_imgui(should_link)
	includedirs {
		export_include_root .. "imgui/",
		export_include_root .. "imgui-sfml/",
		export_include_root .. "sfml/include/",
	}

	defines {
		"IMGUI_USER_CONFIG=\"imconfig-SFML.h\"",
		"SFML_STATIC",
	}

	if should_link then
		links {
			'imgui',
			'imgui-sfml',
		}
	end
end

project "imgui"
	kind "StaticLib"
	set_location()
	include_imgui()
	warnings "Off"
	files {
		"imgui/imgui.cpp",
		"imgui/imgui_demo.cpp",
		"imgui/imgui_widgets.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/*.h",
	}

	filter { "files:imgui/imgui_demo.cpp" }
		flags { "ExcludeFromBuild" }
	filter {}

project "imgui-sfml"
	kind "StaticLib"
	set_location()
	include_imgui()
	warnings "Off"
	files {
		"imgui-sfml/imgui-SFML.cpp",
		"imgui-sfml/**.h",
	}

function include_entt(should_link)
	includedirs {
		export_include_root .. "entt/src/",
	}

	filter { "configurations:Release" }
		defines { "ENTT_DISABLE_ASSERT=1" }
	filter {}
end

project "entt"
	kind "None" -- entt is header-only
	set_location()
	warnings "Off"
	include_entt()
	files {
		"entt/src/**",
		"entt/README.md",
	}

function include_tweeny(should_link)
	includedirs {
		export_include_root .. "tweeny/include"
	}
end

project "tweeny"
	kind "None" -- tweeny is header-only
	set_location()
	warnings "Off"
	include_tweeny()
	files {
		"tweeny/include/**",
	}

function include_icon_headers(should_link)
	includedirs {
		export_include_root .. "icon-font-cpp-headers"
	}
end

function include_cxxopts(should_link)

	filter { "configurations:Release" }
		defines { "CXXOPTS_NO_EXCEPTIONS=1" }
	filter{}

	includedirs {
		export_include_root .. "cxxopts/include"
	}
end

project "cxxopts"
	kind "None" -- cxxopts is header-only
	set_location()
	warnings "Off"
	include_cxxopts()
	files {
		"cxxopts/include/**",
	}

project "icon-font-cpp-headers"
	kind "None" -- header-only
	set_location()
	include_icon_headers()
	files {
		"icon-font-cpp-headers/**.h"
	}

function include_murmerhash(should_link)
	includedirs {
		export_include_root .. "smhasher"
	}

	if should_link then
		links {
			"murmerhash"
		}
	end
end

project "murmerhash"
	kind "StaticLib"
	set_location()
	warnings "Off"
	include_murmerhash()
	files {
		"smhasher/smhasher/src/MurmurHash1.cpp",
		"smhasher/smhasher/src/MurmurHash2.cpp",
		"smhasher/smhasher/src/MurmurHash3.cpp",
	}
