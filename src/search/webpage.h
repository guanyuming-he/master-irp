#pragma once
/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file defines the class that represents a webpage
 * and related helpers.
 *
 * @author Guanyuming He
 */

#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include <chrono>
namespace ch = std::chrono;

extern "C" {
#include <lexbor/html/interfaces/document.h>
}

#include "url2html.h"

/**
 * The class represents a webpage.
 * It has its url, title, date, and other metadata.
 *
 * Optionally, it loads the whole HTML of the page 
 */
class webpage final 
{
public:
	// Load only the metadata without the HTML.
	// The url must be (syntactically) valid.
	// Otherwise an exception is thrown.
	webpage(
		const std::string_view url,
		const std::string& title,
		const ch::year_month_day& date
	);

	// Accepts a parsed HTML and its URL.
	// The url must be (syntactically) valid.
	// Otherwise an exception is thrown.
	template <typename H, typename U>
	requires 
		std::is_same_v<std::remove_cvref_t<H>, html> &&
		std::is_same_v<std::remove_cvref_t<U>, urls::url>
	webpage(U&& url, H&& html_tree):
		html_tree(std::forward<H>(html_tree)),
		url(std::forward<U>(url)),
		title(this->html_tree->get_title()),
		date(this->html_tree->get_date())
	{}
	
	// 3. loads from url, reads the HTML, and calculates the metadata.
	// The url must be (syntactically) valid.
	// Otherwise an exception is thrown.
	template <typename U>
	requires 
		std::is_same_v<std::remove_cvref_t<U>, urls::url>
	webpage(U&& url, url2html& convertor):
		webpage(std::forward<U>(url), convertor.convert(url))
	{}

public:
	inline auto get_date() const { return date; }
	inline auto get_title() const { return title; }

	// @returns the text in lowercase or "" if not loaded.
	inline std::string get_text() const 
	{ return html_tree ? html_tree->text : ""; }
	
	// @returns a vector of urls in the HTML.
	// Different from html::get_urls(),
	// this function will turn every relative one 
	// into absolute one, and will discard invalid urls.
	// 
	// As a consequence,
	// this->get_urls().size() <= html_tree->get_urls();
	std::vector<urls::url> get_urls() const;

	/**
	 * Loads the html from the URL only if it is not loaded.
	 *
	 * @returns true iff it was not loaded and now loaded.
	 */
	bool load_html(const url2html& convertor);

private: // declare this first as it needs to be inited first.
	/**
	 * Use std::optional to allow delayed loading.
	 * It is useful, e.g., when coming from a RSS feed where
	 * only a title and a short desc is provided.
	 * One can decide to load the html or not.
	 */
	std::optional<html> html_tree;
	std::string title;
	ch::year_month_day date;

public: // since they are immutable, no need to go private.
	const urls::url url;

};
