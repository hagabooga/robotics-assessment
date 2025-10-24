// ConsoleApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Args.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	Args args;
	int parse_args_code = parse_args(argc, argv, args);
	if (parse_args_code)
	{
		return parse_args_code;
	}

	std::cout << "Using data directory: " << args.data_directory << std::endl;

	return 0;
}

