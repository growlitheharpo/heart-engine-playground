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

	-- hack! remove this
	defines { "_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING=1" }

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
		"smhasher/smhasher/src/MurmurHash**",
	}

function include_googletest(should_link)
	includedirs {
		export_include_root .. "googletest/googletest/include/",
	}

	defines {
		"GTEST_LINKED_AS_SHARED_LIBRARY=0",
		"GTEST_CREATE_SHARED_LIBRARY=0",
		"GTEST_HAS_TR1_TUPLE=0",
		"GTEST_HAS_CLONE=0",
		"GTEST_HAS_SEH=0",
	}

	if should_link then
		links {
			"googletest"
		}
	end
end

project "googletest"
	kind "StaticLib"
	set_location()
	removeflags { "FatalWarnings" }
	warnings "Off"
	include_googletest()
	includedirs {
		export_include_root .. "googletest/googletest/",
	}
	files {
		"googletest/googletest/src/**",
	}
