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

#include <csignal>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * For my indexer, my filter rule is:
 * 1. If the host is one of the keys, recurse.
 * 2. If (1) and also if the val is the front of the substring.
 */
const std::unordered_map<std::string, std::string> filtermap {
	{std::string("www.theguardian.com"), std::string("/business")},
	{std::string("www.theatlantic.com"), std::string("/economy")},
};

bool index_filter(urls::url& u)
{
	std::string key{u.encoded_host()};
	if (!filtermap.contains(key))
		return false;

	auto val {filtermap.at(key)};
	return u.encoded_path().starts_with(val);
}

bool recurse_filter(urls::url& u)
{
	return filtermap.contains(
		std::string(u.encoded_host())
	);
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
			index_limit
		);
	}
	else // use start_queue.
	{
		auto urls = {
			urls::url{"https://www.theguardian.com/uk/business"},
			urls::url{"https://www.theatlantic.com/economy/"}
		};
		for (auto&& u : urls)
		{
			start_queue.emplace(u);
		}

		i = std::make_unique<indexer>(
			fs::path(argv[1]), fs::path(argv[2]),
			std::move(start_queue),
			&index_filter, &recurse_filter,
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
