#include "Args.hpp"
#include <gflags/gflags.h>
#include <iostream>
#include <filesystem>

DEFINE_string(query, "", "Query points based on a JSON input file");

int parse_args(int argc, char* argv[], Args& args)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_query.empty())
    {
		std::cerr << "Error: --query argument is required." << std::endl;
        return 1;
    }

    std::filesystem::path query(FLAGS_query);
	if (!std::filesystem::exists(query) || !std::filesystem::is_regular_file(query))
    {
        std::cerr << "Error: Specified query does not exist or is not a file." << std::endl;
        return 1;
    }
    
    args.query = FLAGS_query;

	gflags::ShutDownCommandLineFlags();
    return 0;
}
