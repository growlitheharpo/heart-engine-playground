--[[ Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
--]]
project "heart-codegen"
	set_location()

	-- We need "unsafe" because libclang requires it :/
	language "C#"
	clr "Unsafe"

	-- codegen always runs with optimization
	optimize "On"

	kind "ConsoleApp"
	files {
		"src/**.cs"
	}

	links {
		"System",
		"System.Runtime",
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
		"System.Memory",
		"libclang",
		"Microsoft.Bcl.HashCode",
		"System.Buffers",
		"System.Numerics.Vectors",
		"System.Runtime.CompilerServices.Unsafe",
		"System.Memory",
		"ClangSharp",
	}
