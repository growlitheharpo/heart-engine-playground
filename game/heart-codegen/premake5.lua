project "heart-codegen"
	set_location()
	language "C#"
	kind "ConsoleApp"
	files {
		"src/**.cs"
	}
	links {
		"System",
		"System.Runtime"
	}
