/**
 * This file implements a temp tool for me that removes a document 
 * indicated by a given url from the database.
 */

#include <iostream>

#include <boost/url.hpp>

#include "index.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr 
			<< "Usage:\n"
			<< argv[0] << " db_path url_to_remove+" << std::endl;
		return -1;
	}

	class index db(argv[1]);

	for (int i = 2; i < argc; ++i)
	{
		urls::url u(argv[i]);
		auto doc = db.get_document(u);
		if (!doc)
		{
			std::cerr << argv[i] << " was not found.\n";
		}
		else {
			db.rm_document(u);
			std::cout << argv[i] << " was removed.\n";
		}
	}

	return 0;
}
