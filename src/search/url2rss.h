#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file declares the routines that are used to turn a url into a RSS
 * document.
 * They are divided into two classes,
 * a parser and a scraper.
 * The scraper gets the HTML from the URL, handling network logic.
 * The parser parses the HTML into a tree.
 * 
 * Together, they are put in a url2html class.
 *
 * @author Guanyuming He
 */

#include <pugixml.hpp>

#include "scraper.h"
#include "webpage.h"

/**
 * Encapsulates a parsed object of RSS.
 */
struct rss final
{
public:
	~rss();

	/**
	 * Parses rss from memory.
	 *
	 * @throws std::runtime_error if parsing failed.
	 */
	explicit rss(std::string_view content);
	
	// No copy; move only.
	rss(const rss&) = delete;
	rss(rss&&) noexcept;
	rss& operator=(const rss&) = delete;
	rss& operator=(rss&&) noexcept;

public:
	/**
	 * Reads all the webpages in the RSS feed page and returns them.
	 * 
	 * @returns a list of webpages each of which's HTML is not loaded. That is,
	 * only the url, a title, and a short desc (possibly) from the RSS feed is
	 * set for each.
	 */
	std::vector<webpage> read_webpages() const;	
	
private:
	pugi::xml_document doc;


}
