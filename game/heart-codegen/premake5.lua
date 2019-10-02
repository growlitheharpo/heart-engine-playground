project "heart-codegen"
	set_location()
	language "C#"
	clr "Unsafe"

	kind "ConsoleApp"
	files {
		"src/**.cs"
	}

	buildoptions {
		"/unsafe"
	}

	nuget {
		"libclang:8.0.0",
		"Microsoft.Bcl.HashCode:1.0.0-preview6.19303.8",
		"System.Buffers:4.4.0",
		"System.Numerics.Vectors:4.4.0",
		"System.Runtime.CompilerServices.Unsafe:4.5.2",
		"System.Memory:4.5.3",
		"ClangSharp:8.0.0-beta",
	}

	links {
		"System",
		"System.Runtime",
		"System.Memory",
		"libclang",
		"Microsoft.Bcl.HashCode",
		"System.Buffers",
		"System.Numerics.Vectors",
		"System.Runtime.CompilerServices.Unsafe",
		"System.Memory",
		"ClangSharp",
	}
