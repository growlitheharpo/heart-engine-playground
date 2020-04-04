project "heart-stl"
	kind "StaticLib"
	set_location()
	include_self()
	include_heart()
	include_boost()

	filter { "action:vs*" }
		files {
			"hrt.natvis"
		}
	filter {}
