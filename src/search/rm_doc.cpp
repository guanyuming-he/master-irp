/**
 * This file implements a temp tool for me that removes 
 * certain documents from the database.
 *
 * Copyright (C) Guanyuming He 2025
 * The file is licensed under the GNU GPL v3.0
 *
 * @author Guanyuming He
 */

#include <iostream>
#include <random>
#include <unordered_map>

#include <boost/url.hpp>
#include <xapian.h>

#include "index.h"

std::random_device devrand;
std::mt19937 pseudorand(devrand());
std::uniform_real_distribution<float> prob(0.f, 1.f);

std::unordered_map<std::string, float> domain_rm_prob{
	{"www.businessinsider.com", .8f},
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

	// remove 80% of business insider,
	// 40% of the atlantic, and
	// 50% of the guardian, randomly.
	std::string key{u.encoded_host()};
	if (!domain_rm_prob.contains(key))
		return false;

	auto val{domain_rm_prob.at(key)};
	if (prob(pseudorand) < val)
	{
		++num_rmed;
		if (num_rmed % 1000 == 1)
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
	if (argc != 2)
	{
		std::cerr 
			<< "Usage:\n"
			<< argv[0] << " db_path" << std::endl;
		return -1;
	}

	class index db(argv[1]);

	// Clear the database, remove those that are 
	// imbalancely present in the database.
	db.rm_if(rm_func);

	return 0;
}
