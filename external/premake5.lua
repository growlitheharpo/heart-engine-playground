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
		"imgui/imgui_widgets.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/*.h",
	}

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
