#include <iostream>
#include <filesystem>
#include "Args.hpp"
#include <pqxx/pqxx>
#include <fstream>
#include <sstream>
#include <pqxx/zview.hxx>
#include <string_view>
#include <unordered_set>

using namespace std::literals;

static int task_1(int argc, char *argv[], Args &args)
{
	// 1. Takes an argument --data_directory (path to folder containing the files).
	const int parse_args_code = parse_args(argc, argv, args);
	if (parse_args_code)
	{
		return parse_args_code;
	}

	std::cout << "Using data directory: " << args.data_directory << std::endl;

	// 2. Reads the three text files.
	const std::filesystem::path data_dir_path(args.data_directory);
	const std::filesystem::path points_file_path = data_dir_path / "points.txt";
	const std::filesystem::path categories_file_path = data_dir_path / "categories.txt";
	const std::filesystem::path groups_file_path = data_dir_path / "groups.txt";

	if (!std::filesystem::exists(points_file_path))
	{
		std::cerr << "Error: points.txt file does not exist in the specified data directory." << std::endl;
		return 1;
	}
	if (!std::filesystem::exists(categories_file_path))
	{
		std::cerr << "Error: categories.txt file does not exist in the specified data directory." << std::endl;
		return 1;
	}
	if (!std::filesystem::exists(groups_file_path))
	{
		std::cerr << "Error: groups.txt file does not exist in the specified data directory." << std::endl;
		return 1;
	}

	std::cout << "All required files are present." << std::endl;

	std::ifstream points_fstream(points_file_path);
	if (!points_fstream.is_open())
	{
		std::cerr << "Error: Unable to open points.txt file." << std::endl;
		return 1;
	}
	std::vector<std::pair<double, double>> read_points;
	double x, y;
	while (points_fstream >> x >> y)
	{
		read_points.emplace_back(x, y);
	}
	std::cout << "Read " << read_points.size() << " points from points.txt." << std::endl;

	std::ifstream categories_fstream(categories_file_path);
	if (!categories_fstream.is_open())
	{
		std::cerr << "Error: Unable to open categories.txt file." << std::endl;
		return 1;
	}
	std::vector<int> read_categories;
	double category;
	while (categories_fstream >> category)
	{
		read_categories.push_back(static_cast<int>(category));
	}
	std::cout << "Read " << read_categories.size() << " categories from categories.txt." << std::endl;

	std::ifstream groups_fstream(groups_file_path);
	if (!groups_fstream.is_open())
	{
		std::cerr << "Error: Unable to open groups.txt file." << std::endl;
		return 1;
	}
	std::vector<long long> read_groups;
	double group;
	while (groups_fstream >> group)
	{
		read_groups.push_back(static_cast<long long>(group));
	}
	std::cout << "Read " << read_groups.size() << " groups from groups.txt." << std::endl;

	if (read_points.size() != read_categories.size() || read_points.size() != read_groups.size())
	{
		std::cerr << "Error: Mismatch in number of points, categories, and groups." << std::endl;
		return 1;
	}

	// 3. Populates the PostgreSQL database using libpqxx.
	pqxx::connection conn("host=localhost port=5432 dbname=postgres user=postgres password=asdasd");
	if (!conn.is_open())
	{
		std::cerr << "Error: Unable to open database connection." << std::endl;
		return 1;
	}
	std::cout << "Database connection established successfully." << std::endl;

	try
	{
		pqxx::work txn(conn);

		txn.exec(R"(CREATE TABLE IF NOT EXISTS inspection_group (
id BIGINT NOT NULL,
PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS inspection_region (
    id BIGINT NOT NULL,
    group_id BIGINT,
    PRIMARY KEY (id));

ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS coord_x FLOAT;
ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS coord_y FLOAT;
ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS category INTEGER;)");

		std::unordered_set<long long> existing_groups;
		for (size_t i = 0; i < read_points.size(); i++)
		{
			const auto &p = read_points[i];
			const int pcat = read_categories[i];
			const long long pgrp = read_groups[i];
			if (existing_groups.find(pgrp) == existing_groups.end())
			{
				txn.exec(R"(insert into inspection_group (id) 
                            values ($1) on conflict (id) do nothing;)",
						 pqxx::params{pgrp});
				existing_groups.insert(pgrp);
			}
			txn.exec(R"(insert into inspection_region (id, group_id, coord_x, coord_y, category)
                        values ($1, $2, $3, $4, $5) on conflict (id) do nothing;)",
					 pqxx::params{i + 1, pgrp, p.first, p.second, pcat});
		}
		txn.commit();
		std::cout << "Database populated successfully." << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Database operation failed: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	Args args;
	int task_1_code = task_1(argc, argv, args);
	if (task_1_code)
	{
		return task_1_code;
	}
	return 0;
}
