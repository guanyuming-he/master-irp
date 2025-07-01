/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the class searcher.
 * @author Guanyuming He
 */

#include "searcher.h"
#include <xapian.h>

searcher::searcher(
	const fs::path& dbpath, const query_params& par
):
	db(dbpath.string()), g_pars(par)
{
	apply_def_params();
	setup_qparser();
}

searcher::searcher(
	index& inddb, const query_params& par
):
	db(inddb.db), g_pars(par)
{
	apply_def_params();
	setup_qparser();
}

void searcher::apply_def_params()
{
	if (!g_pars.max_num_results.has_value())
		g_pars.max_num_results = DEF_MAX_RESULTS;
}

void searcher::setup_qparser()
{
    qparser.set_stemmer(xp::Stem("en"));
    qparser.set_stemming_strategy(qparser.STEM_SOME);
	
    // Start of prefix configuration.
    qparser.add_prefix("title", "S");
    qparser.add_prefix("text", "XD");
 
	xp::DateRangeProcessor date_proc(
		index::DATE_SLOT, 
		xp::RP_DATE_PREFER_MDY,
		1860
	);
	qparser.add_rangeprocessor(&date_proc);
} 

xp::MSet searcher::query(
	const std::string& q, const query_params& par
) {
	xp::Query xq(qparser.parse_query(q));

	xp::Enquire enq(db);
	enq.set_query(xq);

	// Local par overrides global g_par,
	// if it's value is set.
	//
	// g_pars is guaranteed always set.
	auto max_res = par.max_num_results ?
		par.max_num_results.value() : g_pars.max_num_results.value();

	xp::MSet mset(enq.get_mset(0, max_res));
	return mset;
}
