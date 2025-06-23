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

#include <xapian.h>

namespace fs = std::filesystem;
namespace xp = Xapian;

class url;
class webpage;

/**
 * The index class handles the main database stored on disk,
 * and a url2id map which prevents duplicate url indexing.
 *
 * Similar to Xapian's design, this class is read-only,
 * but that does not mean its invariance includes the Database's immutability.
 * Instead, the invariant says nothing about mutablity; the class just doesn't
 * provide modifier interfaces. That's all. But that doesn't stop 
 *
 * A derived class will have modifiers. Specifically, the indexer (a different
 * class) should be able to add documents to it on the fly.
 *
 * In the end, when it is destructed, or when directly commanded, 
 * it is written back to the disk.
 */
class index 
{
public:
	/**
	 * As Xapian::Database does not store urls to documents,
	 * it is my responsibility to map each url to a docid.
	 *
	 * @param key returned by url::get_essential().
	 * @param value returned by xp::WritableDatabase::add_document().
	 */
	using idmap_t = std::unordered_map<std::string, xp::docid>;

public:
	// Empty index not allowed.
	index() = delete;
	virtual ~index() = default;

	// No, only a single index class is allowed per database.
	index(const index&) = delete;
	index& operator=(const index&) = delete;

	/**
	 * @param dbpath path to the directory that the db is stored in. If no
	 * db is found or if the dir is not present, then it will be created.
	 * Otherwise, it will be opened.  
	 * @param mappath path to the url2id map file. If not present, it will be
	 * created.
	 *
	 * @throws std::runtime_error if db and url2id mismatch.
	 * I can't check if every doc id is present in the db, which is too time
	 * consuming, but I will compare their sizes.
	 */
	index(const fs::path& dbpath, const fs::path& mappath);

	/**
	 * This version is used for testing where the db and idmap can just be in
	 * memory entirely.
	 * @param db a database object.
	 * @param url2id a urlmap object.
	 */
	template <typename D, typename M>
	requires 
		std::is_base_of_v<xp::WritableDatabase, std::remove_cvref_t<D>> &&
		std::is_convertible_v<M, idmap_t>
	index(D&& db, M&& url2id):
		paths(std::nullopt),
		db(std::make_unique<xp::Database>(std::forward<D>(db))), 
		url2id(std::forward<M>(url2id))
	{}

public:
	// @returns true if url has already been indexed.
	bool has_document(const class url& url) const;
	// @returns true if doc's url has already been indexed.
	bool has_document(const webpage& doc) const;
	
private:
	// If not present, then the db and url2id map are in memory.
	std::optional<std::pair<fs::path, fs::path>> paths;

protected:
	std::unique_ptr<xp::Database> db;
	idmap_t url2id;

};


class writable_index : public index
{
public:
	/**
	 * @param dbpath path to the directory that the db is stored in. If no
	 * db is found or if the dir is not present, then it will be created.
	 * Otherwise, it will be opened.  
	 * @param mappath path to the url2id map file. If not present, it will be
	 * created.
	 *
	 * @throws std::runtime_error if db and url2id mismatch.
	 * I can't check if every doc id is present in the db, which is too time
	 * consuming, but I will compare their sizes.
	 */
	writable_index(const fs::path& dbpath, const fs::path& mappath):
		index(dbpath, mappath),
		wdb(*dynamic_cast<xp::WritableDatabase*>(this->db.get()))
	{}

	/**
	 * This version is used for testing where the db and idmap can just be in
	 * memory entirely.
	 * @param db a WritableDatabase object.
	 * @param url2id a urlmap object.
	 */
	template <typename D, typename M>
	requires 
		std::is_same_v<xp::WritableDatabase, std::remove_cvref_t<D>> &&
		std::is_convertible_v<M, idmap_t>
	writable_index(D&& db, M&& url2id):
		index(std::forward<D>(db), std::forward<M>(url2id)),
		wdb(*dynamic_cast<xp::WritableDatabase*>(this->db.get()))
	{}

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
	 */
	void synchronize();

private:

	// Used to turn free text in a document into terms that are indexed.
	// From the official doc, it seems that it can be reused across multiple
	// documents.
	xp::TermGenerator tg{};

	// Whether the in memory content has changed since 
	// its construction from disk.
	// If paths is valid and dirty = true,
	// then the content will be updated to disk at destruction time.
	bool dirty = false;

	// only dynamic cast once at construction time.
	xp::WritableDatabase& wdb;

private:
	/** 
	 * Loads url2id map pointed to by path.
	 * If the file's not present, an empty idmap is returned.
	 * The function will not create the file on that occasion, 
	 * because later the index class will do that, writing the empty map to the
	 * file.
	 */
	static idmap_t load_idmap(const fs::path& path);
};
