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

const indexer::dset_t authentic_domains{
	"www.theguardian.com",
		"www.theatlantic.com"
};

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
			authentic_domains,
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
			authentic_domains,
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
