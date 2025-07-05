/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the class indexer.
 *
 * @author Guanyuming He
 */

#include "indexer.h"
#include "url2html.h"
#include "utility.h"
#include "webpage.h"

#include <cstdint>
#include <fstream>
#include <unordered_set>
#include <stdexcept>
#include <string>

indexer::uque_t indexer::load_url_q(
	const fs::path& p
) {
	
	std::ifstream ifs(p, std::ios::binary);
    if (!ifs) // Does not exist. Error.
		throw std::runtime_error(
			"queue file does not exist: " + p.string()
		);

	// File exists. Read it.
	uque_t ret;
	// We have this many entries.
    uint32_t size;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));

    for (uint32_t i = 0; i < size; ++i) 
	{
		// url is stored as <number of chars> <char string>
		uint32_t num_chars;
    	ifs.read(reinterpret_cast<char*>(&num_chars), sizeof(num_chars));

		std::string u_str(num_chars, '\0');
        ifs.read(reinterpret_cast<char*>(u_str.data()), num_chars);

		ret.emplace(u_str);
    }

    return ret;
}

void indexer::save_url_q() 
{
    std::ofstream fs(
		q_path,
	   	std::ios::out | std::ios::binary
	);
    if (!fs) 
		throw std::runtime_error(
			"Could not open or create queue file: " + q_path.string()
		);

    // Write size
	auto sz = static_cast<uint32_t>(q.size());
	fs.write(reinterpret_cast<char*>(&sz), sizeof(sz));

	// Write urls
	while (!q.empty())
	{
		auto url{std::move(q.front())};
		q.pop();

		// Write size and char string.
		auto full{url.c_str()};
		// I know urls will be < 2^32.
		uint32_t len = static_cast<uint32_t>(
			std::char_traits<char>::length(full)
		);
		fs.write(reinterpret_cast<char*>(&len), sizeof(len));
		fs.write(full, len);
	}
}

void indexer::start_indexing()
{
	// After a while of indexing, I realized that it's helpful to have a set of
	// already recursed items so that I won't recurse them again.
	// A question is whether to persist the set between indexings or
	// to make it local to each indexing.
	// Advantage of persisting: maximize speed.
	// Disadvantage of persisting: the page could have been updated to included
	// new urls.
	//
	// I chose to make the set local to each indexing.
	
	std::unordered_set<std::string> recursed;

	while (
		!q.empty() && 
		!interrupted && 
		num_indexed < index_limit
	) {
		auto url{std::move(q.front())};
		q.pop();

		// Not indexed.
		webpage pg(url, convertor);
		// Only index if this filter returns true
		// and the document not indexed previously.
		// Advantage: much faster.
		// Disadvantage: cannot update an already indexed page.
		if (
			index_filter(url) && wp_index_filter(pg) && 
			!db.get_document(url).has_value()
		)
		{
			db.add_document(pg);
			// log the webpage indexed:
			util_log(
				std::to_string(num_indexed) + "th indexed: " +
				url.c_str()
			);
			++num_indexed;
		}


		// Only recurse when not recursed and the filters return true.
		if (
			!recursed.contains(url_get_essential(url)) &&
			recurse_filter(url) && wp_recurse_filter(pg))
		{
			recursed.emplace(url_get_essential(url));

			auto urls{pg.get_urls()};
			for (auto&& u : urls)
			{
				// If url is already indexed, don't put it into queue at all.
				// Advantage: much faster.
				// Disadvantage: cannot update an already indexed page.
				if(db.get_document(u).has_value())
					continue;

				// if url can neither be indexed nor recursed,
				// then don't put it into the queue at all.
				if (
					!index_filter(u) && !recurse_filter(u)
				)
					continue;

				q.emplace(u);
			}
		}

	}
}

void indexer::interrupt()
{
	interrupted = true;
}

indexer::~indexer()
{
	save_url_q();
}
