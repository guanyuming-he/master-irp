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

#include <iostream>

int main(int argc, char* argv[])
{
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
		query_str += argv[i];
		query_str += ' ';
	}

	xp::MSet result = s.query(query_str);

	std::cout << "Found " << result.size() << " results\n";
	for (auto i = result.begin(); i != result.end(); ++i)
	{
		std::cout << i.get_document().get_data() << '\n';
	}

	return 0;
}
