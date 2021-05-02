project "heart-core"
	kind "StaticLib"
	set_location()
	include_self()
	include_heart()
	include_entt()

	includedirs {
		get_root_location() .. "external/rapidjson/include",
	}
