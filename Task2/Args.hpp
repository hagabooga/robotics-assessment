#pragma once
#include <string>

struct Args
{
	std::string query;
};

int parse_args(int argc, char* argv[], Args& args);