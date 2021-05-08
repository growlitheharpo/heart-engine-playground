project "heart-test"
	kind "ConsoleApp"
	language "C++"
	set_location()
	include_self()
	include_heart(true)
	include_entt()
	include_googletest(true)
