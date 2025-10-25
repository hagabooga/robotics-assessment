#include <iostream>
#include <filesystem>
#include "Args.hpp"
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include <fstream>
#include "JsonQuery.hpp"
#include "JsonQueryInspectionRegionSqlBuilder.hpp"

using namespace std::literals;

static int task_2(int argc, char *argv[], Args &args)
{
	// JSON input file argument parsing
	const int parse_args_code = parse_args(argc, argv, args);
	if (parse_args_code)
	{
		return parse_args_code;
	}

	std::cout << "Using query: " << args.query << std::endl;

	nlohmann::json query_json;
	try
	{
		std::ifstream query_file(args.query);
		query_file >> query_json;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: Failed to read or parse JSON file: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "Successfully parsed JSON file." << std::endl;

	JsonQuery jq;
	try
	{
		jq = query_json.get<JsonQuery>();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: Failed to convert JSON to JsonQuery: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "Successfully converted JSON to JsonQuery structure." << std::endl;
	std::cout << "Valid Region Min Point: (" << jq.valid_region.p_min.x << ", " << jq.valid_region.p_min.y << ")" << std::endl
		<< "Valid Region Max Point: (" << jq.valid_region.p_max.x << ", " << jq.valid_region.p_max.y << ")" << std::endl;



	pqxx::connection conn("host=localhost port=5432 dbname=RoboticsAssessment user=postgres password=asdasd");
	if (!conn.is_open())
	{
		std::cerr << "Error: Unable to open database connection." << std::endl;
		return 1;
	}
	std::cout << "Database connection established successfully." << std::endl;

	std::cout << "Building SQL query from JsonQuery." << std::endl;
	JsonQueryInspectionRegionSqlBuilder sql_builder;
	auto result = sql_builder.build_sql(jq);

	std::cout << "Generated SQL Query:" << result << std::endl;

	try
	{
		pqxx::work txn(conn);
		std::vector<std::string> cstrs;
		for (const auto& param : sql_builder.get_sql_params())
		{
			std::visit([&cstrs](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, int>)
				{
					cstrs.push_back(std::to_string(arg));
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					cstrs.push_back(std::to_string(arg));
				}
				else if constexpr (std::is_same_v<T, std::vector<int>>)
				{
					std::string array_str = "{";
					for (size_t i = 0; i < arg.size(); ++i)
					{
						array_str += std::to_string(arg[i]);
						if (i < arg.size() - 1)
						{
							array_str += ",";
						}
					}
					array_str += "}";
					cstrs.push_back(array_str);
				}
				}, param);
		}
		pqxx::params params{};
		for (size_t i = 0; i < cstrs.size(); ++i)
		{
			std::cout << "Parameter " << (i + 1) << ": " << cstrs[i] << std::endl;
			params.append(cstrs[i]);
		}
		pqxx::result db_result = txn.exec(result, params);
		std::cout << "Query executed successfully. Retrieved " << db_result.size() << " rows." << std::endl;
		for (const auto& row : db_result) {
			int64_t id = row["id"].as<int64_t>();
			int64_t group_id = row["group_id"].as<int64_t>();
			double coord_x = row["coord_x"].as<double>();
			double coord_y = row["coord_y"].as<double>();
			int category = row["category"].as<int>();

			std::cout << "id=" << id
				<< ", group=" << group_id
				<< ", x=" << coord_x
				<< ", y=" << coord_y
				<< ", category=" << category
				<< std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: Database query execution failed: " << e.what() << std::endl;
		return 1;
	}

//	try
//	{
//		pqxx::work txn(conn);
//		const pqxx::params sql_params{
//			jq.valid_region.p_min.x, jq.valid_region.p_min.y,
//			jq.valid_region.p_min.x, jq.valid_region.p_max.y,
//			jq.valid_region.p_max.x, jq.valid_region.p_max.y,
//			jq.valid_region.p_max.x, jq.valid_region.p_min.y,
//			jq.query.operator_crop.region.p_min.x, jq.query.operator_crop.region.p_min.y,
//			jq.query.operator_crop.region.p_min.x, jq.query.operator_crop.region.p_max.y,
//			jq.query.operator_crop.region.p_max.x, jq.query.operator_crop.region.p_max.y,
//			jq.query.operator_crop.region.p_max.x, jq.query.operator_crop.region.p_min.y,
//			jq.query.operator_crop.category,
//			jq.query.operator_crop.one_of_groups
//		};
//
//		pqxx::result result;
//		if (jq.query.operator_crop.proper)
//		{
//			std::cout << "Executing proper containment query." << std::endl;
//			result = txn.exec(R"(WITH points_in_rect AS (
//SELECT *
//FROM inspection_region
//WHERE ST_Within(
//        geom,
//        ST_MakePolygon(ST_MakeLine(ARRAY[
//            ST_MakePoint($1,$2),
//            ST_MakePoint($3,$4),
//            ST_MakePoint($5,$6),
//            ST_MakePoint($7,$8),
//            ST_MakePoint($1,$2)
//        ])))
//AND ST_Within(
//        geom,
//        ST_MakePolygon(ST_MakeLine(ARRAY[
//			ST_MakePoint($9,$10), 
//			ST_MakePoint($11,$12),
//			ST_MakePoint($13,$14), 
//			ST_MakePoint($15,$16),
//			ST_MakePoint($9,$10)
//        ])))
//AND category = $17
//AND group_id = ANY($18))
//SELECT *
//FROM inspection_region r
//WHERE r.group_id IN (
//        SELECT group_id
//        FROM points_in_rect
//        GROUP BY group_id
//        HAVING COUNT(*) = (
//            SELECT COUNT(*)
//            FROM inspection_region r2
//            WHERE r2.group_id = points_in_rect.group_id
//        )
//    )
//AND category = $17
//AND group_id = ANY($18);)", sql_params);
//		}
//		else
//		{
//			std::cout << "Executing non-proper containment query." << std::endl;
//			result = txn.exec(R"(SELECT *
//FROM inspection_region
//WHERE ST_Within(
//        geom,
//        ST_MakePolygon(ST_MakeLine(ARRAY[
//            ST_MakePoint($1,$2),
//            ST_MakePoint($3,$4),
//            ST_MakePoint($5,$6),
//            ST_MakePoint($7,$8),
//            ST_MakePoint($1,$2)
//        ])))
//AND ST_Within(
//        geom,
//        ST_MakePolygon(ST_MakeLine(ARRAY[
//			ST_MakePoint($9,$10), 
//			ST_MakePoint($11,$12),
//			ST_MakePoint($13,$14), 
//			ST_MakePoint($15,$16),
//			ST_MakePoint($9,$10)
//        ])))
//AND category = $17
//AND group_id = ANY($18);)", sql_params);
//		}
//
//		std::cout << "Query executed successfully. Retrieved " << result.size() << " rows." << std::endl;
//
//		for (const auto& row : result) {
//			int64_t id = row["id"].as<int64_t>();
//			int64_t group_id = row["group_id"].as<int64_t>();
//			double coord_x = row["coord_x"].as<double>();
//			double coord_y = row["coord_y"].as<double>();
//			int category = row["category"].as<int>();
//
//			std::cout << "id=" << id
//				<< ", group=" << group_id
//				<< ", x=" << coord_x
//				<< ", y=" << coord_y
//				<< ", category=" << category
//				<< std::endl;
//		}
//
//	}
//	catch (const std::exception &e)
//	{
//		std::cerr << "Error: Database query execution failed: " << e.what() << std::endl;
//		return 1;
//	}


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
