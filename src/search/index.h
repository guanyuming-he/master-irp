#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the indexing functions used in the search engine.
 * After serious contemplation, I decide to use Xapian, instead of building an
 * index engine myself, as that would be very time consuming.
 * A few problems hard to solve:
 * 1. Large amount of documents and terms.
 * 2. Posting lists may be sparse or dense.
 * 3. Support more than just single term boolean queries.
 *
 * @author Guanyuming He
 */

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <utility>
#include <xapian.h>

namespace fs = std::filesystem;
namespace xp = Xapian;

class url;
class webpage;

/**
 * The index class handles the main database stored on disk,
 *
 * In the end, when it is destructed, or when directly commanded, 
 * it is written back to the disk.
 */
class index final
{
public:
	///**
	// * As Xapian::Database does not store urls to documents,
	// * it is my responsibility to map each url to a docid.
	// *
	// * @param key returned by url::get_essential().
	// * @param value returned by xp::WritableDatabase::add_document().
	// */
	//using idmap_t = std::unordered_map<std::string, xp::docid>;
	//
	
private:
	enum value_slots : xp::valueno
	{
		DATE_SLOT = 1,
	};

public:
	// Empty index not allowed.
	index() = delete;
	~index();

	// No, only a single index class is allowed per database.
	index(const index&) = delete;
	index& operator=(const index&) = delete;

	/**
	 * @param dbpath path to the directory that the db is stored in. If no
	 * db is found or if the dir is not present, then it will be created.
	 * Otherwise, it will be opened.  
	 */
	explicit index(
		const fs::path& dbpath
	);

public:
	// @returns the document with the internal id, if present.
	std::optional<xp::Document> get_document(const xp::docid id) const;
	// @returns the document with the url, if present.
	std::optional<xp::Document> get_document(const class url& url) const;
	// @returns true if doc's url has already been indexed.
	std::optional<xp::Document> get_document(const webpage& doc) const;

	inline auto num_documents() const
	{ return db.get_doccount(); }

public:
	/**
	 * Adds document to the index.
	 * You MUST manually call has_document() to check 
	 * if it's already in the index.
	 * For performance reason, it won't be checked here.
	 */
	void add_document(const webpage& doc);

	/**
	 * Updates disk content with in memory content.
	 * Does nothing if dirty = false or paths is invalid.
	 * dirty will be set to false after it.
	 *
	 * Quote from the Xapian doc:
	 * Note that commit() need not be called explicitly: it will be called
	 * automatically when the database is closed, or when a sufficient number
	 * of modifications have been made. By default, this is every 10000
	 * documents added, deleted, or modified. This value is rather
	 * conservative, and if you have a machine with plenty of memory, you can
	 * improve indexing throughput dramatically by setting
	 * XAPIAN_FLUSH_THRESHOLD in the environment to a larger value.
	 */
	void synchronize();

private:
	fs::path dbpath;
	xp::WritableDatabase db;

	// Used to turn free text in a document into terms that are indexed.
	// From the official doc, it seems that it can be reused across multiple
	// documents.
	xp::TermGenerator tg{};

	//// commented out for now as I plan to use SHA256(url) as unique
	///identifier.
	///**
	// * idmap format:
	//	  <64bit unsigned number of elements>.
	//	  elements
	// *
	// * elements:
	// 	  <exactly that number>*element
	// * 
	// * element:
	// 	  \0-terminated-string docid.
	// */	  
	//idmap_t url2id;
	//
	//// To not rewrite the whole map everytime even if only a few are added,
	//// I record the difference here.
	//// Since for now I only allow appending, only the new entries are recorded.
	//idmap_t url2id_diff{};

private:
	// @returns "Q" + SHA256(url.get_essential()).
	static std::string url2hashid(const class url& url);

	//// commented out for now as I plan to use SHA256(url) as unique
	///** 
	// * Loads url2id map pointed to by path.
	// * If the file's not present, an empty idmap is returned.
	// * The function will not create the file on that occasion, 
	// * because later the index class will do that, writing the empty map to the
	// * file.
	// */
	//static idmap_t load_idmap(const fs::path& path);

	///**
	// * Only has effect if paths is valid.
	// * For now, it's an append only impl.
	// * It will only append new diff to the idmap.
	// */
	//void save_idmap();
};
