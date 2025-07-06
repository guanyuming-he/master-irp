/**
 * This file implements a temp tool for me that removes 
 * certain documents from the database.
 *
 * Copyright (C) Guanyuming He 2025
 * The file is licensed under the GNU GPL v3.0
 *
 * @author Guanyuming He
 */

#include <boost/url/url.hpp>
#include <iostream>
#include <random>
#include <unordered_map>

#include <boost/url.hpp>
#include <xapian.h>

#include "../index.h"

std::random_device devrand;
std::mt19937 pseudorand(devrand());
std::uniform_real_distribution<float> prob(0.f, 1.f);

std::unordered_map<std::string, float> domain_rm_prob{
	{"www.businessinsider.com", .95f},
};

bool rm_func(xp::Document& doc)
{
	static unsigned num_rmed = 0;

	std::string date = doc.get_data();
	// date is url \t title.
	std::string_view u_str(
		date.c_str(), 
		date.find_first_of('\t')
	);
	urls::url u(u_str);

	std::string key{u.encoded_host()};
	if (!domain_rm_prob.contains(key))
		return false;

	auto val{domain_rm_prob.at(key)};
	if (prob(pseudorand) < val)
	{
		++num_rmed;
		if (num_rmed % 500 == 1)
		{
			std::cout
				<< std::to_string(num_rmed) 
				<< "th removed.\n";
		}
		return true;
	}

	return false;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr 
			<< "Usage:\n"
			<< argv[0] << " <db_path> purge\n"
			<< argv[0] << " <db_path> <url_to_rm>\n";
		return -1;
	}

	class index db(argv[1]);

	if (std::string("purge") == argv[2])
	{
		// Purge the database based on rm_func.
		std::cout << "Purging...\n";
		db.rm_if(rm_func);
		return 0;
	}

	// Don't purge. remove a specific url.
	urls::url u(argv[2]);
	if (!db.get_document(u).has_value())
	{
		std::cerr << argv[2] << " not found.\n";
		return -1;
	}
	std::cout << "rm " << argv[2] << '\n';
	db.rm_document(u);

	return 0;
}
