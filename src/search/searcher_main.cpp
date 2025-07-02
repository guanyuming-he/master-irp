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

#include "searcher.h"
#include "utility.h"

#include <iostream>
#include <stdexcept>

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
		xp::MSet result = s.query(query_str);

		std::cout << "Found " << result.size() << " results\n";
		for (auto i = result.begin(); i != result.end(); ++i)
		{
			std::cout << i.get_document().get_data() << '\n';
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
