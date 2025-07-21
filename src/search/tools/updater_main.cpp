/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the updater,
 * which does one of the two things, based on given command line arguments.
 * 1. If the database is larger than a specified number,
 * then remove the oldest documents to make the size within the limit.
 * 2. Updates the database by trying to retrieving from the RSS feeds
 * of specific domains.
 *
 * @author Guanyuming He
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "../index.h"

std::unordered_set<std::string> rss_urls {
	"https://feeds.a.dj.com/rss/WSJcomUSBusiness.xml",
	"https://feeds.a.dj.com/rss/RSSMarketsMain.xml",
	"http://rss.nytimes.com/services/xml/rss/nyt/Business.xml",
	"http://www.economist.com/feeds/print-sections/77/business.xml",
	"http://www.business-standard.com/rss/latest.rss",
	"http://feeds.harvardbusiness.org/harvardbusiness?format=xml",
	"https://economictimes.indiatimes.com/rssfeedsdefault.cms",
};

// This is used if cmd arg does not have max_doc.
static constexpr unsigned DEF_MAX_DOC = 100000;
/**
 * It does function 1:
 * removes the oldest documents from the database,
 * until its size is within max_num_doc
 */
void shrink_database(
	const char* path, unsigned max_num_doc
) {
	class index db(path);	
	db.shrink(max_num_doc, index::shrink_policy::OLDEST);
}

static constexpr unsigned DEF_NUM_ADD = 1000;
/**
 * It does function 2:
 * Adds latest document from some internally specified source (typically RSS
 * feed).
 *
 * The basic idea is like this:
 * 1. I have a list of RSS feed URLs.
 * 2. I form a queue of start URLs by scraping the RSS.
 * 3. Then I call indexer::start_indexing()
 */
void update_database(
	const char* path, unsigned num_add
) {
	throw std::runtime_error("Not implemented.\n");
}

int main(int argc, char* argv[])
{
	if (argc < 3 || argc > 4)
	{
		std::cerr 
			<< "Usage:\n"
			<< argv[0] << "<db_path> shrink <max_num>?\n"
			<< argv[0] << "<db_path> update <num_add>?\n";
		return -1;
	}

	if (std::string("shrink") == argv[2])
	{
		unsigned max_num = 
			argc == 4 ?
			std::stoi(argv[3]) :
			DEF_MAX_DOC;
		shrink_database(argv[1], max_num);
	}
	else if (std::string("update") == argv[2])
	{
		unsigned add_num = 
			argc == 4 ?
			std::stoi(argv[3]) :
			DEF_NUM_ADD;
		shrink_database(argv[1], add_num);
	}

	return 0;
}
