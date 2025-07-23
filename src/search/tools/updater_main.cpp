/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the updater,
 * which does the two things, in the order they are given.
 * 1. Updates the database by trying to retrieving from the RSS feeds
 * of specific domains.
 * 2. If the database is larger than a specified number,
 * then remove the oldest documents to make the size within the limit.
 *
 * @author Guanyuming He
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "../index.h"
#include "../indexer.h"
#include "../url2rss.h"
#include "../utility.h"
#include "indexing_common.h"

/*********************** THING 1 ***********************/
std::unordered_set<std::string> rss_urls {
	"https://www.entrepreneur.com/latest.rss",
	"http://rss.nytimes.com/services/xml/rss/nyt/Business.xml",
	//"http://feeds.harvardbusiness.org/harvardbusiness?format=xml",
	"https://www.inc.com/rss",
	"https://moxie.foxbusiness.com/google-publisher/latest.xml",
	"https://feeds.a.dj.com/rss/WSJcomUSBusiness.xml",
	"https://feeds.a.dj.com/rss/RSSMarketsMain.xml",
	// These site blocked me even if I only want their RSS.
	//"http://www.economist.com/feeds/print-sections/77/business.xml",
	//"http://www.business-standard.com/rss/latest.rss",
};
static constexpr unsigned DEF_NUM_ADD = 1000;
/**
 * Adds latest document from rss_urls.
 *
 * The basic idea is like this:
 * 1. I have a list of RSS feed URLs.
 * 2. I form a queue of start URLs by scraping the RSS.
 * 3. Then I call indexer::start_indexing()
 */
void update_database(
	const char* path, unsigned num_add
) {
	// These pages from RSS will only have a title and a link.
	// I only need the links for indexing.
	indexer::uque_t urls_from_rss;
	url2rss convertor;

	for (const auto& url_str : rss_urls)
	{
		std::vector<webpage> pages;

		// If for some reason some RSS cannot be parsed,
		// then fail gracefully. Just ignore that RSS.
		try 
		{
			rss r(convertor.convert(url_str));
			pages = r.read_webpages();
		}
		catch (...)
		{
			continue;
		}

		for (const auto& p : pages)
			urls_from_rss.push_back(p.url);
	}

	util_log(
		"Read " + std::to_string(urls_from_rss.size()) +
		" links from the RSS feeds.\n"
	);

	indexer idxer(
		path, 
		// It probably is not empty, after num_to_add is reached.
		// But it is of little use to us now.
		"./updater_que", 
		std::move(urls_from_rss),
		&index_filter, &recurse_filter,
		&wp_index_filter, &wp_recurse_filter,
		num_add
	);	
	idxer.start_indexing();
}


/*********************** THING 2 ***********************/
// This is used if cmd arg does not have max_doc.
static constexpr unsigned DEF_MAX_DOC = 100000;
/**
 * removes the oldest documents from the database,
 * until its size is within max_num_doc
 */
void shrink_database(
	const char* path, unsigned max_num_doc
) {
	class index db(path);	
	db.shrink(max_num_doc, index::shrink_policy::OLDEST);
}

int main(int argc, char* argv[])
{
	global_init();

	if (argc < 2 || argc > 4)
	{
		std::cerr 
		<< "Usage:\n"
		<< argv[0] << "<db_path> [<num_to_add> [<max_num>]]\n"
		<< 
		", where <num_to_add> is the max number of documents to update\n"
		" from RSS feeds and <max_num> is the maximum number of documents\n"
		" the database can have (i.e. the number to shrink the database\n"
		" to). <num_to_add> defaults to 1000 and <max_num> defaults to \n"
		"100000\n";
		return -1;
	}

	unsigned num_to_add = DEF_NUM_ADD;
	unsigned max_num = DEF_MAX_DOC;

	if (argc >= 3)
	{
		num_to_add = std::stoi(argv[2]);	
	}
	if (argc >= 4)
	{
		max_num = std::stoi(argv[3]);
	}

	// Defensively do this so that I can't accidentally delete most documents
	// from my database.
	if (max_num < 10000)
	{
		std::cerr << "max_num is too small!\n";
		return -1;
	}

	update_database(argv[1], num_to_add);
	shrink_database(argv[1], max_num);

	global_uninit();

	return 0;
}
