#include <iostream>
#include <filesystem>
#include "Args.hpp"

using namespace std::literals;

static int task_2(int argc, char *argv[], Args &args)
{
	const int parse_args_code = parse_args(argc, argv, args);
	if (parse_args_code)
	{
		return parse_args_code;
	}

	std::cout << "Using query: " << args.query << std::endl;

	return 0;
}

int main(int argc, char *argv[])
{
	Args args;
	int task_2_code = task_2(argc, argv, args);
	if (task_2_code)
	{
		return task_2_code;
	}
	return 0;
}
