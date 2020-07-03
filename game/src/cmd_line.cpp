#include "cmd_line.h"

#include <heart/stl/string.h>
#include <heart/stl/vector.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

cxxopts::ParseResult ParseCommandLine()
{
	cxxopts::Options options("");
	options.add_options("",
		{
			cxxopts::Option("dataroot", "Path to the root data. Use {%cwd} as a token for the current working directory.", cxxopts::value<std::string>()->default_value("{%cwd}\\..\\data\\")),
			cxxopts::Option("framecount", "Number of frames to run for CI", cxxopts::value<int>()->implicit_value("60")->default_value("-1")),
		});

	int argc = 0;
	auto argvW = ::CommandLineToArgvW(GetCommandLine(), &argc);

	std::vector<std::string> strs;
	strs.resize(argc);
	for (size_t i = 0; i < strs.size(); ++i)
	{
		int size = ::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);

		if (size > 0)
		{
			strs[i].resize(size);
			::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, strs[i].data(), int(strs[i].size()), NULL, NULL);
		}
	}

	hrt::vector<char*> cstrs;
	std::transform(strs.begin(), strs.end(), std::back_inserter(cstrs), [](auto& s) { return s.data(); });

	char** argv = cstrs.data();
	auto r = options.parse(argc, argv);

	::LocalFree(argvW);
	return r;
}
