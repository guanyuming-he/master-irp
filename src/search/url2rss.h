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

#include <optional>
#include <vector>

#include <pugixml/pugixml.hpp>

#include "scraper.h"
#include "webpage.h"

/**
 * Encapsulates a parsed object of RSS.
 *
 * As we only care about the articles linked to by the RSS, the class only has
 * a single responsibility, reading and returning them, as coded in the
 * read_webpages() methods.
 */
struct rss final
{
public:
	~rss();

	/**
	 * Parses rss from memory.
	 *
	 * @throws std::runtime_error if parsing failed.
	 * @throws system_error if url is incorrect. See boost.url doc.
	 */
	template <typename U>
	rss(
		U&& u,
		std::string_view content
	):
		url(std::forward<U>(u))
	{
		auto result = doc.load_buffer(content.data(), content.size());
		if (!result) 
		{
			throw std::runtime_error(
				"Failed to parse RSS/Atom XML: " + 
				std::string(result.description())
			);
		}
	}
	
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
	/**
	 * RSS 2.0 and Atom are slightly different.
	 * The two each handles one of them independently.
	 */
	std::optional<webpage> 
	webpage_from_rss_item(const pugi::xml_node& item) const;
	std::optional<webpage> 
	webpage_from_atom_item(const pugi::xml_node& item) const;

	/**
	 * Relative urls in a RSS XML is discouraged, but still possible.
	 * We need to handle all different kinds of links from a RSS XML.
	 * This function does that. If link is absolute, then a url constructed out
	 * of it is returned.
	 * Otherwise, it is resolved based on this->url.
	 *
	 * @returns a valid url only if link can be processed.
	 */
	std::optional<urls::url> process_link(std::string_view link) const;
	
public: // no need to make it private as it's immutable.
	const urls::url url;
private:
	pugi::xml_document doc;


};
