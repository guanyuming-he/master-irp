/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines a CLI for searching over a database generated
 * with my indexer. It is primarily used for testing and demonstration,
 * as the searching component can be integrated with the others 
 * in other ways.
 *
 * @author Guanyuming He
 */

#include "../searcher.h"
#include "../utility.h"

#include <iostream>
#include <regex>
#include <stdexcept>

// I want to filter only English words from the main text.
bool is_english_like(const std::string& term) 
{
	// Allow only lowercase alphabetic terms (min 2 chars)
	static const std::regex word_re("[a-z]{2,}");
	return std::regex_match(term, word_re);
}

int main(int argc, char* argv[])
{
	// Not needed.
	// global_init();

	if (argc < 3)
	{
		std::cerr 
			<< "Usage:\n"
			<< argv[0] << " db_path search_terms..."
			<< std::endl;
		return -1;
	}

	searcher s(argv[1]);

	std::string query_str;
	for (int i = 2; i < argc; ++i)
	{
		if (i != 2)
			query_str += ' ';
		query_str += argv[i];
	}

	std::cout << "query_str=" << query_str << '\n';

	try 
	{
		// Return at most 24 results.
		xp::MSet result = s.query(query_str, {24});

		std::cout << "Found " << result.size() << " results\n";
		for (auto i = result.begin(); i != result.end(); ++i)
		{
			auto doc = i.get_document();
			std::cout << doc.get_data() << '\n';

			// Get a list of keywords from the document.
			// I do not store the original text, because I don't
			// want to use too much database space.
			// I get all English words and evenly sample a few keywords from
			// them.
			std::vector<std::string> words;
			// need at least 150 words.
			words.reserve(150);
			for (
				auto it = doc.termlist_begin(); 
				it != doc.termlist_end();
				++it
			) {
				std::string term = *it;
				// The main text are indexed using prefix XD
				if (!term.starts_with("XD"))
					continue;
				term = term.substr(2);
				if (term.empty())
					continue;
				// Given only english words.
				if (!is_english_like(term))
					continue;
				
				words.emplace_back(std::move(term));
			}

			// Evenly sample the list of words to get keywords
			float step = words.size() > 150 ? 
				float(words.size()) / 150.f :
				1.f;
			for (float i = 0.f; (size_t)i < words.size(); i+=step)
			{
				std::cout << words[(size_t)i] << ' ';
			}
			std::cout << "\n\n";
		}
	}
	catch (const xp::Error& e)
	{
		std::cerr 
			<< "Unexpected xp error:\n"
			<< e.get_description()
			<< std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr 
			<< "Unexpected std error:\n"
			<< e.what()
			<< std::endl;
	}
	
	// Not needed.
	// global_uninit();

	return 0;
}
