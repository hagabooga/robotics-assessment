#pragma once
#include <string>

struct Args
{
	std::string data_directory;
};

int parse_args(int argc, char* argv[], Args& args);