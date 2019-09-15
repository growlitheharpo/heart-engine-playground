group "external/SFML"

local sfml_root = "sfml/"
local src_root = sfml_root .. "src/"
local incl_root = sfml_root .. "include/"

function sfml_lib (libName, extra)
	project("sfml-" .. libName)
		kind "StaticLib"
		warnings "Off"

		includedirs {
			sfml_root .. "extlibs/headers/",
			incl_root,
			src_root,
		}

		files { 
			incl_root .. "SFML/" .. libName .. "/**",
			src_root .. "SFML/" .. libName .. "/**",
		}

		defines { "SFML_STATIC" }
		
		removefiles { 
			src_root .. "SFML/" .. libName .. "/*/**", -- remove the platform folders
		}

		filter { "system:windows" }
			files { src_root .. "SFML/" .. libName .. "/Win32/**" }

		set_location()
		debugdir "./"

		if extra then
			extra()
		end
end

sfml_lib("system")

sfml_lib("window", function ()
	removefiles {
		src_root .. "SFML/window/EGL**",
	}
end)

sfml_lib("graphics", function ()
	includedirs {
		sfml_root .. "extlibs/headers/stb_image",
		sfml_root .. "extlibs/headers/freetype2",
	}
end)

sfml_lib("audio", function ()
	includedirs {
		sfml_root .. "extlibs/headers/AL",
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
		"%{wks.location}/../external/imgui/",
		"%{wks.location}/../external/imgui-sfml/",
		"%{wks.location}/../external/sfml/include/",
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
