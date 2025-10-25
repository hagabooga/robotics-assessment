#pragma once
#include <vector>
#include <any>
#include <string>
#include "JsonQuery.hpp"
#include <sstream>
#include <unordered_map>

typedef std::variant<int, double, std::vector<int>> SqlParamValue;

struct SqlNode
{
	std::string cte_name;
	std::string cte_sql;
};

class JsonQueryInspectionRegionSqlBuilder
{
public:
	JsonQueryInspectionRegionSqlBuilder() : param_index(1)
	{
	}
	std::string build_sql(const JsonQuery& jq);
	std::vector<SqlParamValue> get_sql_params() const
	{
		return sql_params;
	}

private:
	int param_index;
	int cte_counter = 0;
	std::vector<SqlNode> cte_sqls;
	std::vector<SqlParamValue> sql_params;


	int add_parameter(const SqlParamValue value);
	std::string get_within_clause(Point pmin, Point pmax);
	std::string get_valid_region_cte(const ValidRegion& vr);
	SqlNode get_crop_cte(const OperatorCrop& oc);
	SqlNode query_recursive_helper(const Query& q);

};