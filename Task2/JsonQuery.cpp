#include "JsonQuery.hpp"

void from_json(const nlohmann::json& j, Point& p)
{
	p.x = j.at("x").get<double>();
	p.y = j.at("y").get<double>();
}

void from_json(const nlohmann::json& j, ValidRegion& vr)
{
	vr.p_min = j.at("p_min").get<Point>();
	vr.p_max = j.at("p_max").get<Point>();
}

void from_json(const nlohmann::json& j, OperatorCrop& oc)
{
	oc.region = j.at("region").get<ValidRegion>();
	if (j.contains("category"))
	{
		oc.category = j.at("category").get<int>();
	}
	if (j.contains("one_of_groups"))
	{
		oc.one_of_groups = j.at("one_of_groups").get<std::vector<int>>();
	}
	if (j.contains("proper"))
	{
		oc.proper = j.at("proper").get<bool>();
	}
}

void from_json(const nlohmann::json& j, OperatorAnd& oa)
{
	for (const auto& item : j)
	{
		oa.ops.push_back(item.get<Query>());
	}
}

void from_json(const nlohmann::json& j, OperatorOr& oo)
{
	for (const auto& item : j)
	{
		oo.ops.push_back(item.get<Query>());
	}
}

void from_json(const nlohmann::json& j, Query& q)
{
	if (j.contains("operator_crop"))
	{
		q.op = j.at("operator_crop").get<OperatorCrop>();
	}
	else if (j.contains("operator_and"))
	{
		q.op = j.at("operator_and").get<OperatorAnd>();
	}
	else if (j.contains("operator_or"))
	{
		q.op = j.at("operator_or").get<OperatorOr>();
	}
	else
	{
		throw std::invalid_argument("Invalid Query JSON: missing operator_crop, operator_and, or operator_or");
	}
}

void from_json(const nlohmann::json& j, JsonQuery& jq)
{
	jq.valid_region = j.at("valid_region").get<ValidRegion>();
	jq.query = j.at("query").get<Query>();
}
