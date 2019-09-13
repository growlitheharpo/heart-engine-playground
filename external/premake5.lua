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

		location "../build/proj"
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
