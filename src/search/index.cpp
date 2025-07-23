/**
* The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements those declared in index.h
 *
 * @author Guanyuming He
 */

#include "index.h"
#include "url2html.h"
#include "webpage.h"

#include <chrono>
#include <cstdio>
#include <optional>
#include <vector>

#include <xapian.h>

extern "C" {
#include "../sha-2/sha-256.h"
}

#include "utility.h"

std::string index::url2hashid(urls::url_view u)
{
	auto essential = url_get_essential(u);

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
	setup_tg();
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

std::optional<xp::Document> index::get_document(const urls::url& u) const 
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
	if (w.get_title().empty() && w.get_text().empty())
		return;

	xp::Document doc;
	tg.set_document(doc);

	// The magical strings "S" and "XD" are from the official
	// example https://getting-started-with-xapian.readthedocs.io/en
	// /latest/practical_example/indexing/writing_the_code.html,
	// which claims that they are the conventional prefixes of the 
	// omega search engine.
	tg.index_text(w.get_title(), 1, "S");
	tg.index_text(w.get_text(), 1, "XD");

	// Index them without prefixes for free search 
	tg.index_text(w.get_title());
	tg.increase_termpos();
	tg.index_text(w.get_text());

	// Add the date as a value.
	// Xapian supports date string parsing during searching,
	// I use the same format as the example:
	// YYYYMMDD
	std::string date_str(4+2+2, '\0'); 
	std::snprintf(
		date_str.data(),
		date_str.size()+1,
		"%4d%02d%02d", 
		(int)w.get_date().year(), 
		(unsigned)w.get_date().month(), 
		(unsigned)w.get_date().day()
	);
	doc.add_value(DATE_SLOT, date_str);

	// Store the full URL + title for display purposes
	doc.set_data(
		std::string(w.url.c_str()) + "\t" + w.get_title()
	);

	// This will be the unique identifier of the doc;
	auto hashid = url2hashid(w.url);
	doc.add_boolean_term(hashid);

	// We can now store the doc in the database.
	// Use replace_document instead of add_document to make sure 
	// one document is only indexed once.
	db.replace_document(hashid, doc);
}

void index::rm_document(const urls::url& u)
{
	db.delete_document((url2hashid(u)));
}
	
void index::rm_if(doc_rm_func_t* func)
{
	// Do not delete while iterating. The documentation didn't
	// say anything about this, but I will not do it to be safe.
	std::vector<xp::docid> to_delete;
	// to get over the start phase where reallocation is often.
	to_delete.reserve(1024);

	// The doc says passing "" to it will yield an iter 
	// over all documents.
	auto beg = db.postlist_begin("");
	auto end = db.postlist_end("");
	for (auto i = beg; i != end; ++i)
	{
		auto doc = db.get_document(*i);
		// rm if func returns true.
		if(func(doc))
		{
			to_delete.push_back(*i);
		}
	}

	for (const auto id : to_delete)
		db.delete_document(id);
}

void index::shrink(
	unsigned max_num, shrink_policy policy
) {
	const auto cur_size = num_documents();

	util_log(
		"Current number of documents = " +
		std::to_string(cur_size) +
		". Will shrink to " + 
		std::to_string(max_num) + ".\n"
	);

	if (cur_size <= max_num)
		return;

	auto num_to_rm = cur_size - max_num;

    xp::Enquire enquire(db);
	enquire.set_query(xp::Query::MatchAll);
    enquire.set_sort_by_value(DATE_SLOT,
		// false: ascending; true: descending
		policy != shrink_policy::OLDEST
	);
	xp::MSet res = enquire.get_mset(0, num_to_rm);

	for (auto i = res.begin(); i != res.end(); ++i)
	{
		db.delete_document(*i);
	}
}

void index::upd_document(
	const urls::url& u, doc_upd_func_t* upd_func
) {
	auto doc = get_document(u);
	if (doc)
	{
		// replace only if updated.
		if(upd_func(doc.value()))
			db.replace_document(doc->get_docid(), doc.value());
	}
}

void index::upd_all(doc_upd_func_t* upd_func)
{
	// The doc says passing "" to it will yield an iter 
	// over all documents.
	auto beg = db.postlist_begin("");
	auto end = db.postlist_end("");
	for (auto i = beg; i != end; ++i)
	{
		auto doc = db.get_document(*i);
		// replace only if updated.
		if(upd_func(doc))
			db.replace_document(*i, doc);
	}
}

void index::synchronize()
{
	db.commit();
}

void index::setup_tg()
{
	tg.set_stemmer(xp::Stem("en"));
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
