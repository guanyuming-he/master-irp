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
#include <optional>
#include <string>

#include <boost/url.hpp>
#include <xapian.h>

namespace fs = std::filesystem;
namespace urls = boost::urls;
namespace xp = Xapian;

class webpage;

/**
 * The index class handles the main database stored on disk,
 *
 * In the end, when it is destructed, or when directly commanded, 
 * it is written back to the disk.
 */
class index final
{
	friend class searcher;

public:
	// @returns true iff doc is updated.
	using doc_upd_func_t = bool(xp::Document&);
	// @returns true iff doc should be rmed.
	using doc_rm_func_t = doc_upd_func_t;

	enum value_slots : xp::valueno
	{
		DATE_SLOT = 1,
	};

	enum class shrink_policy : unsigned 
	{
		OLDEST,	// oldest is removed.
		LATEST,	// latest is removed.
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
	std::optional<xp::Document> get_document(const urls::url& url) const;
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
	 * Attempts to remove the document identified by url 
	 * from the db.
	 *
	 * Xapian delete_document() does not return a value to indicate
	 * whether that succeeds or not, so I can't either.
	 */
	void rm_document(const urls::url& u);

	/**
	 * Iterate through all documents.
	 * Remove if func returns true.
	 */
	void rm_if(doc_rm_func_t* func);

	/**
	 * Updates the document indicated by u with upd_func.
	 */
	void upd_document(const urls::url& u, doc_upd_func_t* upd_func);
	/**
	 * Updates all documents with upd_func.
	 */
	void upd_all(doc_upd_func_t* upd_func);

	/**
	 * Shrinks the database to max_num.
	 * No effect if num_documents() <= max_num.
	 *
	 * @param max_num num_documents() will be <= after the call.
	 * @param policy decides which documents to remove 
	 * if num_documents() > max_num
	 * before the call.
	 */
	void shrink(unsigned max_num, shrink_policy policy);

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

private:
	// @returns "Q" + SHA256(url_get_essential(u)).
	static std::string url2hashid(urls::url_view u);

	void setup_tg();

private:
	//// commented out for now as I plan to use SHA256(url) as unique id.
	///**
	// * As Xapian::Database does not store urls to documents,
	// * it is my responsibility to map each url to a docid.
	// *
	// * @param key returned by url::get_essential().
	// * @param value returned by xp::WritableDatabase::add_document().
	// */
	//using idmap_t = std::unordered_map<std::string, xp::docid>;
	//
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
