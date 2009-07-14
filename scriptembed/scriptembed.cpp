// scriptembed.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _WIN32
#define ARGV_CHAR _TCHAR
#define MAIN_NAME _tmain
#else
#define ARGV_CHAR char
#define MAIN_NAME main
#endif

int MAIN_NAME(int argc, ARGV_CHAR* argv[])
{
	if (argc != 4)
	{
		std::cerr << "Usage: scriptembed source.qts dest.cpp varname" << std::endl;
		return 1;
	}

	std::ifstream ifs(argv[1]);
	if (ifs.bad())
	{
		std::cerr << "Failed to open input file " << argv[1] << std::endl;
		return 1;
	}

	std::ofstream ofs(argv[2]);
	if (ofs.bad())
	{
		std::cerr << "Failed to open output file " << argv[2] << std::endl;
		return 1;
	}

	// Convert varname argument to a valid C++ identifier
	// (As a side effect, convert _TCHAR* to char* to std::string on Win32)
	std::string varname;
	for (ARGV_CHAR* c = argv[3]; *c; ++c)
	{
		if (	(*c >= '0' && *c <= '9')
			||	(*c >= 'A' && *c <= 'Z')
			||	(*c >= 'a' && *c <= 'z')
			||	(*c == '_') )
		{
			varname.push_back(static_cast<char>(*c));
		}
		else
			varname.push_back('_');
	}

	// Write the first line
	ofs << "static const char " << varname << "[] = " << std::endl;

	// Write the other lines
	while (!ifs.eof())
	{
		std::string line; std::getline(ifs, line);
		ofs << "\""; // open quote
		for (std::string::const_iterator i = line.begin(); i != line.end(); ++i)
		{
			if (*i == '\\' || *i == '"') // needs escaping
				ofs << '\\';
			ofs << *i;
		}
		ofs << "\\n\"" << std::endl; // "\n", close quote, new line
	}

	// Write the last line
	ofs << ";" << std::endl;

	return 0;
}

