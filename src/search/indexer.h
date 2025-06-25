#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the class indexer, which runs the scrapers and parsers
 * to index a specific range of URLs configured.
 *
 * @author Guanyuming He
 */

#include <filesystem>
#include <limits>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "url2html.h"
#include "index.h"

/**
 * The indexer takes two main part of configurations:
 * 1. Which index database on disk it will update.
 * 2. Which kinds of URLs to scrape and index.
 *
 * For (2), I adopt such a strategy:
 * 1. Specify a list of hosts that are authentic information sources,
 * 	like The Guardian, The Atlantic, NY Times.
 * 2. I start with a queue which is filled with the business news collection 
 * 	page of each in (1).
 * 3. Then, it works like a BFS: for each url, it is indexed. Then, all urls
 * found within it are added to the queue. 
 * 4. A filtering criterion is that urls not having one of the hosts will be
 * indexed, but its urls will not be added to the queue.
 * 5. Also, repeated urls will not be processed.
 *
 * As web I/O takes a long time, and curl's easy interface is blocking,
 * there is ample opportunity to exploit parallalism.
 * Nevertheless, I will start with a serial version to make sure it works
 * first.
 *
 * Generally, I would want the index to run indefinitly until
 * 1. The queue becomes empty, or
 * 2. The user interrupts it.
 *
 * It would be desirable for the queue to be saved when the indexing is
 * interrupted.
 */
class indexer
{
public:
	// Type of the set of domains.
	using dset_t = std::unordered_set<std::string>;
	using uque_t = std::queue<urls::url>;

public:
	indexer() = delete;
	~indexer();

	/**
	 * Starts indexing with an initial queue.
	 *
	 * @param db_path Path to the database that is updated by the indexer.
	 * @param q_path Should the indexing be interrupted, the queue then will be
	 * stored to this path.
	 * @param q_init The initial queue.
	 * @param domains The set of authentic information source domains.
	 */
	template <typename P, typename Q, typename S>
	requires
		std::is_same_v<fs::path, std::remove_cvref_t<P>> &&
		std::is_same_v<uque_t, std::remove_cvref_t<Q>> &&
		std::is_same_v<dset_t, std::remove_cvref_t<S>>
	indexer(
		P&& db_path, P&& q_path,
		Q&& q_init,
		S&& domains,
		size_t index_limit = std::numeric_limits<size_t>::max()
	):
		db(std::forward<P>(db_path)),
		q_path(std::forward<P>(q_path)),
		q(std::forward<Q>(q_init)), domains(std::forward<S>(domains)),
		index_limit(index_limit)
	{}
	/**
	 * Resumes indexing with a stored queue on disk
	 *
	 * @param q_path Path to a queue that saved the progress of a previously
	 * interrupted indexing.
	 * @param db_path Path to the database that is updated by the indexer.
	 * @param domains The set of authentic information source domains.
	 */
	template <typename P, typename S>
	requires
		std::is_same_v<fs::path, std::remove_cvref_t<P>> &&
		std::is_same_v<dset_t, std::remove_cvref_t<S>>
	indexer(
		P&& db_path, P&& q_path,
		S&& domains,
		size_t index_limit = std::numeric_limits<size_t>::max()
	):
		db(std::forward<P>(db_path)),
		q_path(std::forward<P>(q_path)),
		q(load_url_q(this->q_path)), 
		domains(std::forward<S>(domains)),
		index_limit(index_limit)
	{}

public:
	/**
	 * Start the indexing loop until the queue is exhausted
	 * or interrupted.
	 */
	void start_indexing();

	/**
	 * Should be called when the process recvs SIGINT.
	 */
	void interrupt();


private:
	/**
	 * @throws std::runtime_error if file does not exist.
	 */
	static uque_t load_url_q(const fs::path& q_path);
	/**
	 * Not const because one can't loop over a queue immutably.
	 */
	void save_url_q();

private:
	class index db;

	/**
	 * The queue is stored on disk as:
	 * <uint32_t number of urls>
	 * <that number>*url
	 *
	 * url:
	 * <uint32_t size> <char string>
	 */
	const fs::path q_path;
	uque_t q;

	const dset_t domains;

	url2html convertor{};

	size_t num_indexed = 0;
	const size_t index_limit{};

	bool interrupted = false;
};
