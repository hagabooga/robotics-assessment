#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <variant>

struct Point
{
    double x;
    double y;
};

struct ValidRegion
{
	Point p_min;
	Point p_max;
};

struct OperatorCrop
{
    ValidRegion region;
	int category = -1;
	std::vector<int> one_of_groups{};
    bool proper = false;
};

struct Query;

struct OperatorAnd
{
	std::vector<Query> ops;
};

struct OperatorOr
{
	std::vector<Query> ops;
};

struct Query
{
	std::variant<OperatorCrop, OperatorAnd, OperatorOr> op;
};

struct JsonQuery {
	ValidRegion valid_region;
	Query query;
};


void from_json(const nlohmann::json& j, Point& p);
void from_json(const nlohmann::json& j, ValidRegion& vr);
void from_json(const nlohmann::json& j, OperatorCrop& oc);
void from_json(const nlohmann::json& j, OperatorAnd& oa);
void from_json(const nlohmann::json& j, OperatorOr& oo);
void from_json(const nlohmann::json& j, Query& q);
void from_json(const nlohmann::json& j, JsonQuery& jq);