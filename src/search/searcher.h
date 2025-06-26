#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the class searcher, which searches the database using 
 * string queries.
 *
 * @author Guanyuming He
 */

#include "index.h"

#include <chrono>
#include <xapian.h>
namespace ch = std::chrono;

/**
 * A searcher seaches a database using text queries.
 *
 * Thus, it should be constructed with a database (or a path to it)
 * and have a query method that takes text.
 *
 * In addition, constructors may take global search options, while the query
 * method may take options for that query only.
 */
class searcher
{
public:
	struct query_params
	{
		query_params() {}
		query_params(
			unsigned max_num_results
		) : 
			max_num_results(max_num_results)
		{}

		query_params(const query_params&) = default;
		
		std::optional<unsigned> max_num_results{};
	};

	static constexpr unsigned DEF_MAX_RESULTS = 64u;

public:	
	explicit searcher(
		const fs::path& dbpath, const query_params& par = {}
	);
	explicit searcher(
		index& inddb, const query_params& par = {}
	);
	~searcher() = default;

public:
	/**
	 * @param q the text query. I can't use string_view, as 
	 * query_parser only accepts a const string&.
	 * @param Parameters for this query only. Will override the global
	 * parameters.
	 *
	 * @returns a Mset of matches.
	 */
	xp::MSet query(
		const std::string& q, const query_params& par = {}
	);

private: 
	xp::Database db;
	xp::QueryParser qparser;

	query_params g_pars;

private:
	/**
	 * Apply the default parameter for each 
	 * of query_params that is not set.
	 */
	void apply_def_params();

	void setup_qparser();
};
