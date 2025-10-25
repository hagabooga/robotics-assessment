#include "Args.hpp"
#include <gflags/gflags.h>
#include <iostream>
#include <filesystem>

DEFINE_string(data_directory, "", "Path to folder containing the files");

int parse_args(int argc, char *argv[], Args &args)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_data_directory.empty())
    {
        std::cerr << "Error: --data_directory argument is required." << std::endl;
        return 1;
    }

    std::filesystem::path data_directory(FLAGS_data_directory);
    if (!std::filesystem::exists(data_directory) || !std::filesystem::is_directory(data_directory))
    {
        std::cerr << "Error: Specified data_directory does not exist or is not a directory." << std::endl;
        return 1;
    }

    args.data_directory = FLAGS_data_directory;

    gflags::ShutDownCommandLineFlags();
    return 0;
}
