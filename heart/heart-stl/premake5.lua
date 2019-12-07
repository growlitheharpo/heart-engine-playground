project "heart-stl"
	kind "StaticLib"
	set_location()
	include_self()
	include_heart()

	filter { "action:vs*" }
		files {
			"hrt.natvis"
		}
	filter {}
