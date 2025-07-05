/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the indexing program component of the 
 * search engine.
 *
 * @author Guanyuming He
 */

#include "utility.h"
#include "indexer.h"
#include "webpage.h"

#include <execinfo.h>
#include <csignal>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * In general, the condition of recursing is more loose than that of indexing.
 * If a document is indexed, then naturally we would want to also recurse it 
 * to find related ones.
 *
 * @returns (b1, b2).
 * Recurse iff b1; index iff b2.
 */
using path_filter_func_t = 
std::pair<bool, bool>
(const std::string_view);

// At least three words in a 
// word-word-word-... pattern
// This is common it news article addresses.
static const std::string words_sep_dash_pattern("[A-Za-z](-[A-Za-z]+){2,}");
static const std::regex words_sep_dash_re(words_sep_dash_pattern);
bool has_words_separated_by_dash(const std::string_view p)
{
	return std::regex_search(
		p.begin(), p.end(), 
		words_sep_dash_re
	);
}
/**
 * @returns true iff the path has date encoded in it.
 * 
 * The test cases for this function are done in regex testing websites
 * like regex101.com.
 * Test cases (must accept):
 * 2025-02-01
 * 2025-29-07
 * 08-12-2025
 * 12-09-2025
 * 2025/11/03
 * 2025/03/15
 * 11/20/2025
 * 30/01/2025
 * (must not accept)
 * -1/-2/2025
 * 1/1/1
 * 2021/2022/2023
 */
static const std::string date_in_path_pattern(
	R"((^|\/)\d{4}[-/]\d{1,2}[-/]\d{1,2}($|\/)|(^|\/)\d{1,2}[-/]\d{1,2}[-/]\d{4}($|\/))"
);
static const std::regex date_in_path_regex(date_in_path_pattern);
bool has_dates(const std::string_view p)
{
	return std::regex_search(
		p.begin(), p.end(),
		date_in_path_regex
	);
}
/**
 * For my indexer, my url filter rule is:
 * for the host, executes a function which
 * returns (b_recurse, b_index), given input path of the url.
 */
const std::unordered_map<std::string, path_filter_func_t*> filtermap {
	{std::string("www.ft.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/companies") ||
				p.starts_with("/markets");
			bool b2 = 
				p.starts_with("/content");
			// here I don't use b1||b2 for the first because b2 is too general.
			return {b1, b2};
	}},
	{std::string("edition.cnn.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/business");
			bool b2 = 
				has_words_separated_by_dash(p) &&
				has_dates(p) && 
				p.contains("/business/");
			return {b1||b2, b2};
	}},
	{std::string("www.economist.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.empty() ||
				p.starts_with("/topics");
			bool b2 = 
				has_words_separated_by_dash(p) &&
				has_dates(p);
			return {b1||b2, b2};
	}},
	{std::string("fortune.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> {
			bool b1 = 
				p.starts_with("/the-latest") ||
				p.starts_with("/section");
			bool b2 = 
				p.starts_with("/article") ||
				has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.theguardian.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b = p.starts_with("/business"); 
			return {b, b};
	}},
	{std::string("www.theatlantic.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b = p.starts_with("/economy"); 
			return {b, b};
	}},
	{std::string("www.bloomberg.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/economics");
			bool b2 = p.starts_with("news/articles");
			return {b1||b2, b2};
	}},
	{std::string("www.ibtimes.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/economy-markets");
			bool b2 = has_words_separated_by_dash(p);
			return {b1||b2, b2};
	}},
	{std::string("www.forbes.com"), 
		[](const std::string_view p) -> std::pair<bool,bool> { 
			bool b1 = p.starts_with("/business");
			bool b2 = p.starts_with("/sites");
			return {b1||b2, b2};
	}},
	// Don't index this website, according to Sean.
	//{std::string("www.businessinsider.com"), 
	//	[](const std::string_view p) -> std::pair<bool,bool> {
	//		bool b1 = p.starts_with("/business");
	//		bool b2 = has_words_separated_by_dash(p);
	//		return {b1||b2, b2};
	//}},
};

bool index_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return val(u.encoded_path()).second;
}

bool recurse_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return val(u.encoded_path()).first;
}

bool wp_index_filter(webpage& pg)
{
	// for now just return true if the year
	// is within last 2 years and the text is not empty.
	return 
		(int)pg.date.year() >= 2024 &&
		!pg.get_text().empty();
}

bool wp_recurse_filter(webpage& pg)
{
	// recurse only if either its title or its text is not empty.
	return 
		!pg.get_text().empty() &&
		!pg.title.empty();
}

// Whether use this or load from file depends on the cmd args.
indexer::uque_t start_queue;
std::unique_ptr<indexer> i;

void segfault_handler(int sig) {
	// Get void*'s for all entries on the stack
	void *array[64];
	int size = backtrace(array, 64);

	// Print to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);

	// Before exiting, commit to database lest it corrupt.
	// Resetting i will call indexer's dtor, which in turn calls 
	// index's, which writes to the database.
	if (i.get())
		i.reset(nullptr);

	std::exit(-1);
}

int main(int argc, char* argv[])
{
	std::signal(SIGSEGV, segfault_handler);
	
	global_init();

	if (argc < 3 || argc > 5)
	{
		std::cerr 
			<< "Usage:\n "
			<< argv[0] << " db_path queue_path"
		    << " [load_queue:bool] [index_limit]"
			<< std::endl;
		return -1;
	}

	bool load_queue;
	size_t index_limit{std::numeric_limits<size_t>::max()};

	if (argc >= 4)
	{
		std::string lq_str(argv[3]);
		if (lq_str == "0" || lq_str == "false")
			load_queue = false;
		else
			load_queue = true;

		if (argc >= 5)
		{
			index_limit = std::stoi(argv[4]);
		}
	}
	
	if (load_queue) // don't use start_queue.
	{		
		i = std::make_unique<indexer>(
			fs::path(argv[1]), fs::path(argv[2]),
			&index_filter, &recurse_filter,
			&wp_index_filter, &wp_recurse_filter,
			index_limit
		);
	}
	else // use start_queue.
	{
		auto urls = {
			urls::url{"https://www.ft.com"},
			//urls::url{"https://edition.cnn.com/business"},
			//urls::url{"https://www.economist.com"},
			//urls::url{"https://fortune.com/the-latest"},
			//urls::url{"https://www.theguardian.com/business"},
			//urls::url{"https://www.theatlantic.com/economy"},
			//urls::url{"https://www.bloomberg.com/economics"},
			//urls::url{"https://www.ibtimes.com/economy-markets"},
			//urls::url{"https://www.forbes.com/business"},
			//urls::url{"https://www.businessinsider.com/business"}
		};
		for (auto&& u : urls)
		{
			start_queue.emplace(u);
		}

		i = std::make_unique<indexer>(
			fs::path(argv[1]), fs::path(argv[2]),
			std::move(start_queue),
			&index_filter, &recurse_filter,
			&wp_index_filter, &wp_recurse_filter,
			index_limit
		);
	}


	// Register for SIGINT and start indexing.
	std::signal(
		SIGINT, 
		[](int sig) {
			if (sig == SIGINT) i->interrupt();
		}
	);	
	std::cout << "Indexing started. Press Ctrl+C to interrupt.\n";
	i->start_indexing();

	global_uninit();

	return 0;
}
