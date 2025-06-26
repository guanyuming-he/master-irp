/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the indexing program component of the 
 * search engine.
 *
 * @author Guanyuming He
 */

#include "indexer.h"
#include "webpage.h"

#include <csignal>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

using path_filter_func_t = bool(const std::string_view);

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
 * For my indexer, my filter rule is:
 * 1. If the host is one of the keys, recurse.
 * 2. If (1) and also the path can be filtered by the value.
 */
const std::unordered_map<std::string, path_filter_func_t*> filtermap {
	{std::string("www.theguardian.com"), 
		[](const std::string_view p) { return p.starts_with("/business"); }},
	{std::string("www.theatlantic.com"), 
		[](const std::string_view p) { return p.starts_with("/economy"); }},
	{std::string("www.bloomberg.com"), 
		[](const std::string_view p) { 
			return p.starts_with("/economics") || has_words_separated_by_dash(p);
	}},
	{std::string("www.businessinsider.com"), 
		[](const std::string_view p) { 
			return p.starts_with("/business") || has_words_separated_by_dash(p);
	}},
};

bool index_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return val(u.encoded_path());
}

bool recurse_filter(urls::url& u)
{
	// Let's use the strict version for now and see what happens.
	return index_filter(u);

	//return filtermap.contains(
	//	std::string(u.encoded_host())
	//);
}

bool wp_index_filter(webpage& pg)
{
	// for now just return true if the year
	// is within 5 years and the text is not empty.
	return 
		(int)pg.date.year() >= 2020 &&
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

int main(int argc, char* argv[])
{
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
			urls::url{"https://www.theguardian.com/business"},
			urls::url{"https://www.theatlantic.com/economy"},
			urls::url{"https://www.bloomberg.com/economics"},
			urls::url{"https://www.businessinsider.com/business"}
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

	return 0;
}
