/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the indexing program component of the 
 * search engine.
 *
 * @author Guanyuming He
 */

#include "../utility.h"
#include "indexing_common.h"

#include <execinfo.h>
#include <csignal>
#include <iostream>
#include <limits>
#include <memory>

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
			urls::url{"https://hbr.org/topic/subject/strategy"},
			urls::url{"https://hbr.org/topic/subject/marketing"},
			urls::url{"https://hbr.org/topic/subject/economics"},
			urls::url{"https://www.cnbc.com/business"},
			urls::url{"https://www.ft.com"},
			urls::url{"https://edition.cnn.com/business"},
			urls::url{"https://www.economist.com"},
			urls::url{"https://fortune.com/the-latest"},
			urls::url{"https://www.theguardian.com/business"},
			urls::url{"https://www.theatlantic.com/economy"},
			urls::url{"https://www.ibtimes.com/economy-markets"},
			urls::url{"https://www.forbes.com/business"},
			// reuters needs me to enable JS.
			//urls::url{"https://www.reuters.com/business"},
			// wsj blocks me if I don't enable JS and cookies.
			//urls::url{"https://www.wsj.com"},
			//Sean said this is unreliable. Do not use.
			//urls::url{"https://www.businessinsider.com/business"}
			//bloomberg blocked me
			//urls::url{"https://www.bloomberg.com/economics"},
		};
		for (auto&& u : urls)
		{
			start_queue.emplace_back(u);
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
