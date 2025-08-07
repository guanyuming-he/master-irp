/**
 * This file implements a program that calculates the distribution of documents
 * indexed from different domains in the database, for my thesis.
 * 
 * Copyright (C) Guanyuming He 2025
 * The file is licensed under the GNU GPL v3.0
 *
 * @author Guanyuming He
 */

#include <iostream>
#include <unordered_map>

#include <boost/url.hpp>
#include <xapian.h>

#include "../index.h"

// Count the docs of different hosts and output the distribution.
std::unordered_map<std::string, unsigned> host_to_count;

bool count_doc_host(Xapian::Document& doc)
{
	urls::url u(index::url_from_doc(doc));

	auto host = std::string(u.encoded_host());

	if (host_to_count.contains(host))
		++host_to_count[host];
	else
		host_to_count[host] = 1;

	// always return false to indicate it's read-only.
	return false;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr
			<< "Usage:\n"
			<< argv[0] << " <db_path>\n";
		return -1;
	}

	class index db(argv[1]);
	auto num_doc = db.num_documents(); 
	std::cout << "Num total doc = " << num_doc << '\n';

	// Do the counting using the update func.
	db.upd_all(count_doc_host);

	for (const auto& [host, cnt] : host_to_count)
	{
		std::cout 
			<< "Host: " << host << ": "
			<< cnt 
			<< " (" 
			<< 100.f * float(cnt)/float(num_doc)
			<< "%)\n";
	}

	return 0;
}
