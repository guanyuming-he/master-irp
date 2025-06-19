#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the index data structure that is used
 * by the search engine.
 *
 * @author Guanyuming He
 */

/**
 * For now, it is a simple inverted index,
 * a list of words and maybe phrases
 * each of which comes with its own list of documents that have it.
 *
 * The index is expected to be large and stored on the disk.
 * I want to only load words that are used, i.e.,
 * 1. when updating a word's list
 * 2. when searching
 *
 * As such, I only define the data structure for each word's list,
 * as a std::vector of unsigned int (document ID).
 */

/**
 * An inverted index.
 * It has a dictionary that is always loaded in memory.
 * One can request the posting list of an item in the dictionary,
 * which is loaded from the disk into memory.
 *
 * One can also update the posting list of an item, which is directly done on
 * the disk.
 *
 * A note on the implementation:
 * because I am not dealing with billions of terms, but more likely within
 * 100,000, it is feasible to put each item's posting list in a separate file.
 * General file systems may suffer from performance loss when there are 
 * too many entries in a dir, and thus I can use two levels of dirs of the
 * first two letters, reducing the average number of entires in a dir by 26^2.
 * For example, the word apple's posting list is put inside
 * a/p/apple.index.
 *
 * On this implementation, loading a posting list is just loading the file,
 * while updating (adding doc id to) a list is just appending the id to the
 * file.
 */
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

class index final
{
	
private:
	/**
	 * A set of lowercase words or phrases
	 * that we have seen.
	 */
	std::unordered_set<std::string> dictionary;
	/**
	 * A map of url to document ID.
	 */
	std::unordered_map<std::string, uint32_t> documents;
};
