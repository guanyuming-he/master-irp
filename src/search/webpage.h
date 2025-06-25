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
	// 1. loads from file, which only has metadata. 
	// The url must be (syntactically) valid.
	// Otherwise an exception is thrown.
	webpage(
		const std::string_view url,
		const std::string& title,
		const ch::year_month_day& date
	);

	// 2. (3) uses this which directly accepts a html.
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

private: // declare this first as it needs to be inited first.
	// for now, I don't know if it should be const.
	// I leave it as not to open the possibility of parsing
	// the HTML later.
	std::optional<html> html_tree;

public: // since they are immutable, no need to go private.
	const urls::url url;
	const std::string title;
	const ch::year_month_day date;

};
