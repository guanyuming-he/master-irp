/**
* The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements those declared in index.h
 *
 * @author Guanyuming He
 */

#include "utility.h"
#include "index.h"
#include "webpage.h"

#include <bits/chrono.h>
#include <chrono>
#include <optional>
#include <xapian.h>

extern "C" {
#include "../sha-2/sha-256.h"
}

std::string index::url2hashid(const class url& u)
{
	auto essential{ u.get_essential() };
	uint8_t sha256[32];
	calc_sha_256(sha256, essential.c_str(), essential.size());

	std::string ret;
	ret.reserve(1 + 32);
	ret.push_back('Q');
	ret.append(reinterpret_cast<char*>(sha256), 32);

	return ret;
}

index::index(
	const fs::path& dbpath
):
	dbpath(dbpath),
	db(dbpath.string(), xp::DB_CREATE_OR_OPEN)
{
}


index::~index()
{
	/** 
	 * In principle I would need to call synchronize() here.
	 * However, the Database's commit() is automatically done in the
	 * WritableDatabase's destructor, and thus I don't have to do it
	 * explicitly.
	 * I ascertained it will be done, as the doc indicates 
	 * Database::~Database() is virtual.
	 */
}

std::optional<xp::Document> index::get_document(const url& u) const 
{
	// Query the document with the unique term.
	xp::Enquire enquire(db);
	enquire.set_query(xp::Query(url2hashid(u)));

	// Get only one match.
	Xapian::MSet matches = enquire.get_mset(0, 1);
	if (matches.empty()) {
		return std::nullopt;
	}
	
	return {matches.begin().get_document()};
}

std::optional<xp::Document> index::get_document(const webpage& w) const 
{
	return get_document(w.url);
}


std::optional<xp::Document> index::get_document(const xp::docid id) const
{
	std::optional<xp::Document> ret;
	try 
	{
		ret.emplace(db.get_document(id));
	} 
	catch (xp::DocNotFoundError) 
	{
		// do nothing, as ret will still be empty.
	}

	return ret;
}

void index::add_document(const webpage& w)
{ 
	// do not index an empty document.
	if (w.title.empty() && w.get_text().empty())
		return;

	xp::Document doc;
	tg.set_document(doc);

	// The magical strings "S" and "XD" are from the official
	// example https://getting-started-with-xapian.readthedocs.io/en
	// /latest/practical_example/indexing/writing_the_code.html,
	// which claims that they are the conventional prefixes of the 
	// omega search engine.
	tg.index_text(w.title, 1, "S");
	tg.index_text(w.get_text(), 1, "XD");

	// Index them without prefixes for free search 
	tg.index_text(w.title);
	tg.increase_termpos();
	tg.index_text(w.get_text());

	// Add the date as a value.
	// Because Xapian expects a sortable string or an integer,
	// I convert the date to the number of days since the epoch.
	auto days_epoch = ch::duration_cast<ch::days>(
		ch::sys_days{w.date}.time_since_epoch()
	).count();
	doc.add_value(DATE_SLOT, xp::sortable_serialise(days_epoch));

	// Store the full URL + title for display purposes
	doc.set_data(
		std::string(w.url.get_full().str) + "\t" + w.title
	);

	// This will be the unique identifier of the doc;
	auto hashid = url2hashid(w.url);
	doc.add_boolean_term(hashid);

	// We can now store the doc in the database.
	// Use replace_document instead of add_document to make sure 
	// one document is only indexed once.
	db.replace_document(hashid, doc);
}

void index::synchronize()
{
	db.commit();
}

//// commented out for now as I plan to use SHA256(url) as unique
//index::idmap_t index::load_idmap(const fs::path& p)
//{
//	std::ifstream ifs(p, std::ios::binary);
//    if (!ifs) // does not exist. Create it.
//	{
//		ifs.close();
//
//		std::ofstream ofs(p, std::ios::binary);
//        if (!ofs) 
//			throw std::runtime_error("Unable to create idmap file.");
//
//		// Write size 0 to the new file.
//        uint64_t zero = 0;
//        ofs.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
//		// Return empty map
//    return {}; 
//	}
//
//	// File exists. Read it.
//	idmap_t ret;
//	// We have this many entries.
//    uint64_t size;
//    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
//    for (uint64_t i = 0; i < size; ++i) {
//		// Element is \0 terminated string followed by docid.
//        std::string key;
//        char ch;
//        while (ifs.get(ch)) 
//		{
//            if (ch == '\0') break;
//            key += ch;
//        }
//
//		xp::docid val;
//        ifs.read(reinterpret_cast<char*>(&val), sizeof(val));
//        ret.emplace(std::move(key), val);
//    }
//
//    return ret;
//}
//
//void index::save_idmap()
//{
//    std::fstream fs(
//		url2idpath,
//	   	std::ios::in | std::ios::out | std::ios::binary
//	);
//    if (!fs) 
//		throw std::runtime_error("In save_idmap, file must exist.");
//
//    // File exists — read current count
//    uint64_t total_count = 0;
//    fs.read(reinterpret_cast<char*>(&total_count), sizeof(total_count));
//	if (total_count != url2id.size())
//		throw std::runtime_error("Index corrupted: idmap size mismatch");
//
//    // Seek to end and append
//    fs.seekp(0, std::ios::end);
//	for (const auto& [key, val] : url2id_diff) 
//	{
//        fs.write(key.c_str(), key.size() + 1);
//        fs.write(reinterpret_cast<const char*>(&val), sizeof(val));
//    }
//
//    // Update count at the beginning
//    total_count += url2id_diff.size();
//    fs.seekp(0, std::ios::beg);
//    fs.write(reinterpret_cast<const char*>(&total_count), sizeof(total_count));
//
//	// Don't forget to clear the diff.
//	url2id_diff.clear();
//}
