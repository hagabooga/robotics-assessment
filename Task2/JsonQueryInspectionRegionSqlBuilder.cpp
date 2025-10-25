#include "JsonQueryInspectionRegionSqlBuilder.hpp"

std::string JsonQueryInspectionRegionSqlBuilder::build_sql(const JsonQuery& jq)
{
	std::ostringstream ss;

	ss << get_valid_region_cte(jq.valid_region);
	SqlNode root = query_recursive_helper(jq.query);
	for (const auto& cte : cte_sqls)
	{
		ss << ",\n" << cte.cte_sql;
	}

	ss << "\nselect * from " << root.cte_name 
		<< " order by ST_Y(geom), ST_X(geom) "
		<< ";";

	return ss.str();
}

int JsonQueryInspectionRegionSqlBuilder::add_parameter(const SqlParamValue value)
{
	sql_params.push_back(value);
	return param_index++;
}

std::string JsonQueryInspectionRegionSqlBuilder::get_within_clause(Point pmin, Point pmax)
{
	std::ostringstream ss;
	const int pmin_x_idx = add_parameter(pmin.x);
	const int pmin_y_idx = add_parameter(pmin.y);
	const int pmax_x_idx = add_parameter(pmax.x);
	const int pmax_y_idx = add_parameter(pmax.y);

	ss << "ST_Within(geom, ST_MakePolygon(ST_MakeLine(ARRAY["
		<< "ST_MakePoint($" << pmin_x_idx << ",$" << pmin_y_idx << "), "
		<< "ST_MakePoint($" << pmin_x_idx << ",$" << pmax_y_idx << "), "
		<< "ST_MakePoint($" << pmax_x_idx << ",$" << pmax_y_idx << "), "
		<< "ST_MakePoint($" << pmax_x_idx << ",$" << pmin_y_idx << "), "
		<< "ST_MakePoint($" << pmin_x_idx << ",$" << pmin_y_idx << ")"
		<< "]))) ";

	return ss.str();
}

std::string JsonQueryInspectionRegionSqlBuilder::get_valid_region_cte(const ValidRegion& vr)
{
	std::ostringstream ss;
	ss << "with valid_region as ("
		<< "select * from inspection_region "
		<< "where " << get_within_clause(vr.p_min, vr.p_max)
		<< ") ";
	return ss.str();
}

SqlNode JsonQueryInspectionRegionSqlBuilder::get_crop_cte(const OperatorCrop& oc)
{
	std::ostringstream ss;
	std::string name = "crop" + std::to_string(cte_counter++);

	if (oc.proper)
	{
		ss << name << " as (" <<
			"select * from valid_region r "
			<< "where r.group_id IN ("
			<< "select group_id "
			<< "from valid_region "
			<< "group by group_id "
			<< "having COUNT(*) = ("
			<< "select COUNT(*) "
			<< "from inspection_region r2 "
			<< "where r2.group_id = valid_region.group_id"
			<< ")) "
			<< "and " << get_within_clause(oc.region.p_min, oc.region.p_max);
	}
	else
	{
		ss << name << " as (" <<
			"select * from valid_region "
			<< "where " << get_within_clause(oc.region.p_min, oc.region.p_max);
	}
	if (oc.category != -1)
	{
		ss << " and category = $" << add_parameter(oc.category);
	}
	if (!oc.one_of_groups.empty())
	{
		ss << " and group_id = any($" << add_parameter(oc.one_of_groups) << ")";
	}
	ss << ") ";
	SqlNode node{ name, ss.str() };
	cte_sqls.push_back(node);
	return node;
}

SqlNode JsonQueryInspectionRegionSqlBuilder::query_recursive_helper(const Query& q)
{
	return std::visit([&](auto&& op)
		{
			using T = std::decay_t<decltype(op)>;
			if constexpr (std::is_same_v<T, OperatorCrop>)
			{
				return get_crop_cte(op);
			}
			else if constexpr (std::is_same_v<T, OperatorAnd> || std::is_same_v<T, OperatorOr>)
			{
				std::vector<SqlNode> children;
				for (const auto& sub_q : op.ops)
				{
					children.push_back(query_recursive_helper(sub_q));
				}
				const bool is_and = std::is_same_v<T, OperatorAnd>;
				std::string name = (is_and ? "and" : "or") + std::to_string(cte_counter++);
				std::ostringstream ss;
				ss << name << " as (";
				for (size_t i = 0; i < children.size(); ++i)
				{
					ss << "select * from " << children[i].cte_name;
					if (i < children.size() - 1)
					{
						ss << (is_and ? " intersect " : " union ");
					}
				}
				ss << ") ";
				cte_sqls.push_back(SqlNode{ name, ss.str() });
				return cte_sqls.back();
			}
		}, q.op);
}

